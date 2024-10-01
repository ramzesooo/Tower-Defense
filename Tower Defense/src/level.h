#pragma once
#include "entity/tile.h"
#include "entity/tower.h"
#include "entity/attacker.h"
#include "entity/enemy.h"
#include "entity/base.h"

#include <vector>
#include <memory>
#include <string>
#include <random>

constexpr uint16_t mapWidth = 70;
constexpr uint16_t mapHeight = 70;

class App;

//Layer references to just map's layer
struct Layer
{
	// returns a tile from specific coordinates
	Tile* GetTileFrom(std::size_t posX, std::size_t posY) const { return tiles.at(posY * mapWidth + posX); }

	std::vector<Tile*> tiles;
};

enum class WaveProgress
{
	OnCooldown,
	Initializing,
	InProgress,
	Finished
};

struct Wave
{
	WaveProgress waveProgress = WaveProgress::OnCooldown;
	uint16_t waveNumber = 0;
	uint16_t spawnedEnemies = 0;
	uint32_t waveCooldown = NULL;
};

// Level's map should contain 3 layers counting from zero
// Layer 0 is general layer and contains all tiles displayed at first place
// Layer 1 is layer of additional of tiles displayed right at the layer 0 without collisions
// Layer 2 contains tiles with collisions and any stuff like that, including spawners
// Level's map should contain at least 3 layers counting from zero
// Spawners' ID is 305
class Level
{
public:
	Level();
	//~Level();

	void Setup(std::ifstream& mapFile, uint16_t layerID);

	// Base is signed as regular tile
	void SetupBase(uint32_t posX, uint32_t posY);

	// Position passed in parameters referes to left-upper square
	// Towers take 4 tiles in this scheme:
	// [0][1]
	// [2][3]
	// 
	// Tower position depends on Vector2D and it is scaled by itself to tiles' size
	// So it should look like this: x: 1.0f, y: 2.0f, instead of x: 96.0f, y: 144.0f
	void AddTower(float posX, float posY, SDL_Texture* towerTexture, int32_t tier);
	void AddAttacker(Tower* assignedTower, AttackerType type, uint16_t scale = 2);
	Enemy* AddEnemy(float posX, float posY, EnemyType type, SDL_Texture* texture, uint16_t scale = 2);
	void AddProjectile(ProjectileType type, Attacker* projectileOwner, Enemy* target);

	void HandleMouseButtonEvent();

	void InitWave();
	void ManageWaves();

	void Render();

	uint16_t GetID() const { return m_LevelID; }
	void SetID(uint16_t levelID) { m_LevelID = levelID; }

	uint16_t GetLayersAmount() const { return (uint16_t)layers.size(); }

	bool DidLoadingFail() const { return m_FailedLoading; }

	Base* GetBase() { return &m_Base; }

	// Function GetTileFrom may return nullptr if asked tile is outside of map and/or doesn't exist
	Tile* GetTileFrom(uint32_t posX, uint32_t posY, uint16_t layer = 0) const;
	Tile* GetTileFrom(float posX, float posY, uint16_t layer = 0) const { return GetTileFrom((uint32_t)posX, (uint32_t)posY, layer); }

	// Adjust tiles' view to current camera
	void OnUpdateCamera();

	// NOTE: It's not used anywhere and probably it's not needed anymore.
	// Chunk can contain nullptr as a tile which means it's out of a map
	// For example when the entity is in pos (0, 0) and range equals to 1 then it also goes for (-1, -1)
	//std::vector<std::vector<Tile*>> GetChunkOf(Entity* entity, uint16_t range);

	const uint16_t m_MapSizeX = mapWidth;
	const uint16_t m_MapSizeY = mapHeight;
	const uint16_t m_MapScale = 2;
	const uint16_t m_TileSize = 24;
	const uint16_t m_ScaledTileSize = m_MapScale * m_TileSize;

	// Temporary level's config
	uint16_t m_Waves = 4;
	// enemiesPerWave are multiplied by current wave
	uint16_t m_EnemiesPerWave = 25;
	Vector2D m_BasePos{ 34, 34 };
private:
	bool m_FailedLoading = false;
	SDL_Texture* m_Texture = nullptr; // map tiles
	std::string_view m_BaseTextureID = "base";
	Base m_Base;
	uint16_t m_LevelID = 0;
	//std::vector<Layer> layers;
	std::array<Layer, 3> layers;
	/*std::vector<Entity*>& towers;
	std::vector<Entity*>& attackers;
	std::vector<Entity*>& enemies;
	std::vector<Entity*>& projectiles;*/
	std::vector<Tile*> spawners;

	Wave m_Wave;
};