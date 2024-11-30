#pragma once

#ifdef DEBUG
#define IF_DEBUG(...) __VA_ARGS__
#else
#define IF_DEBUG(...)
#endif