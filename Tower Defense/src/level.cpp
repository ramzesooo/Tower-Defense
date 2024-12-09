#include "common.h"

#include "level.h"
#include "app.h"
#include "entity/label.h"
//#include "findPath.h"

#include <fstream>
#include <sstream>
#include <cmath>
#include <format>
#include <map>
#include <queue>

SDL_Texture *Level::s_Texture = nullptr;

constexpr uint16_t pathID = 1026;
constexpr uint16_t spawnerID = 305;
constexpr uint16_t waveCooldown = 3500; // miliseconds

constexpr char configName[] = ".config";

extern std::vector<Entity*> &g_Projectiles;
extern std::vector<Entity*> &g_Towers;
extern std::vector<Entity*> &g_Attackers;
extern std::vector<Entity*> &g_Enemies;

extern std::default_random_engine g_Rng;

static std::vector<Vector2D> findPath(const Vector2D &start, const Vector2D &goal)
{
	const Level &currentLevel = *App::s_CurrentLevel;
	std::vector<Vector2D> result;

	// Visited positions onto the origins from which they have been visited
	//std::unordered_map<Vector2D, Vector2D> visited;
	std::map<Vector2D, Vector2D> visited;

	struct Node
	{
		uint32_t totalDistance;
		Vector2D pos;

		// Defines a three-way comparison to let std::priority_queue comparing nodes
		auto operator<=>(const Node &other) const
		{
			return totalDistance <=> other.totalDistance;
		}

		std::array<Node, 4> GetNeighbours() const
		{
			// TODO: it's pretty pointless to use Dijkstra if the distance/cost of traversal
			// is always 1 everywhere, so maybe some tiles should be more expensive
			uint32_t dx = totalDistance + 1; // tileSize (=24) * scale (=2) * 2
			uint32_t dy = totalDistance + 1; // tileSize (=24) * scale (=2)
			return { {
				{ dx, Vector2D{ pos.x - 1, pos.y } },
				{ dy, Vector2D{ pos.x, pos.y - 1 } },
				{ dx, Vector2D{ pos.x + 1, pos.y } },
				{ dy, Vector2D{ pos.x, pos.y + 1 } },
			} };
		}
	};


	// we use std::greater because by default, queue.top() returns the greatest element,
	// and by using std::greater instead of std::less (the default) we flip this,
	// and make the queue give us the lowest element
	std::priority_queue<Node, std::vector<Node>, std::greater<Node>> queue;

	queue.push(Node{ 0, start });
	// putting (start, start) there is clever for debugging because the start nodes is the only node
	// which is "visited from itself"
	visited.emplace(start, start); // dummy value for the origin from which start has been visited

	while (!queue.empty())
	{
		Node next = queue.top();
		queue.pop();

		if (next.pos == goal)
		{
			Vector2D origin = goal;
			for (; origin != start; origin = visited.at(origin))
			{
				result.emplace_back(origin);
			}
			result.emplace_back(start);
			// because we push_back'd starting from the end node and continuing to the start node,
			// the result would be in reverse order, and this feels really weird,
			// so we reverse it before returning
			std::ranges::reverse(result);
			return result;
		}

		// A - B - C
		for (const Node &neighbor : next.GetNeighbours())
		{
			if (!currentLevel.IsTileWalkable(neighbor.pos))
				continue;

			// std::unordered_map::emplace will not insert a new element into the set if one already exists
			// in that case, success is false, but we always get an iterator to the
			// new/already existing element in the set
			auto [iterator, success] = visited.emplace(neighbor.pos, next.pos);

			if (success)
				queue.push(neighbor);
		}
	}

	// :/ no path found
	return result;
}

Level::Level(uint16_t levelID)
	: m_LevelID(levelID)
{
	// LOAD CONFIG
	std::ifstream configFile("levels\\" + std::to_string(m_LevelID + 1) + "\\" + configName);

	if (configFile.fail())
	{
		App::s_Logger.AddLog("Config file for level " + std::to_string(m_LevelID + 1) + " doesn't exist!");
		return;
	}

	std::string line;
	uint32_t lineNumber = 0;
	while (std::getline(configFile, line))
	{
		lineNumber++;

		std::istringstream ss(line);
		std::string value;


		// Map data
		if (lineNumber == 1)
		{
			// first line of config must contain map's width, height and scale
			// for example: 70,70,2
			for (auto i = 0u; i < 3; ++i)
			{
				if (!std::getline(ss, value, ',') || strlen(value.c_str()) == 0)
				{
					App::s_Logger.AddLog("Couldn't reach out map data no. " + std::to_string(i) + " from level " + std::to_string(m_LevelID + 1));
					m_MapData[i] = 2;
					break;
				}

				m_MapData[i] = (uint16_t)std::stoi(value);
			}

			continue;
		}
		else if (lineNumber == 2) // Base pos
		{
			if (!std::getline(ss, value, ',') || strlen(value.c_str()) == 0)
			{
				App::s_Logger.AddLog("Couldn't reach out base's X position from level " + std::to_string(m_LevelID + 1));
				m_BasePos.x = 0;
				continue;
			}
			else
			{
				m_BasePos.x = (float)std::stoi(value);
			}

			if (!std::getline(ss, value, ',') || strlen(value.c_str()) == 0)
			{
				App::s_Logger.AddLog("Couldn't reach out base's Y position from level " + std::to_string(m_LevelID + 1));
				m_BasePos.y = 0;
			}
			else
			{
				m_BasePos.y = (float)std::stoi(value);
			}

			continue;
		}
		else if (lineNumber == 3)
		{
			if (!std::getline(ss, value, ','))
			{
				App::s_Logger.AddLog(std::string_view("Couldn't reach out movement speed rate from config file!"));
				m_MovementSpeedRate = 1u;
				continue;
			}

			m_MovementSpeedRate = (uint16_t)std::stoi(value);
			IF_DEBUG(App::s_Logger.AddLog(std::format("Movement speed rate for level #{}: x{}", m_LevelID + 1, m_MovementSpeedRate));)
			continue;
		}

		m_Waves.reserve(1);
		m_Waves.emplace_back(std::array<uint16_t, (std::size_t)EnemyType::size>{});
		auto &wave = m_Waves.back();
		for (auto i = 0u; i < (std::size_t)EnemyType::size; ++i)
		{
			if (!std::getline(ss, value, ','))
				break;

			wave[i] = (uint16_t)std::stoi(value);
		}
	}
	// LOAD CONFIG

	m_ScaledTileSize = m_MapData.at(2) * s_TileSize;

	m_MapData[3] = m_MapData.at(0) * m_ScaledTileSize;
	m_MapData[4] = m_MapData.at(1) * m_ScaledTileSize;
}

void Level::Setup(std::ifstream& mapFile, uint16_t layerID)
{
	if (layerID < 0 || layerID >= m_Layers.size())
	{
		App::s_Logger.AddLog("Failed to load level " + std::to_string(m_LevelID + 1));
		App::s_Logger.AddLog("Layer " + std::to_string(layerID) + " is less than 0 or higher than expected amount of layers");
		return;
	}

	if (mapFile.fail())
	{
		App::s_Logger.AddLog("Failed to load level " + std::to_string(m_LevelID + 1));
		return;
	}
	else
	{
		App::s_Logger.AddLog("Loading level " + std::to_string(m_LevelID + 1), false);
		App::s_Logger.AddLog(" (Layer: " + std::to_string(layerID) + ")");
	}

	// This code can be improved
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
	TileType tileType = (TileType)layerID;
	/*TileTypes tileType;

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
	}*/

	Layer* newLayer = &m_Layers.at(layerID);
	newLayer->tiles.reserve(std::size_t(m_MapData.at(0) * m_MapData.at(1)));

	Tile* tile = nullptr;
	uint32_t srcX, srcY;
	uint32_t x, y;

	if (layerID < 2)
	{
		for (uint16_t i = 0; i < m_MapData.at(0) * m_MapData.at(1); i++)
		{
			x = i % m_MapData.at(0);
			y = i / m_MapData.at(1);
			tileCode = mapData.at(y).at(x);
			srcX = tileCode % 10;
			srcY = tileCode / 10;
			tile = App::s_Manager.NewTile(srcX * s_TileSize, srcY * s_TileSize, x * m_ScaledTileSize, y * m_ScaledTileSize, s_TileSize, m_MapData.at(2), s_Texture, tileType);

			if (!tile)
			{
				m_FailedLoading = true;
				App::s_Logger.AddLog(std::format("Couldn't load a tile ({}, {})", x * m_ScaledTileSize, y * m_ScaledTileSize));
			}

			newLayer->tiles.emplace_back(tile);
		}
	}
	else
	{
		for (uint16_t i = 0; i < m_MapData.at(0) * m_MapData.at(1); i++)
		{
			x = i % m_MapData.at(0);
			y = i / m_MapData.at(1);
			tileCode = mapData.at(y).at(x);
			srcX = tileCode % 10;
			srcY = tileCode / 10;

			if (tileCode == pathID)
			{
				tile = App::s_Manager.NewTile(srcX * s_TileSize, srcY * s_TileSize, x * m_ScaledTileSize, y * m_ScaledTileSize, s_TileSize, m_MapData.at(2), s_Texture, tileType, true);
			}
			else
			{
				tile = App::s_Manager.NewTile(srcX * s_TileSize, srcY * s_TileSize, x * m_ScaledTileSize, y * m_ScaledTileSize, s_TileSize, m_MapData.at(2), s_Texture, tileType, false);

				if (tileCode == spawnerID)
				{
					m_Spawners.emplace_back(tile);
				}
			}

			if (!tile)
			{
				m_FailedLoading = true;
				App::s_Logger.AddLog(std::format("Couldn't load a tile ({}, {})", x * m_ScaledTileSize, y * m_ScaledTileSize));
			}

			newLayer->tiles.emplace_back(tile);
		}

		IF_DEBUG(App::s_Logger.AddLog(std::format("Added {} spawners", m_Spawners.size()));)
	}

	mapFile.close();
}

void Level::SetupBase(uint32_t posX, uint32_t posY)
{
	int32_t scaledPosX = posX * m_ScaledTileSize;
	int32_t scaledPosY = posY * m_ScaledTileSize;
	m_Base.m_Texture = App::s_Textures.GetTexture(m_BaseTextureID);
	m_Base.destRect = { scaledPosX, scaledPosY, Base::srcRect.w * 2, Base::srcRect.h * 2 };
	m_Base.m_Pos = { (float)scaledPosX, (float)scaledPosY };
	m_Base.m_MaxLifes = m_Base.m_Lifes = 5;
	m_Base.m_Tile = GetTileFrom(posX, posY, 0);

	App::s_Logger.AddLog(std::format("Created base ({}, {})", scaledPosX, scaledPosY));
}

void Level::Clean()
{
	App::s_Manager.DestroyAllEntities();

	for (auto &layer : m_Layers)
	{
		layer.tiles.clear();
	}

	m_Spawners.clear();

	App::s_Manager.DestroyAllTiles();

	m_CurrentWave = 0;
	m_WaveProgress = WaveProgress::OnCooldown;
	m_SpecificEnemiesAmount = {};
}

Tower* Level::AddTower(float posX, float posY, SDL_Texture* towerTexture, uint16_t tier)
{
	if (!towerTexture)
	{
		App::s_Logger.AddLog(std::string_view("Level::AddTower: Tower's texture doesn't exist!"));
		return nullptr;
	}

	auto tower = App::s_Manager.NewEntity<Tower>(posX, posY, towerTexture, tier);
	tower->AddGroup(EntityGroup::tower);

	AddAttacker(tower, (AttackerType)(tier - 1));
	return tower;
}

void Level::AddAttacker(Tower* assignedTower, AttackerType type, uint16_t scale)
{
	if (!assignedTower || assignedTower->GetAttacker())
	{
		App::s_Logger.AddLog(std::string_view("Tried to add attacker to not existing tower or an attacker for the specific tower already exists."));
		return;
	}
	
	// Probably will have to use the switch in the future anyway, so let's consider it as a temporary
	uint32_t shotCooldown = 325 - (50 * ((uint32_t)type + 1));

	/*switch (type)
	{
	case AttackerType::archer:
		shotCooldown = 325;
		break;
	case AttackerType::hunter:
		shotCooldown = 275;
		break;
	case AttackerType::musketeer:
		shotCooldown = 225;
		break;
	default:
		shotCooldown = 300;
		break;
	}*/

	auto attacker = App::s_Manager.NewEntity<Attacker>(assignedTower, type, App::s_Textures.GetTexture(App::TextureOf(type)), shotCooldown, scale);
	attacker->AddGroup(EntityGroup::attacker);
	assignedTower->AssignAttacker(attacker);
}

Enemy* Level::AddEnemy(float posX, float posY, EnemyType type, SDL_Texture* texture, uint16_t scale) const
{
	auto enemy = App::s_Manager.NewEntity<Enemy>(posX, posY, type, texture, scale);
	enemy->AddGroup(EntityGroup::enemy);

	IF_DEBUG(App::s_EnemiesAmountLabel->UpdateText("Enemies: " + std::to_string(g_Enemies.size()));)

	enemy->SetPath(findPath({ posX, posY }, m_BasePos));

	return enemy;
}

void Level::AddProjectile(ProjectileType type, Attacker* projectileOwner, Enemy* target)
{
	if (!target->IsActive())
		return;

	Projectile* projectile = App::s_Manager.NewEntity<Projectile>(type, projectileOwner, target);
	projectile->AddGroup(EntityGroup::projectile);
	projectileOwner->m_OwnedProjectiles.emplace_back(projectile);
}

void Level::HandleMouseButtonEvent()
{
	//if (App::s_Event.button.type == SDL_MOUSEBUTTONDOWN)
	//{
		if (App::s_UIState == UIState::building)
		{
			if (App::s_Building.canBuild)
			{
				Tower* tower = AddTower(App::s_Building.coordinates.x, App::s_Building.coordinates.y, App::s_Textures.GetTexture("tower"), 1);
				App::s_Building.originalTexture = App::s_Textures.GetTexture("upgradeTower");
				App::s_Building.buildingPlace->SetTexture(App::s_Building.originalTexture);
				App::s_Building.towerToUpgrade = tower;
				App::s_Building.canBuild = false;
				return;
			}

			if (App::s_Building.towerToUpgrade)
			{
				App::s_Building.towerToUpgrade->Upgrade();
				return;
			}
		}
	//}
	//else if (App::s_Event.button.type == SDL_MOUSEBUTTONUP)
	//{

	//}
}

void Level::InitWave()
{
	if (m_SpawnedEnemies >= m_ExpectedEnemiesAmount)
	{
		m_WaveProgress = WaveProgress::InProgress;
		return;
	}

	static std::uniform_int_distribution<std::size_t> spawnerDistr(0, m_Spawners.size() - 1);

	const Tile* spawner = m_Spawners.at(spawnerDistr(g_Rng));

	const Vector2D spawnPos(spawner->GetPos().x / m_ScaledTileSize, spawner->GetPos().y / m_ScaledTileSize);

	EnemyType type = EnemyType::elf;
	// It might be as well casual variable defined in for loop, but maybe it's better to store it here
	// and use it after the for loop instead of casting the type to std::size_t
	std::size_t enemyTypeIterator = 0u;
	// for each enemy type
	for (; enemyTypeIterator < (std::size_t)EnemyType::size; ++enemyTypeIterator)
	{
		// if currently iterated enemy type didn't reach still the expected amount of spawned enemies
		if (m_SpecificEnemiesAmount.at(enemyTypeIterator) < m_Waves.at(m_CurrentWave)[enemyTypeIterator])
		{
			type = (EnemyType)enemyTypeIterator;
			m_SpecificEnemiesAmount[enemyTypeIterator]++;
			break;
		}
	}

	auto enemy = AddEnemy(spawnPos.x, spawnPos.y, type, App::s_Textures.GetTexture(App::TextureOf(type)), 2);

	if (!enemy)
	{
		App::s_Logger.AddLog(std::format("Failed spawning enemy #{} (type: {})", m_SpawnedEnemies, enemyTypeIterator));
		m_SpecificEnemiesAmount[enemyTypeIterator]--;
		return;
	}
	
	m_NextSpawn = SDL_GetTicks() + s_SpawnCooldown;
	m_SpawnedEnemies++;

	/*std::size_t i = 0;
	std::vector<Vector2D> testVector = findPath(spawnPos, dest);
	for (const auto &pos : testVector)
	{
		printf("#%zd: (%.1f, %.1f)\n", ++i, pos.x, pos.y);
	}
	printf("(%.1f, %.1f)\n", dest.x, dest.y);*/
}

void Level::ManageWaves()
{
	switch (m_WaveProgress)
	{
	case WaveProgress::OnCooldown:
		if (SDL_TICKS_PASSED(SDL_GetTicks(), m_WaveCooldown))
		{
			if (m_Spawners.empty())
			{
				App::s_Logger.AddLog(std::string_view("Level::InitWave: Initializing wave failed, due to missing spawners\n"));
				return;
			}

			m_SpawnedEnemies = 0;
			m_ExpectedEnemiesAmount = 0;
			for (const auto &i : m_Waves.at(m_CurrentWave))
				m_ExpectedEnemiesAmount += i;

			InitWave();

			App::UpdateWaves();

			m_WaveProgress = WaveProgress::Initializing;
		}
		return;
	case WaveProgress::Initializing:
		if (SDL_GetTicks() > m_NextSpawn)
			InitWave();
		return;
	case WaveProgress::InProgress:
		if (g_Enemies.size() == 0)
		{
			m_WaveProgress = WaveProgress::Finished;
			m_SpecificEnemiesAmount = {};
		}
		return;
	case WaveProgress::Finished:
		if (++m_CurrentWave >= m_Waves.size())
		{
			m_CurrentWave = 0;
		}
		m_WaveCooldown = SDL_GetTicks() + waveCooldown;
		m_WaveProgress = WaveProgress::OnCooldown;
		return;
	}
}

void Level::Render()
{
	for (std::size_t i = 0; i < m_Layers.size(); ++i)
	{
		for (const auto &tile : m_Layers.at(i).tiles)
		{
			// Might be necessary, but the game isn't supposed to create nullptr tiles, if it does, then it's probably an issue with level
			//if (!tile)
				//continue;

			tile->Draw();
		}
	}

	for (const auto &enemy : g_Enemies)
		enemy->Draw();

	for (const auto &tower : g_Towers)
		tower->Draw();

	for (const auto &attacker : g_Attackers)
		attacker->Draw();

	m_Base.Draw();

	App::s_Building.buildingPlace->Draw();

	for (const auto &projectile : g_Projectiles)
		projectile->Draw();
}

Tile* Level::GetTileFrom(uint32_t posX, uint32_t posY, uint16_t layer) const
{
	if (layer < 0 || layer >= m_Layers.size())
	{
		App::s_Logger.AddLog(std::format("Requested a tile ({}, {}), but layer {} doesn't exist", posX, posY, layer));
		return nullptr;
	}

	if (posX < 0 || posX >= m_MapData.at(0) || posY < 0 || posY >= m_MapData.at(1))
		return nullptr;

	return m_Layers.at(layer).GetTileFrom(posX, posY, m_MapData.at(0));
}

void Level::OnUpdateCamera()
{
	for (std::size_t i = 0; i < m_Layers.size(); ++i)
	{
		for (const auto &tile : m_Layers.at(i).tiles)
		{
			//if (!tile)
				//continue;

			tile->AdjustToView();
		}
	}

	m_Base.AdjustToView();

	for (const auto &e : g_Enemies)
	{
		e->AdjustToView();
	}

	for (const auto &t : g_Towers)
	{
		// Tower::AdjustToView() triggers also Attacker::AdjustToView()
		t->AdjustToView();
	}

	for (const auto &p : g_Projectiles)
	{
		p->AdjustToView();
	}
}