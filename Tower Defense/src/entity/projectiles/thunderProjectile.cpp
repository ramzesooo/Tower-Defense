#include "thunderProjectile.h"

#include "../../app.h"
#include "../../Vector2D.h"

extern uint32_t g_PausedTicks;

ThunderProjectile::ThunderProjectile(Attacker* owner, Enemy* target) : Projectile(ProjectileType::thunder, owner, target)
{
	m_Pos = target->GetScaledPos();
	srcRect = { 0, 0, 256, 512 };

	//srcRect.w = 256;
	//srcRect.h = 512;

	destRect = { 
		m_Pos.x - App::s_Camera.x,
		m_Pos.y - target->GetRect().h - App::s_Camera.y,
		static_cast<float>(App::s_CurrentLevel->m_ScaledTileSize),
		App::s_CurrentLevel->m_ScaledTileSize * 2.0f
	};

	//destRect.x = m_Pos.x - App::s_Camera.x;
	//destRect.y = m_Pos.y - target->GetRect().h - App::s_Camera.y;
	//destRect.w = App::s_CurrentLevel->m_ScaledTileSize;
	//destRect.h = destRect.w * 2.0f;

	m_Lifetime.timePerFrame = 50;
	m_Lifetime.nextFrame = SDL_GetTicks() - g_PausedTicks + m_Lifetime.timePerFrame;
	m_Lifetime.isRestricted = true;
	m_IsAnimated = true;
	m_Animation = Animation("Attack", 0, 15, m_Lifetime.timePerFrame);
	m_Lifetime.lifetime = SDL_GetTicks() + m_Lifetime.timePerFrame * m_Animation.frames;

	target->OnHit(App::GetDamageOf(ProjectileType::thunder));
}

void ThunderProjectile::Update()
{
	if (!m_Owner || !m_Target)
	{
		Destroy();
		return;
	}

	auto currentTicks = SDL_GetTicks() - g_PausedTicks;

	if (SDL_TICKS_PASSED(currentTicks, m_Lifetime.lifetime))
	{
		Destroy();
		return;
	}

	if (currentTicks >= m_Lifetime.nextFrame)
	{
		// Unnecessary at the moment, because the projectile should disappear on the last frame
		/*if (m_Lifetime.currentFrame++ >= anim.frames)
		{
			m_Lifetime.currentFrame = 0u;
		}*/

		m_Lifetime.currentFrame++;

		if (m_Lifetime.currentFrame >= static_cast<uint32_t>(m_Animation.frames - 1))
		{
			m_Lifetime.lifetime = currentTicks + m_Lifetime.timePerFrame;
			m_Lifetime.nextFrame = UINT32_MAX;
			srcRect.x = srcRect.w * (m_Animation.frames - 1);
			return;
		}

		m_Lifetime.nextFrame = currentTicks + m_Lifetime.timePerFrame;

		srcRect.x = srcRect.w * m_Lifetime.currentFrame;
	}
}

void ThunderProjectile::Draw()
{
	TextureManager::DrawTextureF(m_Texture, srcRect, destRect);
}

void ThunderProjectile::AdjustToView()
{
	destRect.x -= CameraMovement::realVelocity.x;
	destRect.y -= CameraMovement::realVelocity.y;
}