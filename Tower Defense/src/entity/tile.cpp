#include "tile.h"
#include "towers/tower.h"
#include "../app.h"
#include "../level.h"
#include "../textureManager.h"

SDL_Texture *Tile::s_RangeTexture = nullptr;

Tile::Tile(TileType type) : srcRect{ 0, 0, Level::s_TileSize, Level::s_TileSize }, m_Type(type) {}

Tile::Tile(int32_t srcX, int32_t srcY, int32_t posX, int32_t posY, SDL_Texture *texture, TileType type)
	: srcRect{ srcX, srcY, Level::s_TileSize, Level::s_TileSize },
	destRect{ posX - static_cast<int32_t>(App::s_Camera.x), posY - static_cast<int32_t>(App::s_Camera.y), Level::s_TileSize * App::s_CurrentLevel->m_MapData.at(2), Level::s_TileSize * App::s_CurrentLevel->m_MapData.at(2) },
	m_Pos(static_cast<float>(posX), static_cast<float>(posY)), m_Texture(texture), m_Type(type)
{}

Tile::~Tile()
{
	if (m_TowerOnTile)
	{
		auto& tilesFromTower = m_TowerOnTile->GetOccupiedTiles();
		Tile* tile = nullptr;
		for (uint16_t i = 0u; i < tilesFromTower.size(); ++i)
		{
			tile = m_TowerOnTile->GetOccupiedTile(i);
			if (tile == this)
			{
				tilesFromTower[i] = nullptr;
				return;
			}
		}
	}
}

void Tile::InitSpecialTile() // Should be called every time before loading another level in case the level has different scaling
{
	// default tile's size * scale * 2
	// times 2 because towers take 4 tiles
	destRect.w = destRect.h = Level::s_TileSize * App::s_CurrentLevel->m_MapData.at(2) * 2;
}

void Tile::Draw()
{
	TextureManager::DrawTexture(m_Texture, srcRect, destRect);
}

void Tile::DrawHighlight() const
{
	static constexpr SDL_Rect localSrcRect{ 0, 0, 24, 24 };
	TextureManager::DrawTexture(s_RangeTexture, localSrcRect, destRect);
}

void Tile::AdjustToView()
{
	destRect.x = static_cast<int32_t>(m_Pos.x - App::s_Camera.x);
	destRect.y = static_cast<int32_t>(m_Pos.y - App::s_Camera.y);
}