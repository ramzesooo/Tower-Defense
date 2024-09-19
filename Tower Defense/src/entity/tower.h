#pragma once
#include "entity.h"
#include "attacker.h"
#include "tile.h"
#include "../level.h"
#include "../Vector2D.h"

#include "SDL.h"

class Tower : public Entity
{
public:
	Tower(float posX, float posY, SDL_Texture* texture, int32_t tier = 1);
	Tower(const Tower& r) : Entity(r), m_Texture(r.m_Texture), m_Pos(r.m_Pos), srcRect(r.srcRect), destRect(r.destRect),
		m_OccupiedTiles(r.m_OccupiedTiles), m_Attacker(r.m_Attacker) {}
	~Tower() = default;

	inline Tower& operator=(const Tower& r)
	{
		if (this == &r)
		{
			return *this;
		}

		Entity::operator=(r);
		m_Texture = r.m_Texture;
		m_Pos = r.m_Pos;
		srcRect = r.srcRect;
		destRect = r.destRect;
		m_OccupiedTiles = r.m_OccupiedTiles;
		m_Attacker = r.m_Attacker;

		return *this;
	}

	void Destroy() override;

	void AdjustToView() override;
	void Draw() override;

	Vector2D GetPos() const override { return m_Pos; }

	void AssignAttacker(Attacker* attacker) { m_Attacker = attacker; }
	Attacker* GetAttacker() const { return m_Attacker; }

	// Returns specific occupied tile from array m_OccupiedTiles
	// Or returns nullptr if ID is less than 0 or greater than array size
	// The array's size is 4, because the tower occupies just 4 tiles
	inline Tile* GetOccupiedTile(uint16_t ID) const
	{
		if (ID < 0 || ID > m_OccupiedTiles.size())
		{
			return nullptr;
		}

		return m_OccupiedTiles.at(ID);
	}

	// Returns reference to whole array m_OccupiedTiles
	std::array<Tile*, 4>& GetOccupiedTiles() { return m_OccupiedTiles; }
private:
	SDL_Texture* m_Texture = nullptr;
	Vector2D m_Pos;
	SDL_Rect srcRect{ 0, 0, 144, 64 }, destRect{ 0, 0, 48, 21 };
	std::array<Tile*, 4> m_OccupiedTiles;
	Attacker* m_Attacker = nullptr; // m_Attacker is the entity supposed to be shown in the top of tower
};