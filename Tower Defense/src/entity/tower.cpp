#include "tower.h"
#include "attacker.h"
#include "../textureManager.h"
#include "../app.h"

static constexpr int32_t imageWidth = 144;

Tower::Tower(float posX, float posY, SDL_Texture* texture, uint16_t tier)
	: m_Pos(posX * App::s_CurrentLevel->m_ScaledTileSize, posY * App::s_CurrentLevel->m_ScaledTileSize), m_Texture(texture)
{
	uint16_t scaledTileSize = App::s_CurrentLevel->m_ScaledTileSize;

	if (tier > 3)
	{
		App::s_Logger.AddLog(std::string_view("Tried to add a tower with tier higher than 3"));
		tier = 3;
	}
	else if (tier < 1)
	{
		App::s_Logger.AddLog(std::string_view("Tried to add a tower with tier lower than 1"));
		tier = 1;
	}

	m_Tier = tier;

	{
		Tile* tile = nullptr;
		for (auto i = 0u; i < 4; i++)
		{
			tile = App::s_CurrentLevel->GetTileFrom((uint32_t)posX + i % 2, (uint32_t)posY + i / 2);
			m_OccupiedTiles[i] = tile;
			tile->SetTowerOccupying(this);
		}
	}
	
	srcRect.x = (tier - 1) * (imageWidth / 3);
	srcRect.y = 0;
	srcRect.w = (imageWidth / 3);
	srcRect.h = 64;

	destRect.x = static_cast<int32_t>(m_Pos.x - App::s_Camera.x);
	destRect.y = static_cast<int32_t>(m_Pos.y - App::s_Camera.y);
	destRect.w = destRect.h = scaledTileSize * 2;
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

void Tower::AdjustToView()
{
	destRect.x = static_cast<int32_t>(m_Pos.x - App::s_Camera.x);
	destRect.y = static_cast<int32_t>(m_Pos.y - App::s_Camera.y);

	if (m_Attacker)
		m_Attacker->AdjustToView();
}

void Tower::Draw()
{
	TextureManager::DrawTexture(m_Texture, srcRect, destRect);
}

void Tower::Upgrade()
{
	if (m_Tier >= 3)
	{
		App::s_Building.originalTexture = App::s_Textures.GetTexture("cantBuild");
		App::s_Building.buildingPlace.SetTexture(App::s_Building.originalTexture);
		App::s_Building.towerToUpgrade = nullptr;
		return;
	}

	++m_Tier;
	srcRect.x = (m_Tier - 1) * (imageWidth / 3);

	if (m_Attacker->IsAttacking())
		m_Attacker->StopAttacking();

	m_Attacker->Destroy();
	m_Attacker = nullptr;

	App::s_Manager.Refresh();
	App::s_CurrentLevel->AddAttacker(this, (AttackerType)(m_Tier - 1));
}