#pragma once
#include "../typesEnums.h"
#include "../anim.h"

#include "../entity.h"
#include "../../Vector2D.h"

#include "SDL_rect.h"
#include "SDL_render.h"

class Attacker;
class Enemy;

struct ProjectileLifetime
{
	bool isRestricted = false; // specifies whether the projectile's lifetime depends on time
	uint32_t lifetime = 0;
	uint32_t timePerFrame = 0;
	uint32_t nextFrame = 0;
	uint32_t currentFrame = 0;
};

class Projectile : public Entity
{
public:
	Projectile() = delete;
	Projectile(ProjectileType type, Attacker* owner, Enemy* target);
	Projectile(const Projectile &) = delete;
	~Projectile() = default;

	Projectile &operator=(const Projectile &) = delete;

	void Destroy() override;

	virtual void Update() override = 0;
	void Draw() override;

	virtual void AdjustToView() override = 0;

	void SetTarget(Enemy *target) { m_Target = target; }
	void SetOwner(Attacker *owner) { m_Owner = owner; }
	Enemy* GetTarget() const { return m_Target; }
	Attacker* GetOwner() const { return m_Owner; }

	ProjectileType GetType() const { return m_Type; }
protected:
	//static constexpr float s_BaseVelocity = 210.0f;
	//static constexpr SDL_Rect srcRect{ 0, 0, 16, 16 };
	SDL_Rect srcRect{ 0, 0, 16, 16 };
	SDL_FRect destRect{ 0.0f, 0.0f, 18.0f, 18.0f };
	SDL_Texture *m_Texture = nullptr;

	double m_Angle = 360;

	Vector2D m_Pos{ 0.0f, 0.0f };
	Vector2D m_Velocity{ 0.0f, 0.0f };
	Vector2D m_Destination{ 0.0f, 0.0f };
	float m_BaseVelocity = 330.0f;

	ProjectileType m_Type = ProjectileType::arrow;

	Attacker *m_Owner = nullptr;
	Enemy *m_Target = nullptr;

	bool m_IsAnimated = false;
	Animation m_Animation;

	ProjectileLifetime m_Lifetime;
};