#pragma once
#include "typesEnums.h"
#include "anim.h"

#include "entity.h"
#include "../Vector2D.h"

#include "SDL_rect.h"
#include "SDL_render.h"

class Attacker;
class Enemy;

struct ProjectileLifetime
{
	static constexpr uint32_t darkProjectileLifetime = 3000;

	bool isRestricted = false; // specifies whether the projectile's lifetime depends on time
	uint32_t lifetime = 0;
	uint32_t timePerFrame = 0;
	uint32_t nextFrame = 0;
	uint32_t currentFrame = 0;
};

class Projectile : public Entity
{
public:
	Projectile(ProjectileType type, Attacker* owner, Enemy* enemy);
	Projectile(const Projectile &r) : m_Texture(r.m_Texture), destRect(r.destRect), m_Angle(r.m_Angle), m_Pos(r.m_Pos),
		m_Velocity(r.m_Velocity), m_Destination(r.m_Destination), m_Type(r.m_Type), m_Owner(r.m_Owner) {}
	~Projectile() = default;

	inline Projectile& operator=(const Projectile &r)
	{
		if (this == &r)
		{
			return *this;
		}

		m_Texture = r.m_Texture;
		destRect = r.destRect;
		m_Angle = r.m_Angle;
		m_Pos = r.m_Pos;
		m_Velocity = r.m_Velocity;
		m_Destination = r.m_Destination;
		m_Type = r.m_Type;
		m_Owner = r.m_Owner;
		m_Target = r.m_Target;

		return *this;
	}

	void Destroy() override;

	void Update() override;
	void Draw() override;

	void AdjustToView() override;

	void UpdateArrow();
	void UpdateDark();

	void SetTarget(Enemy *target) { m_Target = target; }
	void SetOwner(Attacker *owner) { m_Owner = owner; }
	Enemy* GetTarget() const { return m_Target; }
	Attacker* GetOwner() const { return m_Owner; }

	ProjectileType GetType() const { return m_Type; }
private:
	//static constexpr float s_BaseVelocity = 210.0f;
	//static constexpr SDL_Rect srcRect{ 0, 0, 16, 16 };
	SDL_Rect srcRect{ 0, 0, 16, 16 };
	SDL_Rect destRect{ 0, 0, 18, 18 };
	SDL_Texture* m_Texture = nullptr;

	double m_Angle = 360;

	Vector2D m_Pos{ 0.0f, 0.0f };
	Vector2D m_Velocity{ 0.0f, 0.0f };
	Vector2D m_Destination{ 0.0f, 0.0f };
	float m_BaseVelocity = 210.0f;

	ProjectileType m_Type = ProjectileType::arrow;

	Attacker *m_Owner = nullptr;
	Enemy *m_Target = nullptr;

	bool animated = false;
	Animation anim;

	ProjectileLifetime m_Lifetime;
};