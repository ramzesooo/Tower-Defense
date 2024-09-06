#pragma once
#include "entity.h"
#include "../Vector2D.h"
#include "anim.h"
#include "../textureManager.h"

#include "SDL.h"

#include <unordered_map>

class Tower;

enum class AttackerType
{
	archer = 0,
	size
};

class Attacker : public Entity
{
public:
	Attacker(Tower& occupiedTower, AttackerType type, SDL_Texture* texture, uint16_t scale = 1);

	void Update() override;
	void Draw() override;

	void PlayAnim(std::string_view animID);
private:
	static constexpr int32_t s_AttackerWidth = 32;
	static constexpr int32_t s_AttackerHeight = 32;
	Tower& m_OccupiedTower;
	AttackerType m_Type;
	SDL_Texture* m_Texture = nullptr;
	uint16_t m_Scale = 1;
	Vector2D m_Pos;
	SDL_Rect srcRect{ 0, 0, 32, 32 }, destRect{ 0, 0, 32, 32 };

	int32_t m_AnimIndex = 0;
	int32_t m_AnimFrames = 1;
	int32_t m_AnimSpeed = 100;
	std::unordered_map<std::string, Animation, proxy_hash, std::equal_to<void>> animations;
};