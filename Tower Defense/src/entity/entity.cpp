#include "entity.h"
#include "../app.h"

void Entity::AddToGroup(EntityGroup group)
{
	m_GroupBitSet[static_cast<std::size_t>(group)] = true;
	App::s_Manager.AddToGroup(this, group);
}

void Entity::RemoveFromGroup(EntityGroup group)
{
	//if (!m_GroupBitSet[static_cast<std::size_t>(group)])
	if (!HasGroup(group))
		return;

	std::erase(App::s_Manager.GetGroup(group), this);
	m_GroupBitSet[static_cast<std::size_t>(group)] = false;
}