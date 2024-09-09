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
	SDL_Texture* m_Texture = nullptr;
	double m_Angle = 360;
	SDL_RendererFlip m_TextureFlip = SDL_FLIP_NONE;
	SDL_Rect srcRect{ 0, 0, 16, 16 }, destRect{ 0, 0, 18, 18 };
	Vector2D m_Pos{ 0.0f, 0.0f };
	Vector2D m_Velocity{ 0.0f, 0.0f };
	Vector2D m_Destination{ 0.0f,0.0f };
	ProjectileType m_Type;
	Attacker* m_Owner = nullptr;
	Enemy* m_Target = nullptr;
};