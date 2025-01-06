#include "darkAttacker.h"
#include "../../app.h"

extern uint32_t g_PausedTicks;

DarkAttacker::DarkAttacker(Tower *occupiedTower, AttackerType type, SDL_Texture *texture, uint32_t shotCooldown, uint16_t scale)
	: Attacker(occupiedTower, type, texture, shotCooldown, scale)
{
	m_Invisible = true;
	m_Pos.x -= static_cast<float>(App::s_CurrentLevel->m_ScaledTileSize) / 4.0f;
	m_ProjectileType = ProjectileType::thunder;

	destRect.x = static_cast<int32_t>(m_Pos.x - App::s_Camera.x) + destRect.w / 2;
	destRect.y = static_cast<int32_t>(m_Pos.y - App::s_Camera.y);
}

void DarkAttacker::Update()
{
	uint32_t ticks = SDL_GetTicks() - g_PausedTicks;

	// Create new projectile if got target and passed the cooldown of attack
	if (IsAttacking() && ticks >= m_NextShot)
	{
		m_NextShot = ticks + m_ShotCooldown;
		App::s_CurrentLevel->AddProjectile(m_ProjectileType, this, m_Target);
	}
}