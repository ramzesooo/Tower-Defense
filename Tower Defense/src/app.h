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

class UIElement
{
public:
	static constexpr SDL_Rect srcRect{ 0, 0, 38, 12 }; // background rectangle
	static constexpr SDL_Rect coinRect{ 0, 0, 5, 6 }; // coin rectangle
	static constexpr SDL_Rect heartRect{ 0, 0, 32, 29 };
	static SDL_Rect coinDestRect;
	static SDL_Rect heartDestRect;
	static SDL_Texture *s_BgTexture;
	static SDL_Texture *s_CoinTexture;
	static SDL_Texture *s_HeartTexture;
	SDL_Rect destRect{ 0, 0, 0, 0 };
	Label m_Label;

	inline void Draw()
	{
		TextureManager::DrawTexture(UIElement::s_BgTexture, UIElement::srcRect, this->destRect);
		this->m_Label.Draw();
	}
};

enum class UIState
{
	none = 0, // none means game is running by default
	building
};

//struct BuildingState contains all needed informations and it's one static variable in App class
//It helps to avoid duplicating code
struct BuildingState
{
	//TODO:
	//Swap buildingPlace variable to stack memory
	//Tile buildingPlace;
	std::unique_ptr<Tile> buildingPlace = std::make_unique<Tile>(TileType::special, 2); // tile shown in building state
	Tile *pointedTile = nullptr; // tile pointed by a mouse
	Vector2D coordinates{ 0.0f, 0.0f };
	bool canBuild = false;
	Tower *towerToUpgrade = nullptr;
	SDL_Texture *originalTexture = nullptr;
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

	static bool s_IsWindowMinimized;

	App();
	~App();

	void EventHandler();
	void Update();
	void Render();

	void DrawUI();

	void UpdateCamera();

	void OnResolutionChange();

	bool IsRunning() const { return m_IsRunning; }

	// for checking is specific state going to pause the game
	static inline bool IsGamePaused(UIState state)
	{
		if (s_IsWindowMinimized)
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
	bool m_IsRunning = false;
	bool m_IsFullscreen = false;
	SDL_Window *m_Window = nullptr;
	UIState m_PreviousUIState = UIState::none;
// MAIN SECTION END

public:
	// these textures are needed for rendering RectHP (health.h)
	static SDL_Texture *s_Square;
	static SDL_Texture *s_GreenTex;

	static UIElement s_UIWaves;
	static UIElement s_UICoins;
	static UIElement s_UILifes;

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

	inline void SetCoins(uint16_t coins) { m_Coins = coins; }

	// Arg is not required, adds 1 by default
	inline void AddCoins(uint16_t coins = 1)
	{
		if (coins == 0)
			return;

		m_Coins += coins;
		App::s_UICoins.m_Label.UpdateText(std::to_string(m_Coins));
	}

	// Arg is not required, takes 1 by default
	inline void TakeCoins(uint16_t coins = 1)
	{
		if (coins == 0 || m_Coins == 0)
			return;

		if (coins >= m_Coins)
			m_Coins = 0;
		else
			m_Coins -= coins;

		App::s_UICoins.m_Label.UpdateText(std::to_string(m_Coins));
	}

	static inline void SetLifes(uint16_t lifes) { s_CurrentLevel->GetBase()->m_Lifes = lifes; }

	// Arg is not required, adds 1 by default
	static inline void AddLifes(uint16_t lifes = 1)
	{ 
		if (lifes == 0 || !s_CurrentLevel->GetBase()->m_IsActive)
			return;

		s_CurrentLevel->GetBase()->m_Lifes += lifes;
		App::s_UILifes.m_Label.UpdateText(std::to_string(s_CurrentLevel->GetBase()->m_Lifes));
	}

	// Arg is not required, takes 1 by default
	static inline void TakeLifes(uint16_t lifes = 1)
	{
		if (lifes == 0 || !s_CurrentLevel->GetBase()->m_IsActive)
			return;

		if (lifes >= s_CurrentLevel->GetBase()->m_Lifes)
		{
			s_CurrentLevel->GetBase()->m_Lifes = 0;
			s_CurrentLevel->GetBase()->m_IsActive = false;
		}
		else
		{
			s_CurrentLevel->GetBase()->m_Lifes -= lifes;
		}

		printf("Base active: %d, lifes: %d\n", s_CurrentLevel->GetBase()->m_IsActive, s_CurrentLevel->GetBase()->m_Lifes);

		App::s_UILifes.m_Label.UpdateText(std::to_string(s_CurrentLevel->GetBase()->m_Lifes));
	}
private:
	std::vector<std::unique_ptr<Level>> m_Levels;
	Label *m_PauseLabel = nullptr;
	uint16_t m_Coins = 0;
};