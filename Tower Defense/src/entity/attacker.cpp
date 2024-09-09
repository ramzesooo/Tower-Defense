#include "attacker.h"
#include "tower.h"
#include "../level.h"
#include "../app.h"

constexpr int32_t shotCooldown = 300 * 4; // 400 is delay between frames in Shoot anim times 4 frames (milliseconds)

Attacker::Attacker(Tower& occupiedTower, AttackerType type, SDL_Texture* texture, uint16_t scale)
	: m_OccupiedTower(occupiedTower), m_Type(type), m_Texture(texture), m_Scale(scale), m_Pos(m_OccupiedTower.GetPos()), projectiles(App::s_Manager->GetGroup(EntityGroup::projectile))
{
	m_Pos.x += (float)App::s_CurrentLevel->m_ScaledTileSize / 3.0f;
	destRect.w = Attacker::s_AttackerWidth * m_Scale;
	destRect.h = Attacker::s_AttackerHeight * m_Scale;

	Animation idle = Animation(0, 2, 600);
	Animation shoot = Animation(1, 4, 300);

	animations.emplace("Idle", idle);
	animations.emplace("Shoot", shoot);

	PlayAnim("Idle");
}

void Attacker::Update()
{
	uint32_t ticks = SDL_GetTicks();

	if (m_Target)
	{
		if (!m_Target->IsTowerInRange(&m_OccupiedTower, App::s_TowerRange) || !m_Target->IsActive())
		{
			m_Target = nullptr;
			m_NextShot = NULL;
			PlayAnim("Idle");
			return;
		}

		if (SDL_TICKS_PASSED(ticks, m_NextShot))
		{
			m_NextShot = SDL_GetTicks() + shotCooldown;
			Projectile* projectile = App::s_Manager->NewEntity<Projectile>(ProjectileType::arrow, this, m_Target);
			projectile->AddGroup(EntityGroup::projectile);
		}
	}

	srcRect.x = srcRect.w * ((ticks / m_AnimSpeed) % m_AnimFrames);
	srcRect.y = m_AnimIndex * Attacker::s_AttackerHeight;

	destRect.x = static_cast<int32_t>(m_Pos.x - App::s_Camera.x);
	destRect.y = static_cast<int32_t>(m_Pos.y - App::s_Camera.y);
}

void Attacker::Draw()
{
	TextureManager::DrawTexture(m_Texture, srcRect, destRect);
}

void Attacker::PlayAnim(std::string_view animID)
{
	auto it = animations.find(animID);
	if (it == animations.end())
	{
		App::s_Logger->AddLog("Couldn't find animation called " + std::string(animID));
		return;
	}

	if (animID == m_AnimID)
	{
		return;
	}

	m_AnimID = animID;

	m_AnimFrames = it->second.frames;
	m_AnimIndex = it->second.index;
	m_AnimSpeed = it->second.speed;
}

void Attacker::InitAttack(Enemy* target)
{
	// Don't initialize more attacks than just one
	if (m_Target)
	{
		return;
	}

	m_NextShot = SDL_GetTicks() + shotCooldown;
	m_Target = target;

	PlayAnim("Shoot");
}