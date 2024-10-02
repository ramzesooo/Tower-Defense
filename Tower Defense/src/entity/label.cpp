#include "label.h"
#include "enemy.h"
#include "../app.h"

Label::Label(int32_t posX, int32_t posY, std::string_view text, TTF_Font *font, SDL_Color color, Entity *attachedTo)
	: m_Text(text), m_Font(font), m_Color(color), m_AttachedTo(attachedTo)
{
	destRect.x = posX;
	destRect.y = posY;

	SDL_Surface* surface = TTF_RenderText_Blended(m_Font, std::string(m_Text).c_str(), m_Color);
	if (!surface)
	{
		App::s_Logger.AddLog("Failed to create a surface in Label::UpdateText");
		App::s_Logger.AddLog("Last SDL Error: " + std::string(SDL_GetError()));
		return;
	}

	m_Texture = SDL_CreateTextureFromSurface(App::s_Renderer, surface);
	if (!m_Texture)
	{
		App::s_Logger.AddLog("Failed to create texture from surface in Label::UpdateText");
		App::s_Logger.AddLog("Last SDL Error: " + std::string(SDL_GetError()));
		SDL_FreeSurface(surface);
		return;
	}
	SDL_FreeSurface(surface);

	SDL_QueryTexture(m_Texture, nullptr, nullptr, &destRect.w, &destRect.h);
}

void Label::Destroy()
{
	m_IsActive = false;

	if (m_AttachedTo && m_AttachedTo->HasGroup(EntityGroup::enemy))
	{
		static_cast<Enemy*>(m_AttachedTo)->SetAttachedLabel(nullptr);
	}

	// Don't need to use if (m_Texture), because it'll just throw SDL error about invalid texture if it's nullptr
	SDL_DestroyTexture(m_Texture);
}

void Label::Draw()
{
	/*if (!m_Drawable)
		return;*/

	SDL_RenderCopy(App::s_Renderer, m_Texture, nullptr, &destRect);
}

void Label::UpdateText(std::string_view text)
{
	SDL_Surface *surface = TTF_RenderText_Blended(m_Font, std::string(text).c_str(), m_Color);
	if (!surface)
	{
		App::s_Logger.AddLog("Failed to create a surface in Label::UpdateText");
		App::s_Logger.AddLog("Last SDL Error: " + std::string(SDL_GetError()));
		return;
	}

	SDL_Texture *newTexture = SDL_CreateTextureFromSurface(App::s_Renderer, surface);
	if (!newTexture)
	{
		App::s_Logger.AddLog("Failed to create texture from surface in Label::UpdateText");
		App::s_Logger.AddLog("Last SDL Error: " + std::string(SDL_GetError()));
		SDL_FreeSurface(surface);
		return;
	}

	m_Text = text;
	SDL_FreeSurface(surface);
	SDL_DestroyTexture(m_Texture);
	m_Texture = newTexture;

	SDL_QueryTexture(m_Texture, nullptr, nullptr, &destRect.w, &destRect.h);
}

void Label::UpdateColor(SDL_Color newColor)
{
	if (newColor.a == m_Color.a && newColor.r == m_Color.r && newColor.g == m_Color.g && newColor.b == m_Color.b)
		return;

	SDL_Surface *surface = TTF_RenderText_Blended(m_Font, std::string(m_Text).c_str(), m_Color);
	if (!surface)
	{
		App::s_Logger.AddLog("Failed to create a surface in Label::UpdateText");
		App::s_Logger.AddLog("Last SDL Error: " + std::string(SDL_GetError()));
		return;
	}

	SDL_Texture *newTexture = SDL_CreateTextureFromSurface(App::s_Renderer, surface);
	if (!newTexture)
	{
		App::s_Logger.AddLog("Failed to create texture from surface in Label::UpdateText");
		App::s_Logger.AddLog("Last SDL Error: " + std::string(SDL_GetError()));
		SDL_FreeSurface(surface);
		return;
	}

	m_Color = newColor;
	SDL_FreeSurface(surface);
	SDL_DestroyTexture(m_Texture);
	m_Texture = newTexture;

	SDL_QueryTexture(m_Texture, nullptr, nullptr, &destRect.w, &destRect.h);
}