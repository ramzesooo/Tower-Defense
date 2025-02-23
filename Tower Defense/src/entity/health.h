#pragma once
#include "label.h"

#include "SDL_rect.h"

struct RectHP
{
	//static constexpr SDL_Rect srcRect{ 0, 0, 32, 32 };
	SDL_FRect squareRect{ 0.0f, 0.0f, 32.0f, 32.0f };
	SDL_FRect barRect{ 0.0f, 0.0f, 32.0f, 32.0f };

	Label labelHP; // Label responsible for displaying enemy's hp
	float onePercent; // references to width of 1% hp
};