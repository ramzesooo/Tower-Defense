#include "textureManager.h"
#include "app.h"
#include "logger.h"

#include "SDL_image.h"

#include <format>

TextureManager::~TextureManager()
{
	for (const auto &font : fonts)
	{
		TTF_CloseFont(font.second);
	}

	for (const auto &texture : textures)
	{
		SDL_DestroyTexture(texture.second);
	}

	TTF_Quit();
}

void TextureManager::AddTexture(const std::string &textureID, const char* path)
{
	SDL_Surface* tempSurface = IMG_Load(path);
	if (!tempSurface)
	{
		App::s_Logger.AddLog(std::string_view(SDL_GetError()));
		return;
	}

	SDL_Texture* texture = SDL_CreateTextureFromSurface(App::s_Renderer, tempSurface);
	if (!texture)
	{
		App::s_Logger.AddLog(std::string_view(SDL_GetError()));
		SDL_FreeSurface(tempSurface);
		return;
	}
	SDL_FreeSurface(tempSurface);

	IF_DEBUG(App::s_Logger.AddLog(std::format("Loaded texture \"{}\" from \"{}\"", textureID, path)););

	textures.emplace(textureID, texture);
}

void TextureManager::DrawTexture(SDL_Texture* texture, const SDL_Rect& src, const SDL_Rect& dest, double angle, SDL_RendererFlip flip)
{
	SDL_RenderCopyEx(App::s_Renderer, texture, &src, &dest, angle, NULL, flip);
}

void TextureManager::DrawTextureF(SDL_Texture* texture, const SDL_Rect& src, const SDL_FRect& dest, double angle, SDL_RendererFlip flip)
{
	SDL_RenderCopyExF(App::s_Renderer, texture, &src, &dest, angle, NULL, flip);
	// Basically could be if (SDL_RenderCopyExF(...) != 0) to print errors as well as TextureManager::DrawTexture
	// But there is a high chance that's unnecessary
}

SDL_Texture* TextureManager::GetTexture(std::string_view textureID) const
{
	auto it = textures.find(textureID);
	if (it == textures.end())
	{
		App::s_Logger.AddLog(std::string_view("TextureManager::GetTexture: Missing texture "), false);
		App::s_Logger.AddLog(textureID);
		return nullptr;
	}

	return it->second;
}

void TextureManager::AddFont(std::string_view fontID, const char* path, uint16_t fontSize)
{
	TTF_Font *font = TTF_OpenFont(path, fontSize);

	if (!font)
	{
		App::s_Logger.AddLog(std::format("TextureManager::GetFont: Couldn't open font {}", path));
		return;
	}

	fonts.emplace(fontID, font);
}

TTF_Font* TextureManager::GetFont(std::string_view fontID) const
{
	auto it = fonts.find(fontID);
	if (it == fonts.end())
	{
		App::s_Logger.AddLog(std::string_view("TextureManager::GetFont: Missing font "), false);
		App::s_Logger.AddLog(fontID);
		return nullptr;
	}

	return it->second;
}