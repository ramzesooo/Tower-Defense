#include "UI.h"
#include "app.h"

#include "textureManager.h"

SDL_Rect UIElement::coinDestRect{};
SDL_Rect UIElement::heartDestRect{};
SDL_Rect UIElement::timerDestRect{};
SDL_Rect UIElement::hammerDestRect{};
SDL_Rect UIElement::sellDestRect{};
SDL_Rect UIElement::upgradeDestRect{};

SDL_Texture *UIElement::s_BgTexture = nullptr;
SDL_Texture *UIElement::s_CoinTexture = nullptr;
SDL_Texture *UIElement::s_HeartTexture = nullptr;
SDL_Texture *UIElement::s_TimerTexture = nullptr;
SDL_Texture *UIElement::s_HammerTexture = nullptr;
SDL_Texture *UIElement::s_HammerGreenTexture = nullptr;
SDL_Texture *UIElement::s_SellTexture = nullptr;
SDL_Texture *UIElement::s_UpgradeTexture = nullptr;
SDL_Texture *UIElement::s_TransparentGreenTexture = nullptr;

std::array<SDL_Texture*, std::size_t(TowerType::size)> UIElement::s_ExpandingTowersIcons;

TowerType UIElement::s_ChosenTower = TowerType::size;

void UIElement::InitUI()
{
	static TTF_Font *font = App::s_Textures.GetFont("default");
	constexpr int32_t startX = 0;
	constexpr int32_t startY = 0;
	//static SDL_Rect destRect{ static_cast<int32_t>(App::s_Camera.w / 30.0f), static_cast<int32_t>(App::s_Camera.h / 30.0f), UIElement::srcRect.w * 3, UIElement::srcRect.h * 3};
	SDL_Rect destRect{ startX, startY, UIElement::srcRect.w * 3, UIElement::srcRect.h * 3 };

	App::s_UIElements.at(0).m_DefaultText = "Wave: 1/1";
	App::s_UIElements.at(1).m_DefaultText = "100";
	App::s_UIElements.at(2).m_DefaultText = "100";
	App::s_UIElements.at(3).m_DefaultText = "0:00";

	for (std::size_t i = 0u; i < App::s_UIElements.size(); i++)
	{
		auto &element = App::s_UIElements.at(i);
		element.destRect = destRect;
		element.m_Label = Label(destRect.x, destRect.y, element.m_DefaultText, font);
		const SDL_Rect &labelRect = element.m_Label.GetRect();
		element.m_Label.UpdatePos(labelRect.x + (destRect.w / 2 - labelRect.w / 2), labelRect.y + (destRect.h / 2 - labelRect.h / 2));

		switch (i)
		{
		case 1: // coins
			UIElement::coinDestRect = { destRect.x + UIElement::coinRect.w, destRect.y + destRect.h / 4, UIElement::coinRect.w * 3, element.destRect.h / 2 };
			App::s_UICoinsNotification = Label(labelRect.x + labelRect.w + labelRect.w / 2, labelRect.y, "+0", font);
			App::s_UICoinsNotification.SetAlpha(0);
			break;
		case 2: // lifes
			UIElement::heartDestRect = { destRect.x, destRect.y + destRect.h / 4, UIElement::heartRect.w - UIElement::heartRect.w / 4, destRect.h - UIElement::heartRect.h / 2 };
			break;
		case 3: // timer
			UIElement::timerDestRect = { destRect.x + UIElement::timerRect.w / 4, destRect.y + destRect.h / 4, UIElement::timerRect.w, UIElement::timerRect.h };
			break;
			// Do nothing for the rest
		case 0:
		default:
			break;
		}

		destRect.y += destRect.h;
	}

	UIElement::hammerDestRect = destRect;
	UIElement::hammerDestRect.w /= 3;

	for (int32_t i = 0; i < App::s_ExpandingTowers.size(); i++)
	{
		auto &element = App::s_ExpandingTowers.at(i);
		element.destRect = hammerDestRect;
		element.destRect.x += element.destRect.w * (i % 2);
		element.destRect.y += (element.destRect.h * (i / 2)) + element.destRect.h;
	}

	s_ChosenTower = TowerType::classic;
	App::s_ExpandingTowers.at(0).m_IsPressed = true;

	UIElement::sellDestRect = { startX + destRect.w + destRect.w / 4, startY, destRect.w / 3, destRect.h };

	UIElement::upgradeDestRect = UIElement::sellDestRect;
	UIElement::upgradeDestRect.y += destRect.h;
}

void UIElement::DrawUI()
{
	for (std::size_t i = 0u; i < App::s_UIElements.size(); i++)
	{
		App::s_UIElements.at(i).DrawElement();
	}

	if (App::s_UIState == UIState::building)
	{
		for (std::size_t i = 0u; i < App::s_ExpandingTowers.size(); i++)
		{
			auto &element = App::s_ExpandingTowers.at(i);
			TextureManager::DrawTexture(UIElement::s_ExpandingTowersIcons.at(i), UIElement::expandingTowerSrcRect, element.destRect);
		}

		TextureManager::DrawTexture(s_TransparentGreenTexture, UIElement::expandingTowerSrcRect, App::s_ExpandingTowers.at(static_cast<std::size_t>(UIElement::s_ChosenTower)).destRect);
	}

	TextureManager::DrawTexture(UIElement::s_CoinTexture, UIElement::coinRect, UIElement::coinDestRect);
	TextureManager::DrawTexture(UIElement::s_HeartTexture, UIElement::heartRect, UIElement::heartDestRect);
	TextureManager::DrawTexture(UIElement::s_TimerTexture, UIElement::timerRect, UIElement::timerDestRect);

	if (App::s_UIState == UIState::building)
		TextureManager::DrawTexture(UIElement::s_HammerGreenTexture, UIElement::hammerRect, UIElement::hammerDestRect);
	else
		TextureManager::DrawTexture(UIElement::s_HammerTexture, UIElement::hammerRect, UIElement::hammerDestRect);

	if (App::s_UIState == UIState::upgrading)
	{
		TextureManager::DrawTexture(UIElement::s_UpgradeTexture, UIElement::upgradeRect, UIElement::upgradeDestRect);
		TextureManager::DrawTexture(UIElement::s_UpgradeTexture, UIElement::upgradeRect, UIElement::upgradeDestRect);
	}
	else
	{
		TextureManager::DrawTexture(UIElement::s_UpgradeTexture, UIElement::upgradeRect, UIElement::upgradeDestRect);
	}

	if (App::s_UIState == UIState::selling)
	{
		TextureManager::DrawTexture(UIElement::s_SellTexture, UIElement::sellRect, UIElement::sellDestRect);
		TextureManager::DrawTexture(UIElement::s_SellTexture, UIElement::sellRect, UIElement::sellDestRect);
	}
	else
	{
		TextureManager::DrawTexture(UIElement::s_SellTexture, UIElement::sellRect, UIElement::sellDestRect);
	}
	
	App::s_UICoinsNotification.Draw();
}

void UIElement::DrawElement()
{
	TextureManager::DrawTexture(UIElement::s_BgTexture, UIElement::srcRect, destRect);
	m_Label.Draw();
}

// BUTTON

void Button::Draw()
{
	if (m_Type == ButtonType::check)
	{
		if (m_IsHovered)
		{
			if (m_IsChecked)
			{
				TextureManager::DrawTexture(s_HoveredButtonChecked, srcRect, destRect);
			}
			else
			{
				TextureManager::DrawTexture(s_HoveredButtonUnchecked, srcRect, destRect);
			}
		}
		else
		{
			if (m_IsChecked)
			{
				TextureManager::DrawTexture(s_DefaultButtonChecked, srcRect, destRect);
			}
			else
			{
				TextureManager::DrawTexture(s_DefaultButtonUnchecked, srcRect, destRect);
			}
		}

		m_Label.Draw();
		return;
	}

	if (m_IsHovered)
		TextureManager::DrawTexture(s_HoveredButton, srcRect, destRect);
	else
		TextureManager::DrawTexture(s_DefaultButton, srcRect, destRect);

	m_Label.Draw();
}