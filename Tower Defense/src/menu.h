#pragma once
#include "entity/label.h"
#include "textureManager.h"
#include "SDL.h"

#include <array>

class App;

enum class MenuState
{
	primary = 0
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

class MainMenu
{
public:
	static MenuState s_State;

	MainMenu();
	~MainMenu() = default;

	void Render();

	void HandleMouseButtonEvent();
	void OnCursorMove();
	void OnResolutionChange(bool init = false);

	Button *m_HoveredButton = nullptr;
	// [0] = Play, [1] = Options, [2] = Exit
	std::array<Button, 3> m_PrimaryButtons;
};