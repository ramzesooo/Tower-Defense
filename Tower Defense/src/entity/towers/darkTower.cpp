#include "darkTower.h"
#include "../../app.h"
#include "../attackers/attacker.h"

#include "SDL_rect.h"

extern uint32_t g_PausedTicks;

DarkTower::DarkTower(float posX, float posY, TowerType type) : Tower(posX, posY, type)
{
	static constexpr AttackerType attackerType = AttackerType::darkTower;

	m_TowerWidth = 160;
	m_TowerHeight = 186;
	srcRect.x = srcRect.y = 0;
	srcRect.w = m_TowerWidth;
	srcRect.h = m_TowerHeight;
	m_MaxTier = 1;
	m_AnimData.animated = true;
	m_AnimData.animations.emplace("Idle", Animation("Idle", 0, 13, 75));
	m_AnimData.animations.emplace("Attack", Animation("Attack", 1, 11, 75));
	PlayAnim("Idle");

	App::s_CurrentLevel->AddAttacker(this, attackerType);
}

void DarkTower::Update()
{
	srcRect.x = srcRect.w * static_cast<int32_t>(((SDL_GetTicks() - g_PausedTicks) / m_AnimData.currentAnim.speed) % m_AnimData.currentAnim.frames);
	m_Attacker->Update();
}

void DarkTower::Draw()
{
	if (m_IsHighlighted)
	{
		for (const auto &tile : m_TilesInRange)
		{
			tile->DrawHighlight();
		}
	}

	TextureManager::DrawTexture(m_Texture, srcRect, destRect);
}