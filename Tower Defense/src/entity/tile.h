#pragma once
#include "entity.h"
#include "../Vector2D.h"

#include "SDL.h"

// This enum is adjusted to layers
// 0 suits to all tiles placed at first place
// 1 suits to all additional stuff, but without collision
// 2 suits to layer with collisions (e.g. trees) including spawners
enum class TileTypes
{
	regular = 0,
	additional,
	spawner
};

class Tile : public Entity
{
public:
	Tile(int srcX, int srcY, int posX, int posY, int tileSize, int tileScale, std::string_view textureID, TileTypes type = TileTypes::regular);

	void Update() override;
	void Draw() override;

	// Position for Tile class is already scaled tile size with map scale
	Vector2D GetPos() const override { return m_Pos; }
	void SetPos(Vector2D newPos) { m_Pos = newPos; }
	void SetPos(float x, float y) { m_Pos = { x, y }; }
	void SetPosX(float x) { m_Pos.x = x; }
	void SetPosY(float y) { m_Pos.y = y; }

	inline void Move(float x, float y)
	{ 
		m_Pos = { 
			m_Pos.x + (x * destRect.w), 
			m_Pos.y + (y * destRect.h) 
		};
	}
	void Move(Vector2D newPos) { Move(newPos.x, newPos.y); }

	int32_t GetWidth() const { return destRect.w; }
	int32_t GetHeight() const { return destRect.h; }

	Entity* GetOccupyingEntity() const { return m_EntityOccupying; }
	void SetOccupyingEntity(Entity* entity) { m_EntityOccupying = entity; }
private:
	SDL_Rect srcRect{ 0, 0, 24, 24 }, destRect{ 0, 0, 24, 24 };
	Vector2D m_Pos;
	std::string_view m_TextureID;
	SDL_Texture* m_Texture = nullptr;
	TileTypes m_Type;

	Entity* m_EntityOccupying = nullptr;
};