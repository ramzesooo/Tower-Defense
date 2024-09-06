#pragma once
#include "SDL.h"

#include <unordered_map>
#include <string>

struct proxy_hash {
	using is_transparent = void;
	// abbreviated function template (C++20)
	std::size_t operator()(const auto& key) const {
		return std::hash<std::remove_cvref_t<decltype(key)>>{}(key);
	}
};

class TextureManager
{
public:
	void AddTexture(std::string_view textureID, const char* path);
	void DrawTexture(std::string_view textureID);
	static void DrawTexture(SDL_Texture* texture, const SDL_Rect& src, const SDL_Rect& dest);

	SDL_Texture* GetTexture(std::string_view textureID) const;
private:
	std::unordered_map<std::string, SDL_Texture*, proxy_hash, std::equal_to<void>> textures;
};