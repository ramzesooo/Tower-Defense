#pragma once
#include "projectile.h"

class ArrowProjectile : public Projectile
{
public:
	ArrowProjectile() = delete;
	ArrowProjectile(Attacker* owner, Enemy* target);
	ArrowProjectile(const ArrowProjectile&) = delete;
	~ArrowProjectile() = default;

	ArrowProjectile& operator=(const ArrowProjectile&) = delete;

	void Update() override;
	void AdjustToView() override {};
};