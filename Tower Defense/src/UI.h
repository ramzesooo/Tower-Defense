#pragma once
#include "textureManager.h"
#include "entity/label.h"
#include "SDL_rect.h"
#include "SDL_render.h"

enum class UIState
{
	none = 0, // none means game is running by default
	mainMenu,
	building
};

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
	std::string m_DefaultText = "";

	inline void Draw()
	{
		TextureManager::DrawTexture(UIElement::s_BgTexture, UIElement::srcRect, destRect);
		m_Label.Draw();
	}
};

class Button
{
public:
	static constexpr SDL_Rect srcRect{ 0, 0, 30, 14 };
	static SDL_Texture *s_DefaultButton;
	static SDL_Texture *s_HoveredButton;

	Label m_Label;
	bool m_IsHovered = false;
	SDL_Rect destRect{ 0, 0, 0, 0 };

	inline void Draw()
	{
		if (m_IsHovered)
			TextureManager::DrawTexture(s_HoveredButton, srcRect, destRect);
		else
			TextureManager::DrawTexture(s_DefaultButton, srcRect, destRect);

		m_Label.Draw();
	}
};