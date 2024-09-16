#include "tower.h"
#include "../textureManager.h"
#include "../app.h"

Tower::Tower(float posX, float posY, SDL_Texture* texture, int32_t tier)
	: m_Pos(posX * App::s_CurrentLevel->m_ScaledTileSize, posY * App::s_CurrentLevel->m_ScaledTileSize), m_Texture(texture)
{
	uint16_t scaledTileSize = App::s_CurrentLevel->m_ScaledTileSize;

	if (tier > 3)
	{
		App::s_Logger->AddLog("Tried to add tower with tier higher than 3");
		tier = 3;
	}
	else if (tier < 1)
	{
		App::s_Logger->AddLog("Tried to add tower with tier lower than 1");
		tier = 1;
	}

	{
		Tile* tile = nullptr;
		int16_t i = 0;
		for (int16_t y = 0; y < 2; y++)
		{
			for (int16_t x = 0; x < 2; x++)
			{
				tile = App::s_CurrentLevel->GetTileFrom((uint32_t)posX + x, (uint32_t)posY + y);

				m_OccupiedTiles[i] = tile;
				i++;
			}
		}
	}
	
	int32_t imageWidth = 144;
	srcRect.x = (tier - 1) * (imageWidth / 3);
	srcRect.y = 0;
	srcRect.w = (imageWidth / 3);
	srcRect.h = 64;

	destRect.x = (int32_t)m_Pos.x;
	destRect.y = (int32_t)m_Pos.y;
	destRect.w = destRect.h = scaledTileSize * 2;
}

void Tower::Destroy()
{
	if (m_Attacker)
	{
		m_Attacker->Destroy();
		m_Attacker = nullptr;
	}
}

void Tower::Update()
{
	destRect.x = static_cast<int32_t>(m_Pos.x - App::s_Camera.x);
	destRect.y = static_cast<int32_t>(m_Pos.y - App::s_Camera.y);
}

void Tower::Draw()
{
	TextureManager::DrawTexture(m_Texture, srcRect, destRect);
}