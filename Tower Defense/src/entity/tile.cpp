#include "tile.h"
#include "../app.h"
#include "../textureManager.h"

Tile::Tile(uint32_t srcX, uint32_t srcY, uint32_t posX, uint32_t posY, int32_t tileSize, int32_t tileScale, std::string_view textureID, TileTypes type)
	: m_Pos((float)posX, (float)posY), m_TextureID(textureID), m_Type(type)
{
	m_Texture = App::s_Textures->GetTexture(m_TextureID);
	if (!m_Texture)
	{
		App::s_Logger->AddLog("Tile::Tile: Missing texture: " + std::string(textureID));
	}

	srcRect.x = srcX;
	srcRect.y = srcY;
	srcRect.w = srcRect.h = tileSize; // 24 by default

	destRect.x = posX;
	destRect.y = posY;
	destRect.w = destRect.h = tileSize * tileScale;
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