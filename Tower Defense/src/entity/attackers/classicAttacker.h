#pragma once
#include "attacker.h"

class ClassicAttacker : public Attacker
{
public:
	ClassicAttacker() = delete;
	ClassicAttacker(Tower *occupiedTower, AttackerType type, SDL_Texture *texture, uint32_t shotCooldown, uint16_t scale = 1);
	~ClassicAttacker() = default;

	void Update() override;

	// InitAttack method is canceled if there is already targeted enemy
	// All this method does is playing animation and setting up cooldown at m_NextShoot
	// And the true code for attacking happens in Update()
	void InitAttack(Enemy *target, bool updateShotCD = true) override;

	// arg bool toErase is true by default
	void StopAttacking(bool toErase = true) override;
	void ValidTarget() override;
};