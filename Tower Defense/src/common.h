#pragma once

#ifdef DEBUG
#define IF_DEBUG(...) __VA_ARGS__
#define IF_NDEBUG(...)
#else
#define IF_DEBUG(...)
#define IF_NDEBUG(...) __VA_ARGS__
#endif