#include "base.h"
#include "../app.h"
#include "../textureManager.h"

void Base::Draw() const
{
	if (m_IsActive)
		TextureManager::DrawTexture(m_Texture, srcRect, destRect);
}

void Base::TakeDamage(uint16_t dmg)
{
	if (!m_IsActive)
		return;

	if (dmg >= m_HP)
	{
		m_HP = 0;
		m_IsActive = false;
	}
	else
	{
		m_HP -= dmg;
	}

	printf("Base HP: %d\n", m_HP);
}

void Base::AdjustToView()
{
	destRect.x = static_cast<int32_t>(m_Pos.x - App::s_Camera.x);
	destRect.y = static_cast<int32_t>(m_Pos.y - App::s_Camera.y);
}