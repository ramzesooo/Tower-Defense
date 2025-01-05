#include "tower.h"
#include "../attacker.h"
#include "../../level.h"
#include "../../textureManager.h"
#include "../../app.h"

#include <format>

extern uint32_t g_PausedTicks;

std::array<SDL_Texture*, std::size_t(TowerType::size)> Tower::s_TowerTextures{};

Tower::Tower(float posX, float posY, TowerType type)
	: m_Pos(posX * App::s_CurrentLevel->m_ScaledTileSize, posY * App::s_CurrentLevel->m_ScaledTileSize),
	m_Type(type), m_Texture(s_TowerTextures.at(static_cast<std::size_t>(type)))
{
	uint16_t scaledTileSize = App::s_CurrentLevel->m_ScaledTileSize;

	for (auto i = 0u; i < 4u; i++)
	{
		Tile *tile = App::s_CurrentLevel->GetTileFrom(static_cast<uint32_t>(posX) + i % 2, static_cast<uint32_t>(posY) + i / 2);
		m_OccupiedTiles[i] = tile;
		tile->SetTowerOccupying(this);
	}
	
	destRect.x = static_cast<int32_t>(m_Pos.x - App::s_Camera.x);
	destRect.y = static_cast<int32_t>(m_Pos.y - App::s_Camera.y);
	destRect.w = destRect.h = scaledTileSize * 2;

	AddToGroup(EntityGroup::tower);
}

void Tower::Destroy()
{
	m_IsActive = false;

	if (m_Attacker)
	{
		if (m_Attacker->IsAttacking())
			m_Attacker->StopAttacking();

		m_Attacker->Destroy();
		m_Attacker = nullptr;
	}

	for (const auto &tile : m_OccupiedTiles)
	{
		// It should never happen, but whatever
		if (!tile || tile->GetTowerOccupying() != this)
			continue;

		tile->SetTowerOccupying(nullptr);
	}

	App::s_Manager.m_EntitiesToDestroy = true;
}

void Tower::Draw()
{
	TextureManager::DrawTexture(m_Texture, srcRect, destRect);
}

void Tower::AdjustToView()
{
	destRect.x = static_cast<int32_t>(m_Pos.x - App::s_Camera.x);
	destRect.y = static_cast<int32_t>(m_Pos.y - App::s_Camera.y);

	if (m_Attacker)
		m_Attacker->AdjustToView();
}

Tile *Tower::GetOccupiedTile(uint16_t ID) const
{
	if (ID >= m_OccupiedTiles.size())
	{
		App::s_Logger.AddLog(std::format("Tower::GetOccupiedTile: Requested tile #{}, but maximum is #{}", ID, m_OccupiedTiles.size()));
		return nullptr;
	}

	return m_OccupiedTiles.at(ID);
}

void Tower::PlayAnim(std::string_view animID)
{
	auto it = m_AnimData.animations.find(animID);
	if (it == m_AnimData.animations.end())
	{
		App::s_Logger.AddLog(std::string_view("Couldn't find animation called "));
		App::s_Logger.AddLog(animID);
		return;
	}

	m_AnimData.currentAnim = it->second;
	srcRect.y = m_AnimData.currentAnim.index * m_TowerHeight;
}

void Tower::UpdateAnimSpeed(std::string_view animID, int32_t newSpeed)
{
	auto it = m_AnimData.animations.find(animID);
	if (it == m_AnimData.animations.end())
	{
		App::s_Logger.AddLog(std::string_view("Tower::UpdateAnimSpeed: Couldn't find animation called "));
		App::s_Logger.AddLog(animID);
		return;
	}

	it->second.speed = newSpeed;

	if (it->second.id == m_AnimData.currentAnim.id)
		m_AnimData.currentAnim = it->second;
}

int32_t Tower::GetAnimSpeed(std::string_view animID)
{
	auto it = m_AnimData.animations.find(animID);
	if (it == m_AnimData.animations.end())
	{
		App::s_Logger.AddLog(std::string_view("Tower::GetAnimSpeed: Couldn't find animation called "));
		App::s_Logger.AddLog(animID);
		return 0;
	}

	return it->second.speed;
}