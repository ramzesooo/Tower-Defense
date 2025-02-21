#include "label.h"
#include "entity.h"
#include "../app.h"
#include "../logger.h"

#include <format>

Label::Label(int32_t posX, int32_t posY, const std::string &text, TTF_Font *font,
	SDL_Color color /* = { 255, 255, 255, 255 } */, Entity* attachedTo /* = nullptr */, bool toCopy /* = false */)
	: m_AttachedTo(attachedTo), m_Text(text), m_Font(font), m_Color(color), destRect{ posX, posY, 0, 0 }
{
	if (!toCopy)
	{
		if (!MakeTextureFromText())
		{
			App::s_Logger.AddLog(std::string_view("Label::Label: Failed creating texture from text"));
			m_IsDrawable = false;
		}
	}
}

Label::Label(const Label& other)
	: m_AttachedTo(other.m_AttachedTo), m_Text(other.m_Text), m_Font(other.m_Font), destRect(other.destRect),
	m_Color(other.m_Color)
{
	if (!MakeTextureFromText())
	{
		App::s_Logger.AddLog(std::string_view("Label::Label(reference): Failed creating texture from text"));
		m_IsDrawable = false;
	}
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

	if (!MakeTextureFromText())
	{
		App::s_Logger.AddLog(std::string_view("Label::operator=: Failed creating texture from text"));
		m_IsDrawable = false;
		return *this;
	}

	return *this;
}

Label::~Label()
{
	if (m_Texture)
	{
		SDL_DestroyTexture(m_Texture);
		m_Texture = nullptr;
	}
}

void Label::Destroy()
{
	if (!m_OnStack)
		App::s_Manager.DestroyLabel(this);
}

bool Label::MakeTextureFromText()
{
	SDL_Surface* surface = TTF_RenderText_Blended(m_Font, m_Text.c_str(), m_Color);
	if (!surface)
	{
		App::s_Logger.AddLog(std::format("Failed to create a surface in Label::MakeTextureFromText\nLast SDL Error: {}", SDL_GetError()));
		return false;
	}

	SDL_Texture *texture = SDL_CreateTextureFromSurface(App::s_Renderer, surface);
	if (!texture)
	{
		App::s_Logger.AddLog(std::format("Failed to create a texture from surface in Label::MakeTextureFromText\nLast SDL Error: {}", SDL_GetError()));
		SDL_FreeSurface(surface);
		return false;
	}

	if (m_Texture)
	{
		SDL_DestroyTexture(m_Texture);
	}

	m_Texture = texture;
	SDL_FreeSurface(surface);

	SDL_QueryTexture(m_Texture, nullptr, nullptr, &destRect.w, &destRect.h);
	return true;
}

void Label::Draw()
{
	if (!m_IsDrawable || !m_Texture || m_Alpha == 0)
		return;

	if (IsVanishable() && static_cast<double>(SDL_GetTicks()) >= static_cast<double>(m_Ticks) + m_DelayPerAlphaUnit)
	{
		m_Ticks = SDL_GetTicks();
		SDL_SetTextureAlphaMod(m_Texture, m_Alpha--);
	}

	SDL_RenderCopy(App::s_Renderer, m_Texture, nullptr, &destRect);
}

void Label::UpdateText(const std::string &text)
{
	std::string_view oldText = m_Text;
	m_Text = text;

	if (!MakeTextureFromText())
	{
		App::s_Logger.AddLog(std::format("Label::UpdateText: Failed to create new texture \"{}\"\nLast SDL Error: {}", m_Text, SDL_GetError()));
		m_Text = oldText;
		return;
	}
}

void Label::UpdateColor(const SDL_Color &newColor)
{
	if (newColor.a == m_Color.a && newColor.r == m_Color.r && newColor.g == m_Color.g && newColor.b == m_Color.b)
		return;

	if (!MakeTextureFromText())
	{
		App::s_Logger.AddLog(std::format("Label::UpdateColor: Failed to create new texture \"{}\"\nLast SDL Error: {}", m_Text, SDL_GetError()));
		return;
	}

	m_Color = newColor;
}