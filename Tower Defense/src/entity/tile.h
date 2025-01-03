#pragma once
#include "typesEnums.h"

#include "entity.h"
#include "../Vector2D.h"

#include "SDL_rect.h"
#include "SDL_render.h"

class Tower;

// It's not needed anymore to inheritance from Entity
// TODO: Adjust it to be totally independent from Entity class,
// Manager has another method for tiles NewTile() instead of NewEntity() with another vector
// It has been done to avoid iterating through a lot of tiles in Refresh() and Update() because it's stealing a lot of performance
class Tile : public Entity
{
public:
	Tile() = delete;
	Tile(TileType type, int32_t tileScale);
	Tile(uint32_t srcX, uint32_t srcY, uint32_t posX, uint32_t posY, int32_t tileSize, int32_t tileScale, SDL_Texture* texture, TileType type = TileType::regular);
	Tile(const Tile& r) : srcRect(r.srcRect), destRect(r.destRect), m_Pos(r.m_Pos), m_Texture(r.m_Texture), m_Type(r.m_Type),
		m_EntityOccupying(r.m_EntityOccupying), m_TowerOnTile(r.m_TowerOnTile), m_IsWalkable(r.m_IsWalkable) {}
	~Tile() = default;

	inline Tile& operator=(const Tile& r)
	{
		if (this == &r)
		{
			return *this;
		}

		srcRect = r.srcRect;
		destRect = r.destRect;
		m_Pos = r.m_Pos;
		m_Texture = r.m_Texture;
		m_Type = r.m_Type;
		m_EntityOccupying = r.m_EntityOccupying;
		m_TowerOnTile = r.m_TowerOnTile;
		m_IsWalkable = r.m_IsWalkable;

		return *this;
	}

	void Destroy() override;

	void Update() override {};
	void Draw() override;

	void AdjustToView() override;

	void SetTexture(SDL_Texture* texture) { m_Texture = texture; }

	// Position for Tile class is already scaled tile size with map scale
	Vector2D GetPos() const override { return m_Pos; }
	void SetPos(Vector2D newPos) { m_Pos = newPos; }
	void SetPos(float x, float y) { m_Pos = { x, y }; }
	void SetPosX(float x) { m_Pos.x = x; }
	void SetPosY(float y) { m_Pos.y = y; }

	int32_t GetWidth() const { return destRect.w; }
	int32_t GetHeight() const { return destRect.h; }

	Entity* GetOccupyingEntity() const { return m_EntityOccupying; }
	void SetOccupyingEntity(Entity* entity) { m_EntityOccupying = entity; }

	// if it's nullptr then it's not occupied by any tower
	Tower* GetTowerOccupying() const { return m_TowerOnTile; }
	void SetTowerOccupying(Tower* tower) { m_TowerOnTile = tower; }

	void SetWalkable() { m_IsWalkable = true; }
	bool IsWalkable() const { return m_IsWalkable; }
private:
	bool m_IsWalkable = false;
	SDL_Rect srcRect{ 0, 0, 24, 24 }, destRect{ 0, 0, 24, 24 };
	Vector2D m_Pos;
	SDL_Texture* m_Texture = nullptr;
	TileType m_Type;

	Entity* m_EntityOccupying = nullptr;
	Tower* m_TowerOnTile = nullptr;
};