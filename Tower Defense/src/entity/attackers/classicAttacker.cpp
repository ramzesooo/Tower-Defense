#include "classicAttacker.h"
#include "../towers/tower.h"
#include "../../app.h"

#include "SDL_mixer.h"

extern uint32_t g_PausedTicks;
extern std::vector<Entity*> &g_Enemies;

ClassicAttacker::ClassicAttacker(Tower *occupiedTower, AttackerType type, uint32_t shotCooldown, uint16_t scale)
	: Attacker(occupiedTower, type, shotCooldown * 4u, scale)
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
	static Mix_Chunk *attackSound = App::s_Textures.GetSound("arrowAttack");

	uint32_t ticks = SDL_GetTicks() - g_PausedTicks;

	srcRect.x = srcRect.w * ((ticks / m_CurrentAnim.speed) % m_CurrentAnim.frames);
	srcRect.y = m_CurrentAnim.index * Attacker::s_AttackerHeight;

	if (!ValidTarget())
		return;

	// Create new projectile if got target and passed the cooldown of attack
	if (ticks >= m_NextShot)
	{
		m_NextShot = ticks + m_ShotCooldown;
		App::s_CurrentLevel->AddProjectile(m_ProjectileType, this, m_Target);
		Mix_PlayChannel(-1, attackSound, 0);
	}
}

void ClassicAttacker::InitAttack(Enemy *target, bool updateShotCD)
{
	// Don't initialize more attacks than just one
	if (IsAttacking())
		return;

	target->m_Attackers.emplace_back(this);

	if (updateShotCD)
		m_NextShot = SDL_GetTicks() + m_ShotCooldown - g_PausedTicks;

	m_Target = target;

	PlayAnim("Shoot");
}

void ClassicAttacker::StopAttacking(bool toErase)
{
	if (toErase)
		std::erase(m_Target->m_Attackers, this);

	m_Target = nullptr;

	PlayAnim("Idle");
}

bool ClassicAttacker::ValidTarget()
{
	// Do partially stuff of StopAttacking()
	if (m_Target)
	{
		if (m_Target->IsActive() && m_Target->IsTowerInRange(m_OccupiedTower))
			return true;

		std::erase(m_Target->m_Attackers, this);
		m_Target = nullptr;
	}

	for (const auto &enemy : g_Enemies)
	{
		if (!enemy->IsActive())
			continue;

		Enemy *e = dynamic_cast<Enemy*>(enemy);
		if (!e->IsTowerInRange(m_OccupiedTower))
			continue;

		InitAttack(e, false);
		return true;
	}

	// Do the rest of StopAttacking() if couldn't find another target
	PlayAnim("Idle");
	return false;
}