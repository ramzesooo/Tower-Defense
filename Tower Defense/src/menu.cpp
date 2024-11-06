#include "menu.h"
#include "app.h"

SDL_Texture *Button::s_DefaultButton = nullptr;
SDL_Texture *Button::s_HoveredButton = nullptr;

MenuState MainMenu::s_State = MenuState::primary;

MainMenu::MainMenu()
{
	int32_t centerX = App::WINDOW_WIDTH / 2;
	int32_t centerY = App::WINDOW_HEIGHT / 2;

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

void MainMenu::HandleMouseButtonEvent()
{
	if (!m_HoveredButton)
		return;

	App::s_Logger.AddLog("Pressed button " + m_HoveredButton->m_Label.GetText());

	switch (s_State)
	{
	case MenuState::primary:
		if (m_HoveredButton == &m_PrimaryButtons.at(0))
		{
			App::LoadLevel();
			App::Instance().SetUIState(UIState::none);
			App::Instance().UpdateCamera();
			App::UpdateWaves();
			App::UpdateLifes();
			App::s_CurrentLevel->GetBase()->m_IsActive = true;
 		}
		else if (m_HoveredButton == &m_PrimaryButtons.at(1))
		{

		}
		else if (m_HoveredButton == &m_PrimaryButtons.at(2))
		{
			m_HoveredButton = nullptr;
			App::s_IsRunning = false;
		}
		return;
	}
}

void MainMenu::OnCursorMove()
{
	if (m_HoveredButton)
	{
		const SDL_Rect &destRect = m_HoveredButton->destRect;

		if (App::s_MouseX >= destRect.x && App::s_MouseX <= destRect.x + destRect.w
			&& App::s_MouseY >= destRect.y && App::s_MouseY <= destRect.y + destRect.h)
		{
			return;
		}
		else
		{
			m_HoveredButton->m_IsHovered = false;
			m_HoveredButton = nullptr;
		}
	}

	switch (s_State)
	{
	case MenuState::primary:
		for (auto i = 0; i < m_PrimaryButtons.size(); ++i)
		{
			const SDL_Rect &destRect = m_PrimaryButtons.at(i).destRect;

			if (App::s_MouseX >= destRect.x && App::s_MouseX <= destRect.x + destRect.w
				&& App::s_MouseY >= destRect.y && App::s_MouseY <= destRect.y + destRect.h)
			{
				m_PrimaryButtons.at(i).m_IsHovered = true;
				m_HoveredButton = &m_PrimaryButtons.at(i);
				return;
			}
		}
		return;
	}
}

void MainMenu::OnResolutionChange(bool init)
{
	int32_t centerX = App::WINDOW_WIDTH / 2;
	int32_t centerY = App::WINDOW_HEIGHT / 2;

	if (init)
	{
		for (auto i = 0; i < m_PrimaryButtons.size(); ++i)
		{
			Button *btn = &m_PrimaryButtons.at(i);
			btn->destRect.w = App::WINDOW_WIDTH / 7;
			btn->destRect.h = App::WINDOW_HEIGHT / 14;
			btn->destRect.x = centerX - btn->destRect.w / 2;
			btn->destRect.y = centerY - btn->destRect.h / 2 + (i - 1) * (btn->destRect.h + btn->destRect.h / 4);
		}
	}
	else
	{
		for (auto i = 0; i < m_PrimaryButtons.size(); ++i)
		{
			Button *btn = &m_PrimaryButtons.at(i);
			btn->destRect.x = centerX - btn->destRect.w / 2;
			btn->destRect.y = centerY - btn->destRect.h / 2 + (i - 1) * (btn->destRect.h + btn->destRect.h / 4);

			btn->m_Label.UpdatePos(btn->destRect.x + btn->destRect.w / 2 - btn->m_Label.GetRect().w / 2, btn->destRect.y + btn->destRect.h / 4);
		}
	}
}