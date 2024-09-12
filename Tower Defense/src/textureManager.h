#pragma once
#include "SDL.h"
#include "SDL_ttf.h"

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
	~TextureManager();

	void AddTexture(std::string_view textureID, const char* path);

	// Don't need this, since there are static methods for drawing
	void DrawTexture(std::string_view textureID);

	// DrawTexture is responsible for drawing rectangles with integer values
	// angle is NULL and flip is SDL_FLIP_NONE by default
	static void DrawTexture(SDL_Texture* texture, const SDL_Rect& src, const SDL_Rect& dest, double angle = NULL, SDL_RendererFlip flip = SDL_FLIP_NONE);

	// DrawTextureF is responsible for drawing rectangles with float values
	// angle is NULL and flip is SDL_FLIP_NONE by default
	static void DrawTextureF(SDL_Texture* texture, const SDL_Rect& src, const SDL_FRect& dest, double angle = NULL, SDL_RendererFlip flip = SDL_FLIP_NONE);

	SDL_Texture* GetTexture(std::string_view textureID) const;

	void AddFont(std::string_view fontID, const char* path, uint16_t fontSize);
	TTF_Font* GetFont(std::string_view fontID) const;
private:
	std::unordered_map<std::string, SDL_Texture*, proxy_hash, std::equal_to<void>> textures;
	std::unordered_map<std::string, TTF_Font*, proxy_hash, std::equal_to<void>> fonts;
};