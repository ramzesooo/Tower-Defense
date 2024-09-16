#pragma once
#include "entity.h"
#include "../Vector2D.h"

#include "SDL.h"
#include "SDL_ttf.h"

#include <string_view>

class Label : public Entity
{
public:
	Label(int32_t posX, int32_t posY, std::string_view text, TTF_Font* font, SDL_Color color = { 255, 255, 255, 255 }, Entity* attachedTo = nullptr);

	inline void Destroy() override
	{
		m_IsActive = false;

		if (m_AttachedTo)
		{
			m_AttachedTo->m_AttachedLabel = nullptr;
		}
	}

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

	Vector2D GetPos() { return { (float)destRect.x, (float)destRect.y }; }

	Entity* m_AttachedTo = nullptr;
private:
	std::string_view m_Text = "";
	TTF_Font* m_Font = nullptr;
	SDL_Texture* m_Texture = nullptr;
	SDL_Rect destRect { 0, 0, 0, 0 };
	SDL_Color m_Color { 255, 255, 255, 255 };
};