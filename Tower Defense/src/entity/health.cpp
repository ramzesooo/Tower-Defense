#include "health.h"
#include "../textureManager.h"
#include "../app.h"

SDL_Texture *Health::s_EmptyBarTexture = nullptr;
SDL_Texture *Health::s_FullBarTexture = nullptr;

BaseHealth::BaseHealth()
{
	m_DestRect = { (int32_t)App::s_Camera.w / 32, (int32_t)App::s_Camera.h / 32, 64, 58 };
}

void BaseHealth::Draw()
{
	SDL_Rect destRect = m_DestRect;
	destRect.x += destRect.w * m_HeartsAmount;

	for (auto i = m_HeartsAmount; i > 0; --i)
	{
		if (i > m_LeftHearts)
			TextureManager::DrawTexture(s_FullBarTexture, s_EmptySrcRect, destRect);
		else
			TextureManager::DrawTexture(s_FullBarTexture, s_FullSrcRect, destRect);

		destRect.x -= (destRect.w + destRect.w / 8);
	}
}