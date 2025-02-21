#include "menu.h"
#include "textureManager.h"
#include "app.h"
#include "entity/label.h"

#include "SDL_rect.h"
#include "SDL_ttf.h"
#include "SDL_mixer.h"

#include <format>

SDL_Texture *Button::s_DefaultButton = nullptr;
SDL_Texture *Button::s_DefaultButtonChecked = nullptr;
SDL_Texture *Button::s_DefaultButtonUnchecked = nullptr;
SDL_Texture *Button::s_HoveredButton = nullptr;
SDL_Texture *Button::s_HoveredButtonChecked = nullptr;
SDL_Texture *Button::s_HoveredButtonUnchecked = nullptr;

int32_t MainMenu::s_GapBetweenButtons = 0u;

MenuState MainMenu::s_State = MenuState::primary;
SDL_Rect MainMenu::s_BgDestRect{ 0, 0, App::WINDOW_WIDTH, App::WINDOW_HEIGHT };

static constexpr char returnText[] = "Return";
static constexpr char quitText[] = "Quit";

// TODO: Too much levels will result in wrong displaying
// They should be splitted into pages
void MainMenu::Init()
{
	TTF_Font *defaultFont = App::s_Textures.GetFont("default");

	int32_t centerX = App::WINDOW_WIDTH / 2;
	int32_t centerY = App::WINDOW_HEIGHT / 2;

	MainMenu::s_GapBetweenButtons = (App::WINDOW_HEIGHT / 14) + ((App::WINDOW_HEIGHT / 14) / 4);

	Button *btn = nullptr;

	// Return button
	{
		btn = &m_ReturnButton;
		btn->destRect.w = App::WINDOW_WIDTH / 7;
		btn->destRect.h = App::WINDOW_HEIGHT / 14;
		btn->destRect.x = centerX - btn->destRect.w / 2;
		btn->destRect.y = centerY - btn->destRect.h / 2 + static_cast<int32_t>(m_PrimaryButtons.size()) * MainMenu::s_GapBetweenButtons;
		btn->m_Label = Label(btn->destRect.x + btn->destRect.w / 2, btn->destRect.y + btn->destRect.h / 4, "Quit", defaultFont, SDL_Color{ 255, 255, 255, 255 }, nullptr, true);
		const SDL_Rect &labelRect = btn->m_Label.GetRect();
		btn->m_Label.UpdatePos(labelRect.x - labelRect.w / 2, labelRect.y);
	}
	// Return button

	// Title screen
	for (std::size_t i = 0u; i < m_PrimaryButtons.size(); ++i)
	{
		btn = &m_PrimaryButtons[i];
		btn->destRect.w = App::WINDOW_WIDTH / 7;
		btn->destRect.h = App::WINDOW_HEIGHT / 14;
		btn->destRect.x = centerX - btn->destRect.w / 2;
		btn->destRect.y = centerY - btn->destRect.h / 2 + (static_cast<int32_t>(i) - 1) * MainMenu::s_GapBetweenButtons;
	}

	// Button "Play"
	{
		btn = &m_PrimaryButtons.at(0);
		btn->m_Label = Label(btn->destRect.x + btn->destRect.w / 2, btn->destRect.y + btn->destRect.h / 4, "Play", defaultFont, SDL_Color{ 255, 255, 255, 255 }, nullptr, true);
		const SDL_Rect &labelRect = btn->m_Label.GetRect();
		btn->m_Label.UpdatePos(labelRect.x - labelRect.w / 2, labelRect.y);
	}

	// Button "Options"
	{
		btn = &m_PrimaryButtons.at(1);
		btn->m_Label = Label(btn->destRect.x + btn->destRect.w / 2, btn->destRect.y + btn->destRect.h / 4, "Options", defaultFont, SDL_Color{ 255, 255, 255, 255 }, nullptr, true);
		const SDL_Rect &labelRect = btn->m_Label.GetRect();
		btn->m_Label.UpdatePos(labelRect.x - labelRect.w / 2, labelRect.y);
	}
	// Title screen

	// Options
	for (std::size_t i = 0u; i < m_OptionsButtons.size(); ++i)
	{
		btn = &m_OptionsButtons[i];
		btn->destRect.w = App::WINDOW_WIDTH / 7;
		btn->destRect.h = App::WINDOW_HEIGHT / 14;
		btn->destRect.x = centerX - btn->destRect.w / 2;
		btn->destRect.y = centerY - btn->destRect.h / 2 + (static_cast<int32_t>(i) - 1) * (btn->destRect.h + btn->destRect.h / 4);
	}

	// Button "V-Sync"
	{
		btn = &m_OptionsButtons.at(0);
		btn->m_Label = Label(btn->destRect.x + btn->destRect.w / 2, btn->destRect.y + btn->destRect.h / 4, "V-Sync", defaultFont, SDL_Color{ 255, 255, 255, 255 }, nullptr, true);
		const SDL_Rect &labelRect = btn->m_Label.GetRect();
		btn->m_Label.UpdatePos(labelRect.x - labelRect.w / 2, labelRect.y);

		btn->m_Type = ButtonType::check;
		btn->m_IsChecked = false;
	}
	// Options

	// Levels
	for (std::size_t i = 0u; i < m_LevelsButtons.size(); ++i)
	{
		btn = &m_LevelsButtons[i];
		btn->destRect.w = App::WINDOW_WIDTH / 7;
		btn->destRect.h = App::WINDOW_HEIGHT / 14;
		btn->destRect.x = centerX - btn->destRect.w / 2;
		btn->destRect.y = centerY - btn->destRect.h / 2 + (static_cast<int32_t>(i) - 1) * MainMenu::s_GapBetweenButtons;

		btn->m_Label = Label(btn->destRect.x + btn->destRect.w / 2, btn->destRect.y + btn->destRect.h / 4, std::format("Level #{}", i + 1), defaultFont, SDL_Color{ 255, 255, 255, 255 }, nullptr, true);
		const SDL_Rect &labelRect = btn->m_Label.GetRect();
		btn->m_Label.UpdatePos(labelRect.x - labelRect.w / 2, labelRect.y);
	}
	// Levels
}

void MainMenu::Render()
{
	static constexpr SDL_Rect srcRect{ 0, 0, 1500, 1500 };
	static SDL_Texture *background = App::s_Textures.GetTexture("szpaku");
	TextureManager::DrawTexture(background, srcRect, s_BgDestRect);

	m_ReturnButton.Draw();

	switch (s_State)
	{
		case MenuState::primary:
			for (auto &button : m_PrimaryButtons)
				button.Draw();
			return;
		case MenuState::options:
			for (auto &button : m_OptionsButtons)
				button.Draw();
			return;
		case MenuState::levels:
			for (auto &button : m_LevelsButtons)
				button.Draw();
			return;
		default:
			return;
	}
}

void MainMenu::HandleMouseButtonEvent()
{
	static Mix_Chunk *clickSound = App::s_Textures.GetSound("selectButton");

	// Don't do anything if mouse isn't pointing at any button
	if (!m_HoveredButton)
		return;

	App::s_Logger.AddLog(std::format("Pressed button {} (Menu state: {})", m_HoveredButton->m_Label.GetText(), static_cast<std::size_t>(s_State)));

	Mix_PlayChannel(-1, clickSound, 0);

	// Check if the pointed button is the one for returning/quitting
	if (m_HoveredButton == &m_ReturnButton)
	{
		auto centerY = App::WINDOW_HEIGHT / 2;

		switch (s_State)
		{
		case MenuState::primary: // Quit
			App::s_IsRunning = false;
			break;
		case MenuState::options: // Return to primary
		case MenuState::levels: // Return to primary
			m_HoveredButton->destRect.y = centerY - m_HoveredButton->destRect.h / 2 + static_cast<int32_t>(m_PrimaryButtons.size()) * MainMenu::s_GapBetweenButtons;
			m_HoveredButton->m_Label.UpdateText(quitText);
			const SDL_Rect &labelRect = m_HoveredButton->m_Label.GetRect();
			m_HoveredButton->m_Label.UpdatePos((m_HoveredButton->destRect.x + m_HoveredButton->destRect.w / 2) - labelRect.w / 2,
				m_HoveredButton->destRect.y + m_HoveredButton->destRect.h / 4);

			s_State = MenuState::primary;
			OnCursorMove(); // Look again for hovered button
			break;
		}

		SetHoveredButton(nullptr);

		return;
	}

	switch (s_State)
	{
	case MenuState::primary:
		HandleTitleButtons();
		return;
	case MenuState::options:
		HandleOptionsButtons();
		return;
	case MenuState::levels:
		HandleLevelsButtons();
		return;
	}
}

void MainMenu::HandleTitleButtons()
{
	auto centerY = App::WINDOW_HEIGHT / 2;

	if (m_HoveredButton == &m_PrimaryButtons.at(0))
	{
		m_ReturnButton.destRect.y = centerY - m_ReturnButton.destRect.h / 2 + static_cast<int32_t>(m_LevelsButtons.size()) * MainMenu::s_GapBetweenButtons;
		m_ReturnButton.m_Label.UpdateText(returnText);
		const SDL_Rect &labelRect = m_ReturnButton.m_Label.GetRect();
		m_ReturnButton.m_Label.UpdatePos((m_ReturnButton.destRect.x + m_ReturnButton.destRect.w / 2) - labelRect.w / 2,
			m_ReturnButton.destRect.y + m_ReturnButton.destRect.h / 4);

		s_State = MenuState::levels;
		SetHoveredButton(nullptr);
		OnCursorMove(); // Look again for hovered button
	}
	else if (m_HoveredButton == &m_PrimaryButtons.at(1))
	{
		m_ReturnButton.destRect.y = centerY - m_ReturnButton.destRect.h / 2 + static_cast<int32_t>(m_OptionsButtons.size()) * MainMenu::s_GapBetweenButtons;
		m_ReturnButton.m_Label.UpdateText(returnText);
		const SDL_Rect &labelRect = m_ReturnButton.m_Label.GetRect();
		m_ReturnButton.m_Label.UpdatePos((m_ReturnButton.destRect.x + m_ReturnButton.destRect.w / 2) - labelRect.w / 2,
			m_ReturnButton.destRect.y + m_ReturnButton.destRect.h / 4);

		s_State = MenuState::options;
		SetHoveredButton(nullptr);
		OnCursorMove(); // Look again for hovered button
	}
}

void MainMenu::HandleOptionsButtons()
{
	if (m_HoveredButton == &m_OptionsButtons.at(0))
	{
		m_HoveredButton->m_IsChecked = !m_HoveredButton->m_IsChecked;
		SDL_RenderSetVSync(App::s_Renderer, m_HoveredButton->m_IsChecked);
		App::s_Logger.AddLog(std::format("Changed V-Sync to {}", static_cast<int32_t>(m_HoveredButton->m_IsChecked)));
	}
}

void MainMenu::HandleLevelsButtons()
{
	for (std::size_t i = 0u; i < m_LevelsButtons.size(); i++)
	{
		if (&m_LevelsButtons[i] != m_HoveredButton)
			continue;

		if (!App::Instance().SetCurrentLevel(i))
		{
			App::s_Logger.AddLog(std::format("MainMenu::HandleLevelsButtons: Assigning chosen level (#{}) failed, loading level has been stopped", i + 1));
			return;
		}

		break;
	}

	m_ReturnButton.destRect.y = (App::WINDOW_HEIGHT / 2) - (m_ReturnButton.destRect.h / 2) + (static_cast<int32_t>(m_PrimaryButtons.size()) * MainMenu::s_GapBetweenButtons);
	m_ReturnButton.m_Label.UpdateText(quitText);
	const SDL_Rect &labelRect = m_ReturnButton.m_Label.GetRect();
	m_ReturnButton.m_Label.UpdatePos((m_ReturnButton.destRect.x + m_ReturnButton.destRect.w / 2) - labelRect.w / 2,
		m_ReturnButton.destRect.y + m_ReturnButton.destRect.h / 4);
	
	LoadLevel();

	MainMenu::s_State = MenuState::primary;

	SetHoveredButton(nullptr);
}

void MainMenu::OnCursorMove()
{
	if (m_HoveredButton)
	{
		// Check if mouse is still pointing at the same button as before
		if (IsMousePointingAt(*m_HoveredButton))
			return;

		SetHoveredButton(nullptr);
	}

	if (IsMousePointingAt(m_ReturnButton))
	{
		SetHoveredButton(&m_ReturnButton);
		return;
	}

	// Check for every button for specific menu state if it's the one mouse is pointing
	switch (s_State)
	{
	case MenuState::primary:
		for (auto &button : m_PrimaryButtons)
		{
			if (!IsMousePointingAt(button))
				continue;

			SetHoveredButton(&button);
		}
		return;
	case MenuState::options:
		for (auto &button : m_OptionsButtons)
		{
			if (!IsMousePointingAt(button))
				continue;

			SetHoveredButton(&button);
		}
		return;
	case MenuState::levels:
		for (auto &button : m_LevelsButtons)
		{
			if (!IsMousePointingAt(button))
				continue;

			SetHoveredButton(&button);
		}
		return;
	}
}

void MainMenu::OnResolutionChange()
{
	int32_t centerX = App::WINDOW_WIDTH / 2;
	int32_t centerY = App::WINDOW_HEIGHT / 2;

	// Adjust return button's X position to the new resolution
	m_ReturnButton.destRect.x = centerX - m_ReturnButton.destRect.w / 2;

	// Adjust return button's Y position to the new resolution for specific menu state
	// Have to look at current menu state, because of different amount of buttons
	switch (s_State)
	{
	case MenuState::primary:
		m_ReturnButton.destRect.y = centerY - m_ReturnButton.destRect.h / 2 + static_cast<int32_t>(m_PrimaryButtons.size()) * s_GapBetweenButtons;
		break;
	case MenuState::options:
		m_ReturnButton.destRect.y = centerY - m_ReturnButton.destRect.h / 2 + static_cast<int32_t>(m_OptionsButtons.size()) * s_GapBetweenButtons;
		break;
	case MenuState::levels:
		m_ReturnButton.destRect.y = centerY - m_ReturnButton.destRect.h / 2 + static_cast<int32_t>(m_LevelsButtons.size()) * s_GapBetweenButtons;
		break;
	}
	
	// Update text's position from return button
	{
		const SDL_Rect &labelRect = m_ReturnButton.m_Label.GetRect();
		m_ReturnButton.m_Label.UpdatePos(
			(m_ReturnButton.destRect.x + m_ReturnButton.destRect.w / 2) - labelRect.w / 2,
			m_ReturnButton.destRect.y + m_ReturnButton.destRect.h / 4
		);
	}

	Button *btn = nullptr;

	// Title
	for (std::size_t i = 0u; i < m_PrimaryButtons.size(); ++i)
	{
		btn = &m_PrimaryButtons[i];
		btn->destRect.x = centerX - btn->destRect.w / 2;
		btn->destRect.y = centerY - btn->destRect.h / 2 + (static_cast<int32_t>(i) - 1) * (btn->destRect.h + btn->destRect.h / 4);

		btn->m_Label.UpdatePos(btn->destRect.x + btn->destRect.w / 2 - btn->m_Label.GetRect().w / 2, btn->destRect.y + btn->destRect.h / 4);
	}

	// Options
	for (std::size_t i = 0u; i < m_OptionsButtons.size(); ++i)
	{
		btn = &m_OptionsButtons[i];
		btn->destRect.x = centerX - btn->destRect.w / 2;
		btn->destRect.y = centerY - btn->destRect.h / 2 + (static_cast<int32_t>(i) - 1) * (btn->destRect.h + btn->destRect.h / 4);

		btn->m_Label.UpdatePos(btn->destRect.x + btn->destRect.w / 2 - btn->m_Label.GetRect().w / 2, btn->destRect.y + btn->destRect.h / 4);
	}
	// Options
	
	// Levels
	for (std::size_t i = 0u; i < m_LevelsButtons.size(); ++i)
	{
		btn = &m_LevelsButtons[i];
		btn->destRect.x = centerX - btn->destRect.w / 2;
		btn->destRect.y = centerY - btn->destRect.h / 2 + (static_cast<int32_t>(i) - 1) * (btn->destRect.h + btn->destRect.h / 4);

		btn->m_Label.UpdatePos(btn->destRect.x + btn->destRect.w / 2 - btn->m_Label.GetRect().w / 2, btn->destRect.y + btn->destRect.h / 4);
	}
	// Levels

	s_BgDestRect.w = App::WINDOW_WIDTH;
	s_BgDestRect.h = App::WINDOW_HEIGHT;
}

void MainMenu::SetHoveredButton(Button *button)
{
	static Mix_Chunk *hoverSound = App::s_Textures.GetSound("hoverButton");

	if (m_HoveredButton)
	{
		m_HoveredButton->m_IsHovered = false;
	}

	if (!button)
	{
		m_HoveredButton = nullptr;
		return;
	}

	button->m_IsHovered = true;
	m_HoveredButton = button;
	Mix_PlayChannel(-1, hoverSound, 0);
}

void MainMenu::LoadLevel() const
{
	App::LoadLevel();
	App::Instance().SetUIState(UIState::none);
	App::UpdateWaves();
	App::UpdateLifes();
	App::s_CurrentLevel->SetBaseActive(true);

	App::Instance().OnResolutionChange();
	App::Instance().UpdateCamera();
}

const bool MainMenu::IsMousePointingAt(const Button &button) const
{
	const SDL_Rect &destRect = button.destRect;

	if (App::s_MouseX >= destRect.x && App::s_MouseX <= destRect.x + destRect.w
		&& App::s_MouseY >= destRect.y && App::s_MouseY <= destRect.y + destRect.h)
	{
		return true;
	}

	return false;
}