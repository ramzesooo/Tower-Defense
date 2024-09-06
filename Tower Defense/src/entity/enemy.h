#pragma once
#include "entity.h"

#include "SDL.h"

enum class EnemyType
{
	elf = 0,
	size
};

class Enemy : public Entity
{
public:
	Enemy();
private:
	SDL_Texture* m_Texture = nullptr;
};