#pragma once
#include "../Vector2D.h"

#include "SDL.h"

enum class ProjectileType
{
	arrow = 0,
	size
};

class Projectile
{
public:
	Projectile();

	void Update();
	void Draw();
private:
	SDL_Texture* m_Texture;
	SDL_Rect srcRect{ 0, 0, 16, 16 }, destRect{ 0, 0, 16, 16 };
	Vector2D m_Pos;
	Vector2D m_Velocity;
	Vector2D m_Destination;
	ProjectileType m_Type;
};