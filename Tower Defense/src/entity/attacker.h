#pragma once
#include "entity.h"
#include "enemy.h"
#include "projectile.h"
#include "../Vector2D.h"
#include "anim.h"
#include "../textureManager.h"

#include "SDL.h"

#include <unordered_map>

enum class AttackerType
{
	archer = 0,
	size
};

class Tower;

// Attacker is the entity occupying a tower
class Attacker : public Entity
{
public:
	Attacker(Tower& occupiedTower, AttackerType type, SDL_Texture* texture, uint16_t scale = 1);

	void Update() override;
	void Draw() override;

	Vector2D GetPos() const override { return m_Pos; }

	void PlayAnim(std::string_view animID);

	// InitAttack method is canceled if there is already targeted enemy
	// All this method does is playing animation and setting up cooldown at m_NextShoot
	// And the true code for attacking happens in Update()
	void InitAttack(Enemy* target);

	Enemy* GetTarget() const { return m_Target; }
private:
	static constexpr int32_t s_AttackerWidth = 32;
	static constexpr int32_t s_AttackerHeight = 32;
	Tower& m_OccupiedTower;
	AttackerType m_Type;
	SDL_Texture* m_Texture = nullptr;
	uint16_t m_Scale = 1;

	// m_Pos for Attacker is taken from appropriate tower and it's already scaled with tiles' size
	// The difference is (float)App::s_CurrentLevel->m_ScaledTileSize / 3.0f has been added to m_Pos.x
	// to make it look more like it's on the tower
	Vector2D m_Pos;

	SDL_Rect srcRect{ 0, 0, 32, 32 }, destRect{ 0, 0, 32, 32 };

	int32_t m_AnimIndex = 0;
	int32_t m_AnimFrames = 1;
	int32_t m_AnimSpeed = 100;
	std::string m_AnimID;
	std::unordered_map<std::string, Animation, proxy_hash, std::equal_to<void>> animations;

	Enemy* m_Target = nullptr;
	uint32_t m_NextShot = NULL;
};