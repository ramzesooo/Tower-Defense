#include "entity.h"
#include "tile.h"
#include "label.h"
#include "../app.h"

// class Entity

void Entity::AddGroup(EntityGroup group)
{
	m_GroupBitSet[(std::size_t)group] = true;
	App::s_Manager.AddToGroup(this, group);
}

// class Manager

void Manager::Refresh()
{
	for (auto it = entities.begin(); it != entities.end();)
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
			if ((*it)->HasGroup((EntityGroup)i)) {
				std::erase(groupedEntities[i], it->get());
			}
		}

		//Ready to erase the unique pointer from entities
		it = entities.erase(it);
	}
}
void Manager::Update()
{
	for (const auto &e : GetGroup(EntityGroup::enemy))
		e->Update();

	for (const auto &t : GetGroup(EntityGroup::tower))
		t->Update();

	for (const auto &a : GetGroup(EntityGroup::attacker))
		a->Update();

	for (const auto &p : GetGroup(EntityGroup::projectile))
		p->Update();

	/*for (const auto &e : entities)
		e->Update();*/
}