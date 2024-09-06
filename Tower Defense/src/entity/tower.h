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

	Vector2D GetPos() const { return m_Pos; }

	void AssignAttacker(Attacker* attacker) { m_Attacker = attacker; }
	Attacker* GetAttacker() const { return m_Attacker; }
private:
	SDL_Texture* m_Texture = nullptr;
	Vector2D m_Pos;
	SDL_Rect srcRect{ 0, 0, 144, 64 }, destRect{ 0, 0, 48, 21 };
	std::array<Tile*, 4> occupiedTiles;
	uint16_t m_HP = 10000;
	Attacker* m_Attacker = nullptr;
};