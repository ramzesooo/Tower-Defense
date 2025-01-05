#pragma once
#include "tower.h"

class ClassicTower : public Tower
{
public:
	ClassicTower() = delete;
	ClassicTower(float posX, float posY, TowerType type);
	~ClassicTower() = default;

	//virtual void Destroy() override;

	virtual void Update() override {};
	//virtual void Draw() override;
	//virtual void AdjustToView() override;
	virtual void Upgrade() override;
};