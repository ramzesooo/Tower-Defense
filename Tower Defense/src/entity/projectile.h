#pragma once
#include "entity.h"
#include "enemy.h"
#include "../Vector2D.h"

#include "SDL.h"

enum class ProjectileType
{
	arrow = 0,
	size
};

class Attacker;

class Projectile : public Entity
{
public:
	Projectile(ProjectileType type, Attacker* owner, Enemy* enemy);

	void Update() override;
	void Draw() override;
private:
	SDL_Texture* m_Texture;
	SDL_Rect srcRect{ 0, 0, 16, 16 }, destRect{ 0, 0, 18, 18 };
	Vector2D m_Pos;
	Vector2D m_Velocity{ 0.0f, 0.0f };
	Vector2D m_Destination;
	ProjectileType m_Type;
	Attacker* m_Owner;
	Enemy* m_Target;
};