#include "label.h"
#include "../app.h"

Label::Label(int32_t posX, int32_t posY, std::string_view text, TTF_Font* font, SDL_Color color)
	: m_Text(text), m_Font(font), m_Color(color)
{
	destRect.x = posX;
	destRect.y = posY;

	SDL_Surface* surface = TTF_RenderText_Blended(m_Font, std::string(m_Text).c_str(), m_Color);
	m_Texture = SDL_CreateTextureFromSurface(App::s_Renderer, surface);
	SDL_FreeSurface(surface);

	SDL_QueryTexture(m_Texture, nullptr, nullptr, &destRect.w, &destRect.h);
}

void Label::Draw()
{
	SDL_RenderCopy(App::s_Renderer, m_Texture, nullptr, &destRect);
}

void Label::UpdateText(std::string_view text)
{
	m_Text = text;

	SDL_Surface* surface = TTF_RenderText_Blended(m_Font, std::string(m_Text).c_str(), m_Color);
	m_Texture = SDL_CreateTextureFromSurface(App::s_Renderer, surface);
	SDL_FreeSurface(surface);

	SDL_QueryTexture(m_Texture, nullptr, nullptr, &destRect.w, &destRect.h);
}