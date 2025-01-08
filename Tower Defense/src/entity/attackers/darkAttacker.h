#pragma once
#include "attacker.h"

class DarkAttacker : public Attacker
{
public:
	DarkAttacker() = delete;
	DarkAttacker(Tower *occupiedTower, AttackerType type, SDL_Texture *texture, uint32_t shotCooldown, uint16_t scale = 1);
	~DarkAttacker() = default;

	void Update() override;
	void Draw() override {};

	void AdjustToView() override {};

	// InitAttack method is canceled if there is already targeted enemy
	// All this method does is playing animation and setting up cooldown at m_NextShoot
	// And the true code for attacking happens in Update()
	void InitAttack(Enemy *target, bool updateShotCD = true) override;

	// arg bool toErase is true by default
	void StopAttacking(bool toErase = true) override;
	bool ValidTarget() override;
};