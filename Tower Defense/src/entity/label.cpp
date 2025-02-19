#include "label.h"
#include "entity.h"
#include "../app.h"
#include "../logger.h"

#include <format>

Label::Label(int32_t posX, int32_t posY, const std::string &text, TTF_Font *font, SDL_Color color, Entity *attachedTo)
	: m_AttachedTo(attachedTo), m_Text(text), m_Font(font), m_Color(color), destRect{ posX, posY, 0, 0 }
{
	SDL_Surface* surface = TTF_RenderText_Blended(m_Font, m_Text.c_str(), m_Color);
	if (!surface)
	{
		App::s_Logger.AddLog(std::format("Failed to create a surface in Label::Label\nLast SDL Error: {}", SDL_GetError()));
		return;
	}

	m_Texture = SDL_CreateTextureFromSurface(App::s_Renderer, surface);
	if (!m_Texture)
	{
		App::s_Logger.AddLog(std::format("Failed to create a texture from surface in Label::Label\nLast SDL Error: {}", SDL_GetError()));
		SDL_FreeSurface(surface);
		return;
	}
	SDL_FreeSurface(surface);

	SDL_QueryTexture(m_Texture, nullptr, nullptr, &destRect.w, &destRect.h);
}

Label &Label::operator=(const Label &other)
{
	if (this == &other)
		return *this;

	m_AttachedTo = other.m_AttachedTo;

	m_Text = other.m_Text;
	m_Font = other.m_Font;
	destRect = other.destRect;
	m_Color = other.m_Color;

	SDL_Surface* surface = TTF_RenderText_Blended(m_Font, m_Text.c_str(), m_Color);
	if (!surface)
	{
		App::s_Logger.AddLog(std::format("Failed to create a surface in Label::operator=\nLast SDL Error: {}", SDL_GetError()));
		return *this;
	}

	m_Texture = SDL_CreateTextureFromSurface(App::s_Renderer, surface);
	if (!m_Texture)
	{
		App::s_Logger.AddLog(std::format("Failed to create a texture from surface in Label::operator=\nLast SDL Error: {}", SDL_GetError()));
		SDL_FreeSurface(surface);
		return *this;
	}
	SDL_FreeSurface(surface);

	SDL_QueryTexture(m_Texture, nullptr, nullptr, &destRect.w, &destRect.h);

	return *this;
}

Label::~Label()
{
	if (m_Texture)
	{
		SDL_DestroyTexture(m_Texture);
		m_Texture = nullptr;
	}

	Destroy();
}

void Label::Destroy()
{
	if (!m_OnStack)
		App::s_Manager.DestroyLabel(this);
}

void Label::Draw()
{
	if (!m_IsDrawable || !m_Texture || m_Alpha == 0)
		return;

	if (m_VanishDelay > 0 && static_cast<double>(SDL_GetTicks()) >= static_cast<double>(m_Ticks) + m_DelayPerAlphaUnit)
	{
		m_Ticks = SDL_GetTicks();
		SDL_SetTextureAlphaMod(m_Texture, m_Alpha--);
	}

	SDL_RenderCopy(App::s_Renderer, m_Texture, nullptr, &destRect);
}

void Label::UpdateText(const std::string &text)
{
	SDL_Surface *surface = TTF_RenderText_Blended(m_Font, text.c_str(), m_Color);
	if (!surface)
	{
		App::s_Logger.AddLog(std::format("Failed to create a surface in Label::UpdateText\nLast SDL Error: {}", SDL_GetError()));
		return;
	}

	SDL_Texture *newTexture = SDL_CreateTextureFromSurface(App::s_Renderer, surface);
	if (!newTexture)
	{
		App::s_Logger.AddLog(std::format("Failed to create a texture from surface in Label::UpdateText\nLast SDL Error: {}", SDL_GetError()));
		SDL_FreeSurface(surface);
		return;
	}

	m_Text = text;
	SDL_FreeSurface(surface);
	SDL_DestroyTexture(m_Texture);
	m_Texture = newTexture;

	SDL_QueryTexture(m_Texture, nullptr, nullptr, &destRect.w, &destRect.h);
}

void Label::UpdateColor(const SDL_Color &newColor)
{
	if (newColor.a == m_Color.a && newColor.r == m_Color.r && newColor.g == m_Color.g && newColor.b == m_Color.b)
		return;

	SDL_Surface *surface = TTF_RenderText_Blended(m_Font, m_Text.c_str(), m_Color);
	if (!surface)
	{
		App::s_Logger.AddLog(std::format("Failed to create a surface in Label::UpdateColor\nLast SDL Error: {}", SDL_GetError()));
		return;
	}

	SDL_Texture *newTexture = SDL_CreateTextureFromSurface(App::s_Renderer, surface);
	if (!newTexture)
	{
		App::s_Logger.AddLog(std::format("Failed to create texture from surface in Label::UpdateColor\nLast SDL Error: {}", SDL_GetError()));
		SDL_FreeSurface(surface);
		return;
	}

	m_Color = newColor;
	SDL_FreeSurface(surface);
	SDL_DestroyTexture(m_Texture);
	m_Texture = newTexture;

	SDL_QueryTexture(m_Texture, nullptr, nullptr, &destRect.w, &destRect.h);
}