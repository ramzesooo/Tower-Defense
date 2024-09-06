#pragma once

struct Animation
{
	Animation()
	{}

	Animation(int i, int f, int s) : index(i), frames(f), speed(s)
	{}

	~Animation()
	{}

	int index = 0;
	int frames = 0;
	int speed = 100;
};