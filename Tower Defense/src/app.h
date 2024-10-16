#pragma once
#include "entity/label.h"
#include "entity/entity.h"
#include "entity/tile.h"
#include "entity/attacker.h"
#include "entity/enemy.h"
#include "entity/projectile.h"
#include "level.h"
#include "textureManager.h"
#include "logger.h"

#include "SDL.h"
#include "SDL_image.h"
#include "SDL_ttf.h"

#include <memory>
#include <random>

//struct BuildingState contains all needed informations and it's one static variable in App class
//It helps to avoid duplicating code
struct BuildingState
{
	//Tile *buildingPlace = nullptr; // tile shown in building state
	//Tile buildingPlace; // tile shown in building state
	std::unique_ptr<Tile> buildingPlace = std::make_unique<Tile>(TileTypes::special, 2);
	Tile *pointedTile = nullptr; // tile pointed by a mouse
	Vector2D coordinates{ 0.0f, 0.0f };
	bool canBuild = false;
	Tower *towerToUpgrade = nullptr;
	SDL_Texture *originalTexture = nullptr;
};

enum class UIState
{
	none = 0, // none means game is running by default
	building
};

class App
{
// MAIN SECTION
public:
	static int32_t WINDOW_WIDTH;
	static int32_t WINDOW_HEIGHT;

	static TextureManager s_Textures;
	static Logger s_Logger;
	static Manager s_Manager;

	static SDL_Renderer *s_Renderer;
	static SDL_Event s_Event;
	static SDL_FRect s_Camera;

	static class Level *s_CurrentLevel;
	static uint16_t s_TowerRange;
	static float s_ElapsedTime;
	static UIState s_UIState;
	static int32_t s_MouseX;
	static int32_t s_MouseY;
	static BuildingState s_Building;

	static std::random_device s_Rnd;

	App();
	~App();

	void EventHandler();
	void Update();
	void Render();

	void UpdateCamera();

	void OnResolutionChange();

	bool IsRunning() const { return m_IsRunning; }

	// for checking is specific state going to pause the game
	static inline bool IsGamePaused(UIState state)
	{
		if (s_IsWindowMinimized || s_IsWindowExposed)
			return true;

		switch (state)
		{
		case UIState::building:
			return true;
		case UIState::none:
		default:
			return false;
		}
	}

	// for checking is currently the game paused
	static inline bool IsGamePaused() { return IsGamePaused(s_UIState); }

	inline void SetUIState(UIState state)
	{
		if (s_UIState == state)
			return;

		m_PreviousUIState = s_UIState;
		s_UIState = state;

		switch (state)
		{
		case UIState::none:
			m_PauseLabel->m_Drawable = false;
			return;
		case UIState::building:
			m_PauseLabel->m_Drawable = true;
			ManageBuildingState();
			return;
		}
	}
private:
	static bool s_IsWindowMinimized;
	static bool s_IsWindowExposed;
	bool m_IsRunning = false;
	bool m_IsFullscreen = false;
	SDL_Window *m_Window = nullptr;
	UIState m_PreviousUIState = UIState::none;
// MAIN SECTION END

public:
	// these textures are needed for rendering RectHP (health.h)
	static SDL_Texture *s_Square;
	static SDL_Texture *s_GreenTex;

#ifdef DEBUG
	static Label *s_EnemiesAmountLabel;
#endif

	// NOTE: this method should do all job for starting the level (e.g. creating enemies and whatever feature added in future)
	void LoadLevel(uint32_t baseX, uint32_t baseY);

	void SwitchBuildingState();
	void ManageBuildingState();

	static inline std::string_view TextureOf(AttackerType type)
	{
		switch (type)
		{
		case AttackerType::archer:
			return "attackerArcher";
		case AttackerType::hunter:
			return "attackerHunter";
		case AttackerType::musketeer:
			return "attackerMusketeer";
		}
		return "";
	}

	static inline std::string_view TextureOf(EnemyType type)
	{
		switch (type)
		{
		case EnemyType::elf:
			return "enemyElf";
		case EnemyType::goblinWarrior:
			return "enemyGoblinWarrior";
		case EnemyType::dwarfSoldier:
			return "enemyDwarfSoldier";
		case EnemyType::dwarfKing:
			return "enemyDwarfKing";
		}
		return "";
	}

	static inline std::string_view TextureOf(ProjectileType type)
	{
		switch (type)
		{
		case ProjectileType::arrow:
			return "projectileArrow";
		}
		return "";
	}

	static uint16_t GetDamageOf(ProjectileType type);
private:
	std::vector<std::unique_ptr<Level>> levels;
	Label *m_PauseLabel = nullptr;
	bool m_DestroyTower = false;
};