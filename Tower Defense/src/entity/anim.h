#pragma once

struct Animation
{
	Animation()
	{}

	Animation(int32_t i, int32_t f, int32_t s) : index(i), frames(f), speed(s)
	{}

	~Animation()
	{}

	int32_t index = 0;
	int32_t frames = 0;
	int32_t speed = 100;
};