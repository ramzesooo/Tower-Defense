#include "attacker.h"
#include "tower.h"
#include "../level.h"
#include "../app.h"

Attacker::Attacker(Tower* occupiedTower, AttackerType type, SDL_Texture* texture, uint32_t shotCooldown, uint16_t scale)
	: m_OccupiedTower(occupiedTower), m_Type(type), m_Texture(texture), m_Scale(scale), m_Pos(m_OccupiedTower->GetPos()), m_ShotCooldown(shotCooldown * 5)
{
	switch (m_Type)
	{
		case AttackerType::archer:
		case AttackerType::musketeer:
			m_Pos.x -= (float)App::s_CurrentLevel->m_ScaledTileSize / 3.0f;
			m_ProjectileType = ProjectileType::arrow;
			break;
		case AttackerType::hunter:
			m_Pos.x -= (float)App::s_CurrentLevel->m_ScaledTileSize / 4.0f;
			m_ProjectileType = ProjectileType::arrow;
			break;
	}
	destRect.w = Attacker::s_AttackerWidth * m_Scale;
	destRect.h = Attacker::s_AttackerHeight * m_Scale;
	destRect.x = static_cast<int32_t>(m_Pos.x - App::s_Camera.x) + destRect.w / 2;
	destRect.y = static_cast<int32_t>(m_Pos.y - App::s_Camera.y);

	animations.emplace("Idle", Animation("Idle", 0, 2, 600));
	animations.emplace("Shoot", Animation("Shoot", 1, 4, shotCooldown));
	
	PlayAnim("Idle");
}

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

	App::s_Manager.m_EntitiesToDestroy = true;
}

void Attacker::Update()
{
	uint32_t ticks = SDL_GetTicks();

	if (IsAttacking() && SDL_TICKS_PASSED(ticks, m_NextShot))
	{
		m_NextShot = SDL_GetTicks() + m_ShotCooldown;
		App::s_CurrentLevel->AddProjectile(m_ProjectileType, this, m_Target);
	}

	srcRect.x = srcRect.w * (((ticks - m_AdjustedTicks) / m_CurrentAnim.speed) % m_CurrentAnim.frames);
	srcRect.y = m_CurrentAnim.index * Attacker::s_AttackerHeight;
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
		App::s_Logger.AddLog("Couldn't find animation called " + std::string(animID));
		return;
	}

	if (m_CurrentAnim.id != it->second.id)
	{
		m_CurrentAnim = it->second;
	}
}

void Attacker::InitAttack(Enemy* target)
{
	// Don't initialize more attacks than just one
	if (m_Target)
		return;

	target->m_Attackers.emplace_back(this);

	m_NextShot = SDL_GetTicks();
	m_Target = target;

	PlayAnim("Shoot");
}

void Attacker::StopAttacking(bool toErase)
{
	if (toErase)
	{
		for (auto it = m_Target->m_Attackers.begin(); it != m_Target->m_Attackers.end(); it++)
		{
			if ((*it) == this)
			{
				m_Target->m_Attackers.erase(it);
				break;
			}
		}
	}

	m_AdjustedTicks = SDL_GetTicks();
	m_Target = nullptr;
	m_NextShot = NULL;
	PlayAnim("Idle");
}