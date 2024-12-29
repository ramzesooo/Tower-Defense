#include "projectile.h"
#include "attacker.h"
#include "enemy.h"
#include "../textureManager.h"
#include "../level.h"
#include "../app.h"

#include <cmath>

extern uint32_t g_PausedTicks;

Projectile::Projectile(ProjectileType type, Attacker *owner, Enemy *target)
	: m_Type(type), m_Owner(owner), m_Target(target),
	m_Destination(target->GetPos().x * App::s_CurrentLevel->m_ScaledTileSize, target->GetPos().y * App::s_CurrentLevel->m_ScaledTileSize), 
	m_Texture(App::s_Textures.GetTexture(App::TextureOf(type))), m_Pos(owner->GetPos())
{
	switch (type)
	{
	case ProjectileType::arrow:
		srcRect.w = srcRect.h = 16;
		destRect.x = static_cast<int32_t>(m_Pos.x) + App::s_CurrentLevel->m_ScaledTileSize / 2;
		destRect.y = static_cast<int32_t>(m_Pos.y);
		break;
	case ProjectileType::dark:
		destRect.x = static_cast<int32_t>(m_Pos.x);
		destRect.y = static_cast<int32_t>(m_Pos.y);
		srcRect.w = srcRect.h = 24;
		destRect.w = destRect.h = 48;
		animated = true;
		anim = Animation("Attack", 0, 4, 100);
		break;
	}
}

void Projectile::Destroy()
{
	m_IsActive = false;

	if (!m_Owner)
		return;

	std::erase(m_Owner->m_OwnedProjectiles, this);
	/*for (auto it = m_Owner->m_OwnedProjectiles.begin(); it != m_Owner->m_OwnedProjectiles.end(); it++)
	{
		if ((*it) == this)
		{
			m_Owner->m_OwnedProjectiles.erase(it);
			return;
		}
	}*/

	App::s_Manager.m_EntitiesToDestroy = true;
}

void Projectile::Update()
{
	if (!m_Owner || !m_Target)
	{
		Destroy();
		return;
	}

	if (m_Pos.x + static_cast<float>(destRect.w) >= m_Destination.x - (m_Velocity.x * App::s_ElapsedTime) 
		&& m_Pos.x - static_cast<float>(destRect.w) <= m_Destination.x + (m_Velocity.x * App::s_ElapsedTime)
		&& m_Pos.y + static_cast<float>(destRect.h) >= m_Destination.y - (m_Velocity.y * App::s_ElapsedTime) 
		&& m_Pos.y - static_cast<float>(destRect.h) <= m_Destination.y + (m_Velocity.y * App::s_ElapsedTime))
	{
		m_Target->OnHit(this, App::GetDamageOf(m_Type));
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
		m_Target->OnHit(this, App::GetDamageOf(m_Type));
		return;
	}

	switch (m_Type)
	{
	case ProjectileType::arrow:
		UpdateArrow();
		return;
	case ProjectileType::dark:
		UpdateDark();
		return;
	}
}

void Projectile::Draw()
{
	TextureManager::DrawTexture(m_Texture, srcRect, destRect, m_Angle, SDL_FLIP_NONE);
}

void Projectile::AdjustToView()
{
	destRect.x = static_cast<int32_t>(m_Pos.x) + App::s_CurrentLevel->m_ScaledTileSize / 2 - static_cast<int32_t>(App::s_Camera.x);
	destRect.y = static_cast<int32_t>(m_Pos.y) + destRect.h / 2 - static_cast<int32_t>(App::s_Camera.y);
}

void Projectile::UpdateArrow()
{
	Vector2D fixedVelocity(m_Velocity);

	float magnitude = std::sqrtf(m_Velocity.x * m_Velocity.x + m_Velocity.y * m_Velocity.y);
	if (magnitude > 0.0f)
	{
		/*fixedVelocity.x /= magnitude;
		fixedVelocity.y /= magnitude;*/
		fixedVelocity /= magnitude;
	}

	m_Pos += (m_Velocity + fixedVelocity) * App::s_ElapsedTime;

	double dx = m_Destination.x - m_Pos.x;
	double dy = m_Destination.y - m_Pos.y;

	// If angle is equal to zero, then it is supposed to be directed into right
	// std::atan2(dy, dx) returns radian
	// To convert it to degrees it has to be multiplied by 180 and divided by PI
	m_Angle = std::atan2(dy, dx) * 180.0f / M_PI;

	destRect.x = static_cast<int32_t>(m_Pos.x) + App::s_CurrentLevel->m_ScaledTileSize / 2 - static_cast<int32_t>(App::s_Camera.x);
	destRect.y = static_cast<int32_t>(m_Pos.y) + destRect.h / 2 - static_cast<int32_t>(App::s_Camera.y);
}

void Projectile::UpdateDark()
{
	Vector2D fixedVelocity(m_Velocity);

	float magnitude = std::sqrtf(m_Velocity.x * m_Velocity.x + m_Velocity.y * m_Velocity.y);
	if (magnitude > 0.0f)
		fixedVelocity /= magnitude;

	m_Pos += (m_Velocity + fixedVelocity) * App::s_ElapsedTime;

	destRect.x = static_cast<int32_t>(m_Pos.x) + App::s_CurrentLevel->m_ScaledTileSize / 2 - static_cast<int32_t>(App::s_Camera.x);
	destRect.y = static_cast<int32_t>(m_Pos.y) + destRect.h / 2 - static_cast<int32_t>(App::s_Camera.y);

	srcRect.x = srcRect.w * ((SDL_GetTicks() / anim.speed) % anim.frames);
	//srcRect.y = anim.index * srcRect.h;
}