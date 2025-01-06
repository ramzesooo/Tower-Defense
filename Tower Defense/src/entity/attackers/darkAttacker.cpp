#include "darkAttacker.h"
#include "../towers/tower.h"
#include "../../app.h"


extern uint32_t g_PausedTicks;
extern std::vector<Entity*> &g_Enemies;

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
	if (!IsAttacking())
		return;

	ValidTarget();

	// Create new projectile if got target and passed the cooldown of attack
	if (ticks >= m_NextShot)
	{
		m_NextShot = ticks + m_ShotCooldown;
		App::s_CurrentLevel->AddProjectile(m_ProjectileType, this, m_Target);
	}
}

void DarkAttacker::InitAttack(Enemy *target, bool updateShotCD)
{
	// Don't initialize more attacks than just one
	if (m_Target)
		return;

	target->m_Attackers.emplace_back(this);

	if (updateShotCD)
		m_NextShot = SDL_GetTicks() + m_ShotCooldown - g_PausedTicks;

	m_Target = target;

	m_OccupiedTower->PlayAnim("Attack");
}

void DarkAttacker::StopAttacking(bool toErase)
{
	if (toErase)
		std::erase(m_Target->m_Attackers, this);

	m_Target = nullptr;

	m_OccupiedTower->PlayAnim("Idle");
}

void DarkAttacker::ValidTarget()
{
	if (m_Target->IsActive())
		return;

	// Do partially stuff of StopAttacking()
	std::erase(m_Target->m_Attackers, this);
	m_Target = nullptr;

	for (const auto &enemy : g_Enemies)
	{
		Enemy *e = dynamic_cast<Enemy*>(enemy);
		if (!e->IsTowerInRange(m_OccupiedTower, App::s_TowerRange))
			continue;

		InitAttack(e, false);
		break;
	}

	// Do the rest of StopAttacking() if couldn't find another target
	m_OccupiedTower->PlayAnim("Idle");
}