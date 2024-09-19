#include "app.h"

#include <fstream>

// class App STATIC VARIABLES
int32_t App::WINDOW_WIDTH = 800;
int32_t App::WINDOW_HEIGHT = 600;

std::unique_ptr<TextureManager> App::s_Textures = std::make_unique<TextureManager>();
std::unique_ptr<Logger> App::s_Logger = std::make_unique<Logger>();
std::unique_ptr<Manager> App::s_Manager = std::make_unique<Manager>();

SDL_Renderer* App::s_Renderer = nullptr;
SDL_Event App::s_Event;
SDL_FRect App::s_Camera { 0.0f, 0.0f, (float)App::WINDOW_WIDTH, (float)App::WINDOW_HEIGHT };

Level* App::s_CurrentLevel = nullptr;

uint16_t App::s_TowerRange = 2;

float App::s_ElapsedTime = NULL;

UIState App::s_UIState = UIState::none;

Tile* App::s_BuildingPlace = nullptr;
// END

auto& projectiles = App::s_Manager->GetGroup(EntityGroup::projectile);
auto& labels = App::s_Manager->GetGroup(EntityGroup::label);

App::App()
{
	bool initialized = true;

	m_Window = SDL_CreateWindow("Tower Defense", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, App::WINDOW_WIDTH, App::WINDOW_HEIGHT, 0);
	if (!m_Window)
	{
		App::s_Logger->AddLog(SDL_GetError());
		initialized = false;
	}

	SDL_Surface* iconSurface = IMG_Load("assets\\gugu.png");
	SDL_SetWindowIcon(m_Window, iconSurface);
	SDL_FreeSurface(iconSurface);

	App::s_Renderer = SDL_CreateRenderer(m_Window, -1, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED);
	if (!App::s_Renderer)
	{
		App::s_Logger->AddLog(SDL_GetError());
		initialized = false;
	}

	SDL_SetRenderDrawColor(App::s_Renderer, 50, 50, 200, 255);

	App::s_Textures->AddTexture("mapSheet", "assets\\tileset.png");

	App::s_Textures->AddTexture("canBuild", "assets\\tile_CanBuild.png");
	App::s_Textures->AddTexture("cantBuild", "assets\\tile_CantBuild.png");
	App::s_Textures->AddTexture("base", "assets\\base.png");
	App::s_Textures->AddTexture("tower", "assets\\towers\\tower.png");
	App::s_Textures->AddTexture("square", "assets\\square_32x32.png");
	App::s_Textures->AddTexture("green", "assets\\green_32x32.png");
	App::s_Textures->AddTexture(TextureOf(ProjectileType::arrow), "assets\\arrow_16x16.png");
	App::s_Textures->AddTexture(TextureOf(AttackerType::archer), "assets\\entities\\friendly\\attackerArcher.png");
	App::s_Textures->AddTexture(TextureOf(EnemyType::elf), "assets\\entities\\enemy\\enemyElf.png");

	App::s_Textures->AddFont("default", "assets\\F25_Bank_Printer.ttf", 15);
	App::s_Textures->AddFont("hpBar", "assets\\Rostack.otf", 13);

	constexpr uint16_t levelsToLoad = 1;
	levels.reserve(levelsToLoad);

	for (uint16_t i = 0; i < levelsToLoad; i++)
	{
		levels.emplace_back(std::move(std::make_unique<Level>()));
		levels.at(i)->SetID(i);
	}

	App::s_CurrentLevel = levels.at(0).get();

	if (!App::s_CurrentLevel || App::s_CurrentLevel->DidLoadingFail())
	{
		initialized = false;
		App::s_Logger->AddLog("Beginner level couldn't be loaded properly.");
	}
	else
	{
		LoadLevel((uint32_t)App::s_CurrentLevel->m_BasePos.x, (uint32_t)App::s_CurrentLevel->m_BasePos.y);

		auto newLabel = App::s_Manager->NewEntity<Label>(4, 2, "pos", App::s_Textures->GetFont("default"));
		newLabel->AddGroup(EntityGroup::label);

		Tile* base = App::s_CurrentLevel->GetBase();

		base->AttachLabel(newLabel);
	}

	s_BuildingPlace = App::s_Manager->NewEntity<Tile>(TileTypes::special, 2);
	s_BuildingPlace->SetTexture(App::s_Textures->GetTexture("canBuild"));

	UpdateCamera();

	m_IsRunning = initialized;
}

App::~App()
{
	if (App::s_Renderer)
		SDL_DestroyRenderer(App::s_Renderer);

	if (m_Window)
		SDL_DestroyWindow(m_Window);

	SDL_Quit();
}

void App::EventHandler()
{
	SDL_PollEvent(&App::s_Event);

	switch (App::s_Event.type)
	{
	case SDL_QUIT:
		m_IsRunning = false;
		break;
	case SDL_MOUSEMOTION:
		ManageBuildingState();
		break;
	case SDL_MOUSEBUTTONUP:
	case SDL_MOUSEBUTTONDOWN:
		App::s_CurrentLevel->HandleMouseButtonEvent();
		break;
	case SDL_KEYDOWN:
		switch (App::s_Event.key.keysym.sym)
		{
		// for testing, base is not supposed to move
		case SDLK_w:
			App::s_CurrentLevel->GetBase()->Move(0.0f, -1.0f);
			break;
		case SDLK_a:
			App::s_CurrentLevel->GetBase()->Move(-1.0f, 0.0f);
			break;
		case SDLK_s:
			App::s_CurrentLevel->GetBase()->Move(0.0f, 1.0f);
			break;
		case SDLK_d:
			App::s_CurrentLevel->GetBase()->Move(1.0f, 0.0f);
			break;

		case SDLK_LEFT:
			App::s_CurrentLevel->GetEnemy()->Move(-1.0f, 0.0f);
			break;

		// Function keys
		case SDLK_F1:
			SDL_SetWindowSize(m_Window, 800, 600);
			SDL_SetWindowPosition(m_Window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
			OnResolutionChange();
			break;
		case SDLK_F2:
			SDL_SetWindowSize(m_Window, 1024, 768);
			SDL_SetWindowPosition(m_Window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
			OnResolutionChange();
			break;
		case SDLK_F3:
			SDL_SetWindowSize(m_Window, 1280, 720);
			SDL_SetWindowPosition(m_Window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
			OnResolutionChange();
			break;
		case SDLK_F4:
			SDL_SetWindowSize(m_Window, 1920, 1080);
			SDL_SetWindowPosition(m_Window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
			OnResolutionChange();
			break;
		case SDLK_F5:
			{
				Enemy* enemy = App::s_CurrentLevel->GetEnemy();
				if (enemy)
				{
					enemy->Destroy();
				}
			}
			break;
		case SDLK_F6:
			s_UIState = UIState::building;
			break;
		case SDLK_F7:
			s_UIState = UIState::none;
			break;
		case SDLK_F11:
			if (m_IsFullscreen)
			{
				SDL_SetWindowFullscreen(m_Window, 0);
				SDL_SetWindowPosition(m_Window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
			}
			else
			{
				SDL_SetWindowFullscreen(m_Window, SDL_WINDOW_FULLSCREEN);
			}
			m_IsFullscreen = !m_IsFullscreen;
			break;
		// End of function keys

		case SDLK_ESCAPE:
			m_IsRunning = false;
			break;
		default:
			break;
		}
		break;
	default:
		break;
	}
}

void App::Update(float fElapsedTime) const
{
	App::s_ElapsedTime = fElapsedTime;

	if (IsGamePaused())
		return;

	Label* label = App::s_CurrentLevel->GetBase()->GetAttachedLabel();
	
	if (label)
		label->UpdateText("(" + std::to_string(App::s_CurrentLevel->GetBase()->GetPos().x) + ", " + std::to_string(App::s_CurrentLevel->GetBase()->GetPos().y) + ")");

	App::s_CurrentLevel->ManageWaves();

	App::s_Manager->Refresh();
	App::s_Manager->Update();
}

void App::Render()
{
	SDL_RenderClear(App::s_Renderer);

	App::s_CurrentLevel->Render();

	for (const auto& label : labels)
	{
		label->Draw();
	}

	SDL_RenderPresent(App::s_Renderer);
}

void App::UpdateCamera()
{
	Vector2D basePos = App::s_CurrentLevel->GetBase()->GetPos();
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
	App::s_Camera.w = (float)App::WINDOW_WIDTH;
	App::s_Camera.h = (float)App::WINDOW_HEIGHT;
	UpdateCamera();
}

void App::LoadLevel(uint32_t baseX, uint32_t baseY)
{
	uint16_t i = 0;
	std::string path = "levels\\" + std::to_string(App::s_CurrentLevel->GetID() + 1) + "\\map_layer0.map";
	std::ifstream mapFile(path);

	do
	{
		App::s_CurrentLevel->Setup(mapFile);
		i++;
		path = "levels\\" + std::to_string(App::s_CurrentLevel->GetID() + 1) + "\\map_layer" + std::to_string(i) + ".map";
		mapFile = std::ifstream(path);
	} while (!mapFile.fail());

	App::s_CurrentLevel->SetupBase(baseX, baseY);

	App::s_Logger->AddLog("Loaded level " + std::to_string(App::s_CurrentLevel->GetID() + 1));
}

void App::ManageBuildingState()
{
	int32_t mouseX = App::s_Event.motion.x;
	int32_t mouseY = App::s_Event.motion.y;

	Vector2D coordinates;
	coordinates.x = std::floorf((App::s_Camera.x / s_CurrentLevel->m_ScaledTileSize) + (float)mouseX / s_CurrentLevel->m_ScaledTileSize);
	coordinates.y = std::floorf((App::s_Camera.y / s_CurrentLevel->m_ScaledTileSize) + (float)mouseY / s_CurrentLevel->m_ScaledTileSize);

	Tile* pointedTile = App::s_CurrentLevel->GetTileFrom(coordinates.x, coordinates.y, 0);
	if (!pointedTile)
		return;

	s_BuildingPlace->SetPos(pointedTile->GetPos());
	s_BuildingPlace->AdjustToView(); // in this case this function works like Update()

	if (s_UIState != UIState::building)
		return;

	s_BuildingPlace->Draw();
}