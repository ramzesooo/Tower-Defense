#include "menu.h"
#include "textureManager.h"
#include "app.h"
#include "entity/label.h"

#include "SDL_rect.h"

#include <format>

SDL_Texture *Button::s_DefaultButton = nullptr;
SDL_Texture *Button::s_HoveredButton = nullptr;

MenuState MainMenu::s_State = MenuState::primary;
SDL_Rect MainMenu::s_BgDestRect{ 0, 0, App::WINDOW_WIDTH, App::WINDOW_HEIGHT };

void MainMenu::Render()
{
	static constexpr SDL_Rect srcRect{ 0, 0, 1500, 1500 };
	static SDL_Texture *background = App::s_Textures.GetTexture("szpaku");
	TextureManager::DrawTexture(background, srcRect, s_BgDestRect);

	switch (s_State)
	{
		case MenuState::primary:
			for (std::size_t i = 0u; i < m_PrimaryButtons.size(); ++i)
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

	App::s_Logger.AddLog(std::format("Pressed button {}", m_HoveredButton->m_Label.GetText()));

	switch (s_State)
	{
	case MenuState::primary:
		if (m_HoveredButton == &m_PrimaryButtons.at(0))
		{
			App::LoadLevel();
			App::Instance().SetUIState(UIState::none);
			App::UpdateWaves();
			App::UpdateLifes();
			App::s_CurrentLevel->GetBase()->m_IsActive = true;

			App::Instance().OnResolutionChange();
			App::Instance().UpdateCamera();
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
		for (std::size_t i = 0; i < m_PrimaryButtons.size(); ++i)
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

void MainMenu::OnResolutionChange()
{
	int32_t centerX = App::WINDOW_WIDTH / 2;
	int32_t centerY = App::WINDOW_HEIGHT / 2;

	for (std::size_t i = 0u; i < m_PrimaryButtons.size(); ++i)
	{
		Button *btn = &m_PrimaryButtons.at(i);
		btn->destRect.x = centerX - btn->destRect.w / 2;
		btn->destRect.y = centerY - btn->destRect.h / 2 + (static_cast<int32_t>(i) - 1) * (btn->destRect.h + btn->destRect.h / 4);

		btn->m_Label.UpdatePos(btn->destRect.x + btn->destRect.w / 2 - btn->m_Label.GetRect().w / 2, btn->destRect.y + btn->destRect.h / 4);
	}

	s_BgDestRect.w = App::WINDOW_WIDTH;
	s_BgDestRect.h = App::WINDOW_HEIGHT;
}