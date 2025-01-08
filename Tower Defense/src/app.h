#pragma once
#include "common.h"
#include "UI.h"
#include "entity/typesEnums.h"

#include "menu.h"
#include "entity/label.h"
#include "level.h"
#include "textureManager.h"
#include "logger.h"
#include "Vector2D.h"

#include "SDL_rect.h"
#include "SDL_render.h"
#include "SDL_events.h"
#include "SDL_ttf.h"

#include <string>
#include <random>
#include <vector>
#include <format>

class Manager;
class Tower;
class Tile;

struct CameraMovement
{
	static constexpr float velocity = 420.0f;
	static Vector2D realVelocity;

	int32_t rangeW = 800 / 6;
	int32_t rangeH = 600 / 6;
	//float moveX = 0.0f;
	//float moveY = 0.0f;
	Vector2D move{};
	Vector2D border{};
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
	static constexpr uint16_t s_TowerRange = 5u;
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

	// [0] = waves, [1] = coins, [2] = lifes, [3] = time
	static std::array<UIElement, 4> s_UIElements;
	static Label s_UICoinsNotification;

	static bool s_IsCameraLocked;
	static CameraMovement s_CameraMovement;

	IF_DEBUG(static Label *s_EnemiesAmountLabel;);
	IF_DEBUG(static Label *s_PointedPosition;);
	IF_DEBUG(static Label *s_FrameDelay;);

	IF_DEBUG(static EnemyDebugSpeed s_Speedy;);
	IF_DEBUG(static bool s_SwapTowerType;);
public:
	App();
	~App();

	inline void PrepareUI()
	{
		static TTF_Font *font = App::s_Textures.GetFont("default");
		//static SDL_Rect destRect{ static_cast<int32_t>(App::s_Camera.w / 30.0f), static_cast<int32_t>(App::s_Camera.h / 30.0f), UIElement::srcRect.w * 3, UIElement::srcRect.h * 3};
		static SDL_Rect destRect{ 0, 0, UIElement::srcRect.w * 3, UIElement::srcRect.h * 3};

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
			// Do nothing for the rest
			case 0:
			case 3:
			default:
				break;
			}

			destRect.y += destRect.h;
		}
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
#ifdef DEBUG
		s_Building.coordinates.x = std::floorf((App::s_Camera.x / static_cast<float>(s_CurrentLevel->m_ScaledTileSize)) + static_cast<float>(s_MouseX) / static_cast<float>(s_CurrentLevel->m_ScaledTileSize));
		s_Building.coordinates.y = std::floorf((App::s_Camera.y / static_cast<float>(s_CurrentLevel->m_ScaledTileSize)) + static_cast<float>(s_MouseY) / static_cast<float>(s_CurrentLevel->m_ScaledTileSize));

		s_PointedPosition->UpdateText(std::format("({}, {}), ({}, {}), ({}, {})",
			s_MouseX,
			s_MouseY,
			static_cast<int32_t>(s_Camera.x + s_MouseX),
			static_cast<int32_t>(s_Camera.y + s_MouseY),
			s_Building.coordinates.x,
			s_Building.coordinates.y)
		);

		if (s_Building.coordinates.x >= static_cast<float>(App::s_CurrentLevel->m_MapData.at(0)) - 1.0f)
			s_Building.coordinates.x--;

		if (s_Building.coordinates.y >= static_cast<float>(App::s_CurrentLevel->m_MapData.at(1)) - 1.0f)
			s_Building.coordinates.y--;
#endif

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
	static constexpr inline bool IsGamePaused(UIState state)
	{
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
	static inline bool IsGamePaused() { return IsGamePaused(s_UIState) || s_IsWindowMinimized; }

	void SetUIState(UIState state);

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
			//s_CameraMovement.moveX = 0.0f;
			//s_CameraMovement.moveY = 0.0f;
			s_CameraMovement.move = { 0.0f, 0.0f };
		}
	}

	// Checks if camera is showing correct field and adjusts the camera if not
	inline void MakeCameraCorrect()
	{
		if (s_Camera.x < 0.0f)
		{
			s_Camera.x = 0.0f;
			//s_CameraMovement.moveX = 0.0f;
			s_CameraMovement.move.x = 0.0f;
		}
		else if (s_Camera.x > s_CameraMovement.border.x)
		{
			s_Camera.x = s_CameraMovement.border.x;
			//s_CameraMovement.moveX = 0.0f;
			s_CameraMovement.move.x = 0.0f;
		}

		if (s_Camera.y < 0.0f)
		{
			s_Camera.y = 0.0f;
			//s_CameraMovement.moveY = 0.0f;
			s_CameraMovement.move.y = 0.0f;
		}
		else if (s_Camera.y > s_CameraMovement.border.y)
		{
			s_Camera.y = s_CameraMovement.border.y;
			//s_CameraMovement.moveY = 0.0f;
			s_CameraMovement.move.y = 0.0f;
		}
	}

	inline void ManageCameraX()
	{
		if (s_Camera.x > 0.0f && s_MouseX <= s_CameraMovement.rangeW)
		{
			//s_CameraMovement.moveX = -s_CameraMovement.velocity;
			s_CameraMovement.move.x = -s_CameraMovement.velocity;
			return;
		}

		if (s_Camera.x < s_CameraMovement.border.x && s_MouseX >= static_cast<int32_t>(s_Camera.w) - s_CameraMovement.rangeW)
		{
			//s_CameraMovement.moveX = s_CameraMovement.velocity;
			s_CameraMovement.move.x = s_CameraMovement.velocity;
			return;
		}
	}

	inline void ManageCameraY()
	{
		if (s_Camera.y > 0.0f && s_MouseY <= s_CameraMovement.rangeH)
		{
			//s_CameraMovement.moveY = -s_CameraMovement.velocity;
			s_CameraMovement.move.y = -s_CameraMovement.velocity;
			return;
		}

		if (s_Camera.y < s_CameraMovement.border.y && s_MouseY >= static_cast<int32_t>(s_Camera.h) - s_CameraMovement.rangeH)
		{
			//s_CameraMovement.moveY = s_CameraMovement.velocity;
			s_CameraMovement.move.y = s_CameraMovement.velocity;
			return;
		}
	}

	inline void ManageCamera()
	{
		/*s_CameraMovement.moveX = 0.0f;
		s_CameraMovement.moveY = 0.0f;*/
		s_CameraMovement.move = { 0.0f, 0.0f };

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
		case AttackerType::darkTower:
			return "";
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
		//case ProjectileType::dark:
			//return "projectileDarkTower";
		case ProjectileType::thunder:
			return "projectileDarkTower";
		}
		return "";
	}

	static constexpr inline std::string_view TextureOf(TowerType type)
	{
		switch (type)
		{
		case TowerType::classic:
			return "classicTower";
		case TowerType::dark:
			return "darkTower";
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

	static inline void SetLifes(uint16_t lifes)
	{
		Base *base = s_CurrentLevel->GetBase();

		if (lifes >= base->m_MaxLifes)
			base->m_Lifes = base->m_MaxLifes;
		else
			base->m_Lifes = lifes;
	}

	// Arg is not required, adds 1 by default
	static inline void AddLifes(uint16_t lifes = 1)
	{ 
		if (lifes == 0 || !s_CurrentLevel->GetBase()->m_IsActive)
			return;

		s_CurrentLevel->GetBase()->m_Lifes += lifes;
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
		App::s_UIElements.at(1).m_Label.UpdateText(std::to_string(App::Instance().m_Coins));
		App::s_UICoinsNotification.ResetAlpha();
	}

private:
	static App *s_Instance;

	bool m_IsFullscreen = false;

	SDL_Window *m_Window = nullptr;

	UIState m_PreviousUIState = UIState::none;

	std::vector<Level> m_Levels;

	Label m_PauseLabel;

	uint16_t m_Coins = 0;
};