#include "app.h"

#include <typeinfo>
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
// END

auto& tiles = App::s_Manager->GetGroup(EntityGroup::tile);
auto& enemies = App::s_Manager->GetGroup(EntityGroup::enemy);
auto& towers = App::s_Manager->GetGroup(EntityGroup::tower);

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
	App::s_Textures->AddTexture("attackerArcher", "assets\\entities\\attackerArcher.png");

	constexpr uint16_t levelsToLoad = 1;
	levels.reserve(levelsToLoad);

	for (uint16_t i = 0; i < levelsToLoad; i++)
	{
		levels.emplace_back(std::move(std::make_unique<Level>()));
		levels.at(i)->SetID(i);
	}

	App::s_CurrentLevel = levels.at(0).get();
	LoadLevel(7, 7);

	auto newTower = App::s_CurrentLevel->AddTower(5.0f, 5.0f, App::s_Textures->GetTexture("tower"), 1);
	App::s_CurrentLevel->AddAttacker(newTower, AttackerType::archer, 2);

	newTower = App::s_CurrentLevel->AddTower(3.0f, 3.0f, App::s_Textures->GetTexture("tower"), 2);
	App::s_CurrentLevel->AddAttacker(newTower, AttackerType::archer, 2);

	newTower = App::s_CurrentLevel->AddTower(1.0f, 1.0f, App::s_Textures->GetTexture("tower"), 3);
	App::s_CurrentLevel->AddAttacker(newTower, AttackerType::archer, 2);

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

void App::Update()
{
	App::s_Manager->Refresh();
	App::s_Manager->Update();
}

void App::Render()
{
	SDL_RenderClear(App::s_Renderer);

	/*if (!App::s_CurrentLevel)
	{
		App::s_Logger->AddLog("Tried to render not existing level");
		return;
	}

	if (App::s_CurrentLevel->DidLoadingFail())
	{
		App::s_Logger->AddLog("Level " + std::to_string(App::s_CurrentLevel->GetID() + 1) + " failed to load");
		return;
	}*/

	App::s_CurrentLevel->Render();

	/*for (const auto& t : towers)
	{
		t->Draw();
	}

	for (const auto& e : enemies)
	{
		e->Draw();
	}*/

	SDL_RenderPresent(App::s_Renderer);
}

void App::UpdateCamera()
{
	// to fix:
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

// probably ready to remove, don't need this
Tile* App::AddTile(int srcX, int srcY, int posX, int posY, int tileSize, int tileScale, std::string_view textureID)
{
	Tile* tile = App::s_Manager->NewEntity<Tile>(srcX, srcY, posX, posY, tileSize, tileScale, textureID);
	tile->AddGroup(EntityGroup::tile);
	return tile;
}