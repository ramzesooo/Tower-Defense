#pragma once
#include "UI.h"

#include <array>

class App;

enum class MenuState
{
	primary = 0
};

class MainMenu
{
public:
	static MenuState s_State;
	static SDL_Rect s_BgDestRect;

	Button *m_HoveredButton = nullptr;
	// [0] = Play, [1] = Options, [2] = Exit
	std::array<Button, 3> m_PrimaryButtons;
public:
	void Render();

	void HandleMouseButtonEvent();
	void OnCursorMove();
	void OnResolutionChange();
};