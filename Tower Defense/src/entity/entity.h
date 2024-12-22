#pragma once
#include "../Vector2D.h"

#include <vector>
#include <array>
#include <memory>
#include <bitset>

class Tile;
class Label;

enum class EntityGroup
{
	enemy = 0,
	tower,
	attacker,
	projectile,
	size
};

class Entity
{
public:
	bool m_IsActive = true;
	std::bitset<(std::size_t)EntityGroup::size> m_GroupBitSet;
public:
	Entity() = default;
	Entity(const Entity&) = delete;
	Entity& operator=(const Entity&) = delete;
	virtual ~Entity() { m_IsActive = false; };

	virtual void Update() {}
	virtual void Draw() {}

	virtual void AdjustToView() {}

	virtual Vector2D GetPos() const { return Vector2D(0.0f, 0.0f); }

	virtual void Destroy() { m_IsActive = false; }
	bool IsActive() const { return m_IsActive; }

	bool HasGroup(EntityGroup group) const { return m_GroupBitSet[(std::size_t)group]; }
	void AddToGroup(EntityGroup group);
	void RemoveFromGroup(EntityGroup group);
};

class Manager
{
public:
	// To avoid iterating through all entities if there isn't any entity to erase
	bool m_EntitiesToDestroy = false;
public:
	//void Update();
	inline void Update()
	{
		for (const auto &e : m_GroupedEntities.at((std::size_t)EntityGroup::enemy))
		{
			if (!e->IsActive())
				continue;

			e->Update();
		}

		for (const auto &t : m_GroupedEntities.at((std::size_t)EntityGroup::tower))
		{
			t->Update();
		}

		for (const auto &a : m_GroupedEntities.at((std::size_t)EntityGroup::attacker))
		{
			a->Update();
		}

		for (const auto &p : m_GroupedEntities.at((std::size_t)EntityGroup::projectile))
		{
			p->Update();
		}

		/*for (const auto &e : entities)
			e->Update();*/
	}

	//void Refresh();
	inline void Refresh()
	{
		if (!m_EntitiesToDestroy)
			return;

		for (auto it = m_Entities.begin(); it != m_Entities.end();)
		{
			// Check for every entity activity
			if ((*it)->IsActive())
			{
				// If it's active, then do nothing and just go to the next one
				it++;
				continue;
			}

			// Erase it from specific group if it's there (groupedEntities is an array of groups' vectors)
			for (std::size_t i = 0; i < (std::size_t)EntityGroup::size; ++i)
			{
				/*if ((*it)->HasGroup((EntityGroup)i))
				{
					std::erase(m_GroupedEntities[i], it->get());
				}*/
				(*it)->RemoveFromGroup((EntityGroup)i);
			}

			//Ready to erase the unique pointer from entities
			it = m_Entities.erase(it);
		}

		m_Entities.shrink_to_fit();
		m_EntitiesToDestroy = false;
	}

	void AddToGroup(Entity* entity, EntityGroup group) { m_GroupedEntities[(std::size_t)group].emplace_back(entity); }

	std::vector<Entity*>& GetGroup(EntityGroup group) { return m_GroupedEntities[(std::size_t)group]; }

	template<class T, class... Args>
	inline T *NewEntity(Args&&... args)
	{
		m_Entities.emplace_back(std::move(std::make_unique<T>(std::forward<Args>(args)...)));
		return (T*)m_Entities.back().get();
	}

	template<class... Args>
	inline Tile *NewTile(Args&&... args)
	{
		m_Tiles.emplace_back(std::move(std::make_unique<Tile>(std::forward<Args>(args)...)));
		return m_Tiles.back().get();
	}
	
	template<class... Args>
	inline Label *NewLabel(Args&&... args)
	{
		m_Labels.emplace_back(std::move(std::make_unique<Label>(std::forward<Args>(args)...)));
		return m_Labels.back().get();
	}

	inline void DestroyLabel(Label *label)
	{
		if (!label)
			return;

		for (auto it = m_Labels.begin(); it != m_Labels.end(); ++it)
		{
			if ((*it).get() == label)
			{
				m_Labels.erase(it);
				return;
			}
		}
	}

	inline void DestroyAllEntities()
	{ 
		for (const auto &e : m_Entities)
		{
			e->Destroy();
		}

		/*for (std::size_t i = 0; i < (std::size_t)EntityGroup::size; ++i)
		{
			m_GroupedEntities[i].clear();
		}

		m_Entities.clear();
		m_EntitiesToDestroy = false;*/
	}

	inline void DestroyAllTiles()
	{
		m_Tiles.clear();
	}
private:
	std::vector<std::unique_ptr<Tile>> m_Tiles;
	std::vector<std::unique_ptr<Label>> m_Labels;
	std::vector<std::unique_ptr<Entity>> m_Entities;
	std::array<std::vector<Entity*>, (std::size_t)EntityGroup::size> m_GroupedEntities;
};