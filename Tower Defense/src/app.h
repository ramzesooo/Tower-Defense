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
	static constexpr float velocity = 360.0f;

	int32_t rangeW = 800 / 6;
	int32_t rangeH = 600 / 6;
	float moveX = 0.0f;
	float moveY = 0.0f;
	Vector2D border;
};

//struct BuildingState contains all needed informations and it's one static variable in App class
//It helps to avoid duplicating code
struct BuildingState
{
	static SDL_Texture *originalTexture;

	bool canBuild = false;
	Tile buildingPlace{ TileType::special, 2 };
	Tile *pointedTile = nullptr; // tile pointed by a mouse
	Vector2D coordinates;
	Tower *towerToUpgrade = nullptr;
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

	/*static UIElement s_UIWaves;
	static UIElement s_UICoins;
	static UIElement s_UILifes;
	static UIElement s_UITime;*/

	// [0] = waves, [1] = coins, [2] = lifes, [3] = time
	static std::array<UIElement, 4> s_UIElements;
	static Label s_UICoinsNotification;

	static bool s_IsCameraLocked;
	static CameraMovement s_CameraMovement;

	IF_DEBUG(static Label *s_EnemiesAmountLabel;);
	IF_DEBUG(static Label *s_PointedPosition;);
	IF_DEBUG(static Label *s_FrameDelay;);

	IF_DEBUG(static bool s_Speedy;);
public:
	App();
	~App();

	inline void PrepareUI()
	{
		static TTF_Font *font = App::s_Textures.GetFont("default");
		static SDL_Rect destRect{ static_cast<int32_t>(App::s_Camera.w / 30.0f), static_cast<int32_t>(App::s_Camera.h / 30.0f), UIElement::srcRect.w * 3, UIElement::srcRect.h * 3};

		s_UIElements.at(0).m_DefaultText = "Wave: 1/1";
		s_UIElements.at(1).m_DefaultText = "100";
		s_UIElements.at(2).m_DefaultText = "100";
		s_UIElements.at(3).m_DefaultText = "0:00";

		for (std::size_t i = 0u; i < s_UIElements.size(); i++)
		{
			UIElement &element = s_UIElements.at(i);
			element.destRect = destRect;
			element.m_Label = Label(destRect.x, destRect.y, element.m_DefaultText, font);
			const SDL_Rect &labelRect = element.m_Label.GetRect();
			element.m_Label.UpdatePos(labelRect.x + (destRect.w / 2 - labelRect.w / 2), labelRect.y + (destRect.h / 2 - labelRect.h / 2));

			switch (i)
			{
			case 1: // coins
				UIElement::coinDestRect = { destRect.x + UIElement::coinRect.w, destRect.y + destRect.h / 4, UIElement::coinRect.w * 3, element.destRect.h / 2 };
				s_UICoinsNotification = Label(labelRect.x + labelRect.w + labelRect.w / 2, labelRect.y, "+0", font);
				s_UICoinsNotification.SetAlpha(0);
				break;
			case 2: // lifes
				UIElement::heartDestRect = { destRect.x, destRect.y + destRect.h / 4, UIElement::heartRect.w, destRect.h - UIElement::heartRect.h / 2 };
				break;
			case 0:
			case 3:
			default:
				break;
			}

			destRect.y += destRect.h;
		}

		/*
		// Waves
		s_UIWaves.destRect = destRect;
		s_UIWaves.m_Label = Label(destRect.x, destRect.y, "Wave: 1/1", font);
		labelRect = s_UIWaves.m_Label.GetRect();
		s_UIWaves.m_Label.UpdatePos(labelRect.x + (destRect.w / 2 - labelRect.w / 2), labelRect.y + (destRect.h / 2 - labelRect.h / 2));

		// Coins
		destRect.y += destRect.h;
		s_UICoins.destRect = destRect;
		//s_UICoins.destRect = { static_cast<int32_t>(App::s_Camera.w / 30.0f), s_UIWaves.destRect.y + s_UIWaves.destRect.h, UIElement::srcRect.w * 3, UIElement::srcRect.h * 3 };
		s_UICoins.m_Label = Label(destRect.x, destRect.y, "100", font);
		labelRect = s_UICoins.m_Label.GetRect();
		s_UICoins.m_Label.UpdatePos(labelRect.x + (destRect.w / 2 - labelRect.w / 2), labelRect.y + (destRect.h / 2 - labelRect.h / 2));
		UIElement::coinDestRect = { destRect.x + UIElement::coinRect.w, destRect.y + destRect.h / 4, UIElement::coinRect.w * 3, s_UICoins.destRect.h / 2 };
		s_UICoinsNotification = Label(labelRect.x + labelRect.w + labelRect.w / 2, labelRect.y, "+0", font);
		s_UICoinsNotification.SetAlpha(0);

		// Lifes
		destRect.y += destRect.h;
		s_UILifes.destRect = destRect;
		//s_UILifes.destRect = { static_cast<int32_t>(App::s_Camera.w / 30.0f), s_UICoins.destRect.y + s_UICoins.destRect.h, UIElement::srcRect.w * 3, UIElement::srcRect.h * 3 };
		s_UILifes.m_Label = Label(destRect.x, destRect.y, "100", font);
		labelRect = s_UILifes.m_Label.GetRect();
		s_UILifes.m_Label.UpdatePos(labelRect.x + (destRect.w / 2 - labelRect.w / 2), labelRect.y + (destRect.h / 2 - labelRect.h / 2));
		UIElement::heartDestRect = { destRect.x, destRect.y + destRect.h / 4, UIElement::heartRect.w, destRect.h - UIElement::heartRect.h / 2 };
	
		// Time
		destRect.y += destRect.h;
		s_UITime.destRect = destRect;
		s_UITime.m_Label = Label(destRect.x, destRect.y, "0:00", font);
		labelRect = s_UITime.m_Label.GetRect();
		s_UITime.m_Label.UpdatePos(labelRect.x + (destRect.w / 2 - labelRect.w / 2), labelRect.y + (destRect.h / 2 - labelRect.h / 2));
		*/
	}

	static App &Instance() { return *s_Instance; }
	bool IsRunning() const { return s_IsRunning; }

	void EventHandler();
	void Update();
	void Render();

	void DrawUI();

	void UpdateCamera();

	void OnResolutionChange();

	inline void OnCursorMove()
	{
		IF_DEBUG(
			s_PointedPosition->UpdateText(std::format("({}, {}), ({}, {})",
				s_MouseX,
				s_MouseY,
				std::floorf((App::s_Camera.x / s_CurrentLevel->m_ScaledTileSize) + static_cast<float>(s_MouseX) / s_CurrentLevel->m_ScaledTileSize),
				std::floorf((App::s_Camera.y / s_CurrentLevel->m_ScaledTileSize) + static_cast<float>(s_MouseY) / s_CurrentLevel->m_ScaledTileSize)
			));
		)

		switch (s_UIState)
		{
		case UIState::mainMenu:
			s_MainMenu.OnCursorMove();
			return;
		case UIState::building:
			ManageBuildingState();
			return;
		default:
			break;
		}

		if (!s_IsCameraLocked)
		{
			ManageCamera();
			return;
		}
	}

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

		//s_Logger.AddLog(std::format("App::SetUIState: {}", newState));
		App::s_Logger.AddLog(std::string_view("App::SetUIState: "), false);
		App::s_Logger.AddLog(newState);
	}
	// NOTE: this method should do all job for starting the level (e.g. creating enemies and whatever feature added in future)
	static void LoadLevel();

	void SwitchBuildingState();
	void ManageBuildingState();

	inline void SwitchCameraMode() {
		if (s_IsCameraLocked)
		{
			s_IsCameraLocked = false;
			ManageCamera();
		}
		else
		{
			s_IsCameraLocked = true;
			s_CameraMovement.moveX = 0.0f;
			s_CameraMovement.moveY = 0.0f;
		}
		/*s_IsCameraLocked = !s_IsCameraLocked;
		s_CameraMovement.moveX = 0.0f;
		s_CameraMovement.moveY = 0.0f;*/
	}

	inline void MakeCameraCorrect()
	{
		if (s_Camera.x < 0.0f)
		{
			s_Camera.x = 0.0f;
			s_CameraMovement.moveX = 0.0f;
		}
		else if (s_Camera.x > s_CameraMovement.border.x)
		{
			s_Camera.x = s_CameraMovement.border.x;
			s_CameraMovement.moveX = 0.0f;
		}

		if (s_Camera.y < 0.0f)
		{
			s_Camera.y = 0.0f;
			s_CameraMovement.moveY = 0.0f;
		}
		else if (s_Camera.y > s_CameraMovement.border.y)
		{
			s_Camera.y = s_CameraMovement.border.y;
			s_CameraMovement.moveY = 0.0f;
		}
	}

	inline void ManageCameraX()
	{
		if (s_Camera.x > 0.0f && s_MouseX <= s_CameraMovement.rangeW)
		{
			s_CameraMovement.moveX = -s_CameraMovement.velocity;
			return;
		}

		if (s_Camera.x < s_CameraMovement.border.x && s_MouseX >= static_cast<int32_t>(s_Camera.w) - s_CameraMovement.rangeW)
		{
			s_CameraMovement.moveX = s_CameraMovement.velocity;
			return;
		}
	}

	inline void ManageCameraY()
	{
		if (s_Camera.y > 0.0f && s_MouseY <= s_CameraMovement.rangeH)
		{
			s_CameraMovement.moveY = -s_CameraMovement.velocity;
			return;
		}

		if (s_Camera.y < s_CameraMovement.border.y && s_MouseY >= static_cast<int32_t>(s_Camera.h) - s_CameraMovement.rangeH)
		{
			s_CameraMovement.moveY = s_CameraMovement.velocity;
			return;
		}
	}

	inline void ManageCamera()
	{
		s_CameraMovement.moveX = 0.0f;
		s_CameraMovement.moveY = 0.0f;

		ManageCameraX();
		ManageCameraY();
	}

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
			//App::s_UICoins.m_Label.UpdateText(std::to_string(App::Instance().m_Coins));
			App::s_UIElements.at(1).m_Label.UpdateText(std::to_string(App::Instance().m_Coins));
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
		//App::s_UILifes.m_Label.UpdateText(std::to_string(s_CurrentLevel->GetBase()->m_Lifes));
		App::s_UIElements.at(2).m_Label.UpdateText(std::to_string(s_CurrentLevel->GetBase()->m_Lifes));
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
				App::s_UIElements.at(0).m_Label.UpdateText(std::format("Wave: {}/{}", s_CurrentLevel->GetCurrentWave() + 1, s_CurrentLevel->GetWavesAmount()));
	}

	// Updates lifes displayed in UI
	static inline void UpdateLifes()
	{
		App::s_UIElements.at(2).m_Label.UpdateText(std::to_string(s_CurrentLevel->GetBase()->m_Lifes));
	}

	// Updates coins displayed in UI + resets alpha of amount of took or added coins
	static inline void UpdateCoins()
	{
		//App::s_UICoins.m_Label.UpdateText(std::to_string(App::Instance().m_Coins));
		App::s_UIElements.at(1).m_Label.UpdateText(std::to_string(App::Instance().m_Coins));
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