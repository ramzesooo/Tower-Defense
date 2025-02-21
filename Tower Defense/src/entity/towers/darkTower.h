#pragma once
#include "tower.h"

class DarkTower : public Tower
{
public:
	DarkTower() = delete;
	DarkTower(float posX, float posY);
	DarkTower(const DarkTower &) = delete;
	~DarkTower() = default;

	DarkTower &operator=(const DarkTower &) = delete;

	void Update() override;
	void Draw() const override;

	void AdjustToView() override;

	void UpdateTicks(uint32_t ticks) { m_Ticks = ticks; }
private:
	uint32_t m_Ticks;
};