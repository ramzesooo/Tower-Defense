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
		Button *btn = &m_PrimaryButtons.at(i);
		btn->destRect.w = App::WINDOW_WIDTH / 7;
		btn->destRect.h = App::WINDOW_HEIGHT / 14;
		btn->destRect.x = centerX - btn->destRect.w / 2;
		btn->destRect.y = centerY - btn->destRect.h / 2 + (i - 1) * (btn->destRect.h + btn->destRect.h / 4);
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