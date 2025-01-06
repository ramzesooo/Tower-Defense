#pragma once
#include "attacker.h"

class DarkAttacker : public Attacker
{
public:
	DarkAttacker() = delete;
	DarkAttacker(Tower *occupiedTower, AttackerType type, SDL_Texture *texture, uint32_t shotCooldown, uint16_t scale = 1);
	~DarkAttacker() = default;

	virtual void Update() override;
	virtual void Draw() override {};

	virtual void AdjustToView() override {};
};