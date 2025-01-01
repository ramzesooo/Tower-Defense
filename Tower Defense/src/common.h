#pragma once
#include <type_traits>

#ifdef DEBUG
#define IF_DEBUG(...) __VA_ARGS__
#define IF_NDEBUG(...)
#else
#define IF_DEBUG(...)
#define IF_NDEBUG(...) __VA_ARGS__
#endif

//#define ASYNC_TILES

IF_DEBUG(
enum class EnemyDebugSpeed
{
	none = 0,
	faster,
	stay
};
);

struct proxy_hash {
	using is_transparent = void;
	// abbreviated function template (C++20)
	std::size_t operator()(const auto &key) const
	{
		return std::hash<std::remove_cvref_t<decltype(key)>>{}(key);
	}
};