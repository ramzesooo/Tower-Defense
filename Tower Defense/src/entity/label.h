#pragma once
#include "../Vector2D.h"

#include "SDL_timer.h"
#include "SDL_rect.h"
#include "SDL_render.h"
#include "SDL_ttf.h"

class Entity;

class Label
{
public:
	Entity* m_AttachedTo = nullptr;
	bool m_Drawable = true;

	Label() : m_OnStack(true), m_VanishDelay(NULL), m_Ticks(SDL_GetTicks()), m_DelayPerAlphaUnit(NULL) {}
	Label(uint32_t vanishable) : m_OnStack(true), m_VanishDelay(vanishable), m_Ticks(SDL_GetTicks()), m_DelayPerAlphaUnit(vanishable / 255.0) {}
	Label(int32_t posX, int32_t posY, const std::string &text, TTF_Font *font, SDL_Color color = { 255, 255, 255, 255 }, Entity *attachedTo = nullptr);
	Label(const Label &r) : m_AttachedTo(r.m_AttachedTo), m_Text(r.m_Text), m_Font(r.m_Font), m_Texture(r.m_Texture), destRect(r.destRect),
		m_Color(r.m_Color) {}
	~Label() = default;

	inline Label &operator=(const Label &r)
	{
		if (this == &r)
			return *this;

		m_AttachedTo = r.m_AttachedTo;

		m_Text = r.m_Text;
		m_Font = r.m_Font;
		m_Texture = r.m_Texture;
		destRect = r.destRect;
		m_Color = r.m_Color;

		return *this;
	}

	void Destroy();

	void Draw();

	void UpdateText(const std::string &text);
	void UpdateColor(SDL_Color newColor);

	inline void UpdatePos(const Vector2D &pos)
	{
		destRect.x = static_cast<int32_t>(pos.x);
		destRect.y = static_cast<int32_t>(pos.y);
	}

	inline void UpdatePos(int32_t posX, int32_t posY)
	{
		destRect.x = posX;
		destRect.y = posY;
	}

	inline void ResetAlpha()
	{
		m_Alpha = 255;
		m_Ticks = SDL_GetTicks();
	}

	void SetAlpha(uint8_t alpha) { m_Alpha = alpha; }

	Vector2D GetPos() const { return { static_cast<float>(destRect.x), static_cast<float>(destRect.y) }; }
	const SDL_Rect &GetRect() const { return destRect; }
	const std::string &GetText() const { return m_Text; }
private:
	const uint32_t m_VanishDelay = NULL;
	const double m_DelayPerAlphaUnit = NULL;
	bool m_OnStack = false; // false if it's unique pointer in Manager's vector
	uint32_t m_Ticks = NULL;
	uint8_t m_Alpha = 255;
	std::string m_Text;
	TTF_Font *m_Font = nullptr;
	SDL_Texture *m_Texture = nullptr;
	SDL_Rect destRect{ 0, 0, 0, 0 };
	SDL_Color m_Color{ 255, 255, 255, 255 };
};