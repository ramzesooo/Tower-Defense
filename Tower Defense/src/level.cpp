#include "level.h"
#include "app.h"

#include <fstream>
#include <sstream>
#include <cmath>

constexpr uint16_t spawnerID = 305;
constexpr uint16_t waveCooldown = 3500; // miliseconds

constexpr char configName[] = ".config";

extern std::vector<Entity*> &g_Projectiles;
extern std::vector<Entity*> &g_Towers;
extern std::vector<Entity*> &g_Attackers;
extern std::vector<Entity*> &g_Enemies;

extern std::default_random_engine g_Rng;

Level::Level(uint16_t levelID)
	: m_LevelID(levelID), m_Texture(App::s_Textures.GetTexture("mapSheet"))
{
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
			// first line of config must contain map width, height and scale
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

	m_ScaledTileSize = m_MapData.at(2) * s_TileSize;
}

void Level::Setup(std::ifstream& mapFile, uint16_t layerID)
{
	if (layerID < 0 || layerID >= layers.size())
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
	TileTypes tileType = (TileTypes)layerID;
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

	Layer* newLayer = &layers.at(layerID);
	newLayer->tiles.reserve(std::size_t(m_MapData.at(0) * m_MapData.at(1)));

	Tile* tile = nullptr;
	uint32_t srcX, srcY;
	uint32_t x, y;

	for (uint16_t i = 0; i < m_MapData.at(0) * m_MapData.at(1); i++)
	{
		x = i % m_MapData.at(0);
		y = i / m_MapData.at(1);
		tileCode = mapData.at(y).at(x);
		srcX = tileCode % 10;
		srcY = tileCode / 10;
		//tile = App::s_Manager.NewEntity<Tile>(srcX * m_TileSize, srcY * m_TileSize, x * m_ScaledTileSize, y * m_ScaledTileSize, m_TileSize, m_MapScale, m_Texture, tileType);
		tile = App::s_Manager.NewTile(srcX * s_TileSize, srcY * s_TileSize, x * m_ScaledTileSize, y * m_ScaledTileSize, s_TileSize, m_MapData[2], m_Texture, tileType);

		if (tile)
		{
			//tile->AddGroup(EntityGroup::tile);

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
	m_Base.m_MaxHP = m_Base.m_HP = 100;
	m_Base.m_Tile = GetTileFrom(posX, posY, 0);
	
	/*
	m_Base.m_RectHP.squareRect.x = App::s_Camera.w / 8;
	m_Base.m_RectHP.squareRect.y = App::s_Camera.h / 10;
	//m_Base.m_RectHP.squareRect.w = App::s_Camera.w / 3;
	m_Base.m_RectHP.squareRect.w = m_Base.m_RectHP.squareRect.x / 2;
	m_Base.m_RectHP.squareRect.h = App::s_Camera.h / 20;

	m_Base.m_RectHP.barRect = m_Base.m_RectHP.squareRect;
	m_Base.m_RectHP.barRect.w = std::fabs(m_Base.m_RectHP.squareRect.w / 100 * (-m_Base.m_HPPercent)); */
	
	m_Base.m_RectHP.labelHP = App::s_Manager.NewEntity<Label>(0, 0, "-0", App::s_Textures.GetFont("baseHealth"), SDL_Color(255, 255, 255, 255));
	m_Base.m_RectHP.labelHP->AddGroup(EntityGroup::label);
	m_Base.m_RectHP.labelHP->UpdateText(std::to_string((int32_t)m_Base.m_HPPercent) + "%");
	//float HPBarX = m_Base.m_RectHP.squareRect.x + (m_Base.m_RectHP.squareRect.w / 2.0f) - (float)m_Base.m_RectHP.labelHP->GetRect().w / 2.0f;
	//float HPBarY = m_Base.m_RectHP.barRect.y + (m_Base.m_RectHP.barRect.h - m_Base.m_RectHP.labelHP->GetRect().h);
	//m_Base.m_RectHP.labelHP->UpdatePos(Vector2D(HPBarX, HPBarY));

	App::s_Logger.AddLog("Created base (", false);
	App::s_Logger.AddLog(std::to_string(scaledPosX) + ", " + std::to_string(scaledPosY) + ")");
}

Tower* Level::AddTower(float posX, float posY, SDL_Texture* towerTexture, uint16_t tier)
{
	if (!towerTexture)
	{
		App::s_Logger.AddLog("Level::AddTower: Tower's texture doesn't exist!");
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
		App::s_Logger.AddLog("Tried to add attacker to not existing tower or an attacker for the specific tower already exists.");
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

Enemy* Level::AddEnemy(float posX, float posY, EnemyType type, SDL_Texture* texture, uint16_t scale)
{
	auto enemy = App::s_Manager.NewEntity<Enemy>(posX, posY, type, texture, scale);
	enemy->AddGroup(EntityGroup::enemy);

#ifdef DEBUG
	App::s_EnemiesAmountLabel->UpdateText("Enemies: " + std::to_string(g_Enemies.size()));
#endif
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

	//static std::default_random_engine rng(App::s_Rnd());
	static std::uniform_int_distribution<std::size_t> spawnerDistr(0, spawners.size() - 1);

	Tile* spawner = spawners.at(spawnerDistr(g_Rng));

	Vector2D spawnPos((spawner->GetPos().x / m_ScaledTileSize), spawner->GetPos().y / m_ScaledTileSize);
	Vector2D dest = Vector2D(m_BasePos.x, m_BasePos.y);
	Vector2D moveVector;

	EnemyType type = EnemyType::elf;
	//for (auto i = 0u; i < m_Wave.spawnedSpecificEnemies.size(); ++i)
	/*for (std::size_t i = 0u; i < (std::size_t)EnemyType::size; ++i)
	{
		if (m_Wave.spawnedSpecificEnemies[i] == m_SpecificEnemiesAmount[i])
			continue;

		type = (EnemyType)i;
	}*/
	for (std::size_t i = 0u; i < (std::size_t)EnemyType::size; ++i)
	{
		if (m_SpecificEnemiesAmount[i] == m_Waves.at(m_CurrentWave)[i])
			continue;

		type = (EnemyType)i;
	}

	auto enemy = AddEnemy(spawnPos.x, spawnPos.y, type, App::s_Textures.GetTexture(App::TextureOf(type)), 2);
	m_SpecificEnemiesAmount[(std::size_t)type]++;
	//++m_Wave.spawnedSpecificEnemies[(std::size_t)type];

	moveVector.x = dest.x - spawnPos.x;
	moveVector.y = dest.y - spawnPos.y;

	enemy->Move(moveVector);

	if (g_Enemies.size() == m_ExpectedEnemiesAmount)
		m_WaveProgress = WaveProgress::InProgress;
}

void Level::ManageWaves()
{
	switch (m_WaveProgress)
	{
	case WaveProgress::OnCooldown:
		if (SDL_TICKS_PASSED(SDL_GetTicks(), m_WaveCooldown))
		{
			m_ExpectedEnemiesAmount = 0;
			for (const auto &i : m_Waves.at(m_CurrentWave))
				m_ExpectedEnemiesAmount += i;

			InitWave();
			m_WaveProgress = WaveProgress::Initializing;
		}
		return;
	case WaveProgress::Initializing:
		InitWave();
		return;
	case WaveProgress::InProgress:
		if (g_Enemies.size() == 0)
		{
			m_WaveProgress = WaveProgress::Finished;
			m_SpecificEnemiesAmount = { 0, 0 }; // CHANGE THIS IF ADDING OR REMOVING ENEMY TYPE
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
	for (uint16_t i = 0; i < layers.size(); ++i)
	{
		for (const auto &tile : layers.at(i).tiles)
		{
			if (!tile)
				continue;

			// It's more expensive than rendering all tiles
			
			//Vector2D tilePos = tile->GetPos();
			//if ((tilePos.x + tile->GetWidth() < App::s_Camera.x && tilePos.y + tile->GetHeight() < App::s_Camera.y)
			//	|| (tilePos.x > App::s_Camera.x + App::s_Camera.w && tilePos.y > App::s_Camera.y + App::s_Camera.h))
			//	continue;

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
	if (layer < 0 || layer >= layers.size())
	{
		App::s_Logger.AddLog("Requested a tile from " + std::to_string(posX) + ", " + std::to_string(posY), false);
		App::s_Logger.AddLog(", but layer " + std::to_string(layer) + " doesn't exist");
		return nullptr;
	}

	if (posX < 0 || posX >= m_MapData.at(0) || posY < 0 || posY >= m_MapData.at(1))
		return nullptr;

	return layers.at(layer).GetTileFrom(posX, posY, m_MapData.at(0));
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

	for (const auto &e : g_Enemies)
	{
		e->AdjustToView();
	}

	for (const auto &t : g_Towers)
	{
		// Towers trigger method AdjustToView() for attackers by themselves
		t->AdjustToView();
	}

	for (const auto &p : g_Projectiles)
	{
		p->AdjustToView();
	}
}