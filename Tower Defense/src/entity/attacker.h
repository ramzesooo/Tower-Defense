#pragma once
#include "../common.h"
#include "typesEnums.h"
#include "entity.h"
#include "enemy.h"
#include "projectile.h"
#include "../Vector2D.h"
#include "anim.h"

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

	Attacker(Tower *occupiedTower, AttackerType type, SDL_Texture *texture, uint32_t shotCooldown, uint16_t scale = 1);
	Attacker(const Attacker &r) : m_OccupiedTower(r.m_OccupiedTower), m_Type(r.m_Type), m_Texture(r.m_Texture), m_Scale(r.m_Scale),
		m_Pos(r.m_Pos), srcRect(r.srcRect), destRect(r.destRect), m_CurrentAnim(r.m_CurrentAnim), m_Target(r.m_Target), m_NextShot(r.m_NextShot),
		m_Invisible(r.m_Invisible), animations(r.animations) {}
	~Attacker() = default;

	inline Attacker &operator=(const Attacker &r)
	{
		if (this == &r)
			return *this;

		m_OccupiedTower = r.m_OccupiedTower;
		m_Type = r.m_Type;
		m_Texture = r.m_Texture;
		m_Scale = r.m_Scale;
		m_Pos = r.m_Pos;
		srcRect = r.srcRect;
		destRect = r.destRect;
		m_CurrentAnim = r.m_CurrentAnim;
		m_Target = r.m_Target;
		m_NextShot = r.m_NextShot;
		m_Invisible = r.m_Invisible;
		animations = r.animations;

		return *this;
	}

	void Destroy() override;

	void Update() override;
	void Draw() override;

	void AdjustToView() override;

	Vector2D GetPos() const override { return m_Pos; }

	void PlayAnim(std::string_view animID);

	// InitAttack method is canceled if there is already targeted enemy
	// All this method does is playing animation and setting up cooldown at m_NextShoot
	// And the true code for attacking happens in Update()
	void InitAttack(Enemy* target);

	// arg bool toErase is true by default
	void StopAttacking(bool toErase = true);
	bool IsAttacking() const { return m_Target != nullptr; }

	Enemy* GetTarget() const { return m_Target; }
	void SetTarget(Enemy* target) { m_Target = target; }
private:
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
	Vector2D m_Pos;

	SDL_Rect srcRect{ 0, 0, 32, 32 }, destRect{ 0, 0, 32, 32 };

	Animation m_CurrentAnim;
	std::unordered_map<std::string, Animation, proxy_hash, std::equal_to<void>> animations;

	Enemy *m_Target = nullptr;
	uint32_t m_NextShot = NULL;
	// m_AdjustedTicks is nothing else than just ticks, but it's updated in InitAttack() to display animation in correct way
	//uint32_t m_AdjustedTicks = SDL_GetTicks();
};