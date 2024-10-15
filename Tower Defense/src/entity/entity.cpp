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
	/*for (auto i(0u); i < (std::size_t)EntityGroup::size; i++)
	{
		for (auto it = groupedEntities[i].begin(); it != groupedEntities[i].end();)
		{
			if ((*it)->IsActive())
			{
				it++;
			}
			else
			{
				it = groupedEntities[i].erase(it);
			}
		}
	}*/

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

		/*
		// If it's not active
		for (auto i = 0u; i < (std::size_t)EntityGroup::size; i++)
		{
			//Check for every assigned group for the entity

			//Go to the next one if it doesn't have the group
			if (!(*it)->HasGroup((EntityGroup)i))
				continue;

			//If it has the group then just look for the specific entity in vector groupedEntities and erase it
			for (auto iter = groupedEntities[i].begin(); iter != groupedEntities[i].end(); iter++)
			{
				if ((*iter) != (*it).get())
				{
					continue;
				}

				//If found the specific entity then erase it from the vector and break the loop of specific group
				groupedEntities[i].erase(iter);
				break;
			}
		}
		*/

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