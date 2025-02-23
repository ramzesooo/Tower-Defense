#include "arrowProjectile.h"

#include "../../app.h"
#include "../../Vector2D.h"

#include "../../textureManager.h"

ArrowProjectile::ArrowProjectile(Attacker* owner, Enemy* target) : Projectile(ProjectileType::arrow, owner, target)
{
	m_Pos.x += App::s_CurrentLevel->m_ScaledTileSize / 2.0f;
	destRect.x = m_Pos.x + App::s_CurrentLevel->m_ScaledTileSize / 2.0f - App::s_Camera.x;
	destRect.y = m_Pos.y + destRect.h / 2.0f - App::s_Camera.y;
}

void ArrowProjectile::Update()
{
	if (!m_Owner || !m_Target)
	{
		Destroy();
		return;
	}

	static Vector2D realVelocity{ 0.0f, 0.0f };

	realVelocity = m_Velocity * App::s_ElapsedTime;

	if (m_Pos.x + static_cast<float>(destRect.w) >= m_Destination.x - realVelocity.x
		&& m_Pos.x - static_cast<float>(destRect.w) <= m_Destination.x + realVelocity.x
		&& m_Pos.y + static_cast<float>(destRect.h) >= m_Destination.y - realVelocity.y
		&& m_Pos.y - static_cast<float>(destRect.h) <= m_Destination.y + realVelocity.y)
	{
		m_Target->OnHit(App::GetDamageOf(m_Type));
		Destroy();
		return;
	}

	m_Destination = m_Target->GetScaledPos();
	Vector2D truncatedPos(m_Pos);
	truncatedPos.Truncf();

	if (m_Destination.x == truncatedPos.x)
	{
		m_Velocity.x = 0.0f;
	}
	else
	{
		m_Velocity.x = m_Destination.x > truncatedPos.x ? m_BaseVelocity : -m_BaseVelocity;
	}

	if (m_Destination.y == truncatedPos.y)
	{
		m_Velocity.y = 0.0f;
	}
	else
	{
		m_Velocity.y = m_Destination.y > truncatedPos.y ? m_BaseVelocity : -m_BaseVelocity;
	}

	if (m_Velocity.IsEqualZero())
	{
		m_Target->OnHit(App::GetDamageOf(m_Type));
		Destroy();
		return;
	}

	float magnitude = std::sqrtf(m_Velocity.x * m_Velocity.x + m_Velocity.y * m_Velocity.y);
	Vector2D fixedVelocity(m_Velocity / magnitude);

	m_Pos += (m_Velocity + fixedVelocity) * App::s_ElapsedTime;

	double dx = m_Destination.x - m_Pos.x;
	double dy = m_Destination.y - m_Pos.y;

	// If angle is equal to zero, then it is supposed to be directed into right
	// std::atan2(dy, dx) returns radian
	// To convert it to degrees it has to be multiplied by 180 and divided by PI
	m_Angle = std::atan2(dy, dx) * 180.0 / M_PI;

	destRect.x = m_Pos.x + App::s_CurrentLevel->m_ScaledTileSize / 2.0f - App::s_Camera.x;
	destRect.y = m_Pos.y + destRect.h / 2.0f - App::s_Camera.y;
}

void ArrowProjectile::Draw()
{
	TextureManager::DrawFullTextureF(m_Texture, destRect, m_Angle, SDL_FLIP_NONE);
}