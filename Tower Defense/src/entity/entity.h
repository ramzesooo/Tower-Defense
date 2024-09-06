#pragma once
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
	size
};

class Entity
{
public:
	Entity() = default;

	virtual ~Entity() {}

	virtual void Update() {}
	virtual void Draw() {}

	void Destroy() { m_IsActive = false; }
	bool IsActive() const { return m_IsActive; }

	bool HasGroup(EntityGroup group) const { return m_GroupBitSet[(std::size_t)group]; }
	void AddGroup(EntityGroup group);
	void DeleteGroup(EntityGroup group) { m_GroupBitSet[(std::size_t)group] = false; }

	bool m_IsActive = true;

	std::bitset<(std::size_t)EntityGroup::size> m_GroupBitSet;
};

class Manager
{
public:
	void Refresh();
	void Update();

	void AddToGroup(Entity* entity, EntityGroup group);

	std::vector<Entity*>& GetGroup(EntityGroup group);

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