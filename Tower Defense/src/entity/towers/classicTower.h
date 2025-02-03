#pragma once
#include "tower.h"

class ClassicTower : public Tower
{
public:
	ClassicTower() = delete;
	ClassicTower(float posX, float posY);
	ClassicTower(const ClassicTower &) = delete;
	~ClassicTower() = default;

	ClassicTower &operator=(const ClassicTower &) = delete;

	void Draw() override;

	void Upgrade() override;

	void AdjustToView() override;
};