#include "level.h"
#include "app.h"

#include <fstream>
#include <sstream>

constexpr uint16_t spawnerID = 305;
constexpr uint16_t waveCooldown = 3500; // miliseconds

extern std::vector<Entity*>& towers;
extern std::vector<Entity*>& projectiles;
extern std::vector<Entity*>& enemies;
extern std::vector<Entity*>& attackers;

Level::Level()
	: m_Wave{ WaveProgress::OnCooldown, 1, 0, NULL }, m_Texture(App::s_Textures.GetTexture("mapSheet"))
{}

/*Level::Level() 
	: towers(App::s_Manager.GetGroup(EntityGroup::tower)), attackers(App::s_Manager.GetGroup(EntityGroup::attacker)), enemies(App::s_Manager.GetGroup(EntityGroup::enemy)),
	projectiles(App::s_Manager.GetGroup(EntityGroup::projectile)),
	m_Wave{ WaveProgress::OnCooldown, 1, 0, NULL }, m_Texture(App::s_Textures.GetTexture("mapSheet"))
{}*/

//Level::~Level()
//{
//	for (const auto& enemy : enemies)
//	{
//		static_cast<Enemy*>(enemy)->GetOccupiedTile()->SetOccupyingEntity(nullptr);
//		static_cast<Enemy*>(enemy)->SetOccupiedTile(nullptr);
//		enemy->Destroy();
//	}
//
//	for (const auto& tower : towers)
//	{
//		tower->Destroy();
//	}
//
//	m_BaseTile->Destroy();
//}

void Level::Setup(std::ifstream& mapFile, uint16_t layerID)
{
	if (mapFile.fail())
	{
		App::s_Logger.AddLog("Failed to load level ", false);
		App::s_Logger.AddLog(std::to_string(m_LevelID + 1));
		return;
	}
	else
	{
		App::s_Logger.AddLog("Loading level ", false);
		App::s_Logger.AddLog(std::to_string(m_LevelID + 1), false);
		App::s_Logger.AddLog(" (Layer: ", false);
		App::s_Logger.AddLog(std::to_string(layerID), false);
		App::s_Logger.AddLog(")");
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

	Layer* newLayer = &layers.at(layerID);
	newLayer->tiles.reserve(std::size_t(m_MapSizeX * m_MapSizeY));

	Tile* tile = nullptr;
	uint32_t srcX, srcY;
	uint32_t x, y;

	for (uint16_t i = 0; i < m_MapSizeX * m_MapSizeY; i++)
	{
		x = i % m_MapSizeX;
		y = i / m_MapSizeY;
		tileCode = mapData.at(y).at(x);
		srcX = tileCode % 10;
		srcY = tileCode / 10;
		tile = App::s_Manager.NewEntity<Tile>(srcX * m_TileSize, srcY * m_TileSize, x * m_ScaledTileSize, y * m_ScaledTileSize, m_TileSize, m_MapScale, m_Texture, tileType);

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
			App::s_Logger.AddLog("Couldn't load a tile (", false);
			App::s_Logger.AddLog(std::to_string(x * m_ScaledTileSize) + ", ", false);
			App::s_Logger.AddLog(std::to_string(y * m_ScaledTileSize) + ")");
		}

		newLayer->tiles.emplace_back(tile);
	}

	mapFile.close();
}

void Level::SetupBase(uint32_t posX, uint32_t posY)
{
	int32_t scaledPosX = posX * m_ScaledTileSize;
	int32_t scaledPosY = posY * m_ScaledTileSize;
	m_Base.m_Texture = App::s_Textures.GetTexture(m_BaseTextureID);
	m_Base.destRect = { scaledPosX, scaledPosY, m_Base.destRect.w * 2, m_Base.destRect.h * 2 };
	m_Base.m_Pos = { (float)scaledPosX, (float)scaledPosY };
	m_Base.m_HP = 100;
	m_Base.m_Tile = GetTileFrom(posX, posY, 0);

	App::s_Logger.AddLog("Created base (", false);
	App::s_Logger.AddLog(std::to_string(scaledPosX), false);
	App::s_Logger.AddLog(", ", false);
	App::s_Logger.AddLog(std::to_string(scaledPosY), false);
	App::s_Logger.AddLog(")");
}

void Level::AddTower(float posX, float posY, SDL_Texture* towerTexture, int32_t tier)
{
	if (!towerTexture)
	{
		App::s_Logger.AddLog("Level::AddTower: Tower's texture doesn't exist!");
		return;
	}

	auto tower = App::s_Manager.NewEntity<Tower>(posX, posY, towerTexture, tier);
	tower->AddGroup(EntityGroup::tower);

	AddAttacker(tower, (AttackerType)(tier - 1));
}

void Level::AddAttacker(Tower* assignedTower, AttackerType type, uint16_t scale)
{
	if (!assignedTower || assignedTower->GetAttacker())
	{
		App::s_Logger.AddLog("Tried to add attacker to not existing tower or an attacker for the specific tower already exists.");
		return;
	}

	auto attacker = App::s_Manager.NewEntity<Attacker>(assignedTower, type, App::s_Textures.GetTexture(App::TextureOf(type)), scale);
	attacker->AddGroup(EntityGroup::attacker);
	assignedTower->AssignAttacker(attacker);
}

Enemy* Level::AddEnemy(float posX, float posY, EnemyType type, SDL_Texture* texture, uint16_t scale)
{
	auto enemy = App::s_Manager.NewEntity<Enemy>(posX, posY, type, texture, scale);
	enemy->AddGroup(EntityGroup::enemy);
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
	if (App::s_Event.button.type == SDL_MOUSEBUTTONDOWN)
	{
		if (App::s_UIState == UIState::building)
		{
			if (!App::s_Building.m_CanBuild)
				return;

			AddTower(App::s_Building.m_Coordinates.x, App::s_Building.m_Coordinates.y, App::s_Textures.GetTexture("tower"), 2);
			App::s_Building.m_BuildingPlace->SetTexture(App::s_Textures.GetTexture("cantBuild"));
		}
	}
	else if (App::s_Event.button.type == SDL_MOUSEBUTTONUP)
	{

	}
}

void Level::InitWave()
{
	if (spawners.empty())
	{
		App::s_Logger.AddLog("Level::InitWave: Initializing wave failed, due to missing spawners\n");
		return;
	}

	static std::default_random_engine rng(App::s_Rnd());
	static std::uniform_int_distribution<std::size_t> spawnerDistr(0, spawners.size() - 1);

	Tile* spawner = spawners.at(spawnerDistr(rng));

	Vector2D spawnPos((spawner->GetPos().x / m_ScaledTileSize), spawner->GetPos().y / m_ScaledTileSize);
	Vector2D dest = Vector2D(m_BasePos.x, m_BasePos.y);
	Vector2D moveVector(0.0f, 0.0f);

	auto enemy = AddEnemy(spawnPos.x, spawnPos.y, EnemyType::elf, App::s_Textures.GetTexture(App::TextureOf(EnemyType::elf)), 2);

	moveVector.x = dest.x - spawnPos.x;
	moveVector.y = dest.y - spawnPos.y;

	enemy->Move(moveVector);

	if (++m_Wave.spawnedEnemies >= m_EnemiesPerWave * m_Wave.waveNumber)
	{
		m_Wave.waveProgress = WaveProgress::InProgress;
	}
}

void Level::ManageWaves()
{
	switch (m_Wave.waveProgress)
	{
	case WaveProgress::OnCooldown:
		if (SDL_TICKS_PASSED(SDL_GetTicks(), m_Wave.waveCooldown))
		{
			InitWave();
			m_Wave.waveProgress = WaveProgress::Initializing;
		}
		return;
	case WaveProgress::Initializing:
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
		m_Wave.waveNumber++;
		if (m_Wave.waveNumber > m_Waves)
		{
			m_Wave.waveNumber = 1;
		}
		m_Wave.waveCooldown = SDL_GetTicks() + waveCooldown;
		m_Wave.waveProgress = WaveProgress::OnCooldown;
		return;
	}
}

void Level::Render()
{
	for (uint16_t i = 0; i < 3; ++i)
	{
		for (const auto &tile : layers.at(i).tiles)
		{
			if (!tile)
			{
				continue;
			}

			tile->Draw();
		}
	}

	m_Base.Draw();

	for (const auto& enemy : enemies)
	{
		enemy->Draw();
	}

	if (App::s_UIState == UIState::building)
	{
		App::s_Building.m_BuildingPlace->Draw();
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
		App::s_Logger.AddLog("Requested a tile from " + std::to_string(posX) + ", " + std::to_string(posY), false);
		App::s_Logger.AddLog(", but layer " + std::to_string(layer) + " doesn't exist");
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
	for (uint16_t i = 0; i < 3; ++i)
	{
		for (const auto &tile : layers.at(i).tiles)
		{
			if (!tile)
			{
				continue;
			}

			tile->AdjustToView();
		}
	}

	m_Base.AdjustToView();

	for (const auto &e : enemies)
	{
		e->AdjustToView();
	}

	for (const auto &t : towers)
	{
		// Towers trigger method AdjustToView() for attackers by themselves
		t->AdjustToView();
	}

	for (const auto &p : projectiles)
	{
		p->AdjustToView();
	}
}