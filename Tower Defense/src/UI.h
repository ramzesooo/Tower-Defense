#pragma once
#include "entity/typesEnums.h"

#include "entity/label.h"

#include "SDL_rect.h"
#include "SDL_render.h"

#include <array>

enum class UIState
{
	none = 0, // none means game is running by default
	mainMenu,
	building,
	upgrading,
	selling
};

// Wrong name, but whatever
class UIElement
{
public:
	static constexpr SDL_Rect srcRect{ 0, 0, 38, 12 }; // background rectangle
	static constexpr SDL_Rect coinRect{ 0, 0, 5, 6 }; // coin rectangle
	static constexpr SDL_Rect heartRect{ 0, 0, 32, 29 }; // heart rectangle
	static constexpr SDL_Rect timerRect{ 0, 0, 19, 22 }; // clock rectangle
	static constexpr SDL_Rect hammerRect{ 0, 0, 34, 34 }; // hammer rectangle
	static constexpr SDL_Rect sellRect{ 0, 0, 24, 24 }; // sell rectangle
	static constexpr SDL_Rect upgradeRect{ 0, 0, 24, 24 }; // upgrading hammer rectangle

	static constexpr SDL_Rect expandingTowerSrcRect{ 0, 0, 34, 34 };

	static SDL_Rect coinDestRect;
	static SDL_Rect heartDestRect;
	static SDL_Rect timerDestRect;
	static SDL_Rect hammerDestRect;
	static SDL_Rect sellDestRect;
	static SDL_Rect upgradeDestRect;

	static SDL_Texture *s_BgTexture;
	static SDL_Texture *s_CoinTexture;
	static SDL_Texture *s_HeartTexture;
	static SDL_Texture *s_TimerTexture;
	static SDL_Texture *s_HammerTexture;
	static SDL_Texture *s_HammerGreenTexture;
	static SDL_Texture *s_SellTexture;
	static SDL_Texture *s_UpgradeTexture;
	static SDL_Texture *s_TransparentGreenTexture;

	//static std::array<SDL_Texture*, std::size_t(TowerType::size)> s_ExpandingTowersIcons;

	static TowerType s_ChosenTower;

	static uint32_t s_Timer;

	SDL_Rect destRect{};
	Label m_Label;
	std::string m_DefaultText;

	bool m_IsPressed = false;
public:
	static void InitUI();
	static void DrawUI();
	void DrawElement();
};

enum class ButtonType
{
	classic = 0,
	check
};

class Button
{
public:
	static constexpr SDL_Rect srcRect{ 0, 0, 30, 14 };
	static constexpr SDL_Rect checkSrcRect{ 0, 0, 32, 32 };
	static SDL_Texture *s_DefaultButton;
	static SDL_Texture *s_DefaultButtonChecked;
	static SDL_Texture *s_DefaultButtonUnchecked;
	static SDL_Texture *s_HoveredButton;
	static SDL_Texture *s_HoveredButtonChecked;
	static SDL_Texture *s_HoveredButtonUnchecked;

	ButtonType m_Type = ButtonType::classic;

	Label m_Label;
	SDL_Rect destRect{ 0, 0, 0, 0 };

	bool m_IsHovered = false;
	bool m_IsChecked = false;
public:
	void Draw();
};