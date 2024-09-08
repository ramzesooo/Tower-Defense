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
	~Tower();

	void Update() override;
	void Draw() override;

	Vector2D GetPos() const override { return m_Pos; }

	void AssignAttacker(Attacker* attacker) { m_Attacker = attacker; }
	Attacker* GetAttacker() const { return m_Attacker; }

	// Returns specific occupied tile from array m_OccupiedTiles
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
	uint16_t m_HP = 10000;
	Attacker* m_Attacker = nullptr; // m_Attacker is the entity supposed to be shown in the top of tower
};