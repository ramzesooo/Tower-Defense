#include "menu.h"
#include "app.h"

SDL_Texture *Button::s_DefaultButton = nullptr;
SDL_Texture *Button::s_HoveredButton = nullptr;

MenuState MainMenu::s_State = MenuState::primary;

MainMenu::MainMenu()
{
	int32_t centerX = App::WINDOW_WIDTH / 2;
	int32_t centerY = App::WINDOW_HEIGHT / 2;

	// TODO: Recalculate destination rectangle after changing resolution and whatsoever
	for (auto i = 0; i < m_PrimaryButtons.size(); ++i)
	{
		m_PrimaryButtons.at(i).destRect.w = App::WINDOW_WIDTH / 7;
		m_PrimaryButtons.at(i).destRect.h = App::WINDOW_HEIGHT / 14;
		m_PrimaryButtons.at(i).destRect.x = centerX - m_PrimaryButtons.at(i).destRect.w / 2;
		m_PrimaryButtons.at(i).destRect.y = centerY - m_PrimaryButtons.at(i).destRect.h / 2 + (i - 1) * (m_PrimaryButtons.at(i).destRect.h + m_PrimaryButtons.at(i).destRect.h / 4);
	}
}

void MainMenu::Render()
{
	static constexpr SDL_Rect srcRect{ 0, 0, 1500, 1500 };
	TextureManager::DrawTexture(App::s_Textures.GetTexture("szpaku"), srcRect, { 0, 0, App::WINDOW_WIDTH, App::WINDOW_HEIGHT });

	switch (s_State)
	{
		case MenuState::primary:
			for (auto i = 0; i < m_PrimaryButtons.size(); ++i)
				m_PrimaryButtons.at(i).Draw();

			return;
		default:
			return;
	}
}