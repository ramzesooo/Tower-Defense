#include "entity.h"
#include "../app.h"

// class Entity

void Entity::AddGroup(EntityGroup group)
{
	m_GroupBitSet[(std::size_t)group] = true;
	App::s_Manager->AddToGroup(this, group);
}

// class Manager

void Manager::Refresh()
{
	for (auto i(0u); i < (std::size_t)EntityGroup::size; i++)
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
	}

	for (auto it = entities.begin(); it != entities.end();)
	{
		if ((*it)->IsActive())
		{
			it++;
		}
		else
		{
			it = entities.erase(it);
		}
	}
}

void Manager::Update()
{
	for (const auto& e : entities)
	{
		e->Update();
	}
}

void Manager::AddToGroup(Entity* entity, EntityGroup group)
{
	groupedEntities[(std::size_t)group].emplace_back(entity);
}

std::vector<Entity*>& Manager::GetGroup(EntityGroup group)
{
	return groupedEntities[(std::size_t)group];
}