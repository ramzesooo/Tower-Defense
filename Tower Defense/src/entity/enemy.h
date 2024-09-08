#pragma once
#include "entity.h"
#include "tile.h"
#include "anim.h"
#include "../Vector2D.h"
#include "../textureManager.h"

#include "SDL.h"

enum class EnemyType
{
	elf = 0,
	size
};

class Tower;

class Enemy : public Entity
{
public:
	Enemy(float posX, float posY, EnemyType type, SDL_Texture* texture, uint16_t scale = 1);

	void Update() override;
	void Draw() override;

	void PlayAnim(std::string_view animID);

	Vector2D GetPos() const override { return m_Pos; }

	void Move(Vector2D destination);
	void Move(float destinationX, float destinationY);
	bool IsMoving() const { return m_Velocity.x != 0.0f || m_Velocity.y != 0.0f; }
	void UpdateMovement();

	// Returns true if specific tower has been found in forwarded range
	// range works from x: -range, y: -range to: x: range, y: range
	bool IsTowerInRange(Tower* tower, int32_t range = 2) const;
private:
	static constexpr int32_t s_EnemyWidth = 32;
	static constexpr int32_t s_EnemyHeight = 32;
	static constexpr float s_MovementSpeed = 0.01f;
	SDL_Texture* m_Texture = nullptr;
	EnemyType m_Type;
	Vector2D m_Pos;
	Vector2D m_Velocity{ 0.0f, 0.0f };
	Vector2D m_Destination{ 0.0f, 0.0f };
	SDL_Rect srcRect{ 0, 0, 32, 32 }, destRect{ 0, 0, 32, 32 };
	uint16_t m_Scale = 1;

	Tile* m_OccupiedTile = nullptr;
	std::vector<Entity*>& towers;

	uint16_t m_HP = 100;

	int32_t m_AnimIndex = 0;
	int32_t m_AnimFrames = 1;
	int32_t m_AnimSpeed = 100;
	std::unordered_map<std::string, Animation, proxy_hash, std::equal_to<void>> animations;
};