#include "tile.h"
#include "../app.h"
#include "../textureManager.h"

Tile::Tile(int srcX, int srcY, int posX, int posY, int tileSize, int tileScale, std::string_view textureID, TileTypes type) 
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

Tile::~Tile()
{
	if (static_cast<Enemy*>(m_EntityOccupying))
	{
		static_cast<Enemy*>(m_EntityOccupying)->SetOccupiedTile(nullptr);
	}
}

void Tile::Update()
{
	destRect.x = static_cast<int32_t>(m_Pos.x - App::s_Camera.x);
	destRect.y = static_cast<int32_t>(m_Pos.y - App::s_Camera.y);
}

void Tile::Draw()
{
	TextureManager::DrawTexture(m_Texture, srcRect, destRect);
}