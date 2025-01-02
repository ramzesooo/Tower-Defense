#include "tower.h"
#include "attacker.h"
#include "../level.h"
#include "../textureManager.h"
#include "../app.h"

#include <format>

extern uint32_t g_PausedTicks;

std::array<SDL_Texture *, std::size_t(TowerType::size)> Tower::s_TowerTextures{};

Tower::Tower(float posX, float posY, TowerType type)
	: m_Pos(posX * App::s_CurrentLevel->m_ScaledTileSize, posY * App::s_CurrentLevel->m_ScaledTileSize),
	m_Type(type), m_Texture(s_TowerTextures.at((std::size_t)type))
{
	uint16_t scaledTileSize = App::s_CurrentLevel->m_ScaledTileSize;

	{
		Tile* tile = nullptr;
		for (auto i = 0u; i < 4u; i++)
		{
			tile = App::s_CurrentLevel->GetTileFrom(static_cast<uint32_t>(posX) + i % 2, static_cast<uint32_t>(posY) + i / 2);
			m_OccupiedTiles[i] = tile;
			tile->SetTowerOccupying(this);
		}
	}
	
	destRect.x = static_cast<int32_t>(m_Pos.x - App::s_Camera.x);
	destRect.y = static_cast<int32_t>(m_Pos.y - App::s_Camera.y);
	destRect.w = destRect.h = scaledTileSize * 2;

	switch (m_Type)
	{
	case TowerType::classic:
		m_TowerWidth = 144;
		m_TowerHeight = 64;
		//srcRect.x = (tier - 1) * (imageWidth / 3);
		srcRect.x = srcRect.y = 0;
		srcRect.w = m_TowerWidth / 3;
		srcRect.h = 64;
		m_MaxTier = 3;

		{
			static constexpr AttackerType attackerType = AttackerType::archer;
			App::s_CurrentLevel->AddAttacker(this, attackerType);
		}
		break;
	case TowerType::dark:
		//destRect.w = destRect.h = scaledTileSize * 4;
		m_TowerWidth = 160;
		m_TowerHeight = 186;
		srcRect.x = srcRect.y = 0;
		srcRect.w = m_TowerWidth;
		srcRect.h = m_TowerHeight;
		m_MaxTier = 1;
		m_AnimData.animated = true;
		m_AnimData.animations.emplace("Idle", Animation("Idle", 0, 13, 100));
		m_AnimData.animations.emplace("Attack", Animation("Attack", 1, 11, 100));
		PlayAnim("Idle");
		AddToGroup(EntityGroup::animatedTower);

		{
			static constexpr AttackerType attackerType = AttackerType::darkTower;
			App::s_CurrentLevel->AddAttacker(this, attackerType);
		}
		break;
	}

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

	for (const auto& tile : m_OccupiedTiles)
	{
		if (!tile || tile->GetTowerOccupying() != this)
			continue;

		tile->SetTowerOccupying(nullptr);
	}

	App::s_Manager.m_EntitiesToDestroy = true;
}

void Tower::Update()
{
	srcRect.x = srcRect.w * static_cast<int32_t>(((SDL_GetTicks() - g_PausedTicks) / m_AnimData.currentAnim.speed) % m_AnimData.currentAnim.frames);
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

void Tower::Upgrade()
{
	if (m_Tier >= m_MaxTier)
	{
		App::s_Building.originalTexture = App::s_Textures.GetTexture("cantBuild");
		App::s_Building.buildingPlace.SetTexture(App::s_Building.originalTexture);
		App::s_Building.towerToUpgrade = nullptr;
		return;
	}

	++m_Tier;
	srcRect.x = (m_Tier - 1) * (m_TowerWidth / 3);

	if (m_Attacker)
	{
		if (m_Attacker->IsAttacking())
			m_Attacker->StopAttacking();

		m_Attacker->Destroy();
		m_Attacker = nullptr;

		App::s_Manager.Refresh();
		App::s_CurrentLevel->AddAttacker(this, (AttackerType)(m_Tier - 1));
	}
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
		return 0u;
	}

	return it->second.speed;
}