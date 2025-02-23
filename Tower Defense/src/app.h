#pragma once
#include "common.h"
#include "UI.h"
#include "entity/typesEnums.h"

#include "menu.h"
#include "entity/label.h"
#include "entity/towers/tower.h"
#include "level.h"
#include "textureManager.h"
#include "logger.h"
#include "Vector2D.h"

#include "SDL_rect.h"
#include "SDL_render.h"
#include "SDL_events.h"
#include "SDL_mixer.h"

#include <string>
#include <random>
#include <vector>
#include <format>

class Manager;
class Tile;

struct CameraMovement
{
	static constexpr float velocity = 420.0f;
	static Vector2D realVelocity;

	int32_t rangeW = 800 / 6;
	int32_t rangeH = 600 / 6;
	Vector2D move{};
	Vector2D border{};
};

//struct BuildingState contains all needed informations and it's one static variable in App class
//It helps to avoid duplicating code
struct BuildingState
{
	static SDL_Texture *transparentTexture;
	static SDL_Texture *originalTexture;
	static SDL_Texture *cantBuildTexture;
	static SDL_Texture *upgradingTexture;
	static SDL_Texture *sellingTexture;

	bool canBuild = false;
	Tile buildingPlace{ TileType::special };
	Tile *pointedTile = nullptr; // tile pointed by a mouse
	Vector2D coordinates{};
	Tower *towerToUpgradeOrSell = nullptr;
};

class App
{
public:
	static constexpr uint16_t s_TowerRange = 5u;

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
	// Towers displayed after going into building state
	static std::array<UIElement, Tower::s_TowerTypeSize> s_ExpandingTowers;
	static Label s_UICoinsNotification;

	static bool s_IsCameraLocked;
	static CameraMovement s_CameraMovement;

	IF_DEBUG(static Label s_EnemiesAmountLabel;);
	IF_DEBUG(static Label s_PointedPosition;);
	IF_DEBUG(static Label s_FrameDelay;);

	IF_DEBUG(static EnemyDebugSpeed s_Speedy;);
public:
	App();
	~App();

private:
	void InitWindowAndRenderer();
	void AssignStaticAssets();

public:
	static App &Instance() { return *s_Instance; }
	const bool IsRunning() const { return s_IsRunning; }

	void EventHandler();
	void Update();
	void Render();

	void HandleWindowEvent();
	void HandleKeyboardEvent();

	void UpdateCamera();

	void OnResolutionChange();

	inline void OnCursorMove()
	{
		// Do this only if DEBUG is defined, because if it's undefined, it's already done in App::ManageBuildingState()
		// Basically it's about the label s_PointedPosition for debugging
#ifdef DEBUG
		App::s_Building.coordinates = {
			(App::s_Camera.x / static_cast<float>(App::s_CurrentLevel->m_ScaledTileSize)) + static_cast<float>(App::s_MouseX) / static_cast<float>(App::s_CurrentLevel->m_ScaledTileSize),
			(App::s_Camera.y / static_cast<float>(App::s_CurrentLevel->m_ScaledTileSize)) + static_cast<float>(App::s_MouseY) / static_cast<float>(App::s_CurrentLevel->m_ScaledTileSize)
		};
		App::s_Building.coordinates.Floorf();
		//s_Building.coordinates.x = std::floorf((App::s_Camera.x / static_cast<float>(s_CurrentLevel->m_ScaledTileSize)) + static_cast<float>(s_MouseX) / static_cast<float>(s_CurrentLevel->m_ScaledTileSize));
		//s_Building.coordinates.y = std::floorf((App::s_Camera.y / static_cast<float>(s_CurrentLevel->m_ScaledTileSize)) + static_cast<float>(s_MouseY) / static_cast<float>(s_CurrentLevel->m_ScaledTileSize));

		App::s_PointedPosition.UpdateText(std::format("({}, {}), ({}, {}), ({}, {})",
			App::s_MouseX,
			App::s_MouseY,
			static_cast<int32_t>(App::s_Camera.x + App::s_MouseX),
			static_cast<int32_t>(App::s_Camera.y + App::s_MouseY),
			App::s_Building.coordinates.x,
			App::s_Building.coordinates.y)
		);

		// Disallows to set a tower in the right edge (where 2 from 4 tiles are outside of the map border)
		// It's helpful with avoiding an issue about tiles in towers' range
		if (App::s_Building.coordinates.x + 1.0f >= static_cast<float>(App::s_CurrentLevel->m_MapData[0]))
			App::s_Building.coordinates.x--;
		if (App::s_Building.coordinates.y + 1.0f >= static_cast<float>(App::s_CurrentLevel->m_MapData[1]))
			App::s_Building.coordinates.y--;
#endif

		switch (s_UIState)
		{
		case UIState::mainMenu:
			s_MainMenu.OnCursorMove();
			return;
		case UIState::building:
		case UIState::upgrading:
		case UIState::selling:
			ManageBuildingState();
			return;
		case UIState::none:
		default:
			if (!s_IsCameraLocked)
			{
				ManageCamera();
				return;
			}
			return;
		}
	}

	// for checking is specific state going to pause the game
	[[nodiscard]] static constexpr inline bool IsGamePaused(UIState state)
	{
		switch (state)
		{
		case UIState::mainMenu:
		case UIState::building:
		case UIState::upgrading:
		case UIState::selling:
			return true;
		case UIState::none:
		default:
			return false;
		}
	}

	// for checking is currently the game paused
	[[nodiscard]] static constexpr inline bool IsGamePaused() { return IsGamePaused(s_UIState) || s_IsWindowMinimized; }

	void SetUIState(UIState state);
	[[nodiscard]] static constexpr inline bool IsBuildingState()
	{
		switch (s_UIState)
		{
		case UIState::building:
		case UIState::upgrading:
		case UIState::selling:
			return true;
		default:
			return false;
		}
	}

	// NOTE: this method should do all job for starting the level (e.g. creating enemies and whatever feature added in future)
	static void LoadLevel();
	[[nodiscard]] bool SetCurrentLevel(std::size_t level)
	{
		if (level >= m_Levels.size())
		{
			// Levels are displayed in-game with +1, but counting starts from 0
			App::s_Logger.AddLog(std::format("App::SetCurrentLevel: Level #{} can't be assigned. It's equal or higher than amount of levels ({})", level, m_Levels.size()));
			return false;
		}

		App::s_CurrentLevel = &m_Levels[level];
		return true;
	}

	void SwitchBuildingState(UIState newState);
	void ManageBuildingState() const;

	[[nodiscard]] static uint16_t GetDamageOf(ProjectileType type);

	static inline void SetCameraBorder()
	{
		App::s_CameraMovement.border = {
			static_cast<float>(App::s_CurrentLevel->m_MapData[3]) - App::s_Camera.w,
			static_cast<float>(App::s_CurrentLevel->m_MapData[4]) - App::s_Camera.h
		};
	}

	static inline void ResetCameraPos()
	{
		const Vector2D& basePos = App::s_CurrentLevel->GetBase()->GetPos();

		App::s_Camera.x = basePos.x - App::s_Camera.w / 2.0f;
		App::s_Camera.y = basePos.y - App::s_Camera.h / 2.0f;
	}

	inline void SwitchCameraMode() {
		if (s_IsCameraLocked)
		{
			s_IsCameraLocked = false;
			ManageCamera();
		}
		else
		{
			s_IsCameraLocked = true;
			s_CameraMovement.move.Zero();
		}
	}

	// Checks if camera is showing correct field and adjusts the camera if not
	inline void MakeCameraCorrect()
	{
		if (s_Camera.x < 0.0f)
		{
			s_Camera.x = 0.0f;
			s_CameraMovement.move.x = 0.0f;
		}
		else if (s_Camera.x > s_CameraMovement.border.x)
		{
			s_Camera.x = s_CameraMovement.border.x;
			s_CameraMovement.move.x = 0.0f;
		}

		if (s_Camera.y < 0.0f)
		{
			s_Camera.y = 0.0f;
			s_CameraMovement.move.y = 0.0f;
		}
		else if (s_Camera.y > s_CameraMovement.border.y)
		{
			s_Camera.y = s_CameraMovement.border.y;
			s_CameraMovement.move.y = 0.0f;
		}
	}

	inline void ManageCameraX()
	{
		if (s_Camera.x > 0.0f && s_MouseX <= s_CameraMovement.rangeW)
		{
			s_CameraMovement.move.x = -s_CameraMovement.velocity;
			return;
		}

		if (s_Camera.x < s_CameraMovement.border.x && s_MouseX >= static_cast<int32_t>(s_Camera.w) - s_CameraMovement.rangeW)
		{
			s_CameraMovement.move.x = s_CameraMovement.velocity;
			return;
		}
	}

	inline void ManageCameraY()
	{
		if (s_Camera.y > 0.0f && s_MouseY <= s_CameraMovement.rangeH)
		{
			s_CameraMovement.move.y = -s_CameraMovement.velocity;
			return;
		}

		if (s_Camera.y < s_CameraMovement.border.y && s_MouseY >= static_cast<int32_t>(s_Camera.h) - s_CameraMovement.rangeH)
		{
			s_CameraMovement.move.y = s_CameraMovement.velocity;
			return;
		}
	}

	inline void ManageCamera()
	{
		s_CameraMovement.move.Zero();

		ManageCameraX();
		ManageCameraY();
	}

	[[nodiscard]] inline uint16_t GetCoins() const { return m_Coins; }

	inline void SetCoins(uint16_t coins, bool notify = false)
	{
		m_Coins = coins;

		UpdateCoins();

		if (!notify)
		{
			return;
		}

		App::s_UICoinsNotification.UpdateText(std::format("={}", coins));
		App::s_UICoinsNotification.ResetAlpha();
	}

	// Arg is not required, adds 1 by default
	inline void AddCoins(uint16_t coins = 1)
	{
		if (coins == 0)
			return;

		m_Coins += coins;
		App::s_UICoinsNotification.UpdateText(std::format("+{}", coins));
		App::s_UICoinsNotification.ResetAlpha();
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
		App::s_UICoinsNotification.ResetAlpha();
		UpdateCoins();
	}

	static inline void SetLifes(uint16_t lifes)
	{
		Base *base = s_CurrentLevel->GetBase();

		// I guess it's better without that
		if (lifes >= base->m_MaxLifes)
			base->m_Lifes = base->m_MaxLifes;
		else
			base->m_Lifes = lifes;
	}

	// Arg is not required, adds 1 by default
	static inline void AddLifes(uint16_t lifes = 1)
	{ 
		if (lifes == 0 || !App::s_CurrentLevel->IsBaseActive())
			return;

		App::s_CurrentLevel->GetBase()->m_Lifes += lifes;
		App::s_UIElements[2].m_Label.UpdateText(std::to_string(App::s_CurrentLevel->GetBase()->m_Lifes));
	}

	// Arg is not required, takes 1 by default
	static inline void TakeLifes(uint16_t lifes = 1)
	{
		static Mix_Chunk *hurtSound = App::s_Textures.GetSound("hurt");

		if (lifes == 0 || !s_CurrentLevel->IsBaseActive())
			return;

		Mix_PlayChannel(-1, hurtSound, 0);

		if (lifes >= s_CurrentLevel->GetBase()->m_Lifes)
		{
			s_CurrentLevel->Lost();
			return;
		}
		
		s_CurrentLevel->GetBase()->m_Lifes -= lifes;

		UpdateLifes();
	}

	// Updates waves displayed in UI
	static inline void UpdateWaves()
	{
		App::s_UIElements[0].m_Label.UpdateText(std::format("Wave: {}/{}", s_CurrentLevel->GetCurrentWave() + 1, s_CurrentLevel->GetWavesAmount()));
	}

	// Updates lifes displayed in UI
	static inline void UpdateLifes()
	{
		App::s_UIElements[2].m_Label.UpdateText(std::to_string(s_CurrentLevel->GetBaseCurrentLifes()));
	}

	// Updates coins displayed in UI + resets alpha of amount of took or added coins
	static inline void UpdateCoins()
	{
		App::s_UIElements[1].m_Label.UpdateText(std::to_string(App::Instance().m_Coins));
	}

	static inline void SetCantBuild()
	{
		App::s_Building.buildingPlace.SetTexture(BuildingState::cantBuildTexture);
		App::s_Building.canBuild = false;
		App::s_Building.towerToUpgradeOrSell = nullptr;
	}
private:
	static App *s_Instance;

	bool m_Initialized = true;
	bool m_IsFullscreen = false;

	SDL_Window *m_Window = nullptr;

	std::vector<Level> m_Levels;
	// TODO: Should be an array like this
	//std::array<Level, MainMenu::s_LevelsToLoad> m_Levels;

	Label m_PauseLabel;

	uint16_t m_Coins = 0u;
};