#include "attacker.h"
#include "tower.h"
#include "../level.h"
#include "../app.h"

Attacker::Attacker(Tower& occupiedTower, AttackerType type, SDL_Texture* texture, uint16_t scale)
	: m_OccupiedTower(occupiedTower), m_Type(type), m_Texture(texture), m_Scale(scale), m_Pos(m_OccupiedTower.GetPos())
{
	m_Pos.x += (float)App::s_CurrentLevel->m_ScaledTileSize / 3.0f;
	destRect.w = Attacker::s_AttackerWidth * m_Scale;
	destRect.h = Attacker::s_AttackerHeight * m_Scale;

	Animation idle = Animation(0, 2, 300);

	animations.emplace("Idle", idle);

	PlayAnim("Idle");
}

void Attacker::Update()
{
	srcRect.x = srcRect.w * static_cast<int>((SDL_GetTicks() / m_AnimSpeed) % m_AnimFrames);
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

	m_AnimFrames = it->second.frames;
	m_AnimIndex = it->second.index;
	m_AnimSpeed = it->second.speed;
}