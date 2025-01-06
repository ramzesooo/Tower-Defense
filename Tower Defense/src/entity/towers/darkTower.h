#pragma once
#include "tower.h"

class DarkTower : public Tower
{
public:
	DarkTower() = delete;
	DarkTower(float posX, float posY, TowerType type);
	~DarkTower() = default;

	virtual void Update() override;
};