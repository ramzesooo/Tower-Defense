#pragma once
#include "label.h"
#include "health.h"
#include "tile.h"
#include "../Vector2D.h"

#include "SDL.h"

class Base
{
public:
	static constexpr SDL_Rect srcRect{ 0, 0, 24, 24 };
	bool m_IsActive = true;
	SDL_Rect destRect{ 0, 0, 24, 24 };
	Vector2D m_Pos{ 0, 0 };
	SDL_Texture *m_Texture = nullptr;
#ifdef _DEBUG
	Label *m_AttachedLabel = nullptr;
#endif
	Tile *m_Tile = nullptr;

	RectHP m_RectHP;
	uint16_t m_HP = 0;
	float m_HPPercent = 100;
	uint16_t m_MaxHP = 0;

	const SDL_Rect& GetRect() const { return destRect; }

	void Draw() const;
	void TakeDamage(uint16_t dmg);
	void AdjustToView();
};