#pragma once
#include "tile.h"
#include "../Vector2D.h"

#include "SDL_rect.h"
#include "SDL_render.h"

class Base
{
public:
	//static constexpr SDL_Rect srcRect{ 0, 0, 24, 24 };
	bool m_IsActive = true;
	SDL_Rect destRect{ 0, 0, 24, 24 };
	Vector2D m_Pos{ 0.0f, 0.0f };
	SDL_Texture *m_Texture = nullptr;
	Tile *m_Tile = nullptr;

	uint16_t m_Lifes = 0u;
	uint16_t m_MaxLifes = 0u;
public:
	const SDL_Rect &GetRect() const { return destRect; }
	const Vector2D &GetPos() const { return m_Pos; }

	void Draw() const;
	void AdjustToView();
};