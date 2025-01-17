#pragma once
#include "entity/label.h"

#include "SDL_rect.h"
#include "SDL_render.h"

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

	static SDL_Rect coinDestRect;
	static SDL_Rect heartDestRect;
	static SDL_Rect timerDestRect;
	static SDL_Rect hammerDestRect;
	static SDL_Rect sellDestRect;

	static SDL_Texture *s_BgTexture;
	static SDL_Texture *s_CoinTexture;
	static SDL_Texture *s_HeartTexture;
	static SDL_Texture *s_TimerTexture;
	static SDL_Texture *s_HammerTexture;
	static SDL_Texture *s_HammerGreenTexture;
	static SDL_Texture *s_SellTexture;

	static bool s_IsHammerPressed;

	SDL_Rect destRect{ 0, 0, 0, 0 };
	Label m_Label;
	std::string m_DefaultText;
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