#include "level.h"
#include "app.h"

#include <fstream>
#include <sstream>

Level::Level() : towers(App::s_Manager->GetGroup(EntityGroup::tower)), attackers(App::s_Manager->GetGroup(EntityGroup::attacker))
{}

void Level::Setup(const std::string& path)
{
	std::ifstream mapFile(path);

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

	std::unique_ptr<Layer> newLayer = std::make_unique<Layer>();

	auto& tiles = newLayer->GetTilesVector();

	tiles.reserve(m_MapSizeY);

	for (int32_t y = 0; y < m_MapSizeY; ++y) {
		std::vector<Tile*> rowOfTiles;
		rowOfTiles.reserve(m_MapSizeX);

		for (int32_t x = 0; x < m_MapSizeX; ++x) {
			tileCode = mapData[y][x];
			int32_t srcX = tileCode % 10;
			int32_t srcY = tileCode / 10;
			//Tile* tile = app.AddTile(srcX * m_TileSize, srcY * m_TileSize, x * (m_MapScale * m_TileSize), y * (m_MapScale * m_TileSize), m_TileSize, m_MapScale, m_TextureID);
			Tile* tile = App::s_Manager->NewEntity<Tile>(srcX * m_TileSize, srcY * m_TileSize, x * (m_MapScale * m_TileSize), y * (m_MapScale * m_TileSize), m_TileSize, m_MapScale, m_TextureID);
			tile->AddGroup(EntityGroup::tile);

			if (!tile)
			{
				tile = nullptr;
				m_FailedLoading = true;
				App::s_Logger->AddLog("Couldn't load a tile (", false);
				App::s_Logger->AddLog(std::to_string(x * (m_MapScale * m_TileSize)), false);
				App::s_Logger->AddLog(", ", false);
				App::s_Logger->AddLog(std::to_string(y * (m_MapScale * m_TileSize)), false);
				App::s_Logger->AddLog(")");
			}

			rowOfTiles.emplace_back(tile);
		}

		tiles.emplace_back(rowOfTiles);
	}

	layers.push_back(std::move(newLayer));

	App::s_Logger->AddLog("Loaded map: ", false);
	App::s_Logger->AddLog(path);
}

void Level::Setup(std::ifstream& mapFile)
{
	//std::ifstream mapFile(path);

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

	std::unique_ptr<Layer> newLayer = std::make_unique<Layer>();

	auto& tiles = newLayer->GetTilesVector();

	tiles.reserve(m_MapSizeY);

	for (int32_t y = 0; y < m_MapSizeY; ++y) {
		std::vector<Tile*> rowOfTiles;
		rowOfTiles.reserve(m_MapSizeX);

		for (int32_t x = 0; x < m_MapSizeX; ++x) {
			tileCode = mapData[y][x];
			int32_t srcX = tileCode % 10;
			int32_t srcY = tileCode / 10;
			//Tile* tile = app.AddTile(srcX * m_TileSize, srcY * m_TileSize, x * (m_MapScale * m_TileSize), y * (m_MapScale * m_TileSize), m_TileSize, m_MapScale, m_TextureID);
			Tile* tile = App::s_Manager->NewEntity<Tile>(srcX * m_TileSize, srcY * m_TileSize, x * (m_MapScale * m_TileSize), y * (m_MapScale * m_TileSize), m_TileSize, m_MapScale, m_TextureID);
			tile->AddGroup(EntityGroup::tile);

			if (!tile)
			{
				tile = nullptr;
				m_FailedLoading = true;
				App::s_Logger->AddLog("Couldn't load a tile (", false);
				App::s_Logger->AddLog(std::to_string(x * (m_MapScale * m_TileSize)), false);
				App::s_Logger->AddLog(", ", false);
				App::s_Logger->AddLog(std::to_string(y * (m_MapScale * m_TileSize)), false);
				App::s_Logger->AddLog(")");
			}

			rowOfTiles.emplace_back(tile);
		}

		tiles.emplace_back(rowOfTiles);
	}

	layers.push_back(std::move(newLayer));

	mapFile.close();
	//App::s_Logger->AddLog("Loaded map");
}

void Level::SetupBase(uint32_t posX, uint32_t posY)
{
	baseTile = App::s_Manager->NewEntity<Tile>(0, 0, posX * m_ScaledTileSize, posY * m_ScaledTileSize, m_TileSize, m_MapScale, m_BaseTextureID);
	baseTile->AddGroup(EntityGroup::tile);

	App::s_Logger->AddLog("Created base (", false);
	App::s_Logger->AddLog(std::to_string(posX * m_ScaledTileSize), false);
	App::s_Logger->AddLog(", ", false);
	App::s_Logger->AddLog(std::to_string(posY * m_ScaledTileSize), false);
	App::s_Logger->AddLog(")");
}

Tower* Level::AddTower(float posX, float posY, SDL_Texture* towerTexture, int32_t tier)
{
	if (!towerTexture)
	{
		App::s_Logger->AddLog("Tower's texture doesn't exist!");
	}

	auto tower = App::s_Manager->NewEntity<Tower>(posX, posY, towerTexture, tier);
	tower->AddGroup(EntityGroup::tower);

	return tower;
}

void Level::AddAttacker(Tower* assignedTower, AttackerType type, uint16_t scale)
{
	auto attacker = App::s_Manager->NewEntity<Attacker>(*assignedTower, type, App::s_Textures->GetTexture(App::TextureOf(type)), scale);
	attacker->AddGroup(EntityGroup::attacker);
	assignedTower->AssignAttacker(attacker);
}

void Level::Render()
{
	// Get all layers
	for (const auto& layer : layers)
	{
		// Get the whole vector with tiles: std::vector<std::vector<Tile*>>
		auto& layeredTiles = layer->GetTilesVector();
		for (const auto& row : layeredTiles)
		{
			// Loop for every tile in a row and render them
			for (const auto& tile : row)
			{
				tile->Draw();
			}
		}
	}

	for (const auto& tower : towers)
	{
		tower->Draw();
	}

	for (const auto& attacker : attackers)
	{
		attacker->Draw();
	}

	baseTile->Draw();
}

Tile* Level::GetTileFrom(uint32_t posX, uint32_t posY, uint16_t layer)
{
	if (layer < 0 || layer >= layers.size())
	{
		App::s_Logger->AddLog("Requested a tile from " + std::to_string(posX) + ", " + std::to_string(posY), false);
		App::s_Logger->AddLog(", but layer " + std::to_string(layer) + " doesn't exist");
		return nullptr;
	}

	return layers.at(layer)->GetTileFrom(posX, posY);
}