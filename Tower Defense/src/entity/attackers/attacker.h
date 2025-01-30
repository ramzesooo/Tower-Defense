#pragma once
#include "../../common.h"
#include "../typesEnums.h"
#include "../entity.h"
#include "../enemy.h"
#include "../projectiles/projectile.h"
#include "../../Vector2D.h"
#include "../anim.h"

#include "SDL_rect.h"

#include <unordered_map>

class Tower;

// Attacker is the entity occupying a tower
class Attacker : public Entity
{
public:
	static constexpr int32_t s_AttackerWidth = 32;
	static constexpr int32_t s_AttackerHeight = 32;

	std::vector<Projectile*> m_OwnedProjectiles;
public:
	Attacker() = delete;
	Attacker(Tower *occupiedTower, AttackerType type, uint32_t shotCooldown, uint16_t scale = 1);
	Attacker(const Attacker &) = delete;
	~Attacker() = default;

	Attacker &operator=(const Attacker &) = delete;

	void Destroy() override;

	virtual void Update() override = 0;
	virtual void Draw() override;

	virtual void AdjustToView() override;

	Vector2D GetPos() const override { return m_Pos; }

	void PlayAnim(std::string_view animID);

	// InitAttack method is canceled if there is already targeted enemy
	// All this method does is playing animation and setting up cooldown at m_NextShoot
	// And the true code for attacking happens in Update()
	virtual void InitAttack(Enemy *target, bool updateShotCD = true);

	// arg bool toErase is true by default
	virtual void StopAttacking(bool toErase = true);
	bool IsAttacking() const { return m_Target != nullptr; }

	virtual bool ValidTarget();

	Enemy *GetTarget() const { return m_Target; }
	void SetTarget(Enemy *target) { m_Target = target; }
protected:
	uint32_t m_ShotCooldown = 300 * 4; // 300 is delay between frames in Shoot anim times 4 frames (milliseconds)
	Tower *m_OccupiedTower;
	AttackerType m_Type;
	ProjectileType m_ProjectileType = ProjectileType::arrow;
	SDL_Texture *m_Texture = nullptr;
	bool m_Invisible = false;
	uint16_t m_Scale = 1;

	// m_Pos for Attacker is taken from appropriate tower and it's already scaled with tiles' size
	// The difference is (float)App::s_CurrentLevel->m_ScaledTileSize / 3.0f has been added to m_Pos.x
	// to make it look more like it's on the tower
	Vector2D m_Pos{};

	SDL_Rect srcRect{ 0, 0, 32, 32 }, destRect{ 0, 0, 32, 32 };

	Animation m_CurrentAnim;
	std::unordered_map<std::string, Animation, proxy_hash, std::equal_to<void>> animations;

	Enemy *m_Target = nullptr;
	uint32_t m_NextShot = NULL;
	// m_AdjustedTicks is nothing else than just ticks, but it's updated in InitAttack() to display animation in correct way
	//uint32_t m_AdjustedTicks = SDL_GetTicks();
};