#pragma once
#include "typesEnums.h"

#include "entity.h"
#include "../Vector2D.h"

#include "SDL_rect.h"
#include "SDL_render.h"

class Attacker;
class Enemy;

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

	void Update() override;
	void Draw() override;
	void AdjustToView() override;
	void Destroy() override;

	void SetTarget(Enemy *target) { m_Target = target; }
	void SetOwner(Attacker *owner) { m_Owner = owner; }
	Enemy* GetTarget() const { return m_Target; }
	Attacker* GetOwner() const { return m_Owner; }

	ProjectileType GetType() const { return m_Type; }
private:
	static constexpr float baseVelocity = 200.0f;
	static constexpr SDL_Rect srcRect{ 0, 0, 16, 16 };
	SDL_Texture* m_Texture = nullptr;
	SDL_Rect destRect{ 0, 0, 18, 18 };
	double m_Angle = 360;
	Vector2D m_Pos{ 0.0f, 0.0f };
	Vector2D m_Velocity{ 0.0f, 0.0f };
	Vector2D m_Destination{ 0.0f, 0.0f };
	ProjectileType m_Type = ProjectileType::arrow;
	Attacker *m_Owner = nullptr;
	Enemy *m_Target = nullptr;
};