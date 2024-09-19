#include "level.h"
#include "app.h"

#include <fstream>
#include <sstream>

constexpr uint16_t spawnerID = 305;
constexpr uint16_t waveCooldown = 3500; // miliseconds

Level::Level() 
	: towers(App::s_Manager->GetGroup(EntityGroup::tower)), attackers(App::s_Manager->GetGroup(EntityGroup::attacker)), enemies(App::s_Manager->GetGroup(EntityGroup::enemy)),
	projectiles(App::s_Manager->GetGroup(EntityGroup::projectile)),
	m_Wave{ WaveProgress::OnCooldown, 1, 0 }
{}

Level::~Level()
{
	for (const auto& enemy : enemies)
	{
		static_cast<Enemy*>(enemy)->GetOccupiedTile()->SetOccupyingEntity(nullptr);
		static_cast<Enemy*>(enemy)->SetOccupiedTile(nullptr);
		enemy->Destroy();
	}

	for (const auto& tower : towers)
	{
		tower->Destroy();
	}

	m_BaseTile->Destroy();
}

void Level::Setup(std::ifstream& mapFile)
{
	if (mapFile.fail())
	{
		App::s_Logger->AddLog("Failed to load level ", false);
		App::s_Logger->AddLog(std::to_string(m_LevelID + 1));
		return;
	}
	else
	{
		App::s_Logger->AddLog("Loading level ", false);
		App::s_Logger->AddLog(std::to_string(m_LevelID + 1), false);
		App::s_Logger->AddLog(" (Layer: ", false);
		App::s_Logger->AddLog(std::to_string(layers.size()), false);
		App::s_Logger->AddLog(")");
	}

	std::string line;
	std::vector<std::vector<int>> mapData;

	while (std::getline(mapFile, line)) {
		std::istringstream ss(line);
		std::vector<int> row;
		std::string value;

		while (std::getline(ss, value, ',')) {
			row.push_back(std::stoi(value));
		}

		mapData.push_back(row);
	}

	int32_t tileCode;
	TileTypes tileType;

	switch (layers.size())
	{
	case 0:
		tileType = TileTypes::regular;
		break;
	case 1:
		tileType = TileTypes::additional;
		break;
	case 2:
		tileType = TileTypes::spawner;
		break;
	default:
		tileType = TileTypes::regular;
		break;
	}

	layers.reserve(1);

	layers.emplace_back(Layer());
	Layer* newLayer = &layers.back();
	newLayer->tiles.reserve(std::size_t(m_MapSizeX * m_MapSizeY));

	Tile* tile = nullptr;
	uint32_t srcX, srcY;

	for (uint32_t y = 0; y < m_MapSizeY; y++)
	{
		for (uint32_t x = 0; x < m_MapSizeX; x++)
		{
			tileCode = mapData.at(y).at(x);
			srcX = tileCode % 10;
			srcY = tileCode / 10;
			tile = App::s_Manager->NewEntity<Tile>(srcX * m_TileSize, srcY * m_TileSize, x * m_ScaledTileSize, y * m_ScaledTileSize, m_TileSize, m_MapScale, m_TextureID, tileType);
			
			if (tile)
			{
				tile->AddGroup(EntityGroup::tile);

				if (tileCode == spawnerID)
				{
					spawners.push_back(tile);
				}
			}
			else
			{
				m_FailedLoading = true;
				App::s_Logger->AddLog("Couldn't load a tile (", false);
				App::s_Logger->AddLog(std::to_string(x * m_ScaledTileSize), false);
				App::s_Logger->AddLog(", ", false);
				App::s_Logger->AddLog(std::to_string(y * m_ScaledTileSize), false);
				App::s_Logger->AddLog(")");
			}

			newLayer->tiles.emplace_back(tile);
		}
	}

	mapFile.close();
}

//void Level::Setup(std::ifstream& mapFile)
//{
//	if (mapFile.fail())
//	{
//		App::s_Logger->AddLog("Failed to load level ", false);
//		App::s_Logger->AddLog(std::to_string(m_LevelID + 1));
//		return;
//	}
//	else
//	{
//		App::s_Logger->AddLog("Loading level ", false);
//		App::s_Logger->AddLog(std::to_string(m_LevelID + 1), false);
//		App::s_Logger->AddLog(" (Layer: ", false);
//		App::s_Logger->AddLog(std::to_string(layers.size()), false);
//		App::s_Logger->AddLog(")");
//	}
//
//	std::string line;
//	std::vector<std::vector<int>> mapData;
//
//	while (std::getline(mapFile, line)) {
//		std::istringstream ss(line);
//		std::vector<int> row;
//		std::string value;
//
//		while (std::getline(ss, value, ',')) {
//			row.push_back(std::stoi(value));
//		}
//
//		mapData.push_back(row);
//	}
//
//	int32_t tileCode;
//	TileTypes tileType;
//
//	switch (layers.size())
//	{
//	case 0:
//		tileType = TileTypes::regular;
//		break;
//	case 1:
//		tileType = TileTypes::additional;
//		break;
//	case 2:
//		tileType = TileTypes::spawner;
//		break;
//	default:
//		tileType = TileTypes::regular;
//		break;
//	}
//
//	layers.reserve(1);
//
//	Layer newLayer;
//
//	auto& tiles = newLayer.GetTilesVector();
//
//	tiles.reserve(m_MapSizeY);
//
//	for (int32_t y = 0; y < m_MapSizeY; ++y) {
//		std::vector<Tile*> rowOfTiles;
//		rowOfTiles.reserve(m_MapSizeX);
//
//		for (int32_t x = 0; x < m_MapSizeX; ++x) {
//			tileCode = mapData[y][x];
//			int32_t srcX = tileCode % 10;
//			int32_t srcY = tileCode / 10;
//			Tile* tile = App::s_Manager->NewEntity<Tile>(srcX * m_TileSize, srcY * m_TileSize, x * m_ScaledTileSize, y * m_ScaledTileSize, m_TileSize, m_MapScale, m_TextureID, tileType);
//			tile->AddGroup(EntityGroup::tile);
//
//			if (!tile)
//			{
//				tile = nullptr; // probably it's not even needed
//				m_FailedLoading = true;
//				App::s_Logger->AddLog("Couldn't load a tile (", false);
//				App::s_Logger->AddLog(std::to_string(x * m_ScaledTileSize), false);
//				App::s_Logger->AddLog(", ", false);
//				App::s_Logger->AddLog(std::to_string(y * m_ScaledTileSize), false);
//				App::s_Logger->AddLog(")");
//			}
//			else if (tileCode == spawnerID)
//			{
//				spawners.push_back(tile);
//			}
//
//			rowOfTiles.emplace_back(tile);
//		}
//
//		tiles.emplace_back(rowOfTiles);
//	}
//
//	layers.emplace_back(newLayer);
//
//	mapFile.close();
//}

void Level::SetupBase(uint32_t posX, uint32_t posY)
{
	m_BaseTile = App::s_Manager->NewEntity<Tile>(0, 0, posX * m_ScaledTileSize, posY * m_ScaledTileSize, m_TileSize, m_MapScale, m_BaseTextureID);
	m_BaseTile->AddGroup(EntityGroup::tile);

	App::s_Logger->AddLog("Created base (", false);
	App::s_Logger->AddLog(std::to_string(posX * m_ScaledTileSize), false);
	App::s_Logger->AddLog(", ", false);
	App::s_Logger->AddLog(std::to_string(posY * m_ScaledTileSize), false);
	App::s_Logger->AddLog(")");
}

void Level::AddTower(float posX, float posY, SDL_Texture* towerTexture, int32_t tier)
{
	if (!towerTexture)
	{
		App::s_Logger->AddLog("Level::AddTower: Tower's texture doesn't exist!");
		return;
	}

	auto tower = App::s_Manager->NewEntity<Tower>(posX, posY, towerTexture, tier);
	tower->AddGroup(EntityGroup::tower);

	AddAttacker(tower, AttackerType::archer, 2);
}

void Level::AddAttacker(Tower* assignedTower, AttackerType type, uint16_t scale)
{
	if (!assignedTower || assignedTower->GetAttacker())
	{
		App::s_Logger->AddLog("Tried to add attacker to not existing tower or an attacker for the specific tower already exists.");
		return;
	}

	auto attacker = App::s_Manager->NewEntity<Attacker>(assignedTower, type, App::s_Textures->GetTexture(App::TextureOf(type)), scale);
	attacker->AddGroup(EntityGroup::attacker);
	assignedTower->AssignAttacker(attacker);
}

Enemy* Level::AddEnemy(float posX, float posY, EnemyType type, SDL_Texture* texture, uint16_t scale)
{
	auto enemy = App::s_Manager->NewEntity<Enemy>(posX, posY, type, texture, scale);
	enemy->AddGroup(EntityGroup::enemy);
	return enemy;
}

void Level::HandleMouseButtonEvent()
{
	int32_t mouseX, mouseY;

	mouseX = App::s_Event.button.x;
	mouseY = App::s_Event.button.y;
	
	if (App::s_Event.button.type == SDL_MOUSEBUTTONDOWN)
	{
		Vector2D coordinates;
		coordinates.x = std::floorf((App::s_Camera.x / m_ScaledTileSize) + (float)mouseX / m_ScaledTileSize);
		coordinates.y = std::floorf((App::s_Camera.y / m_ScaledTileSize) + (float)mouseY / m_ScaledTileSize);

		Tile* tile = GetTileFrom(coordinates.x, coordinates.y, 1);
		if (tile)
		{
			AddTower(coordinates.x, coordinates.y, App::s_Textures->GetTexture("tower"), 1);
		}
	}
	else if (App::s_Event.button.type == SDL_MOUSEBUTTONUP)
	{

	}
}

void Level::InitWave()
{
	static std::default_random_engine rng(rnd());
	static std::uniform_int_distribution<std::size_t> spawnerDistr(0, spawners.size() - 1);

	std::size_t spawnPlace = spawnerDistr(rng);

	Tile* spawner = spawners.at(spawnPlace);

	Vector2D spawnPos((spawner->GetPos().x / m_ScaledTileSize), spawner->GetPos().y / m_ScaledTileSize);
	Vector2D dest = Vector2D(m_BasePos.x, m_BasePos.y);
	Vector2D moveVector(0.0f, 0.0f);

	auto enemy = AddEnemy(spawnPos.x, spawnPos.y, EnemyType::elf, App::s_Textures->GetTexture(App::TextureOf(EnemyType::elf)), 2);

	moveVector.x = dest.x - spawnPos.x;
	moveVector.y = dest.y - spawnPos.y;

	enemy->Move(moveVector);

	m_Wave.spawnedEnemies++;
}

void Level::ManageWaves()
{
	switch (m_Wave.waveProgress)
	{
	case WaveProgress::OnCooldown:
		if (SDL_TICKS_PASSED(SDL_GetTicks(), m_WaveCooldown))
		{
			InitWave();
			m_Wave.waveProgress = WaveProgress::Initializing;
		}
		return;
	case WaveProgress::Initializing:
		if (m_Wave.spawnedEnemies >= m_EnemiesPerWave * m_Wave.waveNumber)
		{
			printf("%zu\n", enemies.size());
			m_Wave.waveProgress = WaveProgress::InProgress;
			return;
		}

		InitWave();
		return;
	case WaveProgress::InProgress:
		if (enemies.size() == 0)
		{
			m_Wave.spawnedEnemies = 0;
			m_Wave.waveProgress = WaveProgress::Finished;
		}
		return;
	case WaveProgress::Finished:
		m_WaveCooldown = SDL_GetTicks() + waveCooldown;
		m_Wave.waveProgress = WaveProgress::OnCooldown;
		return;
	}
}

void Level::Render()
{
	// Get all layers
	for (const auto& layer : layers)
	{
		for (const auto& tile : layer.tiles)
		{
			if (!tile)
			{
				continue;
			}

			tile->Draw();
		}
	}

	if (App::s_UIState == UIState::building)
	{
		App::s_BuildingPlace->Draw();
	}

	m_BaseTile->Draw();

	for (const auto& enemy : enemies)
	{
		enemy->Draw();
	}

	for (const auto& tower : towers)
	{
		tower->Draw();
	}

	for (const auto& attacker : attackers)
	{
		attacker->Draw();
	}

	for (const auto& projectile : projectiles)
	{
		projectile->Draw();
	}
}

Tile* Level::GetTileFrom(uint32_t posX, uint32_t posY, uint16_t layer) const
{
	if (layer < 0 || layer >= layers.size())
	{
		App::s_Logger->AddLog("Requested a tile from " + std::to_string(posX) + ", " + std::to_string(posY), false);
		App::s_Logger->AddLog(", but layer " + std::to_string(layer) + " doesn't exist");
		return nullptr;
	}

	if (posX < 0 || posX >= m_MapSizeX || posY < 0 || posY >= m_MapSizeY)
	{
		return nullptr;
	}

	return layers.at(layer).GetTileFrom(posX, posY);
}

void Level::OnUpdateCamera()
{
	for (const auto& layer : layers)
	{
		for (const auto& tile : layer.tiles)
		{
			if (!tile)
			{
				continue;
			}

			tile->AdjustToView();
		}
	}

	m_BaseTile->AdjustToView();

	for (const auto& enemy : enemies)
	{
		static_cast<Enemy*>(enemy)->OnUpdateCamera();
	}
}

//std::vector<std::vector<Tile*>> Level::GetChunkOf(Entity* entity, uint16_t range)
//{
//	if (range < 1)
//	{
//		App::s_Logger->AddLog("Requested chunk in range less than 1, range has been modified to 1");
//		range = 1;
//	}
//
//	std::vector<std::vector<Tile*>> chunk;
//	const uint16_t sizeToReserve = range * 2 + 1;
//	chunk.reserve(sizeToReserve);
//
//	int32_t posX = (int32_t)entity->GetPos().x;
//	int32_t posY = (int32_t)entity->GetPos().y;
//
//	for (int32_t y = -range; y < range; y++)
//	{
//		std::vector<Tile*> rowOfTiles;
//		rowOfTiles.reserve(sizeToReserve);
//
//		for (int32_t x = -range; x < range; x++)
//		{
//			rowOfTiles.emplace_back(GetTileFrom(posX + x, posY + y));
//		}
//
//		chunk.emplace_back(rowOfTiles);
//	}
//
//	return chunk;
//}