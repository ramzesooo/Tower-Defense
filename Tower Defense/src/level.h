#pragma once
#include "common.h"
#include "entity/typesEnums.h"

#include "entity/tile.h"
#include "entity/attackers/attackers_inc.h"
#include "entity/base.h"

#include <vector>
#if ASYNC_TILES == 1
#include <future>
#endif

class App;
class Enemy;
class Tower;

using WavesArray = std::array<uint16_t, static_cast<std::size_t>(EnemyType::size)>;

//struct WaveContainer
//{
//	//std::array<uint16_t, static_cast<std::size_t>(EnemyType::size)> container{};
//	WavesArray container{};
//};

//Layer references to just map's layer of tiles
class Layer
{
public:
	static uint16_t s_MapWidth; // App::LoadLevel assign the correct map width situable for current level

	std::vector<Tile> m_Tiles;
	std::vector<Tile*> m_DrawableTiles;
public:
	// returns a tile from specific coordinates
	Tile *GetTileFrom(std::size_t posX, std::size_t posY) { return &m_Tiles.at(posY * Layer::s_MapWidth + posX); }

	void SetTileDrawable(Tile *tile, bool bDrawable)
	{
		// If it's supposed to make a tile drawable
		if (bDrawable)
		{
			// If it's already in the vector it would be stupid to put it once more
			if (std::find(m_DrawableTiles.begin(), m_DrawableTiles.end(), tile) != m_DrawableTiles.end())
			{
				return;
			}

			// emplace_back if it's yet not there
			m_DrawableTiles.emplace_back(tile);

			return;
		}

		// If it's supposed to make a tile non-drawable

		auto it = std::find(m_DrawableTiles.begin(), m_DrawableTiles.end(), tile);

		// If couldn't find the tile, then just return, since it means it's non-drawable
		if (it == m_DrawableTiles.end())
		{
			return;
		}

		// Erase if it was drawable
		m_DrawableTiles.erase(it);
	}
};

enum class WaveProgress
{
	LoadedLevel,
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

	Vector2D m_BasePos{};
	uint16_t m_MovementSpeedRate = 1;
public:
	Level() = delete;
	Level(uint16_t levelID);
	~Level() = default;

	void Init();
private:
	void SetupLayer(std::ifstream &mapFile, uint16_t layerID);
	void SetupBase();
public:
	void Clear();
	void Lost(); // Game over

	// All methods adding entities (such as tower, attacker and enemy) call AddGroup inside of them

	// Position passed in parameters referees to left-upper square
	// Towers take 4 tiles in this scheme:
	// [0][1]
	// [2][3]
	// 
	// Tower position depends on Vector2D and it is scaled by itself to tiles' size
	// So it should look like this: x: 1.0f, y: 2.0f, instead of x: 96.0f, y: 144.0f
	void AddTower(float posX, float posY, TowerType type) const;
	void AddAttacker(Tower *assignedTower, AttackerType type, uint16_t scale = 2);
	Enemy *AddEnemy(float posX, float posY, EnemyType type, uint16_t scale = 2) const;
	void AddProjectile(ProjectileType type, Attacker *projectileOwner, Enemy *target);

	void LMBEvent(); // Left mouse button event
	inline void HandleMouseButtonEvent(uint8_t button)
	{
		if (button == SDL_BUTTON_LEFT)
		{
			LMBEvent();
		}
		/*else if (button == SDL_BUTTON_RIGHT)
		{

		}*/
	}

	void HighlightTower();

	void InitWave();
	void ManageWaves();

private:
	static void InitTimerForNextWave();
public:
	void UpdateTimer() const;

public:
	void Render() const;

	[[nodiscard]] uint16_t GetID() const { return m_LevelID; }

	[[nodiscard]] Base *GetBase() { return &m_Base; }
	void SetBaseActive(bool active) { m_Base.m_IsActive = active; }
	[[nodiscard]] const bool IsBaseActive() const { return m_Base.m_IsActive; }
	[[nodiscard]] uint16_t GetBaseCurrentLifes() const { return m_Base.m_Lifes; }

	// Function GetTileFrom may return nullptr if asked tile is outside of map and/or doesn't exist
	[[nodiscard]] Tile *GetTileFrom(uint32_t posX, uint32_t posY, uint16_t layer = 0u);
	[[nodiscard]] Tile *GetTileFrom(float posX, float posY, uint16_t layer = 0u) { return GetTileFrom(static_cast<uint32_t>(posX), static_cast<uint32_t>(posY), layer); }

	// Adjust tiles' view to current camera
	void OnUpdateCamera();

	[[nodiscard]] std::size_t GetWavesAmount() const { return m_Waves.size(); }
	[[nodiscard]] std::size_t GetCurrentWave() const { return m_CurrentWave; }

	// The method takes origin of tile, for example 1, 1 instead of 48, 48
	inline bool IsTileWalkable(const Vector2D &pos)
	{
		const Tile *tile = GetTileFrom(pos.x, pos.y, 2u);
		if (tile && tile->IsWalkable())
			return true;

		return false;
	}

	static constexpr inline uint16_t GetBuildPrice(TowerType type)
	{
		switch (type)
		{
		case TowerType::classic:
			return 5u;
		case TowerType::dark:
			return 10u;
		}

		return 1u;
	}
private:
	std::string_view m_BaseTextureID = "base";
	Base m_Base;
	uint16_t m_LevelID = 0u;
	std::array<Layer, s_LayersAmount> m_Layers;
	std::vector<Tile*> m_Spawners;

	/// NOTE: WavesArray is std::array defined in the beginning of the header file

	// m_SpecificEnemiesAmount array is specifying how many enemies of specified type is already spawned
	WavesArray m_SpecificEnemiesAmount{};

	//array in the vector m_Waves declares expected specific enemies spawned at specific wave
	std::vector<WavesArray> m_Waves;
	std::size_t m_ExpectedEnemiesAmount = 0u;
	std::size_t m_SpawnedEnemies = 0u;
	std::size_t m_CurrentWave = 0u;
	WaveProgress m_WaveProgress = WaveProgress::OnCooldown;
	uint32_t m_WaveCooldown = 0u;
	uint32_t m_NextSpawn = 0u;

	Tower *m_HighlightedTower = nullptr;

#if ASYNC_TILES == 1
	std::vector<std::future<void>> m_Futures;
#endif
};