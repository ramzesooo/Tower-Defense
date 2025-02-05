#include "classicTower.h"
#include "../../app.h"
#include "../attackers/attacker.h"

#include "SDL_rect.h"

ClassicTower::ClassicTower(float posX, float posY) : Tower(posX, posY, TowerType::classic, { 144, 64 }, 3)
{
	static constexpr AttackerType attackerType = AttackerType::archer;

	//m_ImageSize = { 144, 64 };
	//srcRect.x = (m_Tier - 1) * (m_ImageSize[0] / 3);
	//srcRect.x = srcRect.y = 0;
	srcRect.w = m_ImageSize[0] / m_MaxTier;
	//srcRect.h = 64;
	//srcRect.h = m_ImageSize[1];

	App::s_CurrentLevel->AddAttacker(this, attackerType);
}

void ClassicTower::Draw()
{
	TextureManager::DrawTexture(m_Texture, srcRect, destRect);

	m_Attacker->Draw();
}

void ClassicTower::Upgrade()
{
	m_Tier++;
	srcRect.x = (m_Tier - 1) * (m_ImageSize[0] / 3);

	if (m_Attacker)
	{
		if (m_Attacker->IsAttacking())
			m_Attacker->StopAttacking();

		m_Attacker->Destroy();
		m_Attacker = nullptr;

		App::s_Manager.Refresh();
		App::s_CurrentLevel->AddAttacker(this, static_cast<AttackerType>(m_Tier - 1));
	}

	if (!CanUpgrade())
	{
		App::s_Building.buildingPlace.SetTexture(App::s_Building.cantBuildTexture);
		App::s_Building.towerToUpgradeOrSell = nullptr;
		App::s_Building.canBuild = false;
		return;
	}
}

void ClassicTower::AdjustToView()
{
	destRect.x = static_cast<int32_t>(m_Pos.x - App::s_Camera.x);
	destRect.y = static_cast<int32_t>(m_Pos.y - App::s_Camera.y);

	m_Attacker->AdjustToView();
}