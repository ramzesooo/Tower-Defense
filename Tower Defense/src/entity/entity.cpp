#include "entity.h"
#include "../app.h"

void Entity::AddToGroup(EntityGroup group)
{
	m_GroupBitSet[(std::size_t)group] = true;
	App::s_Manager.AddToGroup(this, group);
}

void Entity::RemoveFromGroup(EntityGroup group)
{
	if (!m_GroupBitSet[(std::size_t)group])
		return;

	std::erase(App::s_Manager.GetGroup(group), this);
	m_GroupBitSet[(std::size_t)group] = false;
}