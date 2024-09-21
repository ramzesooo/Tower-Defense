#pragma once
#include "../Vector2D.h"

#include "SDL.h"

#include <vector>
#include <array>
#include <memory>
#include <bitset>

enum class EntityGroup
{
	tile = 0,
	enemy,
	tower,
	attacker,
	label,
	projectile,
	size
};

class Label;

class Entity
{
public:
	Entity() = default;
	Entity(const Entity& r) : m_IsActive(r.m_IsActive), m_AttachedLabel(r.m_AttachedLabel), m_GroupBitSet(r.m_GroupBitSet) {}
	virtual ~Entity() = default;

	inline Entity& operator=(const Entity& r)
	{
		m_IsActive = r.m_IsActive;
		m_AttachedLabel = r.m_AttachedLabel;
		m_GroupBitSet = r.m_GroupBitSet;
		return *this;
	}

	virtual void Update() {}
	virtual void Draw() {}

	virtual void AdjustToView() {}

	virtual Vector2D GetPos() const { return Vector2D(0.0f, 0.0f); }

	inline virtual void Destroy() { m_IsActive = false; }
	bool IsActive() const { return m_IsActive; }

	bool HasGroup(EntityGroup group) const { return m_GroupBitSet[(std::size_t)group]; }
	void AddGroup(EntityGroup group);

	//This should be changed, because DeleteGroup should also remove the entity from specific groupedEntities' vector
	void DeleteGroup(EntityGroup group) { m_GroupBitSet[(std::size_t)group] = false; }

	void AttachLabel(Label* label) { m_AttachedLabel = label; }
	Label* GetAttachedLabel() const { return m_AttachedLabel; }

	bool m_IsActive = true;

	std::bitset<(std::size_t)EntityGroup::size> m_GroupBitSet;

	Label* m_AttachedLabel = nullptr;
};

class Manager
{
public:
	void Refresh();
	void Update();

	void AddToGroup(Entity* entity, EntityGroup group) { groupedEntities[(std::size_t)group].emplace_back(entity); }

	std::vector<Entity*>& GetGroup(EntityGroup group) { return groupedEntities[(std::size_t)group]; }

	template<class T, class... Args>
	T* NewEntity(Args&&... args)
	{
		entities.push_back(std::make_unique<T>(std::forward<Args>(args)...));
		return (T*)entities.back().get();
	}
private:
	std::vector<std::unique_ptr<Entity>> entities;
	std::array<std::vector<Entity*>, (std::size_t)EntityGroup::size> groupedEntities;
};