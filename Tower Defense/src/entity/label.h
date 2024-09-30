#pragma once
#include "entity.h"
#include "../Vector2D.h"

#include "SDL.h"
#include "SDL_ttf.h"

#include <string_view>

class Label : public Entity
{
public:
	Entity* m_AttachedTo = nullptr;
	bool m_Drawable = true;

	Label(int32_t posX, int32_t posY, std::string_view text, TTF_Font* font, SDL_Color color = { 255, 255, 255, 255 }, Entity* attachedTo = nullptr);
	Label(const Label& r) : m_AttachedTo(r.m_AttachedTo), m_Text(r.m_Text), m_Font(r.m_Font), m_Texture(r.m_Texture), destRect(r.destRect), m_Color(r.m_Color) {}
	~Label() = default;

	inline Label& operator=(const Label& r)
	{
		if (this == &r)
		{
			return *this;
		}

		m_AttachedTo = r.m_AttachedTo;

		m_Text = r.m_Text;
		m_Font = r.m_Font;
		m_Texture = r.m_Texture;
		destRect = r.destRect;
		m_Color = r.m_Color;

		return *this;
	}

	void Destroy() override;

	void Draw() override;

	void UpdateText(std::string_view text);
	void UpdateColor(SDL_Color& color) { m_Color = color; }

	inline void UpdatePos(Vector2D pos)
	{
		destRect.x = (int32_t)pos.x;
		destRect.y = (int32_t)pos.y;
	}

	inline void UpdatePos(int32_t posX, int32_t posY)
	{
		destRect.x = posX;
		destRect.y = posY;
	}

	Vector2D GetPos() const override { return { (float)destRect.x, (float)destRect.y }; }
	SDL_Rect GetRect() const { return destRect; }
private:
	std::string_view m_Text = "";
	TTF_Font* m_Font = nullptr;
	SDL_Texture* m_Texture = nullptr;
	SDL_Rect destRect { 0, 0, 0, 0 };
	SDL_Color m_Color { 255, 255, 255, 255 };
};