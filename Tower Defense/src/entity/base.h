#pragma once
#include "tile.h"
#include "../Vector2D.h"

#include "SDL_rect.h"
#include "SDL_render.h"

class Base
{
public:
	static constexpr SDL_Rect srcRect{ 0, 0, 24, 24 };
	bool m_IsActive = true;
	SDL_Rect destRect{ 0, 0, 24, 24 };
	Vector2D m_Pos{ 0.0f, 0.0f };
	SDL_Texture *m_Texture = nullptr;
	Tile *m_Tile = nullptr;

	uint16_t m_Lifes = 0u;
	uint16_t m_MaxLifes = 0u;
public:
	Base() = default;
	Base(const Base &r) : m_IsActive(r.m_IsActive), destRect(r.destRect), m_Pos(r.m_Pos), m_Texture(r.m_Texture),
		m_Tile(r.m_Tile), m_Lifes(r.m_Lifes), m_MaxLifes(r.m_MaxLifes) {}
	~Base() = default;

	Base operator=(const Base &r)
	{
		if (this == &r)
		{
			return *this;
		}

		m_IsActive = r.m_IsActive;
		destRect = r.destRect;
		m_Pos = r.m_Pos;
		m_Texture = r.m_Texture;
		m_Tile = r.m_Tile;
		m_Lifes = r.m_Lifes;
		m_MaxLifes = r.m_MaxLifes;

		return *this;
	}

	const SDL_Rect& GetRect() const { return destRect; }

	void Draw() const;
	void AdjustToView();
};