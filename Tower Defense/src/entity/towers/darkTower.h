#pragma once
#include "tower.h"

class DarkTower : public Tower
{
public:
	DarkTower() = delete;
	DarkTower(float posX, float posY, TowerType type);
	DarkTower(const DarkTower &) = delete;
	~DarkTower() = default;

	DarkTower &operator=(const DarkTower &) = delete;

	virtual void Update() override;
	virtual void Draw() override;
};