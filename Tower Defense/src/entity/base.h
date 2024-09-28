#pragma once
#include "label.h"
#include "tile.h"
#include "../Vector2D.h"

#include "SDL.h"

class Base
{
public:
	bool m_IsActive = true;
	static constexpr SDL_Rect srcRect{ 0, 0, 24, 24 };
	SDL_Rect destRect{ 0, 0, 24, 24 };
	Vector2D m_Pos{ 0, 0 };
	SDL_Texture* m_Texture = nullptr;
	uint16_t m_HP = 0;
	Label* m_AttachedLabel = nullptr;
	Tile* m_Tile = nullptr;

	void Draw() const;
	void TakeDamage(uint16_t dmg);
	void AdjustToView();
};