#pragma once
#include "attacker.h"

class ClassicAttacker : public Attacker
{
public:
	ClassicAttacker() = delete;
	ClassicAttacker(Tower *occupiedTower, AttackerType type, SDL_Texture *texture, uint32_t shotCooldown, uint16_t scale = 1);
	~ClassicAttacker() = default;

	virtual void Update() override;
};