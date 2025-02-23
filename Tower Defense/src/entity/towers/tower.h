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
#include <array>

struct TowerAnimation
{
	bool animated = false;
	Animation currentAnim;
	std::unordered_map<std::string, Animation, proxy_hash, std::equal_to<void>> animations;
};

class Tower : public Entity
{
public:
	static constexpr auto s_TowerTypeSize = static_cast<std::size_t>(TowerType::size);

	// [0] = Texture, [1] = Icon
	static std::array<std::array<SDL_Texture*, 2u>, s_TowerTypeSize> s_TowerTextures;
public:
	Tower() = delete;
	Tower(float posX, float posY, TowerType type, const std::array<int32_t, 2>& imageSize, uint16_t maxTier = 1);
	Tower(const Tower &) = delete;
	~Tower() = default;

	Tower &operator=(const Tower &) = delete;

	void Destroy() override;

	virtual void Update() override { m_Attacker->Update(); };
	virtual void Draw() override;
	inline void DrawHighlight() const
	{
		for (const auto &tile : m_TilesInRange)
		{
			tile->DrawHighlight();
		}
	}

	virtual void AdjustToView() override = 0;

	//Vector2D GetPos() const { return m_Pos; }
	const Vector2D &GetPos() const { return m_Pos; }
	const SDL_Rect &GetRect() const { return destRect; }

	void AssignAttacker(Attacker *attacker) { m_Attacker = attacker; }
	Attacker* GetAttacker() const { return m_Attacker; }

	// Upgrades by one tier up
	virtual void Upgrade() {};
	uint16_t GetTier() const { return m_Tier; }
	uint16_t CanUpgrade() const { return m_Tier < m_MaxTier; }

	// Returns specific occupied tile from array m_OccupiedTiles
	// Or returns nullptr if ID is less than 0 or greater than array size
	// The array's size is 4, because the tower occupies just 4 tiles
	Tile *GetOccupiedTile(uint16_t ID) const;

	// Returns reference to whole array m_OccupiedTiles
	std::array<Tile*, 4> &GetOccupiedTiles() { return m_OccupiedTiles; }
	auto &GetTilesInRange() { return m_TilesInRange; }

	void PlayAnim(std::string_view animID);

	bool IsAnimated() const { return m_AnimData.animated; }
	void UpdateAnimSpeed(std::string_view animID, int32_t newSpeed);
	int32_t GetAnimSpeed(std::string_view animID);

	void SetHighlight(bool highlight) { m_IsHighlighted = highlight; }

	uint16_t GetSellPrice() const { return m_SellPrice; }
	uint16_t GetUpgradePrice() const { return m_UpgradePrice; }
protected:
	// tower's rectangle in image
	const std::array<int32_t, 2> m_ImageSize{ 144, 64 }; // [0] = width, [1] = height
	SDL_Texture *m_Texture = nullptr;
	TowerType m_Type = TowerType::classic;
	Vector2D m_Pos{};
	SDL_Rect srcRect{ 0, 0, 144, 64 }, destRect{ 0, 0, 48, 21 };
	std::array<Tile*, 4> m_OccupiedTiles;
	Attacker *m_Attacker = nullptr; // m_Attacker is the entity supposed to be shown on the tower
	uint16_t m_Tier = 1;
	uint16_t m_MaxTier = 3;
	TowerAnimation m_AnimData;

	std::vector<Tile*> m_TilesInRange;
	bool m_IsHighlighted = false;

	uint16_t m_SellPrice = 5u;
	uint16_t m_UpgradePrice = 5u;
};