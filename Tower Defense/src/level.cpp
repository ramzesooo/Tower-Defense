#include "level.h"

#include "UI.h"
#include "app.h"
#include "entity/enemy.h"
#include "entity/label.h"
#include "entity/towers/towers_inc.h"

#include <fstream>
#include <sstream>
#include <cmath>
#include <format>
#include <unordered_map>
#include <queue>

static std::vector<Vector2D> findPath(const Vector2D &start, const Vector2D &goal)
{
	const Level &currentLevel = *App::s_CurrentLevel;
	std::vector<Vector2D> result;

	// Visited positions onto the origins from which they have been visited
	std::unordered_map<Vector2D, Vector2D> visited;

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
			uint32_t dx = totalDistance + 1;
			uint32_t dy = totalDistance + 2;
			// TODO: calculate base's distance from spawn and make dx and dy properly to what is more expensive
			// Usually: totalDistance + 1
			// If the difference between the base and spawn relies more on X then dx = totalDistance + 2
			// The same for Y
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

uint16_t Layer::s_MapWidth = 0u;

SDL_Texture *Level::s_Texture = nullptr;

static constexpr uint16_t pathID = 1026u;
static constexpr uint16_t spawnerID = 305u;
static constexpr uint16_t waveCooldown = 4500u; // milliseconds

static constexpr char configName[] = ".config";

extern std::vector<Entity*> &g_Projectiles;
extern std::vector<Entity*> &g_Towers;
extern std::vector<Entity*> &g_Attackers;
extern std::vector<Entity*> &g_Enemies;

extern std::default_random_engine g_Rng;

extern uint32_t g_PausedTicks;

Level::Level(uint16_t levelID)
	: m_LevelID(levelID)
{
	// LOAD CONFIG
	std::ifstream configFile(std::format("levels\\{}\\{}", m_LevelID + 1, configName));

	if (configFile.fail())
	{
		App::s_Logger.AddLog(std::format("Config file for level {} doesn't exist!", m_LevelID + 1));
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
			for (auto i = 0u; i < 3u; ++i)
			{
				if (!std::getline(ss, value, ',') || strlen(value.c_str()) == 0)
				{
					App::s_Logger.AddLog(std::format("Couldn't reach out map data no. {} from level {}", i, m_LevelID + 1));
					m_MapData[i] = 2;
					break;
				}

				m_MapData[i] = static_cast<uint16_t>(std::stoi(value));
			}

			continue;
		}
		else if (lineNumber == 2) // Base pos
		{
			if (!std::getline(ss, value, ',') || strlen(value.c_str()) == 0)
			{
				App::s_Logger.AddLog(std::format("Couldn't reach out base's X position from level {}", m_LevelID + 1));
				m_BasePos.x = 0.0f;
				continue;
			}

			m_BasePos.x = std::stof(value);

			if (!std::getline(ss, value, ',') || strlen(value.c_str()) == 0)
			{
				App::s_Logger.AddLog(std::format("Couldn't reach out base's Y position from level {}", m_LevelID + 1));
				m_BasePos.y = 0.0f;
			}
			else
			{
				m_BasePos.y = std::stof(value);
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

			m_MovementSpeedRate = static_cast<uint16_t>(std::stoul(value));
			IF_DEBUG(App::s_Logger.AddLog(std::format("Movement speed rate for level #{}: x{}", m_LevelID + 1, m_MovementSpeedRate));)
			continue;
		}

		WaveContainer newWave{};

		for (auto &wave : newWave.container)
		{
			if (!std::getline(ss, value, ','))
				break;

			wave = static_cast<uint16_t>(std::stoi(value));
		}

		/*for (std::size_t i = 0u; i < newWave.container.size(); ++i)
		{
			if (!std::getline(ss, value, ','))
				break;

			newWave.container[i] = static_cast<uint16_t>(std::stoi(value));
		}*/

		m_Waves.emplace_back(newWave);
	}

	m_Waves.shrink_to_fit();
	// LOAD CONFIG

	m_ScaledTileSize = m_MapData.at(2) * s_TileSize;

	m_MapData[3] = m_MapData.at(0) * m_ScaledTileSize;
	m_MapData[4] = m_MapData.at(1) * m_ScaledTileSize;
}

void Level::Init()
{
	for (uint16_t i = 0u; i < Level::s_LayersAmount; i++)
	{
		std::ifstream mapFile(std::format("levels/{}/map_layer{}.map", GetID() + 1u, i));

		SetupLayer(mapFile, i);
	}

	SetupBase();

	InitTimerForNextWave();

	m_WaveCooldown = SDL_GetTicks() - g_PausedTicks + waveCooldown;
	m_WaveProgress = WaveProgress::OnCooldown;
}

void Level::SetupLayer(std::ifstream &mapFile, uint16_t layerID)
{
	if (layerID >= m_Layers.size())
	{
		App::s_Logger.AddLog(std::format("Failed to load level {}: Layer {} doesn't exist", m_LevelID + 1, layerID));
		return;
	}

	if (mapFile.fail())
	{
		App::s_Logger.AddLog(std::format("Failed to load level {}: Couldn't find the file", m_LevelID + 1));
		return;
	}

	App::s_Logger.AddLog(std::format("Loading level {} (Layer: {})", m_LevelID + 1, layerID));

	// This code can be improved
	std::string line;
	std::vector<std::vector<int>> mapData;

	while (std::getline(mapFile, line))
	{
		std::istringstream ss(line);
		std::vector<int> row;
		std::string value;

		while (std::getline(ss, value, ','))
		{
			row.emplace_back(std::stoi(value));
		}

		mapData.emplace_back(row);
	}

	int32_t tileCode;
	TileType tileType = static_cast<TileType>(layerID);
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

	Layer *newLayer = &m_Layers.at(layerID);
	newLayer->m_Tiles.reserve(std::size_t(m_MapData.at(0) * m_MapData.at(1)));
	newLayer->m_DrawableTiles.reserve(std::size_t(m_MapData.at(0) * m_MapData.at(1)));

	Tile *tile = nullptr;
	int32_t srcX, srcY;
	int32_t x, y;

	for (uint16_t i = 0u; i < m_MapData.at(0) * m_MapData.at(1); i++)
	{
		x = i % m_MapData.at(0);
		y = i / m_MapData.at(1);
		tileCode = mapData.at(y).at(x);
		srcX = tileCode % 10;
		srcY = tileCode / 10;

		tile = App::s_Manager.NewTile(srcX * s_TileSize, srcY * s_TileSize, x * m_ScaledTileSize, y * m_ScaledTileSize, s_Texture, tileType);

		if (layerID == 2)
		{
			tile->SetDrawable(false);

			if (tileCode == pathID)
			{
				tile->SetWalkable();
			}
			else if (tileCode == spawnerID)
			{
				m_Spawners.reserve(m_Spawners.size() + 1);
				m_Spawners.emplace_back(tile);
				tile->SetDrawable(true);
			}
		}

		if (!tile)
		{
			m_FailedLoading = true;
			App::s_Logger.AddLog(std::format("Couldn't load a tile in layer {} ({}, {})", layerID, x * m_ScaledTileSize, y * m_ScaledTileSize));
			// We are storing nullptr in the vector even if the tile couldn't be created
			// Because it hasn't been designed to expect any tile to be failed
			// So then maybe if (tile) when needed, but probably something's just wrong if it can't be created
		}

		newLayer->m_Tiles.emplace_back(tile);

		if (tile->IsDrawable())
			newLayer->m_DrawableTiles.emplace_back(tile);
	}

	newLayer->m_DrawableTiles.shrink_to_fit();

	mapFile.close();
}

void Level::SetupBase()
{
	int32_t scaledPosX = static_cast<int32_t>(m_BasePos.x) * m_ScaledTileSize;
	int32_t scaledPosY = static_cast<int32_t>(m_BasePos.y) * m_ScaledTileSize;
	m_Base.m_Texture = App::s_Textures.GetTexture(m_BaseTextureID);
	m_Base.destRect = { scaledPosX, scaledPosY, Base::srcRect.w * 2, Base::srcRect.h * 2 };
	m_Base.m_Pos = { static_cast<float>(scaledPosX), static_cast<float>(scaledPosY) };
	m_Base.m_MaxLifes = m_Base.m_Lifes = 5;
	m_Base.m_Tile = GetTileFrom(m_BasePos.x, m_BasePos.y, 0);

	App::s_Logger.AddLog(std::format("Created base ({}, {})", scaledPosX, scaledPosY));

	InitTimerForNextWave();
}

void Level::Clean()
{
	App::s_Manager.DestroyAllEntities();

	for (auto &layer : m_Layers)
	{
		layer.m_Tiles.clear();
		layer.m_DrawableTiles.clear();
	}

	m_Spawners.clear();

	App::s_Manager.DestroyAllTiles();

	m_CurrentWave = 0u;
	m_WaveProgress = WaveProgress::OnCooldown;
	m_SpecificEnemiesAmount = {};
}

void Level::AddTower(float posX, float posY, TowerType type)
{
	Tower *tower = nullptr;

	switch (type)
	{
	case TowerType::classic:
		tower = App::s_Manager.NewEntity<ClassicTower>(posX, posY, type);
		return;
	case TowerType::dark:
		tower = App::s_Manager.NewEntity<DarkTower>(posX, posY, type);
		return;
	default:
		App::s_Logger.AddLog(std::format("Level::AddTower: TowerType {} is invalid", static_cast<std::size_t>(type)));
		break;
	}

	if (!tower)
	{
		App::s_Logger.AddLog(std::format("Level::AddTower: Failed adding a tower (type #{}) ({}, {})", static_cast<std::size_t>(type), posX, posY));
	}
}

void Level::AddAttacker(Tower *assignedTower, AttackerType type, uint16_t scale)
{
	if (!assignedTower || assignedTower->GetAttacker() != nullptr)
	{
		App::s_Logger.AddLog(std::string_view("Tried to add attacker to non-existing tower or an attacker for the specific tower already exists."));
		return;
	}

	// Probably will have to use the switch in the future anyway, so let's consider it as a temporary
	//uint32_t shotCooldown = 325 - (50 * ((uint32_t)type + 1));
	uint32_t shotCooldown = 0u;
	Attacker *attacker = nullptr;

	switch (type)
	{
	case AttackerType::archer:
		shotCooldown = 315u;
		attacker = App::s_Manager.NewEntity<ClassicAttacker>(assignedTower, type, shotCooldown, scale);
		break;
	case AttackerType::hunter:
		shotCooldown = 280u;
		attacker = App::s_Manager.NewEntity<ClassicAttacker>(assignedTower, type, shotCooldown, scale);
		break;
	case AttackerType::musketeer:
		shotCooldown = 250u;
		attacker = App::s_Manager.NewEntity<ClassicAttacker>(assignedTower, type, shotCooldown, scale);
		break;
	case AttackerType::darkTower:
		shotCooldown = assignedTower->GetAnimSpeed("Attack") * 11;
		attacker = App::s_Manager.NewEntity<DarkAttacker>(assignedTower, type, shotCooldown, scale);
		break;
	default:
		App::s_Logger.AddLog(std::format("Level::AddAttacker: AttackerType {} is invalid", static_cast<std::size_t>(type)));
		return;
	}

	attacker->AddToGroup(EntityGroup::attacker);
	assignedTower->AssignAttacker(attacker);
}

Enemy *Level::AddEnemy(float posX, float posY, EnemyType type, uint16_t scale) const
{
	auto enemy = App::s_Manager.NewEntity<Enemy>(posX, posY, type, scale);
	enemy->AddToGroup(EntityGroup::enemy);

	IF_DEBUG(App::s_EnemiesAmountLabel->UpdateText(std::format("Enemies: {}", g_Enemies.size())););

	enemy->SetPath(findPath({ posX, posY }, m_BasePos));

	return enemy;
}

void Level::AddProjectile(ProjectileType type, Attacker *projectileOwner, Enemy *target)
{
	// Probably don't need this anymore since Attacker has now ValidTarget()
	/*if (!target->IsActive())
		return;*/

	auto *projectile = App::s_Manager.NewEntity<Projectile>(type, projectileOwner, target);
	projectile->AddToGroup(EntityGroup::projectile);
	projectileOwner->m_OwnedProjectiles.emplace_back(projectile);
}

void Level::LMBEvent()
{
	// Check if mouse is pointing at hammer for building
	if (App::s_MouseX >= UIElement::hammerDestRect.x && App::s_MouseX <= UIElement::hammerDestRect.x + UIElement::hammerDestRect.w
		&& App::s_MouseY >= UIElement::hammerDestRect.y && App::s_MouseY <= UIElement::hammerDestRect.y + UIElement::hammerDestRect.h)
	{
		App::Instance().SwitchBuildingState(App::s_UIState == UIState::none ? UIState::building : UIState::none);
		return;
	}

	// Check if mouse is pointing at sell tower icon
	if (App::s_MouseX >= UIElement::sellDestRect.x && App::s_MouseX <= UIElement::sellDestRect.x + UIElement::sellDestRect.w
		&& App::s_MouseY >= UIElement::sellDestRect.y && App::s_MouseY <= UIElement::sellDestRect.y + UIElement::sellDestRect.h)
	{
		App::Instance().SwitchBuildingState(App::s_UIState == UIState::none ? UIState::selling : UIState::none);
		return;
	}

	// Check if mouse is pointing at upgrade tower icon
	if (App::s_MouseX >= UIElement::upgradeDestRect.x && App::s_MouseX <= UIElement::upgradeDestRect.x + UIElement::upgradeDestRect.w
		&& App::s_MouseY >= UIElement::upgradeDestRect.y && App::s_MouseY <= UIElement::upgradeDestRect.y + UIElement::upgradeDestRect.h)
	{
		App::Instance().SwitchBuildingState(App::s_UIState == UIState::none ? UIState::upgrading : UIState::none);
		return;
	}

	/*
	// If there is already chosen tower
	if (UIElement::s_ChosenTower != TowerType::size)
	{
		auto &element = s_ExpandingTowers.at(static_cast<std::size_t>(UIElement::s_ChosenTower));

		// Check if mouse pressed on the already chosen tower
		if (s_MouseX >= element.destRect.x && s_MouseX <= element.destRect.x + element.destRect.w
			&& s_MouseY >= element.destRect.y && s_MouseY <= element.destRect.y + element.destRect.h)
		{
			UIElement::s_ChosenTower = TowerType::size;
			element.m_IsPressed = false;
			return;
		}

		// If pressed somewhere else, check if pressed on any other tower
		for (std::size_t i = 0u; i < s_ExpandingTowers.size(); i++)
		{
			auto &tower = s_ExpandingTowers.at(i);

			if (s_MouseX >= tower.destRect.x && s_MouseX <= tower.destRect.x + tower.destRect.w
				&& s_MouseY >= tower.destRect.y && s_MouseY <= tower.destRect.y + tower.destRect.h)
			{
				UIElement::s_ChosenTower = static_cast<TowerType>(i);
				tower.m_IsPressed = true;
				element.m_IsPressed = false;
				return;
			}
		}

		return;
	}
	*/

	auto &element = App::s_ExpandingTowers.at(static_cast<std::size_t>(UIElement::s_ChosenTower));

	// Check if mouse pressed on the already chosen tower
	if (App::s_MouseX >= element.destRect.x && App::s_MouseX <= element.destRect.x + element.destRect.w
		&& App::s_MouseY >= element.destRect.y && App::s_MouseY <= element.destRect.y + element.destRect.h)
	{
		// Do nothing, avoids unnecessary loop
		return;
	}

	// Check if any tower has been pressed if there isn't any already
	for (std::size_t i = 0u; i < App::s_ExpandingTowers.size(); i++)
	{
		auto &tower = App::s_ExpandingTowers.at(i);

		if (App::s_MouseX >= tower.destRect.x && App::s_MouseX <= tower.destRect.x + tower.destRect.w
			&& App::s_MouseY >= tower.destRect.y && App::s_MouseY <= tower.destRect.y + tower.destRect.h)
		{
			UIElement::s_ChosenTower = static_cast<TowerType>(i);
			tower.m_IsPressed = true;
			element.m_IsPressed = false;
			return;
		}
	}

	if (App::IsBuildingState() && !App::s_Building.canBuild)
		return;

	if (App::s_UIState == UIState::building)
	{
		AddTower(App::s_Building.coordinates.x, App::s_Building.coordinates.y, UIElement::s_ChosenTower);

		App::s_Building.buildingPlace.SetTexture(BuildingState::cantBuildTexture);
		App::s_Building.towerToUpgradeOrSell = nullptr;
		App::s_Building.canBuild = false;

		return;
	} // == s_UIState.building

	if (App::s_UIState == UIState::upgrading)
	{
		App::Instance().TakeCoins(App::s_Building.towerToUpgradeOrSell->GetUpgradePrice());
		App::s_Building.towerToUpgradeOrSell->Upgrade();

		/*if (!App::s_Building.towerToUpgradeOrSell->CanUpgrade())
		{
			App::s_Building.buildingPlace.SetTexture(BuildingState::cantBuildTexture);
			App::s_Building.towerToUpgradeOrSell = nullptr;
			App::s_Building.canBuild = false;
		}*/

		return;
	}

	if (App::s_UIState == UIState::selling)
	{
		App::Instance().AddCoins(App::s_Building.towerToUpgradeOrSell->GetSellPrice());
		App::s_Building.towerToUpgradeOrSell->Destroy();

		App::s_Building.buildingPlace.SetTexture(BuildingState::cantBuildTexture);
		App::s_Building.towerToUpgradeOrSell = nullptr;
		App::s_Building.canBuild = false;

		App::s_Manager.ShrinkToFitTowers();

		return;
	}

	auto posX = std::floorf((App::s_Camera.x / static_cast<float>(m_ScaledTileSize)) + static_cast<float>(App::s_MouseX) / static_cast<float>(m_ScaledTileSize));
	auto posY = std::floorf((App::s_Camera.y / static_cast<float>(m_ScaledTileSize)) + static_cast<float>(App::s_MouseY) / static_cast<float>(m_ScaledTileSize));

	Tile *tile = GetTileFrom(posX, posY);
	if (!tile)
		return;

	Tower *tower = tile->GetTowerOccupying();
	if (!tower)
		return;

	if (m_HighlightedTower)
	{
		if (m_HighlightedTower == tower)
		{
			m_HighlightedTower->SetHighlight(false);
			m_HighlightedTower = nullptr;
		}
		else if (m_HighlightedTower != tower)
		{
			m_HighlightedTower->SetHighlight(false);
			m_HighlightedTower = tower;
			tower->SetHighlight(true);
		}
	}
	else
	{
		m_HighlightedTower = tower;
		tower->SetHighlight(true);
	}
}

// Unused at the moment, and not sure if it's needed also in future
//void Level::RMBEvent()
//{
//	
//}

void Level::InitWave()
{
	if (m_SpawnedEnemies >= m_ExpectedEnemiesAmount)
	{
		m_WaveProgress = WaveProgress::InProgress;
		return;
	}

	static std::uniform_int_distribution<std::size_t> spawnerDistr(0, m_Spawners.size() - 1);

	const Tile *spawner = m_Spawners.at(spawnerDistr(g_Rng));
	const Vector2D &spawnerPos = spawner->GetPos();

	const Vector2D spawnPos(spawnerPos.x / static_cast<float>(m_ScaledTileSize), spawnerPos.y / static_cast<float>(m_ScaledTileSize));

	EnemyType type = EnemyType::elf;
	// It might be as well casual variable defined in for loop, but maybe it's better to store it here
	// and use it after the for loop instead of casting the type to std::size_t
	std::size_t enemyTypeIterator = 0u;
	// for each enemy type
	for (; enemyTypeIterator < static_cast<std::size_t>(EnemyType::size); ++enemyTypeIterator)
	{
		// if currently iterated enemy type didn't reach still the expected amount of spawned enemies
		if (m_SpecificEnemiesAmount.at(enemyTypeIterator) < m_Waves.at(m_CurrentWave).container.at(enemyTypeIterator))
		{
			type = static_cast<EnemyType>(enemyTypeIterator);
			m_SpecificEnemiesAmount[enemyTypeIterator]++;
			break;
		}
	}

	auto enemy = AddEnemy(spawnPos.x, spawnPos.y, type, 2u);

	if (!enemy)
	{
		App::s_Logger.AddLog(std::format("Failed spawning enemy #{} (type: {})", m_SpawnedEnemies, enemyTypeIterator));
		m_SpecificEnemiesAmount[enemyTypeIterator]--;
		return;
	}
	
	m_NextSpawn = SDL_GetTicks() + Level::s_SpawnCooldown - g_PausedTicks;
	m_SpawnedEnemies++;
}

void Level::ManageWaves()
{
	switch (m_WaveProgress)
	{
	case WaveProgress::OnCooldown:
		if (SDL_TICKS_PASSED(SDL_GetTicks() - g_PausedTicks, m_WaveCooldown))
		{
			if (m_Spawners.empty())
			{
				App::s_Logger.AddLog(std::string_view("Level::InitWave: Initializing wave failed, due to missing spawners\n"));
				return;
			}

			m_SpawnedEnemies = 0u;
			m_ExpectedEnemiesAmount = 0u;
			for (const auto &i : m_Waves.at(m_CurrentWave).container)
			{
				m_ExpectedEnemiesAmount += i;
			}

			App::s_Manager.ReserveMemoryForWave(m_ExpectedEnemiesAmount);

			InitWave();

			App::UpdateWaves();

			// Make the timer zero, since it's not going to update by itself if time has already exceeded
			// Because it would reach signed value in UpdateTimer()
			App::s_UIElements.at(3).m_Label.UpdateText("0.000");

			m_WaveProgress = WaveProgress::Initializing;
			return;
		}

		UpdateTimer();
		return;
	case WaveProgress::Initializing:
		if (SDL_GetTicks() - g_PausedTicks >= m_NextSpawn)
			InitWave();
		return;
	case WaveProgress::InProgress:
		if (g_Enemies.empty())
		{
			m_WaveProgress = WaveProgress::Finished;
			m_SpecificEnemiesAmount = {};
			App::s_Manager.RecoveryMemoryAfterWave();
		}
		return;
	case WaveProgress::Finished:
		if (++m_CurrentWave >= m_Waves.size())
		{
			m_CurrentWave = 0u;
		}

		InitTimerForNextWave();

		m_WaveCooldown = SDL_GetTicks() - g_PausedTicks + waveCooldown;
		m_WaveProgress = WaveProgress::OnCooldown;
		return;
	}
}

void Level::InitTimerForNextWave()
{
	int32_t timerSeconds = waveCooldown / 1000;
	int32_t timerMilliseconds = waveCooldown - timerSeconds * 1000;

	App::s_UIElements.at(3).m_Label.UpdateText(std::format("{}.{}", timerSeconds, timerMilliseconds));
}

void Level::UpdateTimer() const
{
	auto howMuchLeft = m_WaveCooldown - (SDL_GetTicks() - g_PausedTicks);
	App::s_UIElements.at(3).m_Label.UpdateText(std::format("{}", howMuchLeft / 1000.0f));
}

void Level::Render()
{
	// Seems to be faster in this case than for-each
	for (std::size_t i = 0u; i < m_Layers.size(); ++i)
	{
		for (const auto &tile : m_Layers.at(i).m_DrawableTiles)
		{
			tile->Draw();
		}
	}

	if (m_HighlightedTower)
		m_HighlightedTower->DrawHighlight();

	for (const auto &enemy : g_Enemies)
		enemy->Draw();

	m_Base.Draw();

	// The tower triggers render for attackers
	for (const auto &tower : g_Towers)
	{
		if (!tower->IsActive())
			continue;

		tower->Draw();
	}

	App::s_Building.buildingPlace.Draw();

	for (const auto &projectile : g_Projectiles)
		projectile->Draw();
}

Tile *Level::GetTileFrom(uint32_t posX, uint32_t posY, uint16_t layer) const
{
	if (layer >= m_Layers.size())
	{
		App::s_Logger.AddLog(std::format("Requested a tile ({}, {}), but layer {} doesn't exist", posX, posY, layer));
		return nullptr;
	}

	if (posX >= m_MapData.at(0) || posY >= m_MapData.at(1))
		return nullptr;

	return m_Layers.at(layer).GetTileFrom(posX, posY);
}

#if ASYNC_TILES == 1
static std::mutex s_TilesMutex;

static void AdjustTilesToView(std::array<Layer, Level::s_LayersAmount> *layers)
{
	std::lock_guard<std::mutex> lock(s_TilesMutex);
	for (std::size_t i = 0u; i < layers->size(); ++i)
	{
		for (const auto &tile : layers->at(i).tiles)
		{
			tile->AdjustToView();
		}
	}
}
#endif

void Level::OnUpdateCamera()
{
#if ASYNC_TILES == 0
	for (std::size_t i = 0u; i < m_Layers.size(); ++i)
	{
		for (const auto &tile : m_Layers.at(i).m_DrawableTiles)
		{
			tile->AdjustToView();
		}
	}
#else
	m_Futures.push_back(std::async(std::launch::async, AdjustTilesToView, &m_Layers));
#endif

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