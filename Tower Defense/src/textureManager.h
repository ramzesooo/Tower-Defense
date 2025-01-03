#pragma once
#include "common.h"

#include "SDL_rect.h"
#include "SDL_render.h"
#include "SDL_ttf.h"

#include <unordered_map>
#include <string>

class TextureManager
{
public:
	TextureManager() = default;
	~TextureManager();

	void AddTexture(const std::string& textureID, const char* path);

	// DrawTexture is responsible for drawing rectangles with integer values
	// angle is NULL and flip is SDL_FLIP_NONE by default
	static void DrawTexture(SDL_Texture *texture, const SDL_Rect &src, const SDL_Rect &dest, double angle = NULL, SDL_RendererFlip flip = SDL_FLIP_NONE);

	// DrawTextureF is responsible for drawing rectangles with float values
	// angle is NULL and flip is SDL_FLIP_NONE by default
	static void DrawTextureF(SDL_Texture* texture, const SDL_Rect& src, const SDL_FRect& dest, double angle = NULL, SDL_RendererFlip flip = SDL_FLIP_NONE);

	SDL_Texture* GetTexture(std::string_view textureID) const;

	void AddFont(std::string_view fontID, const char* path, uint16_t fontSize);
	TTF_Font *GetFont(std::string_view fontID) const;
private:
	std::unordered_map<std::string, SDL_Texture*, proxy_hash, std::equal_to<void>> textures;
	std::unordered_map<std::string, TTF_Font*, proxy_hash, std::equal_to<void>> fonts;
};