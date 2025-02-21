#include "darkTower.h"
#include "../../app.h"
#include "../attackers/attacker.h"

#include "SDL_rect.h"

extern uint32_t g_PausedTicks;

DarkTower::DarkTower(float posX, float posY) : Tower(posX, posY, TowerType::dark, { 160, 186 }, 1), m_Ticks(SDL_GetTicks() - g_PausedTicks)
{
	static constexpr AttackerType attackerType = AttackerType::darkTower;

	m_AnimData.animated = true;
	m_AnimData.animations.emplace("Idle", Animation("Idle", 0, 13, 75));
	m_AnimData.animations.emplace("Attack", Animation("Attack", 1, 11, 75));
	PlayAnim("Idle");

	App::s_CurrentLevel->AddAttacker(this, attackerType);
}

void DarkTower::Update()
{
	srcRect.x = srcRect.w * static_cast<int32_t>(((SDL_GetTicks() - m_Ticks - g_PausedTicks) / m_AnimData.currentAnim.speed) % m_AnimData.currentAnim.frames);
	m_Attacker->Update();
}

void DarkTower::Draw() const
{
	TextureManager::DrawTexture(m_Texture, srcRect, destRect);
}

void DarkTower::AdjustToView()
{
	destRect.x = static_cast<int32_t>(m_Pos.x - App::s_Camera.x);
	destRect.y = static_cast<int32_t>(m_Pos.y - App::s_Camera.y);
}