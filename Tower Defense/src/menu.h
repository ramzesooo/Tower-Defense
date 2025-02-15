#pragma once
#include "UI.h"

#include <array>

class App;

enum class MenuState
{
	primary = 0,
	options,
	levels
};

class MainMenu
{
public:
	// I'm not sure where should I leave it, so let's leave it here at the moment as static
	static constexpr std::size_t s_LevelsToLoad = 1u;
	static int32_t s_GapBetweenButtons;
	static MenuState s_State;
	static SDL_Rect s_BgDestRect;

	Button *m_HoveredButton = nullptr;

	Button m_ReturnButton;
	
	std::array<Button, 2> m_PrimaryButtons; // [0] = Play, [1] = Options, [2] = Exit
	std::array<Button, 1> m_OptionsButtons; // [0] = V-Sync, [1] = Return
	std::array<Button, s_LevelsToLoad> m_LevelsButtons;
public:
	void Init();

	void Render();

	void HandleMouseButtonEvent();
	void HandleTitleButtons(); // Primary
	void HandleOptionsButtons(); // Options
	void HandleLevelsButtons(); // Levels

	void OnCursorMove();
	void OnResolutionChange();

	void SetHoveredButton(Button *button);

	void LoadLevel() const;

	[[nodiscard]] const bool IsMousePointingAt(const Button &button) const;
};