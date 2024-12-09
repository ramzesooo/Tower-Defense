#pragma once
#include "entity/tile.h"
#include "entity/tower.h"
#include "entity/attacker.h"
#include "entity/enemy.h"
#include "entity/base.h"

#include <vector>

class App;

//Layer references to just map's layer
struct Layer
{
	// returns a tile from specific coordinates
	Tile *GetTileFrom(std::size_t posX, std::size_t posY, uint16_t mapWidth) const { return tiles.at(posY * mapWidth + posX); }

	std::vector<Tile*> tiles;
};

enum class WaveProgress
{
	OnCooldown,
	Initializing,
	InProgress,
	Finished
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
	static constexpr uint16_t s_LayersAmount = 3;
	static constexpr uint16_t s_TileSize = 24;
	static constexpr uint16_t s_SpawnCooldown = 150; // how much time has to pass before spawning next enemy

	static SDL_Texture *s_Texture; // map tiles

	std::array<uint16_t, 5> m_MapData{}; // 0 = width (tiles amount), 1 = height (tiles amount), 2 = scale, 3 = width in pixels, 4 = height in pixels
	uint16_t m_ScaledTileSize = 0;

	Vector2D m_BasePos;
	uint16_t m_MovementSpeedRate = 1;
public:
	Level(uint16_t levelID);
	~Level() = default;

	void Setup(std::ifstream& mapFile, uint16_t layerID);

	// Base is signed as a regular tile
	void SetupBase(uint32_t posX, uint32_t posY);

	void Clean();

	// All methods adding entities (such as tower, attacker and enemy) call AddGroup inside of them

	// Position passed in parameters referes to left-upper square
	// Towers take 4 tiles in this scheme:
	// [0][1]
	// [2][3]
	// 
	// Tower position depends on Vector2D and it is scaled by itself to tiles' size
	// So it should look like this: x: 1.0f, y: 2.0f, instead of x: 96.0f, y: 144.0f
	Tower* AddTower(float posX, float posY, SDL_Texture* towerTexture, uint16_t tier);
	void AddAttacker(Tower* assignedTower, AttackerType type, uint16_t scale = 2);
	Enemy* AddEnemy(float posX, float posY, EnemyType type, SDL_Texture* texture, uint16_t scale = 2) const;
	void AddProjectile(ProjectileType type, Attacker* projectileOwner, Enemy* target);

	void HandleMouseButtonEvent();

	void InitWave();
	void ManageWaves();

	void Render();

	uint16_t GetID() const { return m_LevelID; }

	bool HasLoadingFailed() const { return m_FailedLoading; }

	Base* GetBase() { return &m_Base; }

	// Function GetTileFrom may return nullptr if asked tile is outside of map and/or doesn't exist
	Tile *GetTileFrom(uint32_t posX, uint32_t posY, uint16_t layer = 0) const;
	Tile *GetTileFrom(float posX, float posY, uint16_t layer = 0) const { return GetTileFrom((uint32_t)posX, (uint32_t)posY, layer); }

	// Adjust tiles' view to current camera
	void OnUpdateCamera();

	std::size_t GetWavesAmount() const { return m_Waves.size(); }
	std::size_t GetCurrentWave() const { return m_CurrentWave; }

	// NOTE: It's not used anywhere and probably it's not needed anymore.
	// Chunk can contain nullptr as a tile which means it's out of a map
	// For example when the entity is in pos (0, 0) and range equals to 1 then it also goes for (-1, -1)
	//std::vector<std::vector<Tile*>> GetChunkOf(Entity* entity, uint16_t range);

	// The method takes origin of tile, for example 1, 1 instead of 48, 48
	inline bool IsTileWalkable(Vector2D pos) const
	{
		const Tile *tile = m_Layers.at(2).GetTileFrom((std::size_t)pos.x, (std::size_t)pos.y, m_MapData.at(0));
		if (tile && tile->IsWalkable())
			return true;

		return false;
	}
private:
	bool m_FailedLoading = false;
	std::string_view m_BaseTextureID = "base";
	Base m_Base;
	uint16_t m_LevelID = 0;
	std::array<Layer, s_LayersAmount> m_Layers;
	std::vector<Tile*> m_Spawners;

	// m_SpecificEnemiesAmount array is specifying how many enemies of specified type is already spawned
	std::array<uint16_t, (std::size_t)EnemyType::size> m_SpecificEnemiesAmount{};

	//Wave m_Wave;
	//array in the vector m_Waves declares expected specific enemies spawned at specific wave
	std::vector<std::array<uint16_t, (std::size_t)EnemyType::size>> m_Waves;
	std::size_t m_ExpectedEnemiesAmount = 0;
	std::size_t m_SpawnedEnemies = 0;
	std::size_t m_CurrentWave = 0;
	WaveProgress m_WaveProgress = WaveProgress::OnCooldown;
	uint32_t m_WaveCooldown = 0;
	uint32_t m_NextSpawn = 0;
};