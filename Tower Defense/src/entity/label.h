#pragma once
#include "entity.h"
#include "../Vector2D.h"

#include "SDL.h"
#include "SDL_ttf.h"

#include <string_view>

class Label
{
public:
	Entity* m_AttachedTo = nullptr;
	bool m_Drawable = true;

	Label() : m_OnStack(true) {}
	Label(int32_t posX, int32_t posY, const std::string &text, TTF_Font *font, SDL_Color color = { 255, 255, 255, 255 }, Entity *attachedTo = nullptr);
	Label(const Label &r) : m_AttachedTo(r.m_AttachedTo), m_Text(r.m_Text), m_Font(r.m_Font), m_Texture(r.m_Texture), destRect(r.destRect),
		m_Color(r.m_Color) {}
	~Label() = default;

	inline Label& operator=(const Label &r)
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

	void Destroy();

	void Draw();

	void UpdateText(const std::string& text);
	void UpdateColor(SDL_Color newColor);

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

	Vector2D GetPos() const { return { (float)destRect.x, (float)destRect.y }; }
	SDL_Rect GetRect() const { return destRect; }
	const std::string &GetText() const { return m_Text; }
private:
	bool m_OnStack = false; // false if it's unique pointer in Manager's vector
	std::string m_Text = "";
	TTF_Font *m_Font = nullptr;
	SDL_Texture *m_Texture = nullptr;
	SDL_Rect destRect { 0, 0, 0, 0 };
	SDL_Color m_Color { 255, 255, 255, 255 };
};