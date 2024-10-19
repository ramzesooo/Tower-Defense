#pragma once
#include "entity.h"
#include "label.h"

#include "SDL.h"

struct RectHP
{
	static constexpr SDL_Rect srcRect{ 0, 0, 32, 32 };
	SDL_FRect squareRect{ 0.0f, 0.0f, 32.0f, 32.0f };
	SDL_FRect barRect{ 0.0f, 0.0f, 32.0f, 32.0f };
	// Label responsible for displaying enemy's hp
	Label *labelHP = nullptr;
};