#pragma once
#include "entity.h"
#include "attacker.h"
#include "tile.h"
#include "projectile.h"
#include "label.h"
#include "anim.h"
#include "../Vector2D.h"
#include "../textureManager.h"

#include "SDL.h"

struct RectHP
{
	static constexpr SDL_Rect srcRect{ 0, 0, 32, 32 };
	SDL_FRect squareRect{ 0.0f, 0.0f, 32.0f, 32.0f };
	SDL_FRect barRect{ 0.0f, 0.0f, 32.0f, 32.0f };
};

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
	Enemy(const Enemy& r) : Entity(r), towers(r.towers), destRect(r.destRect), m_RectHP(r.m_RectHP), m_OccupiedTile(r.m_OccupiedTile), animations(r.animations),
		m_Type(r.m_Type), m_HP(r.m_HP), m_MaxHP(r.m_MaxHP), m_HPPercent(r.m_HPPercent) {}
	~Enemy() = default;

	inline Enemy& operator=(const Enemy& r)
	{
		if (this == &r)
		{
			return *this;
		}

		Entity::operator=(r);
		m_Texture = r.m_Texture;
		m_Type = r.m_Type;
		m_Pos = r.m_Pos;
		m_ScaledPos = r.m_ScaledPos;

		m_OccupiedTile = r.m_OccupiedTile;

		m_Velocity = r.m_Velocity;
		m_Destination = r.m_Destination;
		srcRect = r.srcRect;
		destRect = r.destRect;
		m_Scale = r.m_Scale;
		m_RectHP = r.m_RectHP;
		m_HP = r.m_HP;
		m_MaxHP = r.m_MaxHP;
		m_HPPercent = r.m_HPPercent;
		m_AnimFrames = r.m_AnimFrames;
		m_AnimID = r.m_AnimID;
		m_AnimIndex = r.m_AnimIndex;
		m_AnimSpeed = r.m_AnimSpeed;

		return *this;
	}

	void Destroy() override;

	void Update() override;
	void Draw() override;

	void PlayAnim(std::string_view animID);

	Vector2D GetPos() const override { return m_Pos; }

	// Vector2D destination is a difference between current position
	// For example it should be x: 1.0f, y: -1.0f to move one tile left and 1 tile up
	// Enemy's position is calculated appropriate to tiles count
	// And rendering is properly calculated to tiles' size
	// So for example its position will be like Vector2D(1.0f, 1.0f)
	// And it will be rendered on x * tile size, y * tile size
	void Move(Vector2D destination);
	void Move(float destinationX, float destinationY);
	bool IsMoving() const { return m_Velocity.x != 0.0f || m_Velocity.y != 0.0f; }
	void UpdateMovement();
	
	void UpdateHealthBar();
	void AdjustToView() override;

	// Returns true if specific tower has been found in forwarded range
	// Range works in loop for every tower's tile
	// And searches towers from x: -range, y: -range to: x: range, y: range
	bool IsTowerInRange(Tower* tower, uint16_t range = 2) const;

	void AddProjectile(ProjectileType type, Attacker* projectileOwner);

	// DelProjectile should be always triggered to destroy it
	// Only one exception is in case when Enemy object has been destroyed before
	// Or projectile somehow has no assigned Enemy object
	// IsHit is responsible for executing appropriate code for hitting target
	void DelProjectile(Projectile* projectile, bool IsHit = false);

	void SetOccupiedTile(Tile* newOccupiedTile) { m_OccupiedTile = newOccupiedTile; }
	Tile* GetOccupiedTile() const { return m_OccupiedTile; }
private:
	static constexpr int32_t s_EnemyWidth = 32;
	static constexpr int32_t s_EnemyHeight = 32;
	static constexpr float s_MovementSpeed = 3.0f;
	static SDL_Texture* s_Square;
	static SDL_Texture* s_GreenTex;
	SDL_Texture* m_Texture = nullptr;
	EnemyType m_Type;

	// m_Pos for Enemy is based on tiles' count and it's scaled with tiles' size only while rendering in destRect.x and destRect.y
	Vector2D m_Pos;
	Vector2D m_ScaledPos;
	Vector2D m_Velocity{ 0.0f, 0.0f };
	Vector2D m_Destination{ 0.0f, 0.0f };
	SDL_Rect srcRect{ 0, 0, 32, 32 }, destRect{ 0, 0, 32, 32 };
	uint16_t m_Scale = 1;

	Tile* m_OccupiedTile = nullptr;
	std::vector<Entity*>& towers;

	RectHP m_RectHP;
	uint16_t m_HP = 0;
	float m_HPPercent = 100;
	uint16_t m_MaxHP = 0;

	std::string_view m_AnimID;
	int32_t m_AnimIndex = 0;
	int32_t m_AnimFrames = 1;
	int32_t m_AnimSpeed = 100;
	std::unordered_map<std::string, Animation, proxy_hash, std::equal_to<void>> animations;
};