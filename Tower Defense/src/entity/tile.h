#pragma once
#include "entity.h"
#include "../Vector2D.h"

#include "SDL.h"

class Tile : public Entity
{
public:
	Tile(int srcX, int srcY, int posX, int posY, int tileSize, int tileScale, std::string_view textureID);

	void Update() override;
	void Draw() override;

	Vector2D GetPos() const { return m_Pos; }
	void SetPos(Vector2D newPos) { m_Pos = newPos; }
	void SetPos(float x, float y) { m_Pos = { x, y }; }
	void SetPosX(float x) { m_Pos.x = x; }
	void SetPosY(float y) { m_Pos.y = y; }

	inline void Move(float x, float y)
	{ 
		m_Pos.x += x * destRect.w;
		m_Pos.y += y * destRect.h;
	}
	inline void Move(Vector2D newPos)
	{
		Move(newPos.x, newPos.y);
	}

	int32_t GetWidth() const { return destRect.w; }
	int32_t GetHeight() const { return destRect.h; }
private:
	SDL_Rect srcRect{ 0, 0, 24, 24 }, destRect{ 0, 0, 24, 24 };
	Vector2D m_Pos;
	std::string_view m_TextureID;
	SDL_Texture* m_Texture = nullptr;
};