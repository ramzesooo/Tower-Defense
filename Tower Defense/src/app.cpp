#include "app.h"
#include "entity/entity.h"
#include "entity/enemy.h"
#include "entity/towers/tower.h"
#include "entity/tile.h"

#include "SDL_image.h"

#include <fstream>
#include <cmath>

static constexpr uint16_t levelsToLoad = 1;

// class App STATIC VARIABLES
App *App::s_Instance = nullptr;

bool App::s_IsRunning = false;

int32_t App::WINDOW_WIDTH = 800;
int32_t App::WINDOW_HEIGHT = 600;

MainMenu App::s_MainMenu;

TextureManager App::s_Textures;
Logger App::s_Logger;
Manager App::s_Manager;

SDL_Renderer *App::s_Renderer = nullptr;
SDL_Event App::s_Event;
SDL_FRect App::s_Camera { 0.0f, 0.0f, static_cast<float>(App::WINDOW_WIDTH), static_cast<float>(App::WINDOW_HEIGHT) };

Level *App::s_CurrentLevel = nullptr;

float App::s_ElapsedTime = 0.0f;

UIState App::s_UIState = UIState::mainMenu;

int32_t App::s_MouseX = 0;
int32_t App::s_MouseY = 0;

BuildingState App::s_Building;

std::random_device App::s_Rnd;

bool App::s_IsWindowMinimized = false;

SDL_Texture *App::s_GreenTex = nullptr;
SDL_Texture *App::s_Square = nullptr;

SDL_Texture *UIElement::s_BgTexture = nullptr;
SDL_Texture *UIElement::s_CoinTexture = nullptr;
SDL_Texture *UIElement::s_HeartTexture = nullptr;

SDL_Rect UIElement::coinDestRect{};
SDL_Rect UIElement::heartDestRect{};

// [0] = waves, [1] = coins, [2] = lifes, [3] = time
std::array<UIElement, 4> App::s_UIElements{};
Label App::s_UICoinsNotification(1000);

bool App::s_IsCameraLocked = true;

CameraMovement App::s_CameraMovement;
Vector2D CameraMovement::realVelocity{ 0.0f, 0.0f };

IF_DEBUG(Label *App::s_EnemiesAmountLabel = nullptr;);
IF_DEBUG(Label *App::s_PointedPosition = nullptr;);
IF_DEBUG(Label *App::s_FrameDelay = nullptr;);

IF_DEBUG(EnemyDebugSpeed App::s_Speedy;);
IF_DEBUG(bool App::s_SwapTowerType = false;);
// class App STATIC VARIABLES

SDL_Texture *BuildingState::originalTexture = nullptr;

// class App GLOBAL VARIABLES
std::default_random_engine g_Rng(App::s_Rnd());

auto &g_Projectiles = App::s_Manager.GetGroup(EntityGroup::projectile);
auto &g_Towers = App::s_Manager.GetGroup(EntityGroup::tower);
auto &g_Attackers = App::s_Manager.GetGroup(EntityGroup::attacker);
auto &g_Enemies = App::s_Manager.GetGroup(EntityGroup::enemy);

uint32_t g_PausedTicks = 0;
// class App GLOBAL VARIABLES

extern SDL_DisplayMode displayInfo;

struct TextureData
{
	std::string_view id, path;
};

static constexpr TextureData textures[]
{
	{ "mapSheet", "assets/tileset.png" },
	{ "szpaku", "assets/szpaku.jpg" },
	{ "buttonUI", "assets/ui/ui_button.png" },
	{ "hoveredButtonUI", "assets/ui/ui_button_hovered.png" },
	{ "canBuild", "assets/ui/tile_CanBuild.png" },
	{ "cantBuild", "assets/ui/tile_CantBuild.png" },
	{ "upgradeTower", "assets/ui/tile_Upgrade.png" },
	{ "elementUI", "assets/ui/ui_element.png" },
	{ "coinUI", "assets/ui/coin.png" },
	{ "heartUI", "assets/ui/heart.png" },

	{ "base", "assets/base.png" },
	{ "square", "assets/square_32x32.png" },
	{ "green", "assets/green_32x32.png" },
	{ "transparent", "assets/transparent.png" },
	{ "grayArrow", "assets/grayArrow_32x32.png" },

	{ App::TextureOf(TowerType::classic), "assets/towers/classic/tower.png"},
	{ App::TextureOf(TowerType::dark), "assets/towers/dark/DarkTower-Sheet.png"},

	{ App::TextureOf(ProjectileType::arrow), "assets/projectiles/arrow_16x16.png" },
	{ App::TextureOf(ProjectileType::thunder), "assets/projectiles/thunder.png" },

	{ App::TextureOf(AttackerType::archer), "assets/entities/friendly/attackerArcher.png" },
	{ App::TextureOf(AttackerType::hunter), "assets/entities/friendly/attackerHunter.png"},
	{ App::TextureOf(AttackerType::musketeer), "assets/entities/friendly/attackerMusketeer.png" },

	{ App::TextureOf(EnemyType::elf), "assets/entities/enemy/enemyElf.png" },
	{ App::TextureOf(EnemyType::goblinWarrior), "assets/entities/enemy/enemyGoblinWarrior.png" },
	{ App::TextureOf(EnemyType::dwarfSoldier), "assets/entities/enemy/enemyDwarfSoldier.png" },
	{ App::TextureOf(EnemyType::dwarfKing), "assets/entities/enemy/enemyDwarfKing.png" }
};

App::App()
{
	TTF_Font *defaultFont = nullptr;

	bool initialized = true;

	if (!App::s_Instance)
		App::s_Instance = this;
	else
		initialized = false;

	m_Window = SDL_CreateWindow("Tower Defense", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, App::WINDOW_WIDTH, App::WINDOW_HEIGHT, SDL_WINDOW_RESIZABLE);
	if (!m_Window)
	{
		App::s_Logger.AddLog(std::string_view(SDL_GetError()));
		initialized = false;
	}

	SDL_Surface *iconSurface = IMG_Load("assets\\gugu.png");
	if (!iconSurface)
	{
		App::s_Logger.AddLog(std::string_view(SDL_GetError()));
	}
	else
	{
		SDL_SetWindowIcon(m_Window, iconSurface);
		SDL_FreeSurface(iconSurface);
	}

#ifdef DEBUG
	App::s_Renderer = SDL_CreateRenderer(m_Window, -1, 0);
#else
	App::s_Renderer = SDL_CreateRenderer(m_Window, -1, SDL_RENDERER_PRESENTVSYNC);
#endif
	if (!App::s_Renderer)
	{
		App::s_Logger.AddLog(std::string_view(SDL_GetError()));
		initialized = false;
	}

	SDL_GetRendererOutputSize(App::s_Renderer, &WINDOW_WIDTH, &WINDOW_HEIGHT);
	SDL_SetRenderDrawColor(App::s_Renderer, 90, 0, 220, 255);

	for (const auto &[id, path] : textures)
	{
		App::s_Textures.AddTexture(std::string(id), std::string(path).c_str());
	}

	for (std::size_t i = 0u; i < std::size_t(TowerType::size); i++)
	{
		Tower::s_TowerTextures[i] = App::s_Textures.GetTexture(App::TextureOf(TowerType(i)));
	}

	App::s_Textures.AddFont("default", "assets\\F25_Bank_Printer.ttf", 15);
	App::s_Textures.AddFont("enemyHealth", "assets\\Rostack.otf", 13);
	App::s_Textures.AddFont("baseHealth", "assets\\Rostack.otf", 26);

	defaultFont = App::s_Textures.GetFont("default");

	App::s_GreenTex = App::s_Textures.GetTexture("green");
	App::s_Square = App::s_Textures.GetTexture("square");

	Button::s_DefaultButton = App::s_Textures.GetTexture("buttonUI");
	Button::s_HoveredButton = App::s_Textures.GetTexture("hoveredButtonUI");

	UIElement::s_BgTexture = App::s_Textures.GetTexture("elementUI");
	UIElement::s_CoinTexture = App::s_Textures.GetTexture("coinUI");
	UIElement::s_HeartTexture = App::s_Textures.GetTexture("heartUI");

	Level::s_Texture = App::s_Textures.GetTexture("mapSheet");

	Enemy::s_ArrowTexture = App::s_Textures.GetTexture("grayArrow");

	m_Levels.reserve(levelsToLoad);

	for (uint16_t i = 0u; i < levelsToLoad; i++)
	{
		m_Levels.emplace_back(i);
	}

	App::s_CurrentLevel = &m_Levels.at(0);

	if (!App::s_CurrentLevel || App::s_CurrentLevel->HasLoadingFailed())
	{
		App::s_Logger.AddLog(std::string_view("First level couldn't be loaded properly."));
		initialized = false;
	}
	else
	{
		s_CameraMovement.border.x = static_cast<float>(App::s_CurrentLevel->m_MapData.at(3)) - s_Camera.w;
		s_CameraMovement.border.y = static_cast<float>(App::s_CurrentLevel->m_MapData.at(4)) - s_Camera.h;
	}
	
	s_CameraMovement.rangeW = static_cast<int32_t>(s_Camera.w / 6.0f);
	s_CameraMovement.rangeH = static_cast<int32_t>(s_Camera.h / 6.0f);

	PrepareUI();

	BuildingState::originalTexture = s_Textures.GetTexture("canBuild");
	s_Building.buildingPlace.SetTexture(s_Textures.GetTexture("transparent"));

	m_PauseLabel = Label(int32_t(s_Camera.w) - 10, 10, "PAUSED", defaultFont);
	m_PauseLabel.m_Drawable = false;

	const SDL_Rect &pauseLabelRect = m_PauseLabel.GetRect();
	m_PauseLabel.UpdatePos(pauseLabelRect.x - pauseLabelRect.w, pauseLabelRect.y);

	IF_DEBUG(s_EnemiesAmountLabel = s_Manager.NewLabel(10, 200, " ", defaultFont););
	IF_DEBUG(s_PointedPosition = s_Manager.NewLabel(150, 10, " ", defaultFont););
	IF_DEBUG(s_FrameDelay = s_Manager.NewLabel(500, 10, " ", defaultFont, SDL_Color{ 0, 200, 0, 255 }););

	int32_t centerX = App::WINDOW_WIDTH / 2;
	int32_t centerY = App::WINDOW_HEIGHT / 2;

	// MAIN MENU
	Button *btn = nullptr;

	for (std::size_t i = 0u; i < s_MainMenu.m_PrimaryButtons.size(); ++i)
	{
		btn = &s_MainMenu.m_PrimaryButtons.at(i);
		btn->destRect.w = App::WINDOW_WIDTH / 7;
		btn->destRect.h = App::WINDOW_HEIGHT / 14;
		btn->destRect.x = centerX - btn->destRect.w / 2;
		btn->destRect.y = centerY - btn->destRect.h / 2 + (static_cast<int32_t>(i) - 1) * (btn->destRect.h + btn->destRect.h / 4);
	}

	// Button "Play"
	{
		btn = &s_MainMenu.m_PrimaryButtons.at(0);
		btn->m_Label = Label(btn->destRect.x + btn->destRect.w / 2, btn->destRect.y + btn->destRect.h / 4, "Play", defaultFont);
		const SDL_Rect &labelRect = btn->m_Label.GetRect();
		btn->m_Label.UpdatePos(labelRect.x - labelRect.w / 2, labelRect.y);
	}

	// Button "Options"
	{
		btn = &s_MainMenu.m_PrimaryButtons.at(1);
		btn->m_Label = Label(btn->destRect.x + btn->destRect.w / 2, btn->destRect.y + btn->destRect.h / 4, "Options", defaultFont);
		const SDL_Rect &labelRect = btn->m_Label.GetRect();
		btn->m_Label.UpdatePos(labelRect.x - labelRect.w / 2, labelRect.y);
	}

	// Button "Exit"
	{
		btn = &s_MainMenu.m_PrimaryButtons.at(2);
		btn->m_Label = Label(btn->destRect.x + btn->destRect.w / 2, btn->destRect.y + btn->destRect.h / 4, "Exit", defaultFont);
		const SDL_Rect &labelRect = btn->m_Label.GetRect();
		btn->m_Label.UpdatePos(labelRect.x - labelRect.w / 2, labelRect.y);
	}
	// MAIN MENU

	App::s_IsRunning = initialized;
}

App::~App()
{
	if (s_Renderer)
	{
		SDL_DestroyRenderer(s_Renderer);
		IF_DEBUG(App::s_Logger.AddLog(std::string_view("App::~App: Renderer has been destroyed")););
		s_Renderer = nullptr;
	}

	if (m_Window)
	{
		SDL_DestroyWindow(m_Window);
		m_Window = nullptr;
		IF_DEBUG(App::s_Logger.AddLog(std::string_view("App::~App: Window has been destroyed")););
	}

	// TTF_Quit() is called in ~TextureManager()
	SDL_Quit();
	IF_DEBUG(App::s_Logger.AddLog(std::string_view("App::~App: Triggered SDL_Quit()")););

	if (App::s_Instance == this)
		App::s_Instance = nullptr;

	IF_DEBUG(App::s_Logger.AddLog(std::string_view("App::~App: Application has been cleared")););
	IF_DEBUG(App::s_Logger.PrintQueuedLogs(););
}

void App::EventHandler()
{
	static uint32_t windowMinimizedTicks = 0u;

	SDL_PollEvent(&s_Event);

	switch (s_Event.type)
	{
	// WINDOW EVENTS
	case SDL_WINDOWEVENT:
		switch (s_Event.window.event)
		{
			case SDL_WINDOWEVENT_SIZE_CHANGED:
				OnResolutionChange();
				return;
			case SDL_WINDOWEVENT_MINIMIZED:
				windowMinimizedTicks = SDL_GetTicks();
				s_IsWindowMinimized = true;
				return;
			case SDL_WINDOWEVENT_RESTORED:
				g_PausedTicks += SDL_GetTicks() - windowMinimizedTicks;
				windowMinimizedTicks = 0u;
				s_IsWindowMinimized = false;
				return;
			default:
				return;
		}
	// END OF WINDOW EVENTS
	
	// MOUSE EVENTS
	case SDL_MOUSEMOTION:
		s_MouseX = s_Event.motion.x;
		s_MouseY = s_Event.motion.y;

		OnCursorMove();
		return;
	//case SDL_MOUSEBUTTONUP:
	case SDL_MOUSEBUTTONDOWN:
		if (s_UIState == UIState::mainMenu)
			s_MainMenu.HandleMouseButtonEvent();
		else
			App::s_CurrentLevel->HandleMouseButtonEvent();
		return;
	// END OF MOUSE EVENTS

	// KEYBOARD EVENTS
	case SDL_KEYDOWN:
		switch (App::s_Event.key.keysym.sym)
		{
		case SDLK_b: // switch between building state
			SwitchBuildingState();
			return;
		IF_DEBUG(
		case SDLK_n:
			s_SwapTowerType = !s_SwapTowerType;
			App::s_Logger.AddLog(std::format("App::s_SwapTowerType: {}", static_cast<int32_t>(s_SwapTowerType)));
			return;
		);
		case SDLK_y: // lock/unlock camera movement
			SwitchCameraMode();
			return;
		case SDLK_TAB: // move the camera to the primary point (base)
			{
				const Vector2D &basePos = s_CurrentLevel->GetBase()->m_Pos;

				s_Camera.x = basePos.x - s_Camera.w / 2.0f;
				s_Camera.y = basePos.y - s_Camera.h / 2.0f;

				UpdateCamera();
			}
			return;
		// Function keys
		case SDLK_F1: // resolution 800x600
			SDL_SetWindowSize(m_Window, 800, 600);
			SDL_SetWindowPosition(m_Window, (SDL_WINDOWPOS_CENTERED | (0)), (SDL_WINDOWPOS_CENTERED | (0)));
			return;
		case SDLK_F2: // resolution 1280x720
			SDL_SetWindowSize(m_Window, 1280, 720);
			SDL_SetWindowPosition(m_Window, (SDL_WINDOWPOS_CENTERED | (0)), (SDL_WINDOWPOS_CENTERED | (0)));
			return;
		case SDLK_F3:
			SDL_MaximizeWindow(m_Window);
			return;
#ifdef DEBUG
		case SDLK_F4: // Speed up enemies' movement speed
			{
				static std::string_view debugSpeedName = "";

				switch (s_Speedy)
				{
				case EnemyDebugSpeed::none:
					s_Speedy = EnemyDebugSpeed::faster;
					debugSpeedName = "EnemyDebugSpeed::faster";
					break;
				case EnemyDebugSpeed::faster:
					s_Speedy = EnemyDebugSpeed::stay;
					debugSpeedName = "EnemyDebugSpeed::stay";
					break;
				case EnemyDebugSpeed::stay:
					s_Speedy = EnemyDebugSpeed::none;
					debugSpeedName = "EnemyDebugSpeed::none";
					break;
				}

				for (const auto &e : g_Enemies)
				{
					if (!e->IsActive())
						continue;

					dynamic_cast<Enemy*>(e)->DebugSpeed();
				}

				s_Logger.AddLog(std::string_view("Enemies' speed up: "), false);
				s_Logger.AddLog(debugSpeedName);
			}
			return;
#endif
		case SDLK_F5: // Add life
			AddLifes();
			return;
		case SDLK_F6: // Take life
			TakeLifes();
			return;
		case SDLK_F7: // Add coin
			AddCoins();
			return;
		case SDLK_F8: // Take coin
			TakeCoins();
			return;
#ifdef DEBUG
		case SDLK_F9: // Refresh attack (in case of EnemyDebugSpeed::stay)
			for (const auto &e : g_Enemies)
				dynamic_cast<Enemy*>(e)->ValidAttacker();
			return;
#endif
		case SDLK_F10: // Destroy all enemies
			for (const auto &e : g_Enemies)
				e->Destroy();
			return;
		case SDLK_F11: // Switch fullscreen
			if (m_IsFullscreen)
			{
				SDL_SetWindowFullscreen(m_Window, 0);
				SDL_SetWindowPosition(m_Window, (SDL_WINDOWPOS_CENTERED | (0)), (SDL_WINDOWPOS_CENTERED | (0)));
			}
			else
			{
				SDL_SetWindowFullscreen(m_Window, SDL_WINDOW_FULLSCREEN);
			}
			m_IsFullscreen = !m_IsFullscreen;
			return;
		// End of function keys
		case SDLK_ESCAPE:
			s_IsRunning = false;
			return;
		}
		return;
	// END OF KEYBOARD EVENTS
	case SDL_QUIT:
		s_IsRunning = false;
		return;
	default:
		return;
	}
}

void App::Update()
{
	if (IsGamePaused())
		return;

	if (s_CameraMovement.moveX != 0.0f || s_CameraMovement.moveY != 0.0f)
	{
		UpdateCamera();
	}

	App::s_CurrentLevel->ManageWaves();

	App::s_Manager.Refresh();
	App::s_Manager.Update();
}

void App::Render()
{
	SDL_RenderClear(App::s_Renderer);

	if (s_UIState == UIState::mainMenu)
	{
		s_MainMenu.Render();
	}
	else
	{
		App::s_CurrentLevel->Render();

		DrawUI();
	}

	IF_DEBUG(s_PointedPosition->Draw(););

	SDL_RenderPresent(App::s_Renderer);
}

void App::DrawUI()
{
	IF_DEBUG(s_EnemiesAmountLabel->Draw(););
	IF_DEBUG(s_FrameDelay->Draw(););

	m_PauseLabel.Draw();

	for (std::size_t i = 0u; i < s_UIElements.size(); i++)
	{
		s_UIElements.at(i).Draw();
	}

	TextureManager::DrawTexture(UIElement::s_CoinTexture, UIElement::coinRect, UIElement::coinDestRect);
	TextureManager::DrawTexture(UIElement::s_HeartTexture, UIElement::heartRect, UIElement::heartDestRect);

	s_UICoinsNotification.Draw();
}

void App::UpdateCamera()
{
	CameraMovement::realVelocity.x = s_CameraMovement.moveX * App::s_ElapsedTime;
	CameraMovement::realVelocity.y = s_CameraMovement.moveY * App::s_ElapsedTime;

	s_Camera.x += CameraMovement::realVelocity.x;
	s_Camera.y += CameraMovement::realVelocity.y;

	MakeCameraCorrect();

	IF_DEBUG(
		s_PointedPosition->UpdateText(std::format("({}, {}), ({}, {}), ({}, {})",
			s_MouseX,
			s_MouseY,
			static_cast<int32_t>(s_Camera.x + s_MouseX),
			static_cast<int32_t>(s_Camera.y + s_MouseY),
			s_Building.coordinates.x,
			s_Building.coordinates.y)
		);
	)

	s_CurrentLevel->OnUpdateCamera();

	CameraMovement::realVelocity = { 0.0f, 0.0f };
}

void App::OnResolutionChange()
{
	SDL_GetRendererOutputSize(s_Renderer, &WINDOW_WIDTH, &WINDOW_HEIGHT);

	if (s_UIState == UIState::mainMenu)
	{
		s_MainMenu.OnResolutionChange();
		return;
	}

	m_PauseLabel.UpdatePos({ m_PauseLabel.GetPos().x + (static_cast<float>(App::WINDOW_WIDTH) - s_Camera.w), 10.0f });

	if (static_cast<float>(App::WINDOW_WIDTH) > s_Camera.w)
		s_Camera.x -= static_cast<float>(App::WINDOW_WIDTH) - s_Camera.w;
	else if (static_cast<float>(App::WINDOW_WIDTH) < s_Camera.w)
		s_Camera.x += s_Camera.w - static_cast<float>(App::WINDOW_WIDTH);

	if (static_cast<float>(App::WINDOW_HEIGHT) > s_Camera.h)
		s_Camera.y -= static_cast<float>(App::WINDOW_HEIGHT) - s_Camera.h;
	else if (static_cast<float>(App::WINDOW_HEIGHT) < s_Camera.h)
		s_Camera.y += s_Camera.h - static_cast<float>(App::WINDOW_HEIGHT);

	s_Camera.w = static_cast<float>(App::WINDOW_WIDTH);
	s_Camera.h = static_cast<float>(App::WINDOW_HEIGHT);

	s_CameraMovement.border.x = static_cast<float>(App::s_CurrentLevel->m_MapData.at(3)) - s_Camera.w;
	s_CameraMovement.border.y = static_cast<float>(App::s_CurrentLevel->m_MapData.at(4)) - s_Camera.h;
	s_CameraMovement.rangeW = static_cast<int32_t>(s_Camera.w / 6.0f);
	s_CameraMovement.rangeH = static_cast<int32_t>(s_Camera.h / 6.0f);

	MakeCameraCorrect();
	s_CurrentLevel->OnUpdateCamera();
}

void App::SetUIState(UIState state)
{
	if (s_UIState == state)
		return;

	std::string_view newState;

	m_PreviousUIState = s_UIState;
	s_UIState = state;

	static uint32_t startPausedTicks = 0u;

	if (startPausedTicks == 0u)
	{
		if (IsGamePaused(state))
		{
			startPausedTicks = SDL_GetTicks();
		}
	}
	else if (!IsGamePaused(state))
	{
		g_PausedTicks += SDL_GetTicks() - startPausedTicks;
		startPausedTicks = 0u;
	}

	switch (state)
	{
	case UIState::mainMenu:
		newState = "mainMenu";
		m_PauseLabel.m_Drawable = false;
		App::Instance().OnResolutionChange();
		break;
	case UIState::none:
		newState = "none";
		m_PauseLabel.m_Drawable = false;
		break;
	case UIState::building:
		newState = "building";
		m_PauseLabel.m_Drawable = true;
		ManageBuildingState();
		break;
	}

	App::s_Logger.AddLog(std::string_view("App::SetUIState: "), false);
	App::s_Logger.AddLog(newState);
}

void App::LoadLevel()
{
	for (uint16_t i = 0u; i < Level::s_LayersAmount; i++)
	{
		std::ifstream mapFile(std::format("levels/{}/map_layer{}.map", s_CurrentLevel->GetID() + 1, i));

		s_CurrentLevel->Setup(mapFile, i);
	}

	s_CurrentLevel->SetupBase(s_CurrentLevel->m_BasePos);

	const Vector2D &basePos = s_CurrentLevel->GetBase()->m_Pos;

	s_Camera.x = basePos.x - s_Camera.w / 2.0f;
	s_Camera.y = basePos.y - s_Camera.h / 2.0f;

	App::Instance().SetCoins(5);

	App::s_Logger.AddLog(std::format("Loaded level {}", s_CurrentLevel->GetID() + 1));
}

void App::SwitchBuildingState()
{
	static SDL_Texture *transparentTexture = s_Textures.GetTexture("transparent");

	switch (s_UIState)
	{
	case UIState::building:
		s_Building.buildingPlace.SetTexture(transparentTexture);
		SetUIState(UIState::none);
		return;
	case UIState::none:
		s_Building.buildingPlace.SetTexture(BuildingState::originalTexture);
		SetUIState(UIState::building);
		return;
	default:
		return;
	}

	/*if (s_UIState == UIState::building)
	{
		s_Building.buildingPlace.SetTexture(transparentTexture);
		SetUIState(UIState::none);
	}
	else if (s_UIState == UIState::none)
	{
		s_Building.buildingPlace.SetTexture(BuildingState::originalTexture);
		SetUIState(UIState::building);
	}
	else
	{
		return;
	}*/
}

void App::ManageBuildingState()
{
	IF_NDEBUG(
		s_Building.coordinates.x = std::floorf((App::s_Camera.x / static_cast<float>(s_CurrentLevel->m_ScaledTileSize)) + static_cast<float>(s_MouseX) / static_cast<float>(s_CurrentLevel->m_ScaledTileSize));
		s_Building.coordinates.y = std::floorf((App::s_Camera.y / static_cast<float>(s_CurrentLevel->m_ScaledTileSize)) + static_cast<float>(s_MouseY) / static_cast<float>(s_CurrentLevel->m_ScaledTileSize));
	);

	s_Building.pointedTile = App::s_CurrentLevel->GetTileFrom(s_Building.coordinates.x, s_Building.coordinates.y, 0);
	if (!s_Building.pointedTile)
		return;

	if (App::s_CurrentLevel->IsTileWalkable(s_Building.coordinates))
	{
		s_Building.buildingPlace.SetPos(s_Building.pointedTile->GetPos());
		BuildingState::originalTexture = s_Textures.GetTexture("cantBuild");
		s_Building.buildingPlace.SetTexture(BuildingState::originalTexture);
		s_Building.canBuild = false;
		s_Building.buildingPlace.AdjustToView();
		return;
	}

	s_Building.buildingPlace.SetPos(s_Building.pointedTile->GetPos());
	BuildingState::originalTexture = s_Textures.GetTexture("canBuild");
	s_Building.buildingPlace.SetTexture(BuildingState::originalTexture);
	s_Building.canBuild = true;
	s_Building.towerToUpgrade = nullptr;
	
	s_Building.buildingPlace.AdjustToView();

	// pointedTile refers to one of four tiles pointed by building tile (basically by a mouse and 3 more tiles in the building tile's range)
	Tile *pointedTile = s_Building.pointedTile;

	// Show to player the tower can be upgraded, but tower can be upgraded only if it's pointing the first tile of Tower to avoid confusion
	{
		Tower *tower = pointedTile->GetTowerOccupying();
		if (tower && tower->CanUpgrade() && pointedTile == tower->GetOccupiedTile(0))
		{
			BuildingState::originalTexture = s_Textures.GetTexture("upgradeTower");
			s_Building.buildingPlace.SetTexture(BuildingState::originalTexture);
			s_Building.canBuild = false;
			s_Building.towerToUpgrade = tower;
			return;
		}
	}

	for (auto i = 0u; i < 4u; i++)
	{
		pointedTile = App::s_CurrentLevel->GetTileFrom(static_cast<uint32_t>(s_Building.coordinates.x) + i % 2, static_cast<uint32_t>(s_Building.coordinates.y) + i / 2, 0);
		if (!pointedTile || !pointedTile->GetTowerOccupying() && s_CurrentLevel->GetBase()->m_Tile != pointedTile)
			continue;

		BuildingState::originalTexture = s_Textures.GetTexture("cantBuild");
		s_Building.buildingPlace.SetTexture(BuildingState::originalTexture);
		s_Building.canBuild = false;
		return;
	}
}

uint16_t App::GetDamageOf(ProjectileType type)
{
	uint16_t minDmg = 0, maxDmg = 0;

	switch (type)
	{
		case ProjectileType::arrow:
			minDmg = 17;
			maxDmg = 30;
			break;
		case ProjectileType::thunder:
			minDmg = 35;
			maxDmg = 40;
			break;
	}

	static std::uniform_int_distribution<uint16_t> dmg(minDmg, maxDmg);
	return dmg(g_Rng);
}