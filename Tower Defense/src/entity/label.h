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
	bool m_IsDrawable = true;
public:
	Label() : /*m_OnStack(true),*/ m_VanishDelay(NULL), m_Ticks(SDL_GetTicks()), m_DelayPerAlphaUnit(NULL) {}
	Label(uint32_t vanishable) : /*m_OnStack(true),*/ m_VanishDelay(vanishable), m_Ticks(SDL_GetTicks()), m_DelayPerAlphaUnit(vanishable / 255.0) {}
	Label(int32_t posX, int32_t posY, const std::string &text, TTF_Font *font, SDL_Color color = { 255, 255, 255, 255 }, Entity *attachedTo = nullptr, bool toCopy = false);
	Label(const Label& other);
	~Label();

	Label &operator=(const Label &other);

	//void Destroy();
private:
	[[nodiscard]] bool MakeTextureFromText();
public:
	void Draw();

	void UpdateText(const std::string &text);
	void UpdateColor(const SDL_Color &newColor);

	inline void UpdatePos(const Vector2D &pos)
	{
		destRect.x = static_cast<int32_t>(pos.x);
		destRect.y = static_cast<int32_t>(pos.y);
	}

	inline void UpdatePos(float x, float y)
	{
		destRect.x = static_cast<int32_t>(x);
		destRect.y = static_cast<int32_t>(y);
	}

	inline void UpdatePos(int32_t posX, int32_t posY)
	{
		destRect.x = posX;
		destRect.y = posY;
	}

	inline void MoveY(int32_t y)
	{
		destRect.y += y;
	}

	inline void ResetAlpha()
	{
		m_Alpha = 255;
		m_Ticks = SDL_GetTicks();
	}

	void SetAlpha(uint8_t alpha) { m_Alpha = alpha; }

	const SDL_Rect &GetRect() const { return destRect; }
	const std::string& GetText() const { return m_Text; }
	const SDL_Color &GetColor() const { return m_Color; }
	[[nodiscard]] constexpr bool IsVanishable() const { return m_VanishDelay > 0; }
private:
	const uint32_t m_VanishDelay = NULL;
	const double m_DelayPerAlphaUnit = NULL;
	//const bool m_OnStack = false; // false if it's unique pointer in Manager's vector

	uint32_t m_Ticks = NULL;
	uint8_t m_Alpha = 255;

	std::string m_Text;

	TTF_Font *m_Font = nullptr;
	SDL_Texture *m_Texture = nullptr;
	SDL_Rect destRect{ 0, 0, 0, 0 };
	SDL_Color m_Color{ 255, 255, 255, 255 };
};