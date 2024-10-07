#include "app.h"

#include <fstream>
#include <cmath>

constexpr uint16_t levelsToLoad = 1;

// class App STATIC VARIABLES
int32_t App::WINDOW_WIDTH = 800;
int32_t App::WINDOW_HEIGHT = 600;

TextureManager App::s_Textures;
Logger App::s_Logger;
Manager App::s_Manager;

SDL_Renderer* App::s_Renderer = nullptr;
SDL_Event App::s_Event;
SDL_FRect App::s_Camera { 0.0f, 0.0f, (float)App::WINDOW_WIDTH, (float)App::WINDOW_HEIGHT };

Level *App::s_CurrentLevel = nullptr;

uint16_t App::s_TowerRange = 3;

float App::s_ElapsedTime = NULL;

UIState App::s_UIState = UIState::none;

int32_t App::s_MouseX = 0;
int32_t App::s_MouseY = 0;

BuildingState App::s_Building;

std::random_device App::s_Rnd;

Label *App::s_EnemiesAmountLabel = nullptr;
// END

std::default_random_engine rng(App::s_Rnd());

auto& projectiles = App::s_Manager.GetGroup(EntityGroup::projectile);
auto& labels = App::s_Manager.GetGroup(EntityGroup::label);
auto& towers = App::s_Manager.GetGroup(EntityGroup::tower);
auto& attackers = App::s_Manager.GetGroup(EntityGroup::attacker);
auto& enemies = App::s_Manager.GetGroup(EntityGroup::enemy);

App::App()
{
	bool initialized = true;

	m_Window = SDL_CreateWindow("Tower Defense", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, App::WINDOW_WIDTH, App::WINDOW_HEIGHT, SDL_WINDOW_RESIZABLE);
	if (!m_Window)
	{
		App::s_Logger.AddLog(SDL_GetError());
		initialized = false;
	}

	SDL_Surface* iconSurface = IMG_Load("assets\\gugu.png");
	SDL_SetWindowIcon(m_Window, iconSurface);
	SDL_FreeSurface(iconSurface);

	App::s_Renderer = SDL_CreateRenderer(m_Window, -1, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED);
	if (!App::s_Renderer)
	{
		App::s_Logger.AddLog(SDL_GetError());
		initialized = false;
	}

	SDL_GetRendererOutputSize(App::s_Renderer, &WINDOW_WIDTH, &WINDOW_HEIGHT);
	SDL_SetRenderDrawColor(App::s_Renderer, 50, 50, 200, 255);

	App::s_Textures.AddTexture("mapSheet", "assets\\tileset.png");

	App::s_Textures.AddTexture("canBuild", "assets\\tile_CanBuild.png");
	App::s_Textures.AddTexture("cantBuild", "assets\\tile_CantBuild.png");
	App::s_Textures.AddTexture("upgradeTower", "assets\\tile_Upgrade.png");
	App::s_Textures.AddTexture("base", "assets\\base.png");
	App::s_Textures.AddTexture("tower", "assets\\towers\\tower.png");
	App::s_Textures.AddTexture("square", "assets\\square_32x32.png");
	App::s_Textures.AddTexture("green", "assets\\green_32x32.png");
	App::s_Textures.AddTexture("transparent", "assets\\transparent.png");
	App::s_Textures.AddTexture(TextureOf(ProjectileType::arrow), "assets\\arrow_16x16.png");
	App::s_Textures.AddTexture(TextureOf(AttackerType::archer), "assets\\entities\\friendly\\attackerArcher.png");
	App::s_Textures.AddTexture(TextureOf(AttackerType::hunter), "assets\\entities\\friendly\\attackerHunter.png");
	App::s_Textures.AddTexture(TextureOf(AttackerType::musketeer), "assets\\entities\\friendly\\attackerMusketeer.png");
	App::s_Textures.AddTexture(TextureOf(EnemyType::elf), "assets\\entities\\enemy\\enemyElf.png");
	App::s_Textures.AddTexture(TextureOf(EnemyType::goblinWarrior), "assets\\entities\\enemy\\enemyGoblinWarrior.png");
	App::s_Textures.AddTexture(TextureOf(EnemyType::dwarfSoldier), "assets\\entities\\enemy\\enemyDwarfSoldier.png");
	App::s_Textures.AddTexture(TextureOf(EnemyType::dwarfKing), "assets\\entities\\enemy\\enemyDwarfKing.png");

	App::s_Textures.AddFont("default", "assets\\F25_Bank_Printer.ttf", 15);
	App::s_Textures.AddFont("hpBar", "assets\\Rostack.otf", 13);

	levels.reserve(levelsToLoad);

	for (uint16_t i = 0; i < levelsToLoad; i++)
	{
		levels.emplace_back(std::move(std::make_unique<Level>(i)));
	}

	App::s_CurrentLevel = levels.at(0).get();

	if (!App::s_CurrentLevel || App::s_CurrentLevel->DidLoadingFail())
	{
		initialized = false;
		App::s_Logger.AddLog("Beginner level couldn't be loaded properly.");
	}
	else
	{
		LoadLevel((uint32_t)App::s_CurrentLevel->m_BasePos.x, (uint32_t)App::s_CurrentLevel->m_BasePos.y);

		auto newLabel = App::s_Manager.NewEntity<Label>(4, 2, "pos", App::s_Textures.GetFont("default"));
		newLabel->AddGroup(EntityGroup::label);

		Base* base = App::s_CurrentLevel->GetBase();

		base->m_AttachedLabel = newLabel;
		newLabel->UpdateText("(" + std::to_string(base->m_Pos.x) + ", " + std::to_string(base->m_Pos.y) + ")");
	}

	s_Building.buildingPlace = App::s_Manager.NewEntity<Tile>(TileTypes::special, 2);
	s_Building.originalTexture = s_Textures.GetTexture("canBuild");
	s_Building.buildingPlace->SetTexture(s_Textures.GetTexture("transparent"));

	UpdateCamera();

	m_PauseLabel = App::s_Manager.NewEntity<Label>(int32_t(App::s_Camera.w) - 10, 10, "PAUSED", App::s_Textures.GetFont("default"));
	//m_PauseLabel->m_Drawable = false;
	m_PauseLabel->AddGroup(EntityGroup::label);

	SDL_Rect pauseLabelRect = m_PauseLabel->GetRect();
	m_PauseLabel->UpdatePos(pauseLabelRect.x - pauseLabelRect.w, pauseLabelRect.y);
	m_PauseLabel->UpdateText(" ");

	s_EnemiesAmountLabel = App::s_Manager.NewEntity<Label>(10, 100, " ", App::s_Textures.GetFont("default"));
	s_EnemiesAmountLabel->AddGroup(EntityGroup::label);

	m_IsRunning = initialized;
}

App::~App()
{
	s_Manager.DestroyAllEntities();
	levels.clear();

	if (App::s_Renderer)
		SDL_DestroyRenderer(App::s_Renderer);

	if (m_Window)
		SDL_DestroyWindow(m_Window);

	// TTF_Quit() is called in ~TextureManager()
	SDL_Quit();
}

void App::EventHandler()
{
	SDL_PollEvent(&App::s_Event);

	switch (App::s_Event.type)
	{
	case SDL_WINDOWEVENT:
		if (App::s_Event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
		{
			OnResolutionChange();
		}
		return;
	case SDL_QUIT:
		m_IsRunning = false;
		return;
	case SDL_MOUSEMOTION:
		s_MouseX = App::s_Event.motion.x;
		s_MouseY = App::s_Event.motion.y;
		ManageBuildingState();
		return;
	case SDL_MOUSEBUTTONUP:
	case SDL_MOUSEBUTTONDOWN:
		App::s_CurrentLevel->HandleMouseButtonEvent();
		return;
	case SDL_KEYDOWN:
		switch (App::s_Event.key.keysym.sym)
		{
		// Function keys
		case SDLK_F1:
			SDL_SetWindowSize(m_Window, 800, 600);
			SDL_SetWindowPosition(m_Window, (SDL_WINDOWPOS_CENTERED | (0)), (SDL_WINDOWPOS_CENTERED | (0)));
			return;
		case SDLK_F2:
			SDL_SetWindowSize(m_Window, 1024, 768);
			SDL_SetWindowPosition(m_Window, (SDL_WINDOWPOS_CENTERED | (0)), (SDL_WINDOWPOS_CENTERED | (0)));
			return;
		case SDLK_F3:
			SDL_SetWindowSize(m_Window, 1280, 720);
			SDL_SetWindowPosition(m_Window, (SDL_WINDOWPOS_CENTERED | (0)), (SDL_WINDOWPOS_CENTERED | (0)));
			return;
		case SDLK_F4:
			SDL_SetWindowSize(m_Window, 1920, 1080);
			SDL_SetWindowPosition(m_Window, (SDL_WINDOWPOS_CENTERED | (0)), (SDL_WINDOWPOS_CENTERED | (0)));
			return;
		case SDLK_F6:
			s_Building.buildingPlace->SetTexture(s_Building.originalTexture);
			SetUIState(UIState::building);
			return;
		case SDLK_F7:
			s_Building.buildingPlace->SetTexture(s_Textures.GetTexture("transparent"));
			SetUIState(UIState::none);
			return;
		case SDLK_F8:
			for (const auto& e : enemies)
				e->Destroy();
			return;
		case SDLK_F10:
			if (towers.empty())
				return;
			m_DestroyTower = true;
			return;
		case SDLK_F11:
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
			m_IsRunning = false;
			return;
		default:
			return;
		}
		return;
	default:
		return;
	}
}

void App::Update()
{
	if (IsGamePaused())
		return;

	// NOTE: The only case where m_DestroyTower can be modified is EventHandler case SDLK_F10
	// This code should be removed after removing SDLK_F10 from EventHandler
	if (m_DestroyTower)
	{
		static_cast<Tower*>(towers.back())->Destroy();
		m_DestroyTower = false;
	}

	App::s_CurrentLevel->ManageWaves();

	App::s_Manager.Refresh();
	App::s_Manager.Update();
}

void App::Render()
{
	SDL_RenderClear(App::s_Renderer);

	App::s_CurrentLevel->Render();

	for (const auto &label : labels)
	{
		label->Draw();
	}

	SDL_RenderPresent(App::s_Renderer);
}

void App::UpdateCamera()
{
	Vector2D basePos = App::s_CurrentLevel->GetBase()->m_Pos;
	float calculatedMapSizeX = float(App::s_CurrentLevel->m_MapSizeX * App::s_CurrentLevel->m_ScaledTileSize);
	float calculatedMapSizeY = float(App::s_CurrentLevel->m_MapSizeY * App::s_CurrentLevel->m_ScaledTileSize);

	App::s_Camera.x = basePos.x - App::s_Camera.w / 2.0f;
	App::s_Camera.y = basePos.y - App::s_Camera.h / 2.0f;

	if (App::s_Camera.x < 0)
	{
		App::s_Camera.x = 0;
	}
	else if (App::s_Camera.x > calculatedMapSizeX - App::s_Camera.w)
	{
		App::s_Camera.x = calculatedMapSizeX - App::s_Camera.w;
	}

	if (App::s_Camera.y < 0)
	{
		App::s_Camera.y = 0;
	}
	else if (App::s_Camera.y > calculatedMapSizeY - App::s_Camera.h)
	{
		App::s_Camera.y = calculatedMapSizeY - App::s_Camera.h;
	}

	App::s_CurrentLevel->OnUpdateCamera();
}

void App::OnResolutionChange()
{
	SDL_GetRendererOutputSize(App::s_Renderer, &WINDOW_WIDTH, &WINDOW_HEIGHT);

	m_PauseLabel->UpdatePos({ m_PauseLabel->GetPos().x + ((float)App::WINDOW_WIDTH - App::s_Camera.w), 10 });

	App::s_Camera.w = (float)App::WINDOW_WIDTH;
	App::s_Camera.h = (float)App::WINDOW_HEIGHT;

	UpdateCamera();
}

void App::LoadLevel(uint32_t baseX, uint32_t baseY)
{
	std::string path;
	std::ifstream mapFile;
	for (uint16_t i = 0; i < 3; i++)
	{
		path = "levels\\" + std::to_string(App::s_CurrentLevel->GetID() + 1) + "\\map_layer" + std::to_string(i) + ".map";
		mapFile = std::ifstream(path);

		App::s_CurrentLevel->Setup(mapFile, i);
	}

	App::s_CurrentLevel->SetupBase(baseX, baseY);

	App::s_Logger.AddLog("Loaded level " + std::to_string(App::s_CurrentLevel->GetID() + 1));
}

void App::ManageBuildingState()
{
	if (s_UIState != UIState::building)
		return;

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

	// 0: 0, 0
	// x: 0 % 2 y: 0 / 2
	// 1: 1, 0,5
	// x: 1 % 2 y: 1 / 2
	// 2: 0, 1
	// x: 2 % 2 y: 2 / 2
	// 3: 1, 1,5
	// x: 3 % 2 y: 3 / 2

	for (auto i = 0u; i < 3; i++)
	{
		pointedTile = App::s_CurrentLevel->GetTileFrom((uint32_t)s_Building.coordinates.x + i % 2, (uint32_t)s_Building.coordinates.y + i / 2, 0);
		if (!pointedTile || !pointedTile->GetTowerOccupying())
			continue;

		s_Building.originalTexture = s_Textures.GetTexture("cantBuild");
		s_Building.buildingPlace->SetTexture(s_Building.originalTexture);
		s_Building.canBuild = false;
		return;
	}
}

uint16_t App::GetDamageOf(ProjectileType type)
{
	//static std::default_random_engine rng(App::s_Rnd());
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
	return dmg(rng);
}