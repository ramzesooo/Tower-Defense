#include "level.h"

#include "UI.h"
#include "app.h"
#include "entity/enemy.h"
#include "entity/label.h"
#include "entity/towers/towers_inc.h"
#include "entity/projectiles/projectiles_inc.h"

#include <fstream>
#include <sstream>
#include <cmath>
#include <format>
#include <unordered_map>
#include <queue>

static uint32_t expenseOfX = 1;
static uint32_t expenseOfY = 1;

static std::vector<Vector2D> findPath(const Vector2D &start, const Vector2D &goal)
{
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
			uint32_t dx = totalDistance + expenseOfX;
			uint32_t dy = totalDistance + expenseOfY;
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
			if (!App::s_CurrentLevel->IsTileWalkable(neighbor.pos))
				continue;

			// std::unordered_map::emplace will not insert a new element into the map if one already exists
			// in that case, success is false, but we always get an iterator to the
			// new/already existing element in the map
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
static constexpr char mapDataStr[] = "mapData=";
static constexpr char basePosStr[] = "basePos=";
static constexpr char movementRateStr[] = "movementRate=";
static constexpr char wavesStr[] = "waves=";

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

	bool readWaves = false;

	while (std::getline(configFile, line))
	{
		std::string value;

		if (line.find(wavesStr) != std::string::npos || line.find("}") != std::string::npos)
		{
			readWaves = !readWaves;
			continue;
		}

		if (readWaves)
		{
			std::stringstream ss(line);

			WavesArray container{};

			for (auto &wave : container)
			{
				if (!std::getline(ss, value, ','))
					break;

				wave = static_cast<uint16_t>(std::stoi(value));
			}

			m_Waves.emplace_back(container);

			continue;
		}

		std::stringstream ss;

		if (line.find(mapDataStr) != std::string::npos)
		{
			line.erase(line.begin(), line.begin() + strlen(mapDataStr));

			ss << line;

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
		else if (line.find(basePosStr) != std::string::npos)
		{
			line.erase(line.begin(), line.begin() + strlen(basePosStr));

			ss << line;

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
		else if (line.find(movementRateStr) != std::string::npos)
		{
			line.erase(line.begin(), line.begin() + strlen(movementRateStr));

			ss << line;

			if (!std::getline(ss, value, ','))
			{
				App::s_Logger.AddLog(std::string_view("Couldn't reach out movement speed rate from config file!"));
				m_MovementSpeedRate = 1u;
				continue;
			}

			m_MovementSpeedRate = static_cast<uint16_t>(std::stoul(value));
			IF_DEBUG(App::s_Logger.AddLog(std::format("Movement speed rate for level #{}: x{}", m_LevelID + 1, m_MovementSpeedRate)););
			continue;
		}
	}

	m_Waves.shrink_to_fit();

	m_ScaledTileSize = m_MapData[2] * s_TileSize;

	m_MapData[3] = m_MapData[0] * m_ScaledTileSize;
	m_MapData[4] = m_MapData[1] * m_ScaledTileSize;
}

void Level::Init()
{
	std::string mapFilePath;
	for (uint16_t i = 0u; i < Level::s_LayersAmount; i++)
	{
		mapFilePath = std::format("levels/{}/map_layer{}.map", GetID() + 1u, i);
		std::ifstream mapFile(mapFilePath);

		if (i >= m_Layers.size())
		{
			App::s_Logger.AddLog(std::format("Failed to load level {}: Layer {} doesn't exist", m_LevelID + 1, i));
			break;
		}

		if (mapFile.fail())
		{
			App::s_Logger.AddLog(std::format("Failed to load level {}: File \"{}\" doesn't exist", m_LevelID + 1, mapFilePath));
			continue;
		}

		SetupLayer(mapFile, i);
	}

	SetupBase();

	InitTimerForNextWave();

	m_WaveProgress = WaveProgress::LoadedLevel;
}

void Level::SetupLayer(std::ifstream &mapFile, uint16_t layerID)
{
	App::s_Logger.AddLog(std::format("Loading layer #{} (Level: #{})", layerID, m_LevelID + 1));

	// This code can be improved
	std::string line;
	std::vector<std::vector<int32_t>> mapData;

	while (std::getline(mapFile, line))
	{
		std::istringstream ss(line);
		std::vector<int32_t> row;
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

	const uint16_t mapSize = m_MapData[0] * m_MapData[1];

	Layer *newLayer = &m_Layers.at(layerID);
	newLayer->m_Tiles.reserve(static_cast<size_t>(mapSize));
	newLayer->m_DrawableTiles.reserve(static_cast<size_t>(mapSize));

	Tile *tile = nullptr;
	int32_t srcX, srcY;
	int32_t x, y;

	constexpr int32_t tilesetWidth = 10;

	for (uint16_t i = 0u; i < mapSize; i++)
	{
		x = i % m_MapData[0];
		y = i / m_MapData[1];
		tileCode = mapData.at(y).at(x);
		srcX = tileCode % tilesetWidth;
		srcY = tileCode / tilesetWidth;

		newLayer->m_Tiles.emplace_back(srcX * s_TileSize, srcY * s_TileSize, x * m_ScaledTileSize, y * m_ScaledTileSize, s_Texture, tileType);
		tile = &newLayer->m_Tiles.back();

		if (layerID == 2)
		{
			if (tileCode == pathID)
			{
				tile->SetWalkable();
			}
			else if (tileCode == spawnerID)
			{
				m_Spawners.reserve(m_Spawners.size() + 1);
				m_Spawners.emplace_back(tile);
				newLayer->SetTileDrawable(tile, true);
				continue;
			}

			newLayer->SetTileDrawable(tile, false);

			continue;
		}

		newLayer->SetTileDrawable(tile, true);
	}

	newLayer->m_DrawableTiles.shrink_to_fit();

	mapFile.close();
}

void Level::SetupBase()
{
	int32_t scaledPosX = static_cast<int32_t>(m_BasePos.x) * m_ScaledTileSize;
	int32_t scaledPosY = static_cast<int32_t>(m_BasePos.y) * m_ScaledTileSize;
	m_Base.m_Texture = App::s_Textures.GetTexture(m_BaseTextureID);
	m_Base.destRect = { scaledPosX, scaledPosY, m_ScaledTileSize, m_ScaledTileSize };
	m_Base.m_Pos = { static_cast<float>(scaledPosX), static_cast<float>(scaledPosY) };
	m_Base.m_MaxLifes = m_Base.m_Lifes = 5u;
	m_Base.m_Tile = GetTileFrom(m_BasePos.x, m_BasePos.y, 0);

	App::s_Logger.AddLog(std::format("Created base ({}, {})", scaledPosX, scaledPosY));
}

void Level::Clear()
{
	App::s_Manager.DestroyAllEntities();

	for (auto &layer : m_Layers)
	{
		layer.m_DrawableTiles.clear();
		layer.m_Tiles.clear();
	}

	m_Spawners.clear();

	m_WaveCooldown = 0u;
	m_CurrentWave = 0u;
	m_WaveProgress = WaveProgress::OnCooldown;
	m_SpecificEnemiesAmount = {};

	App::s_Manager.Refresh();
}

void Level::Lost()
{
	m_Base.m_Lifes = 0u;
	SetBaseActive(false);
	Clear();

	App::Instance().SetUIState(UIState::mainMenu);

	// Check if the cursor is already pointing at any button
	App::s_MainMenu.OnCursorMove();
}

void Level::AddTower(float posX, float posY, TowerType type) const
{
	Tower *tower = nullptr;

	switch (type)
	{
	case TowerType::classic:
		tower = App::s_Manager.NewEntity<ClassicTower>(posX, posY);
		break;
	case TowerType::dark:
		tower = App::s_Manager.NewEntity<DarkTower>(posX, posY);
		break;
	default:
		App::s_Logger.AddLog(std::format("Level::AddTower: TowerType {} is invalid", static_cast<std::size_t>(type)));
		break;
	}

	if (!tower)
	{
		App::s_Logger.AddLog(std::format("Level::AddTower: Failed adding a tower (type #{}) ({}, {})", static_cast<std::size_t>(type), posX, posY));
		return;
	}
}

void Level::AddAttacker(Tower *assignedTower, AttackerType type, uint16_t scale)
{
	if (!assignedTower || assignedTower->GetAttacker() != nullptr)
	{
		App::s_Logger.AddLog(std::string_view("Level::AddAttacker: Tried to add attacker to non-existing tower or an attacker for the specific tower already exists."));
		return;
	}

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
		attacker = App::s_Manager.NewEntity<DarkAttacker>(assignedTower, shotCooldown, scale);
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
	return App::s_Manager.NewEntity<Enemy>(posX, posY, type, scale);
}

void Level::AddProjectile(ProjectileType type, Attacker *projectileOwner, Enemy *target)
{
	Projectile *projectile = nullptr;
	
	switch (type)
	{
	case ProjectileType::arrow:
		projectile = App::s_Manager.NewEntity<ArrowProjectile>(projectileOwner, target);
		break;
	case ProjectileType::thunder:
		projectile = App::s_Manager.NewEntity<ThunderProjectile>(projectileOwner, target);
		break;
	default:
		App::s_Logger.AddLog(std::format("Level::AddProjectile: ProjectileType {} is invalid", static_cast<std::size_t>(type)));
		return;
	}

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

	auto &element = App::s_ExpandingTowers.at(static_cast<std::size_t>(UIElement::s_ChosenTower));

	// Check if mouse pressed on the already chosen tower
	if (App::s_MouseX >= element.destRect.x && App::s_MouseX <= element.destRect.x + element.destRect.w
		&& App::s_MouseY >= element.destRect.y && App::s_MouseY <= element.destRect.y + element.destRect.h)
	{
		// Do nothing, avoids unnecessary loop
		return;
	}

	// Check if any tower has been pressed if there isn't any already
	for (std::size_t i = 0u; i < Tower::s_TowerTypeSize; i++)
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

	switch (App::s_UIState)
	{
	case UIState::building:
		if (!App::s_Building.canBuild)
			return;

		App::Instance().TakeCoins(Level::GetBuildPrice(UIElement::s_ChosenTower));
		AddTower(App::s_Building.coordinates.x, App::s_Building.coordinates.y, UIElement::s_ChosenTower);

		App::SetCantBuild();
		return;
	case UIState::upgrading:
		if (!App::s_Building.canBuild)
			return;

		App::Instance().TakeCoins(App::s_Building.towerToUpgradeOrSell->GetUpgradePrice());
		App::s_Building.towerToUpgradeOrSell->Upgrade();
		return;
	case UIState::selling:
		if (!App::s_Building.canBuild)
			return;

		App::Instance().AddCoins(App::s_Building.towerToUpgradeOrSell->GetSellPrice());
		App::s_Building.towerToUpgradeOrSell->Destroy();

		App::s_Manager.RefreshTowersAfterSell(App::s_Building.towerToUpgradeOrSell);

		App::SetCantBuild();
		return;
	default: // go further if not in building state
		break;
	}

	HighlightTower();
}

void Level::HighlightTower()
{
	// Get current mouse's position
	Vector2D pointedPos(
		(App::s_Camera.x / static_cast<float>(m_ScaledTileSize)) + static_cast<float>(App::s_MouseX) / static_cast<float>(m_ScaledTileSize),
		(App::s_Camera.y / static_cast<float>(m_ScaledTileSize)) + static_cast<float>(App::s_MouseY) / static_cast<float>(m_ScaledTileSize)
	);

	pointedPos.Floorf();

	// Return if couldn't get any tile pointed by mouse
	Tile* tile = GetTileFrom(pointedPos.x, pointedPos.y);
	if (!tile)
		return;

	// Return if there isn't any tower on pointed tile
	Tower* tower = tile->GetTowerOccupying();
	if (!tower)
		return;

	if (m_HighlightedTower)
	{
		if (m_HighlightedTower == tower)
		{
			m_HighlightedTower->SetHighlight(false);
			m_HighlightedTower = nullptr;
			return;
		}

		m_HighlightedTower->SetHighlight(false);
		m_HighlightedTower = tower;
		tower->SetHighlight(true);
		return;
	}

	m_HighlightedTower = tower;
	tower->SetHighlight(true);
}

void Level::InitWave()
{
	if (m_SpawnedEnemies >= m_ExpectedEnemiesAmount)
	{
		m_WaveProgress = WaveProgress::InProgress;
		return;
	}

	static std::uniform_int_distribution<std::size_t> spawnerDistr(0, m_Spawners.size() - 1);

	// Check if it stays the same, since it's static and spawners amount might be different for different level
	if (spawnerDistr.b() != m_Spawners.size() - 1)
	{
		// If level has changed as well as m_Spawners.size(), then adjust new size
		spawnerDistr.param(std::uniform_int_distribution<std::size_t>::param_type(0, m_Spawners.size() - 1));
	}

	const Tile *spawner = m_Spawners[spawnerDistr(g_Rng)];

	const Vector2D spawnPos(Vector2D(spawner->GetPos()) / static_cast<float>(m_ScaledTileSize));

	EnemyType type = EnemyType::elf;
	// It might be as well casual variable defined in for loop, but maybe it's better to store it here
	// and use it after the for loop instead of casting the type to std::size_t
	std::size_t enemyTypeIterator = 0u;
	// for each enemy type
	for (; enemyTypeIterator < static_cast<std::size_t>(EnemyType::size); ++enemyTypeIterator)
	{
		// if currently iterated enemy type didn't reach still the expected amount of spawned enemies
		//if (m_SpecificEnemiesAmount.at(enemyTypeIterator) < m_Waves.at(m_CurrentWave).at(enemyTypeIterator))
		if (m_SpecificEnemiesAmount[enemyTypeIterator] < m_Waves.at(m_CurrentWave)[enemyTypeIterator])
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

	enemy->AddToGroup(EntityGroup::enemy);

	IF_DEBUG(App::s_EnemiesAmountLabel.UpdateText(std::format("Enemies: {}", g_Enemies.size())););

	// Check whether X is more expensive for enemy's movement than Y
	// Basically if there is more difference between enemy's X and base's X than in Y-axis,
	// Then just assign appropriate values
	if (std::fabsf(m_BasePos.x - spawnPos.x) > std::fabsf(m_BasePos.y - spawnPos.y))
	{
		// Enemy should focus on moving on X-axis, so Y is more expensive
		expenseOfX = 1;
		expenseOfY = 2;
	}
	else
	{
		// Enemy should focus on moving on Y-axis, so X is more expensive
		expenseOfX = 2;
		expenseOfY = 1;
	}

	enemy->SetPath(findPath({ spawnPos.x, spawnPos.y }, m_BasePos));
	
	m_NextSpawn = SDL_GetTicks() + Level::s_SpawnCooldown - g_PausedTicks;
	m_SpawnedEnemies++;
}

void Level::ManageWaves()
{
	switch (m_WaveProgress)
	{
		// Assigning WaveProgress::OnCooldown directly in Init()
		// Kind of fucks up the timer, so it should be assigned when everything is initialized
		// So now, basically App::Update() assigns correct cooldown by itself
	case WaveProgress::LoadedLevel:
		m_WaveCooldown = SDL_GetTicks() - g_PausedTicks + waveCooldown;
		m_WaveProgress = WaveProgress::OnCooldown;
		return;
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
			for (const auto &i : m_Waves.at(m_CurrentWave))
			{
				m_ExpectedEnemiesAmount += i;
			}

			App::s_Manager.ReserveMemoryForWave(m_ExpectedEnemiesAmount);

			InitWave();

			App::UpdateWaves();

			// Make the timer zero, since it's not going to update by itself if time has been already exceeded
			// Because it would reach negative value in UpdateTimer()
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
	// (m_WaveCooldown - (SDL_GetTicks() - g_PausedTicks)) / 1000.0f
	// Calculates left time to start wave
	// 
	// I have no idea if std::format is better than std::to_string, but for sure it is here
	// Since it ignores the zeros at the end by default
	App::s_UIElements.at(3).m_Label.UpdateText(std::format("{}", (m_WaveCooldown - (SDL_GetTicks() - g_PausedTicks)) / 1000.0f));

	// This is also very expensive, so it would be cool making it another way than allocating new string every frame
}

void Level::Render() const
{
	for (std::size_t i = 0u; i < m_Layers.size(); ++i)
	{
		for (const auto &tile : m_Layers[i].m_DrawableTiles)
		{
			tile->Draw();
		}
	}

	if (m_HighlightedTower)
		m_HighlightedTower->DrawHighlight();

	for (const auto &enemy : g_Enemies)
	{
		enemy->Draw();
	}

	m_Base.Draw();

	// The tower triggers render for attackers
	for (const auto &tower : g_Towers)
	{
		tower->Draw();
	}

	App::s_Building.buildingPlace.Draw();

	for (const auto &projectile : g_Projectiles)
	{
		projectile->Draw();
	}
}

Tile *Level::GetTileFrom(uint32_t posX, uint32_t posY, uint16_t layer)
{
	if (layer >= m_Layers.size())
	{
		App::s_Logger.AddLog(std::format("Requested a tile ({}, {}), but layer {} doesn't exist", posX, posY, layer));
		return nullptr;
	}

	if (posX >= m_MapData[0] || posY >= m_MapData[1])
		return nullptr;

	// At the beginning there is already if statement and layer is unsigned int, so we don't have to use .at()
	return m_Layers[layer].GetTileFrom(posX, posY);
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