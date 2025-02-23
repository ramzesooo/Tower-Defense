#pragma once
#include "projectile.h"

class ThunderProjectile : public Projectile
{
public:
	ThunderProjectile() = delete;
	ThunderProjectile(Attacker* owner, Enemy* target);
	ThunderProjectile(const ThunderProjectile&) = delete;
	~ThunderProjectile() = default;

	ThunderProjectile& operator=(const ThunderProjectile&) = delete;

	void Update() override;
	void Draw() override;
	void AdjustToView() override;
};