#include "app.h"
#include "entity/entity.h"
#include "entity/enemy.h"
#include "entity/tile.h"

#include "SDL_image.h"
#include "SDL_ttf.h"

#include <fstream>
#include <cmath>

static constexpr uint32_t coinsCooldownNotification = 1000u;

extern SDL_DisplayMode displayInfo;

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

// [0] = waves, [1] = coins, [2] = lifes, [3] = time
std::array<UIElement, 4> App::s_UIElements{};
std::array<UIElement, Tower::s_TowerTypeSize> App::s_ExpandingTowers{};
Label App::s_UICoinsNotification(coinsCooldownNotification);

bool App::s_IsCameraLocked = true;

CameraMovement App::s_CameraMovement{};

IF_DEBUG(Label *App::s_EnemiesAmountLabel = nullptr;);
IF_DEBUG(Label *App::s_PointedPosition = nullptr;);
IF_DEBUG(Label *App::s_FrameDelay = nullptr;);

IF_DEBUG(EnemyDebugSpeed App::s_Speedy;);
// class App STATIC VARIABLES

Vector2D CameraMovement::realVelocity{};

SDL_Texture *BuildingState::transparentTexture = nullptr;
SDL_Texture *BuildingState::originalTexture = nullptr;
SDL_Texture *BuildingState::cantBuildTexture = nullptr;
SDL_Texture *BuildingState::upgradingTexture = nullptr;
SDL_Texture *BuildingState::sellingTexture = nullptr;

// class App GLOBAL VARIABLES
std::default_random_engine g_Rng(App::s_Rnd());

auto &g_Projectiles = App::s_Manager.GetGroup(EntityGroup::projectile);
auto &g_Towers = App::s_Manager.GetGroup(EntityGroup::tower);
auto &g_Attackers = App::s_Manager.GetGroup(EntityGroup::attacker);
auto &g_Enemies = App::s_Manager.GetGroup(EntityGroup::enemy);

uint32_t g_PausedTicks = 0u;
// class App GLOBAL VARIABLES

App::App()
{
	TTF_Font *defaultFont = nullptr;

	if (!App::s_Instance)
		App::s_Instance = this;
	else
		m_Initialized = false;

	InitWindowAndRenderer();

	s_Textures.LoadAssets();

	defaultFont = App::s_Textures.GetFont("default");

	AssignStaticAssets();

	m_Levels.reserve(MainMenu::s_LevelsToLoad);

	for (uint16_t i = 0u; i < MainMenu::s_LevelsToLoad; i++)
	{
		m_Levels.emplace_back(i);
	}

	App::s_CurrentLevel = &m_Levels.at(0);

	if (!App::s_CurrentLevel || App::s_CurrentLevel->HasLoadingFailed())
	{
		App::s_Logger.AddLog(std::string_view("First level couldn't be loaded properly."));
		m_Initialized = false;
	}
	else
	{
		s_CameraMovement.border.x = static_cast<float>(App::s_CurrentLevel->m_MapData.at(3)) - s_Camera.w;
		s_CameraMovement.border.y = static_cast<float>(App::s_CurrentLevel->m_MapData.at(4)) - s_Camera.h;

		s_Building.buildingPlace.InitSpecialTile();
	}
	
	s_CameraMovement.rangeW = static_cast<int32_t>(s_Camera.w / 6.0f);
	s_CameraMovement.rangeH = static_cast<int32_t>(s_Camera.h / 6.0f);

	UIElement::InitUI();

	s_Building.buildingPlace.SetTexture(BuildingState::transparentTexture);

	m_PauseLabel = Label(int32_t(s_Camera.w) - 10, 10, "PAUSED", defaultFont);
	m_PauseLabel.m_Drawable = false;

	const SDL_Rect &pauseLabelRect = m_PauseLabel.GetRect();
	m_PauseLabel.UpdatePos(pauseLabelRect.x - pauseLabelRect.w, pauseLabelRect.y);

	IF_DEBUG(s_EnemiesAmountLabel = s_Manager.NewLabel(10, 200, " ", defaultFont););
	IF_DEBUG(s_PointedPosition = s_Manager.NewLabel(0, App::WINDOW_HEIGHT, " ", defaultFont););
	IF_DEBUG(s_FrameDelay = s_Manager.NewLabel(400, 10, " ", defaultFont, SDL_Color{ 0, 200, 0, 255 }););

	IF_DEBUG(
		s_PointedPosition->UpdatePos({ 0.0f, static_cast<float>(App::WINDOW_HEIGHT - s_PointedPosition->GetRect().h) });
	);

	InitMainMenu();

	App::s_IsRunning = m_Initialized;
}

App::~App()
{
	if (s_Renderer)
	{
		SDL_DestroyRenderer(s_Renderer);
		IF_DEBUG(App::s_Logger.AddInstantLog(std::string_view("App::~App: Renderer has been destroyed")););
		s_Renderer = nullptr;
	}

	if (m_Window)
	{
		SDL_DestroyWindow(m_Window);
		m_Window = nullptr;
		IF_DEBUG(App::s_Logger.AddInstantLog(std::string_view("App::~App: Window has been destroyed")););
	}

	if (App::s_Instance == this)
		App::s_Instance = nullptr;

	IF_DEBUG(App::s_Logger.AddInstantLog(std::string_view("App::~App: Application has been cleared")););
}

void App::InitWindowAndRenderer()
{
	// Window
	m_Window = SDL_CreateWindow("Tower Defense", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, App::WINDOW_WIDTH, App::WINDOW_HEIGHT, SDL_WINDOW_RESIZABLE);
	if (!m_Window)
	{
		App::s_Logger.AddLog(std::string_view(SDL_GetError()));
		m_Initialized = false;
	}

	SDL_Surface *iconSurface = IMG_Load("assets/gugu.png");
	if (!iconSurface)
	{
		App::s_Logger.AddLog(std::string_view(SDL_GetError()));
	}
	else
	{
		SDL_SetWindowIcon(m_Window, iconSurface);
		SDL_FreeSurface(iconSurface);
	}

	// Renderer
#ifdef DEBUG
	App::s_Renderer = SDL_CreateRenderer(m_Window, -1, 0);
#else
	App::s_Renderer = SDL_CreateRenderer(m_Window, -1, SDL_RENDERER_PRESENTVSYNC);
#endif
	if (!App::s_Renderer)
	{
		App::s_Logger.AddLog(std::string_view(SDL_GetError()));
		m_Initialized = false;
	}

	SDL_GetRendererOutputSize(App::s_Renderer, &WINDOW_WIDTH, &WINDOW_HEIGHT);
	SDL_SetRenderDrawColor(App::s_Renderer, 90, 0, 220, 255);
}

void App::AssignStaticAssets()
{
	for (std::size_t i = 0u; i < Tower::s_TowerTypeSize; i++)
	{
		Tower::s_TowerTextures[i][0] = App::s_Textures.GetTextureOf(TowerType(i));
		Tower::s_TowerTextures[i][1] = App::s_Textures.GetIconOf(TowerType(i));
	}

	UIElement::s_TransparentGreenTexture = App::s_Textures.GetTexture("transparentGreen");

	Tile::s_RangeTexture = App::s_Textures.GetTexture("highlightTowerRange");

	BuildingState::transparentTexture = App::s_Textures.GetTexture("transparent");
	//BuildingState::originalTexture = App::s_Textures.GetTexture("canBuild");
	BuildingState::originalTexture = App::s_Textures.GetTexture("upgradeIcon");
	BuildingState::cantBuildTexture = App::s_Textures.GetTexture("cantBuild");
	BuildingState::sellingTexture = App::s_Textures.GetTexture("sellIcon");

	App::s_GreenTex = App::s_Textures.GetTexture("green");
	App::s_Square = App::s_Textures.GetTexture("square");

	Button::s_DefaultButton = App::s_Textures.GetTexture("buttonUI");
	Button::s_DefaultButtonChecked = App::s_Textures.GetTexture("checkedButtonUI");
	Button::s_DefaultButtonUnchecked = App::s_Textures.GetTexture("uncheckedButtonUI");
	Button::s_HoveredButton = App::s_Textures.GetTexture("hoveredButtonUI");
	Button::s_HoveredButtonChecked = App::s_Textures.GetTexture("checkedHoveredButtonUI");
	Button::s_HoveredButtonUnchecked = App::s_Textures.GetTexture("uncheckedHoveredButtonUI");

	UIElement::s_BgTexture = App::s_Textures.GetTexture("elementUI");
	UIElement::s_CoinTexture = App::s_Textures.GetTexture("coinUI");
	UIElement::s_HeartTexture = App::s_Textures.GetTexture("heartUI");
	UIElement::s_TimerTexture = App::s_Textures.GetTexture("timerUI");
	UIElement::s_HammerTexture = App::s_Textures.GetTexture("buildHammer");
	UIElement::s_HammerGreenTexture = App::s_Textures.GetTexture("buildHammerGreen");
	UIElement::s_SellTexture = App::s_Textures.GetTexture("sellIcon");
	UIElement::s_UpgradeTexture = App::s_Textures.GetTexture("upgradeIcon");

	Level::s_Texture = App::s_Textures.GetTexture("mapSheet");

	Enemy::s_ArrowTexture = App::s_Textures.GetTexture("grayArrow");
}

void App::InitMainMenu()
{
	TTF_Font *defaultFont = App::s_Textures.GetFont("default");

	int32_t centerX = App::WINDOW_WIDTH / 2;
	int32_t centerY = App::WINDOW_HEIGHT / 2;

	MainMenu::s_GapBetweenButtons = (App::WINDOW_HEIGHT / 14) + ((App::WINDOW_HEIGHT / 14) / 4);

	Button *btn = nullptr;

	// Return button
	{
		btn = &s_MainMenu.m_ReturnButton;
		btn->destRect.w = App::WINDOW_WIDTH / 7;
		btn->destRect.h = App::WINDOW_HEIGHT / 14;
		btn->destRect.x = centerX - btn->destRect.w / 2;
		btn->destRect.y = centerY - btn->destRect.h / 2 + static_cast<int32_t>(s_MainMenu.m_PrimaryButtons.size()) * MainMenu::s_GapBetweenButtons;
		btn->m_Label = Label(btn->destRect.x + btn->destRect.w / 2, btn->destRect.y + btn->destRect.h / 4, "Quit", defaultFont);
		const SDL_Rect &labelRect = btn->m_Label.GetRect();
		btn->m_Label.UpdatePos(labelRect.x - labelRect.w / 2, labelRect.y);
	}
	// Return button

	// Title screen
	for (std::size_t i = 0u; i < s_MainMenu.m_PrimaryButtons.size(); ++i)
	{
		btn = &s_MainMenu.m_PrimaryButtons.at(i);
		btn->destRect.w = App::WINDOW_WIDTH / 7;
		btn->destRect.h = App::WINDOW_HEIGHT / 14;
		btn->destRect.x = centerX - btn->destRect.w / 2;
		btn->destRect.y = centerY - btn->destRect.h / 2 + (static_cast<int32_t>(i) - 1) * MainMenu::s_GapBetweenButtons;
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
	// Title screen

	// Options
	for (std::size_t i = 0u; i < s_MainMenu.m_OptionsButtons.size(); ++i)
	{
		btn = &s_MainMenu.m_OptionsButtons.at(i);
		btn->destRect.w = App::WINDOW_WIDTH / 7;
		btn->destRect.h = App::WINDOW_HEIGHT / 14;
		btn->destRect.x = centerX - btn->destRect.w / 2;
		btn->destRect.y = centerY - btn->destRect.h / 2 + (static_cast<int32_t>(i) - 1) * (btn->destRect.h + btn->destRect.h / 4);
	}

	// Button "V-Sync"
	{
		btn = &s_MainMenu.m_OptionsButtons.at(0);
		btn->m_Label = Label(btn->destRect.x + btn->destRect.w / 2, btn->destRect.y + btn->destRect.h / 4, "V-Sync", defaultFont);
		const SDL_Rect &labelRect = btn->m_Label.GetRect();
		btn->m_Label.UpdatePos(labelRect.x - labelRect.w / 2, labelRect.y);

		btn->m_Type = ButtonType::check;
		btn->m_IsChecked = SDL_GetHintBoolean(SDL_HINT_RENDER_VSYNC, SDL_FALSE);
	}
	// Options

	// Levels
	for (std::size_t i = 0u; i < s_MainMenu.m_LevelsButtons.size(); ++i)
	{
		btn = &s_MainMenu.m_LevelsButtons.at(i);
		btn->destRect.w = App::WINDOW_WIDTH / 7;
		btn->destRect.h = App::WINDOW_HEIGHT / 14;
		btn->destRect.x = centerX - btn->destRect.w / 2;
		btn->destRect.y = centerY - btn->destRect.h / 2 + (static_cast<int32_t>(i) - 1) * MainMenu::s_GapBetweenButtons;

		btn->m_Label = Label(btn->destRect.x + btn->destRect.w / 2, btn->destRect.y + btn->destRect.h / 4, std::format("Level #{}", i + 1), defaultFont);
		const SDL_Rect &labelRect = btn->m_Label.GetRect();
		btn->m_Label.UpdatePos(labelRect.x - labelRect.w / 2, labelRect.y);
	}
	// Levels
}

void App::EventHandler()
{
	SDL_PollEvent(&s_Event);

	switch (s_Event.type)
	{
	case SDL_WINDOWEVENT:
		HandleWindowEvent();
		return;
	
	// MOUSE EVENTS
	case SDL_MOUSEMOTION:
		s_MouseX = s_Event.motion.x;
		s_MouseY = s_Event.motion.y;

		OnCursorMove();
		return;
	//case SDL_MOUSEBUTTONUP:
	case SDL_MOUSEBUTTONDOWN:
		if (s_UIState == UIState::mainMenu)
		{
			s_MainMenu.HandleMouseButtonEvent();
		}
		else
		{
			App::s_CurrentLevel->HandleMouseButtonEvent(s_Event.button.button);
		}
		return;
	// END OF MOUSE EVENTS

	// KEYBOARD EVENTS
	case SDL_KEYDOWN:
		HandleKeyboardEvent();
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

	if (!s_CameraMovement.move.IsEqualZero())
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

		IF_DEBUG(s_EnemiesAmountLabel->Draw(););

		m_PauseLabel.Draw();

		UIElement::DrawUI();
	}

	IF_DEBUG(s_FrameDelay->Draw(););
	IF_DEBUG(s_PointedPosition->Draw(););

	SDL_RenderPresent(App::s_Renderer);
}

void App::HandleWindowEvent()
{
	static uint32_t windowMinimizedTicks = 0u;

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
}

void App::HandleKeyboardEvent()
{
	switch (App::s_Event.key.keysym.sym)
	{
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
		static std::string_view debugSpeedName;

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
	case SDLK_F11: // Switch full screen mode
		if (m_IsFullscreen)
		{
			SDL_SetWindowFullscreen(m_Window, 0);
			SDL_MaximizeWindow(m_Window);
			SDL_SetWindowPosition(m_Window, (SDL_WINDOWPOS_CENTERED | (0)), (SDL_WINDOWPOS_CENTERED | (0)));
		}
		else
		{
			SDL_SetWindowSize(m_Window, displayInfo.w, displayInfo.h); // Set the highest resolution before turning into full screen
			SDL_SetWindowFullscreen(m_Window, SDL_WINDOW_FULLSCREEN);
		}
		m_IsFullscreen = !m_IsFullscreen;
		return;
		// End of function keys
	case SDLK_ESCAPE:
		s_IsRunning = false;
		return;
	default:
		return;
	}
}

void App::UpdateCamera()
{
	CameraMovement::realVelocity = s_CameraMovement.move * App::s_ElapsedTime;

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

	CameraMovement::realVelocity.Zero();
}

void App::OnResolutionChange()
{
	SDL_GetRendererOutputSize(s_Renderer, &WINDOW_WIDTH, &WINDOW_HEIGHT);

	IF_DEBUG(
		s_PointedPosition->UpdatePos({ 0.0f, static_cast<float>(App::WINDOW_HEIGHT - s_PointedPosition->GetRect().h) });
	);

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

	s_UIState = state;

	static uint32_t startPausedTicks = 0u;

	// Check if there is already a counting of ticks
	if (startPausedTicks == 0u)
	{
		if (IsGamePaused(state))
		{
			// Assign current ticks if the game is paused (so as well it should count the ticks on pause state)
			startPausedTicks = SDL_GetTicks();
		}
	}
	else if (!IsGamePaused(state)) // If it should count the ticks in pause state and it's about to leave the pause
	{
		g_PausedTicks += SDL_GetTicks() - startPausedTicks; // Add the difference between old ticks and current ticks to g_PausedTicks
		startPausedTicks = 0u; // Assign 0 to startPausedTicks, so next time it'll be possible to say is it already counting ticks
	}

	m_PauseLabel.m_Drawable = IsGamePaused(state);

	switch (state)
	{
	case UIState::mainMenu:
		App::Instance().OnResolutionChange();
		return;
	case UIState::building:
	case UIState::upgrading:
	case UIState::selling:
		ManageBuildingState();
		return;
	}
}

void App::LoadLevel()
{
	Layer::s_MapWidth = App::s_CurrentLevel->m_MapData.at(0);
	App::s_CameraMovement.border.x = static_cast<float>(App::s_CurrentLevel->m_MapData.at(3)) - App::s_Camera.w;
	App::s_CameraMovement.border.y = static_cast<float>(App::s_CurrentLevel->m_MapData.at(4)) - App::s_Camera.h;

	App::s_Building.buildingPlace.InitSpecialTile();

	App::s_CurrentLevel->Init();

	const Vector2D &basePos = App::s_CurrentLevel->GetBase()->m_Pos;

	App::s_Camera.x = basePos.x - App::s_Camera.w / 2.0f;
	App::s_Camera.y = basePos.y - App::s_Camera.h / 2.0f;

	App::Instance().SetCoins(5u);

	App::s_Logger.AddLog(std::format("Loaded level {}", s_CurrentLevel->GetID() + 1));
}

void App::SwitchBuildingState(UIState newState)
{
	switch (newState)
	{
	case UIState::none:
		s_Building.buildingPlace.SetTexture(BuildingState::transparentTexture);
		break;
	case UIState::building:
	case UIState::upgrading:
		s_Building.buildingPlace.SetTexture(BuildingState::originalTexture);
		break;
	case UIState::selling:
		s_Building.buildingPlace.SetTexture(BuildingState::sellingTexture);
		break;
	default:
		return;
	}

	SetUIState(newState);
}

void App::ManageBuildingState()
{
	// Do this only if DEBUG is undefined, because if it's defined, it's already done in App::OnCursorMove()
	IF_NDEBUG(
		s_Building.coordinates.x = std::floorf((App::s_Camera.x / static_cast<float>(s_CurrentLevel->m_ScaledTileSize)) + static_cast<float>(s_MouseX) / static_cast<float>(s_CurrentLevel->m_ScaledTileSize));
		s_Building.coordinates.y = std::floorf((App::s_Camera.y / static_cast<float>(s_CurrentLevel->m_ScaledTileSize)) + static_cast<float>(s_MouseY) / static_cast<float>(s_CurrentLevel->m_ScaledTileSize));
		
		// Disallows to set a tower in the right edge (where 2 from 4 tiles are outside of the map border)
		// It's helpful with avoiding an issue about tiles in towers' range
		if (s_Building.coordinates.x + 1.0f >= static_cast<float>(App::s_CurrentLevel->m_MapData.at(0)))
			s_Building.coordinates.x--;
		if (s_Building.coordinates.y + 1.0f >= static_cast<float>(App::s_CurrentLevel->m_MapData.at(1)))
			s_Building.coordinates.y--;
	);

	s_Building.pointedTile = App::s_CurrentLevel->GetTileFrom(s_Building.coordinates.x, s_Building.coordinates.y, 0);
	if (!s_Building.pointedTile)
		return;

	s_Building.buildingPlace.SetPos(s_Building.pointedTile->GetPos());
	s_Building.canBuild = true;
	s_Building.towerToUpgradeOrSell = nullptr;
	s_Building.buildingPlace.AdjustToView();

	if (s_UIState == UIState::building)
	{
		if (App::s_CurrentLevel->IsTileWalkable(s_Building.coordinates))
		{
			s_Building.buildingPlace.SetPos(s_Building.pointedTile->GetPos());
			s_Building.buildingPlace.SetTexture(BuildingState::cantBuildTexture);
			s_Building.canBuild = false;
			s_Building.buildingPlace.AdjustToView();
			return;
		}

		s_Building.buildingPlace.SetTexture(BuildingState::originalTexture);

		// pointedTile refers to one of four tiles pointed by building tile (basically by a mouse and 3 more tiles in the building tile's range)
		Tile *pointedTile = s_Building.pointedTile;

		/*
		// Show to player the tower can be upgraded, but tower can be upgraded only if it's pointing the first tile of Tower to avoid confusion
		{
			Tower *tower = pointedTile->GetTowerOccupying();
			if (tower && tower->CanUpgrade() && pointedTile == tower->GetOccupiedTile(0u))
			{
				s_Building.buildingPlace.SetTexture(BuildingState::upgradingTexture);
				s_Building.canBuild = false;
				s_Building.towerToUpgradeOrSell = tower;
				return;
			}
		}
		*/

		for (auto i = 0u; i < 4u; i++)
		{
			pointedTile = App::s_CurrentLevel->GetTileFrom(static_cast<uint32_t>(s_Building.coordinates.x) + i % 2, static_cast<uint32_t>(s_Building.coordinates.y) + i / 2, 0);
			if (!pointedTile || !pointedTile->GetTowerOccupying() && s_CurrentLevel->GetBase()->m_Tile != pointedTile)
				continue;

			s_Building.buildingPlace.SetTexture(BuildingState::cantBuildTexture);
			s_Building.canBuild = false;
			return;
		}

		return;
	}
	
	if (s_UIState == UIState::upgrading)
	{
		Tower *tower = s_Building.pointedTile->GetTowerOccupying();
		if (!tower || s_Building.pointedTile != tower->GetOccupiedTile(0u) || !tower->CanUpgrade())
		{
			s_Building.buildingPlace.SetTexture(BuildingState::cantBuildTexture);
			s_Building.canBuild = false;
			s_Building.towerToUpgradeOrSell = nullptr;
			return;
		}

		s_Building.buildingPlace.SetTexture(BuildingState::originalTexture);
		s_Building.canBuild = true;
		s_Building.towerToUpgradeOrSell = tower;

		return;
	}

	if (s_UIState == UIState::selling)
	{
		Tower *tower = s_Building.pointedTile->GetTowerOccupying();
		if (!tower || s_Building.pointedTile != tower->GetOccupiedTile(0u))
		{
			s_Building.buildingPlace.SetTexture(BuildingState::cantBuildTexture);
			s_Building.canBuild = false;
			s_Building.towerToUpgradeOrSell = nullptr;
			return;
		}

		s_Building.buildingPlace.SetTexture(BuildingState::sellingTexture);
		s_Building.canBuild = true;
		s_Building.towerToUpgradeOrSell = tower;

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