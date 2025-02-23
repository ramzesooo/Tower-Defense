#include "UI.h"
#include "app.h"
#include "entity/towers/tower.h"
#include "textureManager.h"

#include "SDL_image.h"
#include "SDL_ttf.h"

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

TowerType UIElement::s_ChosenTower = TowerType::classic;

uint32_t UIElement::s_Timer = 0u;

void UIElement::InitUI()
{
	TTF_Font *font = App::s_Textures.GetFont("default");
	constexpr std::array<int32_t, 2> startPos{ 0, 0 }; // x, y
	constexpr std::array<int32_t, 2> elementImageSize{ 38, 12 }; // width, height
	//static SDL_Rect destRect{ static_cast<int32_t>(App::s_Camera.w / 30.0f), static_cast<int32_t>(App::s_Camera.h / 30.0f), UIElement::srcRect.w * 3, UIElement::srcRect.h * 3};
	SDL_Rect destRect{ startPos[0], startPos[1], elementImageSize[0] * 3, elementImageSize[1] * 3 };

	App::s_UIElements[0].m_DefaultText = "Wave: 1/1";
	App::s_UIElements[1].m_DefaultText = "100";
	App::s_UIElements[2].m_DefaultText = "100";
	App::s_UIElements[3].m_DefaultText = "0.000";

	for (std::size_t i = 0u; i < App::s_UIElements.size(); i++)
	{
		auto &element = App::s_UIElements[i];
		element.destRect = destRect;
		element.m_Label = Label(destRect.x, destRect.y, element.m_DefaultText, font, SDL_Color{ 255, 255, 255, 255 }, nullptr, true);
		const SDL_Rect &labelRect = element.m_Label.GetRect();
		element.m_Label.UpdatePos(labelRect.x + (destRect.w / 2 - labelRect.w / 2), labelRect.y + (destRect.h / 2 - labelRect.h / 2));

		switch (i)
		{
		case 1: // coins
			UIElement::coinDestRect = { destRect.x + UIElement::coinRect.w, destRect.y + destRect.h / 4, UIElement::coinRect.w * 3, element.destRect.h / 2 };
			App::s_UICoinsNotification = Label(labelRect.x + labelRect.w, labelRect.y, "+0", font, SDL_Color{ 255, 255, 255, 255 }, nullptr, true);
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

	for (uint32_t i = 0u; i < Tower::s_TowerTypeSize; i++)
	{
		auto &element = App::s_ExpandingTowers[i];
		element.destRect = hammerDestRect;
		element.destRect.x += element.destRect.w * (i % 2);
		element.destRect.y += (element.destRect.h * (i / 2)) + element.destRect.h;
	}

	App::s_ExpandingTowers.front().m_IsPressed = true;

	UIElement::sellDestRect = UIElement::upgradeDestRect = { startPos[0] + destRect.w + destRect.w / 4, startPos[1], destRect.w / 3, destRect.h};

	//UIElement::upgradeDestRect = UIElement::sellDestRect;
	UIElement::upgradeDestRect.y += destRect.h;
}

void UIElement::DrawUI()
{
	for (std::size_t i = 0u; i < App::s_UIElements.size(); i++)
	{
		App::s_UIElements[i].DrawElement();
	}

	if (App::s_UIState == UIState::building)
	{
		for (std::size_t i = 0u; i < Tower::s_TowerTypeSize; i++)
		{
			auto &element = App::s_ExpandingTowers[i];
			//TextureManager::DrawTexture(Tower::s_TowerTextures[i][1], UIElement::expandingTowerSrcRect, element.destRect);
			TextureManager::DrawFullTexture(Tower::s_TowerTextures[i][1], element.destRect);
		}

		//TextureManager::DrawTexture(s_TransparentGreenTexture, UIElement::expandingTowerSrcRect, App::s_ExpandingTowers.at(static_cast<std::size_t>(UIElement::s_ChosenTower)).destRect);
		TextureManager::DrawFullTexture(s_TransparentGreenTexture, App::s_ExpandingTowers.at(static_cast<std::size_t>(UIElement::s_ChosenTower)).destRect);
	}

	//TextureManager::DrawTexture(UIElement::s_CoinTexture, UIElement::coinRect, UIElement::coinDestRect);
	TextureManager::DrawFullTexture(UIElement::s_CoinTexture, UIElement::coinDestRect);
	//TextureManager::DrawTexture(UIElement::s_HeartTexture, UIElement::heartRect, UIElement::heartDestRect);
	TextureManager::DrawFullTexture(UIElement::s_HeartTexture, UIElement::heartDestRect);
	//TextureManager::DrawTexture(UIElement::s_TimerTexture, UIElement::timerRect, UIElement::timerDestRect);
	TextureManager::DrawFullTexture(UIElement::s_TimerTexture, UIElement::timerDestRect);

	if (App::s_UIState == UIState::building)
		//TextureManager::DrawTexture(UIElement::s_HammerGreenTexture, UIElement::hammerRect, UIElement::hammerDestRect);
		TextureManager::DrawFullTexture(UIElement::s_HammerGreenTexture, UIElement::hammerDestRect);
	else
		//TextureManager::DrawTexture(UIElement::s_HammerTexture, UIElement::hammerRect, UIElement::hammerDestRect);
		TextureManager::DrawFullTexture(UIElement::s_HammerTexture, UIElement::hammerDestRect);

	if (App::s_UIState == UIState::upgrading)
	{
		for (auto i = 0; i < 2; i++)
			//TextureManager::DrawTexture(UIElement::s_UpgradeTexture, UIElement::upgradeRect, UIElement::upgradeDestRect);
			TextureManager::DrawFullTexture(UIElement::s_UpgradeTexture, UIElement::upgradeDestRect);
	}
	else
	{
		//TextureManager::DrawTexture(UIElement::s_UpgradeTexture, UIElement::upgradeRect, UIElement::upgradeDestRect);
		TextureManager::DrawFullTexture(UIElement::s_UpgradeTexture, UIElement::upgradeDestRect);
	}

	if (App::s_UIState == UIState::selling)
	{
		for (auto i = 0; i < 2; i++)
			//TextureManager::DrawTexture(UIElement::s_SellTexture, UIElement::sellRect, UIElement::sellDestRect);
			TextureManager::DrawFullTexture(UIElement::s_SellTexture, UIElement::sellDestRect);
	}
	else
	{
		//TextureManager::DrawTexture(UIElement::s_SellTexture, UIElement::sellRect, UIElement::sellDestRect);
		TextureManager::DrawFullTexture(UIElement::s_SellTexture, UIElement::sellDestRect);
	}
	
	App::s_UICoinsNotification.Draw();
}

void UIElement::DrawElement()
{
	//TextureManager::DrawTexture(UIElement::s_BgTexture, UIElement::srcRect, destRect);
	TextureManager::DrawFullTexture(UIElement::s_BgTexture, destRect);
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
				TextureManager::DrawFullTexture(s_HoveredButtonChecked, destRect);
			}
			else
			{
				TextureManager::DrawFullTexture(s_HoveredButtonUnchecked, destRect);
			}
		}
		else
		{
			if (m_IsChecked)
			{
				TextureManager::DrawFullTexture(s_DefaultButtonChecked, destRect);
			}
			else
			{
				TextureManager::DrawFullTexture(s_DefaultButtonUnchecked, destRect);
			}
		}

		m_Label.Draw();
		return;
	}

	if (m_IsHovered)
		TextureManager::DrawFullTexture(s_HoveredButton, destRect);
	else
		TextureManager::DrawFullTexture(s_DefaultButton, destRect);

	m_Label.Draw();
}