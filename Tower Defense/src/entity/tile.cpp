#include "tile.h"
#include "tower.h"
#include "../app.h"
#include "../textureManager.h"

Tile::Tile(TileType type, int32_t tileScale) : m_Type(type)
{
	destRect.w = srcRect.w * tileScale * 2;
	destRect.h = srcRect.h * tileScale * 2;
}

Tile::Tile(uint32_t srcX, uint32_t srcY, uint32_t posX, uint32_t posY, int32_t tileSize, int32_t tileScale, SDL_Texture* texture, TileType type)
	: m_Pos((float)posX, (float)posY), m_Texture(texture), m_Type(type)
{
	srcRect.x = srcX;
	srcRect.y = srcY;
	srcRect.w = srcRect.h = tileSize; // 24 by default

	destRect.x = static_cast<int32_t>(posX - App::s_Camera.x);
	destRect.y = static_cast<int32_t>(posY - App::s_Camera.y);
	destRect.w = destRect.h = tileSize * tileScale;
}

void Tile::Destroy()
{
	m_IsActive = false;

	if (m_TowerOnTile)
	{
		auto& tilesFromTower = m_TowerOnTile->GetOccupiedTiles();
		Tile* tile = nullptr;
		for (uint16_t i = 0; i < tilesFromTower.size(); ++i)
		{
			tile = m_TowerOnTile->GetOccupiedTile(i);
			if (tile == this)
			{
				tilesFromTower[i] = nullptr;
				break;
			}
		}
	}
}

void Tile::AdjustToView()
{
	destRect.x = static_cast<int32_t>(m_Pos.x - App::s_Camera.x);
	destRect.y = static_cast<int32_t>(m_Pos.y - App::s_Camera.y);
}

void Tile::Draw()
{
	TextureManager::DrawTexture(m_Texture, srcRect, destRect);
}