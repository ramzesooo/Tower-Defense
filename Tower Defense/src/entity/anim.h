#pragma once
#include <string_view>

struct Animation
{
	Animation() = default;
	Animation(const Animation& r) : id(r.id), index(r.index), frames(r.frames), speed(r.speed){}
	Animation(std::string_view animID, int32_t i, int32_t f, int32_t s) : id(animID), index(i), frames(f), speed(s){}

	~Animation() = default;

	inline Animation& operator=(const Animation& r)
	{
		if (this == &r)
		{
			return *this;
		}

		id = r.id;
		index = r.index;
		frames = r.frames;
		speed = r.speed;

		return *this;
	}

	std::string_view id = "";
	int32_t index = 0;
	int32_t frames = 0;
	int32_t speed = 100;
};