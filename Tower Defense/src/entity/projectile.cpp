#include "projectile.h"
#include "attacker.h"
#include "enemy.h"
#include "../textureManager.h"
#include "../level.h"
#include "../app.h"

#include <cmath>

Projectile::Projectile(ProjectileType type, Attacker* owner, Enemy* target)
	: m_Type(type), m_Owner(owner), m_Target(target),
	m_Destination(target->GetPos().x * App::s_CurrentLevel->m_ScaledTileSize, target->GetPos().y * App::s_CurrentLevel->m_ScaledTileSize), 
	m_Texture(App::s_Textures.GetTexture(App::TextureOf(type))), m_Pos(owner->GetPos())
{
	destRect.x = (int32_t)m_Pos.x + App::s_CurrentLevel->m_ScaledTileSize / 2;
	destRect.y = (int32_t)m_Pos.y;

	switch (type)
	{
	case ProjectileType::arrow:
		m_Damage = 15;
		break;
	default:
		m_Damage = 0;
		break;
	}
}

void Projectile::Update()
{
	if (!m_Owner || !m_Owner->IsActive())
	{
		m_Target->DelProjectile(this);
		return;
	}

	if (m_ToDestroy)
	{
		Destroy();
		return;
	}

	if (m_Pos.x + (float)destRect.w >= m_Destination.x - (m_Velocity.x * App::s_ElapsedTime) && m_Pos.x - (float)destRect.w <= m_Destination.x + (m_Velocity.x * App::s_ElapsedTime)
		&& m_Pos.y + (float)destRect.h >= m_Destination.y - (m_Velocity.y * App::s_ElapsedTime) && m_Pos.y - (float)destRect.h <= m_Destination.y + (m_Velocity.y * App::s_ElapsedTime))
	{
		m_Target->DelProjectile(this, true);
		return;
	}

	uint16_t scaledTileSize = App::s_CurrentLevel->m_ScaledTileSize;

	m_Destination = (m_Target->GetPos() * scaledTileSize);

	if (m_Destination.x == trunc(m_Pos.x))
	{
		m_Velocity.x = 0.0f;
	}
	else
	{
		m_Velocity.x = m_Destination.x > trunc(m_Pos.x) ? baseVelocity : -baseVelocity;
	}

	if (m_Destination.y == trunc(m_Pos.y))
	{
		m_Velocity.y = 0.0f;
	}
	else
	{
		m_Velocity.y = m_Destination.y > trunc(m_Pos.y) ? baseVelocity : -baseVelocity;
	}
	
	if (m_Velocity.y == 0.0f && m_Velocity.x == 0.0f)
	{
		m_Target->DelProjectile(this, true);
		return;
	}

	Vector2D fixedVelocity(m_Velocity);

	float magnitude = std::sqrtf(m_Velocity.x * m_Velocity.x + m_Velocity.y * m_Velocity.y);
	if (magnitude > 0)
	{
		fixedVelocity.x /= magnitude;
		fixedVelocity.y /= magnitude;
	}

	m_Pos += (m_Velocity + fixedVelocity) * App::s_ElapsedTime;
	//m_Pos.x += (m_Velocity.x + fixedVelocity.x) * App::s_ElapsedTime;
	//m_Pos.y += (m_Velocity.y + fixedVelocity.y) * App::s_ElapsedTime;

	double dx = m_Destination.x - m_Pos.x;
	double dy = m_Destination.y - m_Pos.y;

	// If angle is equal to zero, then it is supposed to be directed into right
	// std::atan2(dy, dx) returns radian
	// To convert it to degrees it has to be multiplied by 180 and divided by PI
	m_Angle = std::atan2(dy, dx) * 180.0f / M_PI;

	destRect.x = (int32_t)m_Pos.x + scaledTileSize / 2 - (int32_t)App::s_Camera.x;
	destRect.y = (int32_t)m_Pos.y + destRect.h / 2 - (int32_t)App::s_Camera.y;
}

void Projectile::AdjustToView()
{
	destRect.x = (int32_t)m_Pos.x + App::s_CurrentLevel->m_ScaledTileSize / 2 - (int32_t)App::s_Camera.x;
	destRect.y = (int32_t)m_Pos.y + destRect.h / 2 - (int32_t)App::s_Camera.y;
}

void Projectile::Draw()
{
	TextureManager::DrawTexture(m_Texture, srcRect, destRect, m_Angle, SDL_FLIP_NONE);
}