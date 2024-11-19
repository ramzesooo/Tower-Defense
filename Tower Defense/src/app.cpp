#include "app.h"

#include <fstream>
#include <cmath>
#include <format>

constexpr uint16_t levelsToLoad = 1;

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
SDL_FRect App::s_Camera { 0.0f, 0.0f, (float)App::WINDOW_WIDTH, (float)App::WINDOW_HEIGHT };

Level *App::s_CurrentLevel = nullptr;

uint16_t App::s_TowerRange = 3;

float App::s_ElapsedTime = NULL;

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

SDL_Rect UIElement::coinDestRect;
SDL_Rect UIElement::heartDestRect;

UIElement App::s_UICoins;
UIElement App::s_UIWaves;
UIElement App::s_UILifes;
Label App::s_UICoinsNotification(2000);

#ifdef DEBUG
Label *App::s_EnemiesAmountLabel = nullptr;
#endif

bool App::s_IsCameraLocked = true;

CameraMovement App::s_CameraMovement;

#ifdef DEBUG
bool App::s_Speedy = false;
#endif
// END

std::default_random_engine g_Rng(App::s_Rnd());

auto &g_Projectiles = App::s_Manager.GetGroup(EntityGroup::projectile);
auto &g_Towers = App::s_Manager.GetGroup(EntityGroup::tower);
auto &g_Attackers = App::s_Manager.GetGroup(EntityGroup::attacker);
auto &g_Enemies = App::s_Manager.GetGroup(EntityGroup::enemy);

App::App()
{
	bool initialized = true;

	if (!App::s_Instance)
		App::s_Instance = this;
	else
		initialized = false;

	m_Window = SDL_CreateWindow("Tower Defense", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, App::WINDOW_WIDTH, App::WINDOW_HEIGHT, SDL_WINDOW_RESIZABLE);
	if (!m_Window)
	{
		App::s_Logger.AddLog(SDL_GetError());
		initialized = false;
	}

	SDL_Surface* iconSurface = IMG_Load("assets\\gugu.png");
	SDL_SetWindowIcon(m_Window, iconSurface);
	SDL_FreeSurface(iconSurface);

#ifdef DEBUG
	App::s_Renderer = SDL_CreateRenderer(m_Window, -1, 0);
#else
	App::s_Renderer = SDL_CreateRenderer(m_Window, -1, SDL_RENDERER_PRESENTVSYNC);
#endif
	if (!App::s_Renderer)
	{
		App::s_Logger.AddLog(SDL_GetError());
		initialized = false;
	}

	SDL_GetRendererOutputSize(App::s_Renderer, &WINDOW_WIDTH, &WINDOW_HEIGHT);
	SDL_SetRenderDrawColor(App::s_Renderer, 90, 0, 220, 255);

	s_CameraMovement.rangeW = WINDOW_WIDTH / 6;
	s_CameraMovement.rangeH = WINDOW_HEIGHT / 6;

	App::s_Textures.AddTexture("mapSheet", "assets\\tileset.png");

	App::s_Textures.AddTexture("szpaku", "assets\\szpaku.jpg");
	App::s_Textures.AddTexture("buttonUI", "assets\\ui\\ui_button.png");
	App::s_Textures.AddTexture("hoveredButtonUI", "assets\\ui\\ui_button_hovered.png");
	App::s_Textures.AddTexture("canBuild", "assets\\ui\\tile_CanBuild.png");
	App::s_Textures.AddTexture("cantBuild", "assets\\ui\\tile_CantBuild.png");
	App::s_Textures.AddTexture("upgradeTower", "assets\\ui\\tile_Upgrade.png");
	//App::s_Textures.AddTexture("fullHealth", "assets\\ui\\health_bar.png");
	//App::s_Textures.AddTexture("emptyHealth", "assets\\ui\\empty_bar.png");
	App::s_Textures.AddTexture("elementUI", "assets\\ui\\ui_element.png");
	App::s_Textures.AddTexture("coinUI", "assets\\ui\\coin.png");
	App::s_Textures.AddTexture("heartUI", "assets\\ui\\heart.png");

	App::s_Textures.AddTexture("base", "assets\\base.png");
	App::s_Textures.AddTexture("tower", "assets\\towers\\tower.png");
	App::s_Textures.AddTexture("square", "assets\\square_32x32.png");
	App::s_Textures.AddTexture("green", "assets\\green_32x32.png");
	App::s_Textures.AddTexture("transparent", "assets\\transparent.png");
	App::s_Textures.AddTexture("grayArrow", "assets\\grayArrow_32x32.png");

	App::s_Textures.AddTexture(TextureOf(ProjectileType::arrow), "assets\\arrow_16x16.png");
	App::s_Textures.AddTexture(TextureOf(AttackerType::archer), "assets\\entities\\friendly\\attackerArcher.png");
	App::s_Textures.AddTexture(TextureOf(AttackerType::hunter), "assets\\entities\\friendly\\attackerHunter.png");
	App::s_Textures.AddTexture(TextureOf(AttackerType::musketeer), "assets\\entities\\friendly\\attackerMusketeer.png");
	App::s_Textures.AddTexture(TextureOf(EnemyType::elf), "assets\\entities\\enemy\\enemyElf.png");
	App::s_Textures.AddTexture(TextureOf(EnemyType::goblinWarrior), "assets\\entities\\enemy\\enemyGoblinWarrior.png");
	App::s_Textures.AddTexture(TextureOf(EnemyType::dwarfSoldier), "assets\\entities\\enemy\\enemyDwarfSoldier.png");
	App::s_Textures.AddTexture(TextureOf(EnemyType::dwarfKing), "assets\\entities\\enemy\\enemyDwarfKing.png");

	App::s_Textures.AddFont("default", "assets\\F25_Bank_Printer.ttf", 15);
	App::s_Textures.AddFont("enemyHealth", "assets\\Rostack.otf", 13);
	App::s_Textures.AddFont("baseHealth", "assets\\Rostack.otf", 26);

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

	for (uint16_t i = 0; i < levelsToLoad; i++)
	{
		m_Levels.emplace_back(std::move(std::make_unique<Level>(i)));
	}

	App::s_CurrentLevel = m_Levels.at(0).get();

	if (!App::s_CurrentLevel || App::s_CurrentLevel->HasLoadingFailed())
	{
		App::s_Logger.AddLog("First level couldn't be loaded properly.");
		initialized = false;
	}

	// UI ELEMENTS
	{
		s_UIWaves.destRect = { (int32_t)App::s_Camera.w / 30, (int32_t)App::s_Camera.h / 30, UIElement::srcRect.w * 3, UIElement::srcRect.h * 3 };
		s_UIWaves.m_Label = Label(s_UIWaves.destRect.x, s_UIWaves.destRect.y, "Wave: 1/1", App::s_Textures.GetFont("default"));
		
		const SDL_Rect &labelRect = s_UIWaves.m_Label.GetRect();
		s_UIWaves.m_Label.UpdatePos(labelRect.x + (s_UIWaves.destRect.w / 2 - labelRect.w / 2), labelRect.y + (s_UIWaves.destRect.h / 2 - labelRect.h / 2));
	}

	{
		s_UICoins.destRect = { (int32_t)App::s_Camera.w / 30, s_UIWaves.destRect.y + s_UIWaves.destRect.h, UIElement::srcRect.w * 3, UIElement::srcRect.h * 3 };
		s_UICoins.m_Label = Label(s_UICoins.destRect.x, s_UICoins.destRect.y, "100", App::s_Textures.GetFont("default"));
		
		const SDL_Rect &labelRect = s_UICoins.m_Label.GetRect();
		s_UICoins.m_Label.UpdatePos(labelRect.x + (s_UICoins.destRect.w / 2 - labelRect.w / 2), labelRect.y + (s_UICoins.destRect.h / 2 - labelRect.h / 2));
		UIElement::coinDestRect = { s_UICoins.destRect.x + UIElement::coinRect.w, s_UICoins.destRect.y + s_UICoins.destRect.h / 4, UIElement::coinRect.w * 3, s_UICoins.destRect.h / 2 };
	
		s_UICoinsNotification = Label(labelRect.x + labelRect.w + labelRect.w / 2, labelRect.y, "+0", App::s_Textures.GetFont("default"));
		s_UICoinsNotification.SetAlpha(0);
	}

	{
		s_UILifes.destRect = { (int32_t)App::s_Camera.w / 30, s_UICoins.destRect.y + s_UICoins.destRect.h, UIElement::srcRect.w * 3, UIElement::srcRect.h * 3 };
		s_UILifes.m_Label = Label(s_UILifes.destRect.x, s_UILifes.destRect.y, "100", App::s_Textures.GetFont("default"));
		
		const SDL_Rect &labelRect = s_UILifes.m_Label.GetRect();
		s_UILifes.m_Label.UpdatePos(labelRect.x + (s_UILifes.destRect.w / 2 - labelRect.w / 2), labelRect.y + (s_UILifes.destRect.h / 2 - labelRect.h / 2));
		UIElement::heartDestRect = { s_UILifes.destRect.x, s_UILifes.destRect.y + s_UILifes.destRect.h / 4, UIElement::heartRect.w, s_UILifes.destRect.h - UIElement::heartRect.h / 2 };
	}
	// UI ELEMENTS

	s_Building.originalTexture = s_Textures.GetTexture("canBuild");
	s_Building.buildingPlace->SetTexture(s_Textures.GetTexture("transparent"));

	m_PauseLabel = s_Manager.NewLabel(int32_t(s_Camera.w) - 10, 10, "PAUSED", s_Textures.GetFont("default"));
	m_PauseLabel->m_Drawable = false;

	const SDL_Rect &pauseLabelRect = m_PauseLabel->GetRect();
	m_PauseLabel->UpdatePos(pauseLabelRect.x - pauseLabelRect.w, pauseLabelRect.y);

#ifdef DEBUG
	s_EnemiesAmountLabel = s_Manager.NewLabel(10, 200, " ", s_Textures.GetFont("default"));
#endif

	int32_t centerX = App::WINDOW_WIDTH / 2;
	int32_t centerY = App::WINDOW_HEIGHT / 2;

	// MAIN MENU
	Button *btn = nullptr;

	for (std::size_t i = 0; i < s_MainMenu.m_PrimaryButtons.size(); ++i)
	{
		btn = &s_MainMenu.m_PrimaryButtons.at(i);
		btn->destRect.w = App::WINDOW_WIDTH / 7;
		btn->destRect.h = App::WINDOW_HEIGHT / 14;
		btn->destRect.x = centerX - btn->destRect.w / 2;
		btn->destRect.y = centerY - btn->destRect.h / 2 + ((int32_t)i - 1) * (btn->destRect.h + btn->destRect.h / 4);
	}

	// Button "Play"
	{
		btn = &s_MainMenu.m_PrimaryButtons.at(0);
		btn->m_Label = Label(btn->destRect.x + btn->destRect.w / 2, btn->destRect.y + btn->destRect.h / 4, "Play", App::s_Textures.GetFont("default"));
		const SDL_Rect &labelRect = btn->m_Label.GetRect();
		btn->m_Label.UpdatePos(labelRect.x - labelRect.w / 2, labelRect.y);
	}

	// Button "Options"
	{
		btn = &s_MainMenu.m_PrimaryButtons.at(1);
		btn->m_Label = Label(btn->destRect.x + btn->destRect.w / 2, btn->destRect.y + btn->destRect.h / 4, "Options", App::s_Textures.GetFont("default"));
		const SDL_Rect &labelRect = btn->m_Label.GetRect();
		btn->m_Label.UpdatePos(labelRect.x - labelRect.w / 2, labelRect.y);
	}

	// Button "Exit"
	{
		btn = &s_MainMenu.m_PrimaryButtons.at(2);
		btn->m_Label = Label(btn->destRect.x + btn->destRect.w / 2, btn->destRect.y + btn->destRect.h / 4, "Exit", App::s_Textures.GetFont("default"));
		const SDL_Rect &labelRect = btn->m_Label.GetRect();
		btn->m_Label.UpdatePos(labelRect.x - labelRect.w / 2, labelRect.y);
	}

	// MAIN MENU

	App::s_IsRunning = initialized;
}

App::~App()
{
	m_Levels.clear();

	if (s_Renderer)
		SDL_DestroyRenderer(s_Renderer);

	if (m_Window)
		SDL_DestroyWindow(m_Window);

	// TTF_Quit() is called in ~TextureManager()
	SDL_Quit();
}

void App::EventHandler()
{
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
				s_IsWindowMinimized = true;
				return;
			case SDL_WINDOWEVENT_RESTORED:
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

		if (s_UIState == UIState::building)
		{
			ManageBuildingState();
			return;
		}

		if (!s_IsCameraLocked)
		{
			ManageCamera();
			return;
		}

		if (s_UIState == UIState::mainMenu)
		{
			s_MainMenu.OnCursorMove();
			return;
		}

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
		case SDLK_F3: // resolution 1920x1080
			SDL_SetWindowSize(m_Window, 1920, 1080);
			SDL_SetWindowPosition(m_Window, (SDL_WINDOWPOS_CENTERED | (0)), (SDL_WINDOWPOS_CENTERED | (0)));
			//SDL_MaximizeWindow(m_Window);
			return;
#ifdef DEBUG
		case SDLK_F4: // Speed up enemies' movement speed
			s_Speedy = !s_Speedy;

			for (const auto &e : g_Enemies)
			{
				static_cast<Enemy*>(e)->SpeedUp();
			}

			s_Logger.AddLog(std::format("Enemies' speed up: {}", s_Speedy));
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

	App::s_CurrentLevel->ManageWaves();

	App::s_Manager.Refresh();
	App::s_Manager.Update();

	if (!s_IsCameraLocked && (s_CameraMovement.moveX != 0.0f || s_CameraMovement.moveY != 0.0f))
	{
		s_Camera.x += s_CameraMovement.moveX;
		s_Camera.y += s_CameraMovement.moveY;

		UpdateCamera();
	}
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

	SDL_RenderPresent(App::s_Renderer);
}

void App::DrawUI()
{
#ifdef DEBUG
	s_EnemiesAmountLabel->Draw();
#endif
	m_PauseLabel->Draw();

	s_UIWaves.Draw();

	s_UICoins.Draw();
	TextureManager::DrawTexture(UIElement::s_CoinTexture, UIElement::coinRect, UIElement::coinDestRect);

	s_UICoinsNotification.Draw();
	
	s_UILifes.Draw();
	TextureManager::DrawTexture(UIElement::s_HeartTexture, UIElement::heartRect, UIElement::heartDestRect);
}

void App::UpdateCamera()
{
	if (s_Camera.x < 0.0f)
	{
		s_Camera.x = 0.0f;
		s_CameraMovement.moveX = 0.0f;
	}
	else if (s_Camera.x > s_CurrentLevel->m_MapData.at(3) - s_Camera.w)
	{
		s_Camera.x = s_CurrentLevel->m_MapData.at(3) - s_Camera.w;
		s_CameraMovement.moveX = 0.0f;
	}

	if (s_Camera.y < 0.0f)
	{
		s_Camera.y = 0.0f;
		s_CameraMovement.moveY = 0.0f;
	}
	else if (s_Camera.y > s_CurrentLevel->m_MapData.at(4) - s_Camera.h)
	{
		s_Camera.y = s_CurrentLevel->m_MapData.at(4) - s_Camera.h;
		s_CameraMovement.moveY = 0.0f;
	}

	s_CurrentLevel->OnUpdateCamera();
}

void App::OnResolutionChange()
{
	SDL_GetRendererOutputSize(s_Renderer, &WINDOW_WIDTH, &WINDOW_HEIGHT);

	m_PauseLabel->UpdatePos({ m_PauseLabel->GetPos().x + ((float)App::WINDOW_WIDTH - s_Camera.w), 10 });

	s_Camera.w = (float)App::WINDOW_WIDTH;
	s_Camera.h = (float)App::WINDOW_HEIGHT;

	s_CameraMovement.rangeW = WINDOW_WIDTH / 6;
	s_CameraMovement.rangeH = WINDOW_HEIGHT / 6;

	if (s_IsCameraLocked || (s_CameraMovement.moveX == 0.0f && s_CameraMovement.moveY == 0.0f))
	{
		const Vector2D &basePos = s_CurrentLevel->GetBase()->m_Pos;

		s_Camera.x = basePos.x - s_Camera.w / 2.0f;
		s_Camera.y = basePos.y - s_Camera.h / 2.0f;
	}

	s_MainMenu.OnResolutionChange();

	UpdateCamera();
}

void App::LoadLevel()
{
	std::string path;
	std::ifstream mapFile;
	for (uint16_t i = 0; i < Level::s_LayersAmount; i++)
	{
		path = "levels\\" + std::to_string(s_CurrentLevel->GetID() + 1) + "\\map_layer" + std::to_string(i) + ".map";
		mapFile = std::ifstream(path);

		s_CurrentLevel->Setup(mapFile, i);
	}

	s_CurrentLevel->SetupBase((uint32_t)s_CurrentLevel->m_BasePos.x, (uint32_t)s_CurrentLevel->m_BasePos.y);

	App::Instance().SetCoins(5);

	const Vector2D &basePos = s_CurrentLevel->GetBase()->m_Pos;

	s_Camera.x = basePos.x - s_Camera.w / 2.0f;
	s_Camera.y = basePos.y - s_Camera.h / 2.0f;

	s_Logger.AddLog("Loaded level " + std::to_string(s_CurrentLevel->GetID() + 1));
}

void App::SwitchBuildingState()
{
	if (s_UIState == UIState::building)
	{
		s_Building.buildingPlace->SetTexture(s_Textures.GetTexture("transparent"));
		SetUIState(UIState::none);
	}
	else if (s_UIState == UIState::none)
	{
		s_Building.buildingPlace->SetTexture(s_Building.originalTexture);
		SetUIState(UIState::building);
	}
	else
	{
		return;
	}
}

void App::ManageBuildingState()
{
	s_Building.coordinates.x = std::floorf((App::s_Camera.x / s_CurrentLevel->m_ScaledTileSize) + (float)s_MouseX / s_CurrentLevel->m_ScaledTileSize);
	s_Building.coordinates.y = std::floorf((App::s_Camera.y / s_CurrentLevel->m_ScaledTileSize) + (float)s_MouseY / s_CurrentLevel->m_ScaledTileSize);

	s_Building.pointedTile = App::s_CurrentLevel->GetTileFrom(s_Building.coordinates.x, s_Building.coordinates.y, 0);
	if (!s_Building.pointedTile)
		return;

	s_Building.buildingPlace->SetPos(s_Building.pointedTile->GetPos());
	s_Building.originalTexture = s_Textures.GetTexture("canBuild");
	s_Building.buildingPlace->SetTexture(s_Building.originalTexture);
	s_Building.canBuild = true;
	s_Building.towerToUpgrade = nullptr;
	
	s_Building.buildingPlace->AdjustToView();

	// pointedTile refers to one of four tiles pointed by building tile (basically by a mouse and 3 more tiles in the building tile's range)
	Tile *pointedTile = s_Building.pointedTile;

	// Show to player the tower can be upgraded, but tower can be upgraded only if it's pointing the first tile of Tower to avoid confusion
	{
		Tower *tower = pointedTile->GetTowerOccupying();
		if (tower && tower->GetTier() < 3 && pointedTile == tower->GetOccupiedTile(0))
		{
			s_Building.originalTexture = s_Textures.GetTexture("upgradeTower");
			s_Building.buildingPlace->SetTexture(s_Building.originalTexture);
			s_Building.canBuild = false;
			s_Building.towerToUpgrade = tower;
			return;
		}
	}

	for (auto i = 0; i < 4; i++)
	{
		pointedTile = App::s_CurrentLevel->GetTileFrom((uint32_t)s_Building.coordinates.x + i % 2, (uint32_t)s_Building.coordinates.y + i / 2, 0);
		if (!pointedTile || !pointedTile->GetTowerOccupying() && s_CurrentLevel->GetBase()->m_Tile != pointedTile)
			continue;

		s_Building.originalTexture = s_Textures.GetTexture("cantBuild");
		s_Building.buildingPlace->SetTexture(s_Building.originalTexture);
		s_Building.canBuild = false;
		return;
	}
}

void App::ManageCamera()
{
	if (s_MouseX <= int32_t(s_CameraMovement.rangeW))
	{
		s_CameraMovement.moveX = -330.0f * s_ElapsedTime;
	}
	else if (s_MouseX >= int32_t(s_Camera.w - s_CameraMovement.rangeW))
	{
		s_CameraMovement.moveX = 330.0f * s_ElapsedTime;
	}
	else
	{
		s_CameraMovement.moveX = 0.0f;
	}

	if (s_MouseY <= int32_t(s_CameraMovement.rangeH))
	{
		s_CameraMovement.moveY = -330.0f * s_ElapsedTime;
	}
	else if (s_MouseY >= int32_t(s_Camera.h - s_CameraMovement.rangeH))
	{
		s_CameraMovement.moveY = 330.0f * s_ElapsedTime;
	}
	else
	{
		s_CameraMovement.moveY = 0.0f;
	}
}

uint16_t App::GetDamageOf(ProjectileType type)
{
	uint16_t minDmg = 0, maxDmg = 0;

	switch (type)
	{
		case ProjectileType::arrow:
		{
			minDmg = 17;
			maxDmg = 30;
		}
	}

	static std::uniform_int_distribution<uint16_t> dmg(minDmg, maxDmg);
	return dmg(g_Rng);
}