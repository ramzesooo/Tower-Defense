#pragma once
#include "../../common.h"
#include "../typesEnums.h"
#include "../../Vector2D.h"
#include "../anim.h"

#include "../entity.h"
#include "../tile.h"
#include "../attackers/attacker.h"

#include "SDL_rect.h"

#include <unordered_map>

struct TowerAnimation
{
	bool animated = false;
	Animation currentAnim;
	std::unordered_map<std::string, Animation, proxy_hash, std::equal_to<void>> animations;
};

class Tower : public Entity
{
public:
	static std::array<SDL_Texture*, std::size_t(TowerType::size)> s_TowerTextures;
public:
	Tower() = delete;
	Tower(float posX, float posY, TowerType type);
	/*Tower(const Tower &r) : m_Texture(r.m_Texture), m_Pos(r.m_Pos), srcRect(r.srcRect), destRect(r.destRect),
		m_OccupiedTiles(r.m_OccupiedTiles), m_Attacker(r.m_Attacker), m_Tier(r.m_Tier), m_MaxTier(r.m_MaxTier),
		m_TowerWidth(r.m_TowerWidth), m_TowerHeight(r.m_TowerHeight), m_AnimData(r.m_AnimData), m_Type(r.m_Type) {}*/
	~Tower() = default;

	inline Tower &operator=(const Tower &r)
	{
		printf("dupa\n");
		if (this == &r)
			return *this;

		m_Texture = r.m_Texture;
		m_Pos = r.m_Pos;
		srcRect = r.srcRect;
		destRect = r.destRect;
		m_OccupiedTiles = r.m_OccupiedTiles;
		m_Attacker = r.m_Attacker;
		m_Tier = r.m_Tier;
		m_MaxTier = r.m_MaxTier;
		m_TowerWidth = r.m_TowerWidth;
		m_TowerHeight = r.m_TowerHeight;
		m_AnimData = r.m_AnimData;
		m_Type = r.m_Type;

		return *this;
	}

	virtual void Destroy() override;

	virtual void Update() override { m_Attacker->Update(); };
	virtual void Draw() override;

	virtual void AdjustToView() override;

	Vector2D GetPos() const override { return m_Pos; }
	const SDL_Rect &GetRect() const { return destRect; }

	void AssignAttacker(Attacker *attacker) { m_Attacker = attacker; }
	Attacker* GetAttacker() const { return m_Attacker; }

	// Upgrades by one tier up
	virtual void Upgrade();
	uint16_t GetTier() const { return m_Tier; }
	uint16_t CanUpgrade() const { return m_Tier < m_MaxTier; }

	// Returns specific occupied tile from array m_OccupiedTiles
	// Or returns nullptr if ID is less than 0 or greater than array size
	// The array's size is 4, because the tower occupies just 4 tiles
	Tile *GetOccupiedTile(uint16_t ID) const;

	// Returns reference to whole array m_OccupiedTiles
	std::array<Tile*, 4> &GetOccupiedTiles() { return m_OccupiedTiles; }

	void PlayAnim(std::string_view animID);

	bool IsAnimated() const { return m_AnimData.animated; }
	void UpdateAnimSpeed(std::string_view animID, int32_t newSpeed);
	int32_t GetAnimSpeed(std::string_view animID);
protected:
	int32_t m_TowerWidth = 144;
	int32_t m_TowerHeight = 64;
	SDL_Texture* m_Texture = nullptr;
	TowerType m_Type = TowerType::classic;
	Vector2D m_Pos;
	SDL_Rect srcRect{ 0, 0, 144, 64 }, destRect{ 0, 0, 48, 21 };
	std::array<Tile*, 4> m_OccupiedTiles;
	Attacker* m_Attacker = nullptr; // m_Attacker is the entity supposed to be shown on the tower
	uint16_t m_Tier = 1;
	uint16_t m_MaxTier = 3;
	TowerAnimation m_AnimData;
};