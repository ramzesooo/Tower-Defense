#pragma once
#include "../common.h"
#include "typesEnums.h"

#include "entity.h"
#include "attacker.h"
#include "projectile.h"
#include "health.h"
#include "tile.h"
#include "label.h"
#include "anim.h"
#include "../Vector2D.h"

#include "SDL_rect.h"
#include "SDL_render.h"

#include <vector>
#include <unordered_map>

class Tower;

class Enemy : public Entity
{
public:
	static constexpr int32_t s_EnemyWidth = 32;
	static constexpr int32_t s_EnemyHeight = 32;

	static SDL_Texture *s_ArrowTexture; // texture of arrow showing where are the enemies out of camera
	// vector of attackers targeting this specific enemy
	std::vector<Attacker*> m_Attackers;
public:
	Enemy(float posX, float posY, EnemyType type, SDL_Texture* texture, uint16_t scale = 1);
	Enemy(const Enemy &r) : destRect(r.destRect), m_RectHP(r.m_RectHP), m_OccupiedTile(r.m_OccupiedTile),
		m_MovementSpeed(r.m_MovementSpeed), m_Velocity(r.m_Velocity), m_Destination(r.m_Destination), animations(r.animations), m_Type(r.m_Type),
		m_HP(r.m_HP), m_MaxHP(r.m_MaxHP), m_HPPercent(r.m_HPPercent), m_CurrentAnim(r.m_CurrentAnim) {}
	~Enemy();

	inline Enemy& operator=(const Enemy &r)
	{
		if (this == &r)
		{
			return *this;
		}

		m_Texture = r.m_Texture;
		m_Type = r.m_Type;
		m_Pos = r.m_Pos;
		m_ScaledPos = r.m_ScaledPos;

		m_OccupiedTile = r.m_OccupiedTile;

		m_MovementSpeed = r.m_MovementSpeed;
		m_Velocity = r.m_Velocity;
		m_Destination = r.m_Destination;
		srcRect = r.srcRect;
		destRect = r.destRect;
		m_Scale = r.m_Scale;
		m_RectHP = r.m_RectHP;
		m_HP = r.m_HP;
		m_MaxHP = r.m_MaxHP;
		m_HPPercent = r.m_HPPercent;
		animations = r.animations;
		m_CurrentAnim = r.m_CurrentAnim;

		return *this;
	}

	void Destroy() override;

	void Update() override;
	void Draw() override;

	void PlayAnim(std::string_view animID);

	Vector2D GetPos() const override { return m_Pos; }
	Vector2D GetScaledPos() const { return m_ScaledPos; }
	const SDL_Rect &GetRect() const { return destRect; }

	// Vector2D destination is a difference between current position
	// For example it should be x: 1.0f, y: -1.0f to move one tile left and 1 tile up
	// Enemy's position is calculated appropriate to tiles count
	// And rendering is properly calculated to tiles' size
	// So for example its position will be like Vector2D(1.0f, 1.0f)
	// And it will be rendered on x * tile size, y * tile size
	/*void Move(Vector2D destination);
	void Move(float destinationX, float destinationY);*/
	bool IsMoving() const { return m_Velocity.x != 0.0f || m_Velocity.y != 0.0f; }
	void UpdateMovement();
	void Move();
	
	void UpdateHealthBar();
	void AdjustToView() override;

	void OnHit(uint16_t dmg);

	void ValidAttacker();

	// Returns true if specific tower has been found in forwarded range
	// Range works in loop for every tower's tile
	// And searches towers from x: -range, y: -range to: x: range, y: range
	bool IsTowerInRange(Tower *tower, uint16_t range = 2) const;

	void SetOccupiedTile(Tile *newOccupiedTile) { m_OccupiedTile = newOccupiedTile; }
	Tile* GetOccupiedTile() const { return m_OccupiedTile; }

	IF_DEBUG(void DebugSpeed(););

	void SetPath(const std::vector<Vector2D> &pathVector) { m_Path = pathVector; }
private:
	IF_DEBUG(EnemyDebugSpeed m_Speedy = EnemyDebugSpeed::none;);
	IF_DEBUG(float m_MovementDebugSpeed = 1.4f;);
	float m_MovementSpeed = 1.4f;
	SDL_Texture* m_Texture = nullptr;
	EnemyType m_Type;

	// m_Pos for Enemy is based on tiles' count and it's scaled with tiles' size only while rendering in destRect.x and destRect.y
	// To get already scaled enemy's position use m_ScaledPos
	Vector2D m_Pos;
	Vector2D m_ScaledPos;
	Vector2D m_Velocity{ 0.0f, 0.0f };
	Vector2D m_Destination{ 0.0f, 0.0f };

	std::vector<Vector2D> m_Path;
	std::size_t m_MoveCount = 0;

	SDL_Rect srcRect{ 0, 0, 32, 32 }, destRect{ 0, 0, 32, 32 };
	uint16_t m_Scale = 1;

	Tile *m_OccupiedTile = nullptr;

	RectHP m_RectHP;
	uint16_t m_HP = 0;
	uint16_t m_MaxHP = 0;
	float m_HPPercent = 100.0f;

	uint16_t m_Coins = 1; // coins granted for killing the enemy

	Animation m_CurrentAnim;
	std::unordered_map<std::string, Animation, proxy_hash, std::equal_to<void>> animations;

	SDL_Rect m_PointingArrowDest{ 0, 0, 48, 48 };
};