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

int32_t App::s_TowerRange = 2;

float App::s_ElapsedTime = NULL;
// END

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

	App::s_Renderer = SDL_CreateRenderer(m_Window, -1, SDL_RENDERER_PRESENTVSYNC);
	if (!App::s_Renderer)
	{
		App::s_Logger->AddLog(SDL_GetError());
		initialized = false;
	}

	SDL_SetRenderDrawColor(App::s_Renderer, 50, 50, 200, 255);

	App::s_Textures->AddTexture("mapSheet", "assets\\tileset.png");

	App::s_Textures->AddTexture("base", "assets\\base.png");
	App::s_Textures->AddTexture("tower", "assets\\towers\\tower.png");
	App::s_Textures->AddTexture(TextureOf(ProjectileType::arrow), "assets\\arrow_16x16.png");
	App::s_Textures->AddTexture(TextureOf(AttackerType::archer), "assets\\entities\\friendly\\attackerArcher.png");
	App::s_Textures->AddTexture(TextureOf(EnemyType::elf), "assets\\entities\\enemy\\enemyElf.png");

	App::s_Textures->AddFont("default", "assets\\F25_Bank_Printer.ttf", 16);
	//App::s_Textures->AddFont("default", "assets\\ThaleahFat.ttf", 20);

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
		LoadLevel(7, 7);

		auto newLabel = App::s_Manager->NewEntity<Label>(4, 2, "dupa", App::s_Textures->GetFont("default"));
		newLabel->AddGroup(EntityGroup::label);

		Tile* base = App::s_CurrentLevel->GetBase();

		base->AttachLabel(newLabel);
	}


	auto newTower = App::s_CurrentLevel->AddTower(5.0f, 5.0f, App::s_Textures->GetTexture("tower"), 1);
	App::s_CurrentLevel->AddAttacker(newTower, AttackerType::archer, 2);

	newTower = App::s_CurrentLevel->AddTower(3.0f, 3.0f, App::s_Textures->GetTexture("tower"), 2);
	App::s_CurrentLevel->AddAttacker(newTower, AttackerType::archer, 2);

	newTower = App::s_CurrentLevel->AddTower(1.0f, 1.0f, App::s_Textures->GetTexture("tower"), 3);
	App::s_CurrentLevel->AddAttacker(newTower, AttackerType::archer, 2);

	App::s_CurrentLevel->AddEnemy(10.0f, 10.0f, EnemyType::elf, App::s_Textures->GetTexture(TextureOf(EnemyType::elf)), 2);
	App::s_CurrentLevel->AddEnemy(11.0f, 11.0f, EnemyType::elf, App::s_Textures->GetTexture(TextureOf(EnemyType::elf)), 2);

	UpdateCamera();

	m_IsRunning = initialized;
}

App::~App()
{
	SDL_DestroyRenderer(App::s_Renderer);
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

		case SDLK_UP:
			{
				Enemy* enemy = App::s_CurrentLevel->GetEnemy();
				enemy->Move(Vector2D(0.0f, -1.0f));

				/*std::vector<std::vector<Tile*>> chunk = s_CurrentLevel->GetChunkOf(enemy, 2);

				for (const auto& row : chunk)
				{
					for (const auto& tile : row)
					{
						if (tile)
							s_Logger->AddLog("(" + std::to_string(tile->GetPos().x) + ", " + std::to_string(tile->GetPos().y) + ")");
						else
							s_Logger->AddLog("missing tile");
					}
				}*/
			}
			break;
		case SDLK_DOWN:
			{
				Enemy* enemy = App::s_CurrentLevel->GetEnemy();
				enemy->Move(Vector2D(0.0f, 1.0f));
			}
			break;
		case SDLK_LEFT:
			{
				Enemy* enemy = App::s_CurrentLevel->GetEnemy();
				enemy->Move(Vector2D(-1.0f, 0.0f));
			}
			break;
		case SDLK_RIGHT:
			{
				Enemy* enemy = App::s_CurrentLevel->GetEnemy();
				enemy->Move(Vector2D(1.0f, 0.0f));
			}
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

void App::Update(float fElapsedTime)
{
	App::s_ElapsedTime = fElapsedTime;

	Label* label = App::s_CurrentLevel->GetBase()->GetAttachedLabel();
	
	if (label)
	{
		label->UpdateText("(" + std::to_string(App::s_CurrentLevel->GetBase()->GetPos().x) + ", " + std::to_string(App::s_CurrentLevel->GetBase()->GetPos().y) + ")");
	}

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