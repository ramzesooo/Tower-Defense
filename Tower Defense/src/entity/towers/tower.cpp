#include "tower.h"
#include "../../level.h"
#include "../../textureManager.h"
#include "../../app.h"

#include "SDL_mixer.h"

#include <format>

extern uint32_t g_PausedTicks;

//std::array<SDL_Texture*, std::size_t(TowerType::size)> Tower::s_TowerTextures{};
std::array<std::array<SDL_Texture*, 2u>, std::size_t(TowerType::size)> Tower::s_TowerTextures{};

Tower::Tower(float posX, float posY, TowerType type)
	: m_Pos(posX * App::s_CurrentLevel->m_ScaledTileSize, posY * App::s_CurrentLevel->m_ScaledTileSize),
	m_Type(type), m_Texture(s_TowerTextures[static_cast<std::size_t>(type)][0])
{
	static Mix_Chunk *buildSound = App::s_Textures.GetSound("finishBuild");
	static constexpr uint16_t towerOffset = 1;

	Tile *tile = nullptr;
	for (auto i = 0u; i < 4u; i++)
	{
		tile = App::s_CurrentLevel->GetTileFrom(static_cast<uint32_t>(posX) + i % 2, static_cast<uint32_t>(posY) + i / 2);
		m_OccupiedTiles[i] = tile;
		tile->SetTowerOccupying(this);
	}

	tile = nullptr;

	// Squared triangle * 4 for all sides
	m_TilesInRange.reserve(static_cast<std::size_t>((App::s_TowerRange + 1) * (App::s_TowerRange + 2) / 2 * 4));

	for (auto y = 0; y <= static_cast<int16_t>(App::s_TowerRange); y++)
	{
		for (auto x = y; x <= static_cast<int16_t>(App::s_TowerRange); x++)
		{
			// Loop for reaching mirror reflect
			for (auto i = 1; i >= -1; i = i - 2)
			{
				// Left side
				uint32_t tileX = static_cast<uint32_t>(posX - x + y);

				// std::min(i, 0) lets to make the mirror reflect appropriate for Y position
				// Since tower takes 4 tiles and basically loop considers only case of 0, 0,
				// it's needed to be adjusted and look for position Y - 1
				uint32_t tileY = static_cast<uint32_t>(posY - std::min(i, 0) - (y * i)); // posY - 1 - y or posY - 0 + y

				tile = App::s_CurrentLevel->GetTileFrom(tileX, tileY);

				if (tile)
				{
					m_TilesInRange.emplace_back(tile);
				}

				// Right side
				// towerOffset should be changed only in case when the tower takes more than 4 tiles
				tileX = static_cast<uint32_t>(posX + towerOffset + x - y);

				tile = App::s_CurrentLevel->GetTileFrom(tileX, tileY);

				if (tile)
				{
					m_TilesInRange.emplace_back(tile);
				}
			} // for loop i = 1; i = i - 2
		} // for loop x = y
	} // for loop y = 0

	m_TilesInRange.shrink_to_fit();

	Mix_PlayChannel(-1, buildSound, 0); // play sound

	destRect.x = static_cast<int32_t>(m_Pos.x - App::s_Camera.x);
	destRect.y = static_cast<int32_t>(m_Pos.y - App::s_Camera.y);
	destRect.w = destRect.h = App::s_CurrentLevel->m_ScaledTileSize * 2;

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

	if (m_Attacker)
		m_Attacker->Draw();
}

void Tower::AdjustToView()
{
	destRect.x = static_cast<int32_t>(m_Pos.x - App::s_Camera.x);
	destRect.y = static_cast<int32_t>(m_Pos.y - App::s_Camera.y);

	if (m_Attacker)
		m_Attacker->AdjustToView();
}

//void Tower::Upgrade()
//{
//	App::s_Building.buildingPlace.SetTexture(App::s_Building.cantBuildTexture);
//	App::s_Building.canBuild = false;
//	App::s_Building.towerToUpgradeOrSell = nullptr;
//}

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