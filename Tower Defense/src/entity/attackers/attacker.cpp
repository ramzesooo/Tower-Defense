#include "attacker.h"
#include "../towers/tower.h"
#include "../../app.h"

// TODO: Currently not used, bot necessary for adjusting animation just like for animated towers
extern uint32_t g_PausedTicks;
extern std::vector<Entity*> &g_Enemies;

Attacker::Attacker(Tower *occupiedTower, AttackerType type, SDL_Texture* texture, uint32_t shotCooldown, uint16_t scale)
	: m_OccupiedTower(occupiedTower), m_Type(type), m_Texture(texture), m_Scale(scale), m_Pos(m_OccupiedTower->GetPos()), m_ShotCooldown(shotCooldown),
	destRect({ destRect.x, destRect.y, Attacker::s_AttackerWidth * m_Scale, Attacker::s_AttackerHeight * m_Scale })
{}

void Attacker::Destroy()
{
	m_IsActive = false;

	m_Target = nullptr;

	for (const auto &p : m_OwnedProjectiles)
	{
		p->SetOwner(nullptr);
		p->SetTarget(nullptr);
		p->Destroy();
	}

	// It's unnecessary since tower already assigns nullptr by itself
	// But might be safer as well
	//m_OccupiedTower->AssignAttacker(nullptr);

	App::s_Manager.m_EntitiesToDestroy = true;
}

void Attacker::Draw()
{
	TextureManager::DrawTexture(m_Texture, srcRect, destRect);
}

void Attacker::AdjustToView()
{
	destRect.x = static_cast<int32_t>(m_Pos.x - App::s_Camera.x) + destRect.w / 2;
	destRect.y = static_cast<int32_t>(m_Pos.y - App::s_Camera.y);
}

void Attacker::PlayAnim(std::string_view animID)
{
	auto it = animations.find(animID);
	if (it == animations.end())
	{
		App::s_Logger.AddLog(std::string_view("Couldn't find animation called "), false);
		App::s_Logger.AddLog(animID);
		return;
	}

	m_CurrentAnim = it->second;
}

void Attacker::InitAttack(Enemy *target, bool updateShotCD)
{
	// Don't initialize more attacks than just one
	if (m_Target)
		return;

	target->m_Attackers.emplace_back(this);

	if (updateShotCD)
		m_NextShot = SDL_GetTicks() + m_ShotCooldown - g_PausedTicks;

	m_Target = target;

	if (!m_Invisible)
		PlayAnim("Shoot");

	if (m_OccupiedTower->IsAnimated())
		m_OccupiedTower->PlayAnim("Attack");
}

void Attacker::StopAttacking(bool toErase)
{
	if (toErase)
		std::erase(m_Target->m_Attackers, this);

	m_Target = nullptr;
	m_NextShot = 0u;

	if (!m_Invisible)
		PlayAnim("Idle");

	if (m_OccupiedTower->IsAnimated())
		m_OccupiedTower->PlayAnim("Idle");
}

bool Attacker::ValidTarget()
{
	// Do partially stuff of StopAttacking()
	if (IsAttacking())
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
	if (!m_Invisible)
		PlayAnim("Idle");

	if (m_OccupiedTower->IsAnimated())
		m_OccupiedTower->PlayAnim("Idle");

	return false;
}