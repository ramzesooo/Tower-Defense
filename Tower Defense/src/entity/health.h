#pragma once
#include "entity.h"
#include "label.h"

#include "SDL.h"

class Health
{
public:
	SDL_Rect m_DestRect{ 0, 0, 0, 0 };
	static SDL_Texture *s_EmptyBarTexture;
	static SDL_Texture *s_FullBarTexture;

	Health() = default;
	~Health() = default;
};

class EntityHealth : public Health
{
public:
	static constexpr SDL_Rect s_SrcRect{ 0, 0, 0, 0 };
	Label *m_LabelHP = nullptr;
};

class BaseHealth : public Health
{
public:
	static constexpr SDL_Rect s_EmptySrcRect{ 65, 39, 33, 30 };
	static constexpr SDL_Rect s_FullSrcRect{ 33, 39, 33, 30 };
	uint16_t m_HeartsAmount = 3;
	uint16_t m_LeftHearts = 3;
	// { 0, 40, 32, 29 }
	// { 34, 40, 32, 29 }
	// { 66, 40, 32, 29 }

	BaseHealth();
	~BaseHealth() = default;

	void Draw();
};

struct RectHP
{
	static constexpr SDL_Rect srcRect{ 0, 0, 32, 32 };
	SDL_FRect squareRect{ 0.0f, 0.0f, 32.0f, 32.0f };
	SDL_FRect barRect{ 0.0f, 0.0f, 32.0f, 32.0f };
	// Label responsible for displaying enemy's hp
	Label *labelHP = nullptr;
};