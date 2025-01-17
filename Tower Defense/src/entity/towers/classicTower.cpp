#include "classicTower.h"
#include "../../app.h"
#include "../attackers/attacker.h"

#include "SDL_rect.h"

ClassicTower::ClassicTower(float posX, float posY, TowerType type) : Tower(posX, posY, type)
{
	static constexpr AttackerType attackerType = AttackerType::archer;

	m_TowerWidth = 144;
	m_TowerHeight = 64;
	//srcRect.x = (tier - 1) * (imageWidth / 3);
	srcRect.x = srcRect.y = 0;
	srcRect.w = m_TowerWidth / 3;
	srcRect.h = 64;
	m_MaxTier = 3;

	App::s_CurrentLevel->AddAttacker(this, attackerType);
}

void ClassicTower::Upgrade()
{
	m_Tier++;
	srcRect.x = (m_Tier - 1) * (m_TowerWidth / 3);

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
		App::s_Building.originalTexture = App::s_Textures.GetTexture("cantBuild");
		App::s_Building.buildingPlace.SetTexture(App::s_Building.originalTexture);
		App::s_Building.towerToUpgradeOrSell = nullptr;
		return;
	}
}