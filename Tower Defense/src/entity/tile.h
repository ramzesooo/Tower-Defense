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
	static SDL_Texture *s_RangeTexture;
public:
	Tile() = delete;
	Tile(TileType type);
	Tile(int32_t srcX, int32_t srcY, int32_t posX, int32_t posY, SDL_Texture *texture, TileType type = TileType::regular);
	Tile(const Tile &r) : srcRect(r.srcRect), destRect(r.destRect), m_Pos(r.m_Pos), m_Texture(r.m_Texture), m_Type(r.m_Type),
		m_EntityOccupying(r.m_EntityOccupying), m_TowerOnTile(r.m_TowerOnTile), m_IsWalkable(r.m_IsWalkable), m_IsDrawable(r.m_IsDrawable) {}
	~Tile() = default;

	inline Tile &operator=(const Tile &r)
	{
		if (this == &r)
			return *this;

		srcRect = r.srcRect;
		destRect = r.destRect;
		m_Pos = r.m_Pos;
		m_Texture = r.m_Texture;
		m_Type = r.m_Type;
		m_EntityOccupying = r.m_EntityOccupying;
		m_TowerOnTile = r.m_TowerOnTile;
		m_IsWalkable = r.m_IsWalkable;
		m_IsDrawable = r.m_IsDrawable;

		return *this;
	}

	void InitSpecialTile();

	void Destroy() override;

	void Update() override {};
	void Draw() override;

	void DrawHighlight() const;

	void AdjustToView() override;

	void SetTexture(SDL_Texture* texture) { m_Texture = texture; }

	// Position for Tile class is already scaled tile size with map scale
	Vector2D GetPos() const override { return m_Pos; }
	void SetPos(const Vector2D &newPos) { m_Pos = newPos; }

	int32_t GetWidth() const { return destRect.w; }
	int32_t GetHeight() const { return destRect.h; }

	Entity* GetOccupyingEntity() const { return m_EntityOccupying; }
	void SetOccupyingEntity(Entity *entity) { m_EntityOccupying = entity; }

	// if it's nullptr then it's not occupied by any tower
	Tower *GetTowerOccupying() const { return m_TowerOnTile; }
	void SetTowerOccupying(Tower *tower) { m_TowerOnTile = tower; }

	void SetWalkable() { m_IsWalkable = true; }
	bool IsWalkable() const { return m_IsWalkable; }

	void SetDrawable(bool drawable) { m_IsDrawable = drawable; }
	bool IsDrawable() const { return m_IsDrawable; }
private:
	bool m_IsDrawable = true;
	bool m_IsWalkable = false;
	SDL_Rect srcRect{ 0, 0, 24, 24 }, destRect{ 0, 0, 24, 24 };
	Vector2D m_Pos{};
	SDL_Texture *m_Texture = nullptr;
	TileType m_Type = TileType::regular;

	Entity *m_EntityOccupying = nullptr;
	Tower *m_TowerOnTile = nullptr;
};