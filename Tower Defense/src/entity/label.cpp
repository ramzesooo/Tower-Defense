#include "label.h"
#include "enemy.h"
#include "../app.h"

Label::Label(int32_t posX, int32_t posY, std::string_view text, TTF_Font* font, SDL_Color color, Entity* attachedTo)
	: m_Text(text), m_Font(font), m_Color(color), m_AttachedTo(attachedTo)
{
	destRect.x = posX;
	destRect.y = posY;

	SDL_Surface* surface = TTF_RenderText_Blended(m_Font, std::string(m_Text).c_str(), m_Color);
	if (!surface)
	{
		App::s_Logger.AddLog("Failed to create a surface in Label::UpdateText");
		App::s_Logger.AddLog("Last SDL Error: " + std::string(SDL_GetError()));
		App::s_Logger.AddLog("Last TTF Error: " + std::string(TTF_GetError()));
		return;
	}
	m_Texture = SDL_CreateTextureFromSurface(App::s_Renderer, surface);
	if (!m_Texture)
	{
		App::s_Logger.AddLog("Failed to create texture from surface in Label::UpdateText");
		App::s_Logger.AddLog("Last SDL Error: " + std::string(SDL_GetError()));
		App::s_Logger.AddLog("Last TTF Error: " + std::string(TTF_GetError()));
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
	SDL_RenderCopy(App::s_Renderer, m_Texture, nullptr, &destRect);
}

void Label::UpdateText(std::string_view text)
{
	m_Text = text;

	SDL_Surface* surface = TTF_RenderText_Blended(m_Font, std::string(m_Text).c_str(), m_Color);
	if (!surface)
	{
		App::s_Logger.AddLog("Failed to create a surface in Label::UpdateText");
		App::s_Logger.AddLog("Last SDL Error: " + std::string(SDL_GetError()));
		App::s_Logger.AddLog("Last TTF Error: " + std::string(TTF_GetError()));
		return;
	}
	SDL_DestroyTexture(m_Texture);
	m_Texture = SDL_CreateTextureFromSurface(App::s_Renderer, surface);
	if (!m_Texture)
	{
		App::s_Logger.AddLog("Failed to create texture from surface in Label::UpdateText");
		App::s_Logger.AddLog("Last SDL Error: " + std::string(SDL_GetError()));
		App::s_Logger.AddLog("Last TTF Error: " + std::string(TTF_GetError()));
		SDL_FreeSurface(surface);
		return;
	}
	SDL_FreeSurface(surface);

	SDL_QueryTexture(m_Texture, nullptr, nullptr, &destRect.w, &destRect.h);
}