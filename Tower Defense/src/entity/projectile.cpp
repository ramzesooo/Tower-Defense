#include "projectile.h"
#include "attacker.h"
#include "enemy.h"
#include "../textureManager.h"
#include "../level.h"
#include "../app.h"

#include <cmath>

extern uint32_t g_PausedTicks;

//static constexpr uint32_t projectileLifetime = 5000;

Projectile::Projectile(ProjectileType type, Attacker *owner, Enemy *target)
	: m_Type(type), m_Owner(owner), m_Target(target),
	m_Destination(target->GetScaledPos()), 
	m_Texture(App::s_Textures.GetTexture(App::TextureOf(type))), m_Pos(owner->GetPos())
{
	switch (type)
	{
	case ProjectileType::arrow:
		//srcRect.w = srcRect.h = 16;
		m_Pos.x += App::s_CurrentLevel->m_ScaledTileSize / 2.0f;
		destRect.x = m_Pos.x + App::s_CurrentLevel->m_ScaledTileSize / 2.0f - App::s_Camera.x;
		destRect.y = m_Pos.y + destRect.h / 2.0f - App::s_Camera.y;
		break;
	/*case ProjectileType::dark:
		srcRect.w = srcRect.h = 24;
		destRect.x = static_cast<int32_t>(m_Pos.x);
		destRect.y = static_cast<int32_t>(m_Pos.y);
		destRect.w = destRect.h = 48;
		m_Lifetime.timePerFrame = m_Lifetime.darkProjectileLifetime / 7;
		m_Lifetime.lifetime = SDL_GetTicks() + m_Lifetime.darkProjectileLifetime;
		m_Lifetime.nextFrame = SDL_GetTicks() + m_Lifetime.timePerFrame - g_PausedTicks;
		m_Lifetime.isRestricted = true;
		animated = true;
		anim = Animation("Attack", 0, 4, m_Lifetime.timePerFrame);
		m_BaseVelocity *= 1.10f;
		break;*/
	case ProjectileType::thunder:
	{
		const SDL_Rect &targetRect = target->GetRect();
		m_Pos = { target->GetScaledPos() };
		srcRect.w = 256;
		srcRect.h = 512;
		destRect.x = m_Pos.x - App::s_Camera.x;
		destRect.y = m_Pos.y - targetRect.h - App::s_Camera.y;
		destRect.w = App::s_CurrentLevel->m_ScaledTileSize;
		destRect.h = destRect.w * 2.0f;
		m_Lifetime.timePerFrame = 50;
		m_Lifetime.nextFrame = SDL_GetTicks() - g_PausedTicks + m_Lifetime.timePerFrame;
		m_Lifetime.isRestricted = true;
		m_IsAnimated = true;
		m_Animation = Animation("Attack", 0, 15, m_Lifetime.timePerFrame);
		m_Lifetime.lifetime = SDL_GetTicks() + m_Lifetime.timePerFrame * m_Animation.frames;

		target->OnHit(App::GetDamageOf(type));
	}
		break;
	}
}

void Projectile::Destroy()
{
	m_IsActive = false;

	if (m_Owner)
		std::erase(m_Owner->m_OwnedProjectiles, this);

	App::s_Manager.m_EntitiesToDestroy = true;
}

void Projectile::Update()
{
	if (!m_Owner || !m_Target)
	{
		Destroy();
		return;
	}

	if (m_Type == ProjectileType::thunder)
	{
		UpdateThunder();
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

	auto scaledTileSize = App::s_CurrentLevel->m_ScaledTileSize;

	m_Destination = m_Target->GetPos() * scaledTileSize;
	Vector2D truncatedPos = { m_Pos.Trunc() };

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

	switch (m_Type)
	{
	case ProjectileType::arrow:
		UpdateArrow();
		return;
	/*case ProjectileType::dark:
		UpdateDark();
		return;*/
	}
}

void Projectile::Draw()
{
	TextureManager::DrawTextureF(m_Texture, srcRect, destRect, m_Angle, SDL_FLIP_NONE);
}

void Projectile::AdjustToView()
{
	if (m_Type != ProjectileType::thunder)
	{
		return;
	}
	
	destRect.x -= CameraMovement::realVelocity.x;
	destRect.y -= CameraMovement::realVelocity.y;

	//switch (m_Type)
	//{
	//case ProjectileType::arrow:
	//	/*destRect.x = m_Pos.x + App::s_CurrentLevel->m_ScaledTileSize / 2.0f - App::s_Camera.x;
	//	destRect.y = m_Pos.y + destRect.h / 2.0f - App::s_Camera.y;*/
	//	destRect.x -= CameraMovement::realVelocity.x;
	//	destRect.y -= CameraMovement::realVelocity.y;
	//	return;
	//case ProjectileType::dark:
	//	destRect.x = static_cast<int32_t>(m_Pos.x) - static_cast<int32_t>(App::s_Camera.x);
	//	destRect.y = static_cast<int32_t>(m_Pos.y) - static_cast<int32_t>(App::s_Camera.y);
	//	return;
	//case ProjectileType::thunder:
	//		destRect.x -= CameraMovement::realVelocity.x;
	//		destRect.y -= CameraMovement::realVelocity.y;
	//	return;
	//}
}

void Projectile::UpdateArrow()
{
	Vector2D fixedVelocity(m_Velocity);

	float magnitude = std::sqrtf(m_Velocity.x * m_Velocity.x + m_Velocity.y * m_Velocity.y);
	if (magnitude > 0.0f)
		fixedVelocity /= magnitude;

	m_Pos += (m_Velocity + fixedVelocity) * App::s_ElapsedTime;

	double dx = m_Destination.x - m_Pos.x;
	double dy = m_Destination.y - m_Pos.y;

	// If angle is equal to zero, then it is supposed to be directed into right
	// std::atan2(dy, dx) returns radian
	// To convert it to degrees it has to be multiplied by 180 and divided by PI
	m_Angle = std::atan2(dy, dx) * 180.0f / M_PI;

	destRect.x = m_Pos.x + App::s_CurrentLevel->m_ScaledTileSize / 2.0f - App::s_Camera.x;
	destRect.y = m_Pos.y + destRect.h / 2.0f - App::s_Camera.y;
}

//void Projectile::UpdateDark()
//{
//	Vector2D fixedVelocity(m_Velocity);
//
//	float magnitude = std::sqrtf(m_Velocity.x * m_Velocity.x + m_Velocity.y * m_Velocity.y);
//	if (magnitude > 0.0f)
//		fixedVelocity /= magnitude;
//
//	m_Pos += (m_Velocity + fixedVelocity) * App::s_ElapsedTime;
//
//	destRect.x = static_cast<int32_t>(m_Pos.x) - static_cast<int32_t>(App::s_Camera.x);
//	destRect.y = static_cast<int32_t>(m_Pos.y) - static_cast<int32_t>(App::s_Camera.y);
//
//	/*m_Lifetime.ticks = SDL_GetTicks() - g_PausedTicks;*/
//	if (SDL_GetTicks() - g_PausedTicks >= m_Lifetime.nextFrame)
//	{
//		// Unnecessary at the moment, because the projectile should disappear on the last frame
//		/*if (m_Lifetime.currentFrame++ >= anim.frames)
//		{
//			m_Lifetime.currentFrame = 0u;
//		}*/
//
//		m_Lifetime.currentFrame++;
//
//		if (m_Lifetime.currentFrame >= static_cast<uint32_t>(anim.frames - 1))
//		{
//			m_Lifetime.lifetime = SDL_GetTicks() + 200;
//			m_Lifetime.nextFrame = UINT32_MAX;
//			m_BaseVelocity = 0.1f;
//			m_Velocity = Vector2D(0.1f, 0.1f);
//			srcRect.x = srcRect.w * (anim.frames - 1);
//			return;
//		}
//
//		m_Lifetime.nextFrame = SDL_GetTicks() + m_Lifetime.timePerFrame;
//
//		srcRect.x = srcRect.w * m_Lifetime.currentFrame;
//	}
//}

void Projectile::UpdateThunder()
{
	if (SDL_TICKS_PASSED(SDL_GetTicks() - g_PausedTicks, m_Lifetime.lifetime))
	{
		Destroy();
		return;
	}
	
	if (SDL_GetTicks() - g_PausedTicks >= m_Lifetime.nextFrame)
	{
		// Unnecessary at the moment, because the projectile should disappear on the last frame
		/*if (m_Lifetime.currentFrame++ >= anim.frames)
		{
			m_Lifetime.currentFrame = 0u;
		}*/

		m_Lifetime.currentFrame++;

		if (m_Lifetime.currentFrame >= static_cast<uint32_t>(m_Animation.frames - 1))
		{
			m_Lifetime.lifetime = SDL_GetTicks() - g_PausedTicks + m_Lifetime.timePerFrame;
			m_Lifetime.nextFrame = UINT32_MAX;
			srcRect.x = srcRect.w * (m_Animation.frames - 1);
			return;
		}

		m_Lifetime.nextFrame = SDL_GetTicks() - g_PausedTicks + m_Lifetime.timePerFrame;

		srcRect.x = srcRect.w * m_Lifetime.currentFrame;
	}
}