#pragma once
#include "tower.h"

class ClassicTower : public Tower
{
public:
	ClassicTower() = delete;
	ClassicTower(float posX, float posY, TowerType type);
	~ClassicTower() = default;

	virtual void Upgrade() override;
};