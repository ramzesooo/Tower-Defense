#pragma once

struct Animation
{
	Animation() = default;
	Animation(const Animation& r) : index(r.index), frames(r.frames), speed(r.speed){}

	Animation(int32_t i, int32_t f, int32_t s) : index(i), frames(f), speed(s){}

	inline Animation& operator=(const Animation& r)
	{
		if (this == &r)
		{
			return *this;
		}

		index = r.index;
		frames = r.frames;
		speed = r.speed;

		return *this;
	}

	int32_t index = 0;
	int32_t frames = 0;
	int32_t speed = 100;
};