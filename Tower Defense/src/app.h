#pragma once
#include "common.h"
#include "UI.h"
#include "entity/typesEnums.h"

#include "menu.h"
#include "entity/label.h"
#include "entity/entity.h"
#include "entity/tile.h"
#include "entity/attacker.h"
#include "entity/enemy.h"
#include "entity/projectile.h"
#include "entity/tower.h"
#include "level.h"
#include "textureManager.h"
#include "logger.h"
#include "Vector2D.h"

#include "SDL.h"
#include "SDL_image.h"
#include "SDL_ttf.h"

#include <string>
#include <memory>
#include <random>
#include <vector>
#include <format>

struct CameraMovement
{
	int32_t rangeW = 800 / 6;
	int32_t rangeH = 600 / 6;
	float moveX = 0;
	float moveY = 0;
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
public:
	static bool s_IsRunning;

	static int32_t WINDOW_WIDTH;
	static int32_t WINDOW_HEIGHT;

	static MainMenu s_MainMenu;

	static TextureManager s_Textures;
	static Logger s_Logger;
	static Manager s_Manager;

	static SDL_Renderer *s_Renderer;
	static SDL_Event s_Event;
	static SDL_FRect s_Camera;

	static Level *s_CurrentLevel;
	static constexpr uint16_t s_TowerRange = 3;
	static float s_ElapsedTime;
	static UIState s_UIState;
	static int32_t s_MouseX;
	static int32_t s_MouseY;
	static BuildingState s_Building;

	static std::random_device s_Rnd;

	static bool s_IsWindowMinimized;

	// these textures are needed for rendering RectHP (health.h)
	static SDL_Texture *s_Square;
	static SDL_Texture *s_GreenTex;

	static UIElement s_UIWaves;
	static UIElement s_UICoins;
	static UIElement s_UILifes;
	static Label s_UICoinsNotification;

	IF_DEBUG(static Label *s_EnemiesAmountLabel;)
	IF_DEBUG(static Label *s_PointedPosition;;)

	static bool s_IsCameraLocked;
	static CameraMovement s_CameraMovement;

	IF_DEBUG(static bool s_Speedy;)

public:
	App();
	~App();

	static App &Instance() { return *s_Instance; }

	void EventHandler();
	void Update();
	void Render();

	void DrawUI();

	void UpdateCamera();

	void OnResolutionChange();

	bool IsRunning() const { return s_IsRunning; }

	// for checking is specific state going to pause the game
	static inline bool IsGamePaused(UIState state)
	{
		if (s_IsWindowMinimized)
			return true;

		switch (state)
		{
		case UIState::mainMenu:
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

		std::string_view newState;

		m_PreviousUIState = s_UIState;
		s_UIState = state;

		switch (state)
		{
		case UIState::mainMenu:
			newState = "mainMenu";
			m_PauseLabel->m_Drawable = false;
			App::Instance().OnResolutionChange();
			break;
		case UIState::none:
			newState = "none";
			m_PauseLabel->m_Drawable = false;
			break;
		case UIState::building:
			newState = "building";
			m_PauseLabel->m_Drawable = true;
			ManageBuildingState();
			break;
		}

		s_Logger.AddLog(std::format("App::SetUIState: {}", newState));
	}
	// NOTE: this method should do all job for starting the level (e.g. creating enemies and whatever feature added in future)
	static void LoadLevel();

	void SwitchBuildingState();
	void ManageBuildingState();

	inline void SwitchCameraMode() {
		s_IsCameraLocked = !s_IsCameraLocked;
		s_CameraMovement.moveX = 0;
		s_CameraMovement.moveY = 0;
	}

	void ManageCamera();

	static constexpr inline std::string_view TextureOf(AttackerType type)
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

	static constexpr inline std::string_view TextureOf(EnemyType type)
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

	static constexpr inline std::string_view TextureOf(ProjectileType type)
	{
		switch (type)
		{
		case ProjectileType::arrow:
			return "projectileArrow";
		}
		return "";
	}

	static uint16_t GetDamageOf(ProjectileType type);

	inline void SetCoins(uint16_t coins, bool notify = false)
	{
		m_Coins = coins;

		if (notify)
		{
			App::s_UICoinsNotification.UpdateText(std::format("={}", coins));
			UpdateCoins();
		}
		else
		{
			App::s_UICoins.m_Label.UpdateText(std::to_string(App::Instance().m_Coins));
		}
	}

	// Arg is not required, adds 1 by default
	inline void AddCoins(uint16_t coins = 1)
	{
		if (coins == 0)
			return;

		m_Coins += coins;
		App::s_UICoinsNotification.UpdateText(std::format("+{}", coins));
		UpdateCoins();
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

		App::s_UICoinsNotification.UpdateText(std::format("-{}", coins));
		UpdateCoins();
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
			s_CurrentLevel->Clean();
			App::Instance().SetUIState(UIState::mainMenu);
		}
		else
		{
			s_CurrentLevel->GetBase()->m_Lifes -= lifes;
		}

		UpdateLifes();
	}

	// Updates waves displayed in UI
	static inline void UpdateWaves()
	{
		App::s_UIWaves.m_Label.UpdateText("Wave: " + std::to_string(s_CurrentLevel->GetCurrentWave() + 1) + "/" + std::to_string(s_CurrentLevel->GetWavesAmount()));
	}

	// Updates lifes displayed in UI
	static inline void UpdateLifes()
	{
		App::s_UILifes.m_Label.UpdateText(std::to_string(s_CurrentLevel->GetBase()->m_Lifes));
	}

	// Updates coins displayed in UI + resets alpha of amount of took or added coins
	static inline void UpdateCoins()
	{
		App::s_UICoins.m_Label.UpdateText(std::to_string(App::Instance().m_Coins));
		App::s_UICoinsNotification.ResetAlpha();
	}

private:
	static App *s_Instance;

	bool m_MainMenu = true;
	bool m_IsFullscreen = false;

	SDL_Window *m_Window = nullptr;

	UIState m_PreviousUIState = UIState::none;

	std::vector<std::unique_ptr<Level>> m_Levels;

	Label *m_PauseLabel = nullptr;

	uint16_t m_Coins = 0;
};

struct TextureData
{
	std::string_view id, path;
};

static constexpr TextureData textures[]
{
	{ "mapSheet", "assets/tileset.png" },
	{ "szpaku", "assets/szpaku.jpg" },
	{ "buttonUI", "assets/ui/ui_button.png" },
	{ "hoveredButtonUI", "assets/ui/ui_button_hovered.png" },
	{ "canBuild", "assets/ui/tile_CanBuild.png" },
	{ "cantBuild", "assets/ui/tile_CantBuild.png" },
	{ "upgradeTower", "assets/ui/tile_Upgrade.png" },
	{ "elementUI", "assets/ui/ui_element.png" },
	{ "coinUI", "assets/ui/coin.png" },
	{ "heartUI", "assets/ui/heart.png" },

	{ "base", "assets/base.png" },
	{ "tower", "assets/towers/tower.png" },
	{ "square", "assets/square_32x32.png" },
	{ "green", "assets/green_32x32.png" },
	{ "transparent", "assets/transparent.png" },
	{ "grayArrow", "assets/grayArrow_32x32.png" },

	{ App::TextureOf(ProjectileType::arrow), "assets/arrow_16x16.png" },
	{ App::TextureOf(AttackerType::archer), "assets/entities/friendly/attackerArcher.png" },
	{ App::TextureOf(AttackerType::hunter), "assets/entities/friendly/attackerHunter.png"},
	{ App::TextureOf(AttackerType::musketeer), "assets/entities/friendly/attackerMusketeer.png" },
	{ App::TextureOf(EnemyType::elf), "assets/entities/enemy/enemyElf.png" },
	{ App::TextureOf(EnemyType::goblinWarrior), "assets/entities/enemy/enemyGoblinWarrior.png" },
	{ App::TextureOf(EnemyType::dwarfSoldier), "assets/entities/enemy/enemyDwarfSoldier.png" },
	{ App::TextureOf(EnemyType::dwarfKing), "assets/entities/enemy/enemyDwarfKing.png" }
};

/*
App::s_Textures.AddTexture("mapSheet", "assets\\tileset.png");

App::s_Textures.AddTexture("szpaku", "assets\\szpaku.jpg");
App::s_Textures.AddTexture("buttonUI", "assets\\ui\\ui_button.png");
App::s_Textures.AddTexture("hoveredButtonUI", "assets\\ui\\ui_button_hovered.png");
App::s_Textures.AddTexture("canBuild", "assets\\ui\\tile_CanBuild.png");
App::s_Textures.AddTexture("cantBuild", "assets\\ui\\tile_CantBuild.png");
App::s_Textures.AddTexture("upgradeTower", "assets\\ui\\tile_Upgrade.png");
//App::s_Textures.AddTexture("fullHealth", "assets\\ui\\health_bar.png");
//App::s_Textures.AddTexture("emptyHealth", "assets\\ui\\empty_bar.png");
App::s_Textures.AddTexture("elementUI", "assets\\ui\\ui_element.png");
App::s_Textures.AddTexture("coinUI", "assets\\ui\\coin.png");
App::s_Textures.AddTexture("heartUI", "assets\\ui\\heart.png");

App::s_Textures.AddTexture("base", "assets\\base.png");
App::s_Textures.AddTexture("tower", "assets\\towers\\tower.png");
App::s_Textures.AddTexture("square", "assets\\square_32x32.png");
App::s_Textures.AddTexture("green", "assets\\green_32x32.png");
App::s_Textures.AddTexture("transparent", "assets\\transparent.png");
App::s_Textures.AddTexture("grayArrow", "assets\\grayArrow_32x32.png");

App::s_Textures.AddTexture(TextureOf(ProjectileType::arrow), "assets\\arrow_16x16.png");
App::s_Textures.AddTexture(TextureOf(AttackerType::archer), "assets\\entities\\friendly\\attackerArcher.png");
App::s_Textures.AddTexture(TextureOf(AttackerType::hunter), "assets\\entities\\friendly\\attackerHunter.png");
App::s_Textures.AddTexture(TextureOf(AttackerType::musketeer), "assets\\entities\\friendly\\attackerMusketeer.png");
App::s_Textures.AddTexture(TextureOf(EnemyType::elf), "assets\\entities\\enemy\\enemyElf.png");
App::s_Textures.AddTexture(TextureOf(EnemyType::goblinWarrior), "assets\\entities\\enemy\\enemyGoblinWarrior.png");
App::s_Textures.AddTexture(TextureOf(EnemyType::dwarfSoldier), "assets\\entities\\enemy\\enemyDwarfSoldier.png");
App::s_Textures.AddTexture(TextureOf(EnemyType::dwarfKing), "assets\\entities\\enemy\\enemyDwarfKing.png");
*/