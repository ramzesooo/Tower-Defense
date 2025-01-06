#include "classicAttacker.h"
#include "../../app.h"

extern uint32_t g_PausedTicks;

ClassicAttacker::ClassicAttacker(Tower *occupiedTower, AttackerType type, SDL_Texture *texture, uint32_t shotCooldown, uint16_t scale)
	: Attacker(occupiedTower, type, texture, shotCooldown * 4u, scale)
{
	if (m_Type == AttackerType::hunter)
	{
		m_Pos.x -= static_cast<float>(App::s_CurrentLevel->m_ScaledTileSize) / 4.0f;
	}
	else
	{
		m_Pos.x -= static_cast<float>(App::s_CurrentLevel->m_ScaledTileSize) / 3.0f;
	}

	destRect.x = static_cast<int32_t>(m_Pos.x - App::s_Camera.x) + destRect.w / 2;
	destRect.y = static_cast<int32_t>(m_Pos.y - App::s_Camera.y);

	m_ProjectileType = ProjectileType::arrow;
	animations.emplace("Idle", Animation("Idle", 0, 2, 600));
	animations.emplace("Shoot", Animation("Shoot", 1, 4, shotCooldown));

	/*switch (m_Type)
	{
	case AttackerType::archer:
	case AttackerType::musketeer:
		m_Pos.x -= static_cast<float>(App::s_CurrentLevel->m_ScaledTileSize) / 3.0f;
		m_ProjectileType = projectileType;
		animations.emplace("Idle", Animation("Idle", 0, 2, 600));
		animations.emplace("Shoot", Animation("Shoot", 1, 4, shotCooldown));
		break;
	case AttackerType::hunter:
		m_Pos.x -= static_cast<float>(App::s_CurrentLevel->m_ScaledTileSize) / 4.0f;
		m_ProjectileType = projectileType;
		animations.emplace("Idle", Animation("Idle", 0, 2, 600));
		animations.emplace("Shoot", Animation("Shoot", 1, 4, shotCooldown));
		break;
	}*/

	PlayAnim("Idle");
}

void ClassicAttacker::Update()
{
	uint32_t ticks = SDL_GetTicks() - g_PausedTicks;

	// Create new projectile if got target and passed the cooldown of attack
	if (IsAttacking() && ticks >= m_NextShot)
	{
		m_NextShot = ticks + m_ShotCooldown;
		App::s_CurrentLevel->AddProjectile(m_ProjectileType, this, m_Target);
	}

	srcRect.x = srcRect.w * ((ticks / m_CurrentAnim.speed) % m_CurrentAnim.frames);
	srcRect.y = m_CurrentAnim.index * Attacker::s_AttackerHeight;
}