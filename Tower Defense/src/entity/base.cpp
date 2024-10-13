#include "base.h"
#include "../app.h"
#include "../textureManager.h"

extern std::vector<Entity*> &g_Enemies;

void Base::Draw() const
{
	if (!m_IsActive)
		return;

	TextureManager::DrawTexture(m_Texture, srcRect, destRect);
	TextureManager::DrawTextureF(App::s_GreenTex, RectHP::srcRect, m_RectHP.barRect);
	TextureManager::DrawTextureF(App::s_Square, RectHP::srcRect, m_RectHP.squareRect);
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

	m_HPPercent = float(m_HP) / float(m_MaxHP) * 100.0f;

	m_RectHP.barRect = m_RectHP.squareRect;
	m_RectHP.barRect.w = std::fabs(m_RectHP.squareRect.w / 100 * (-m_HPPercent));

	float HPBarX = m_RectHP.barRect.x + (m_RectHP.squareRect.w / 3.0f);
	m_RectHP.labelHP->UpdatePos(Vector2D(HPBarX, m_RectHP.barRect.y + (m_RectHP.barRect.h / 4.0f)));
	m_RectHP.labelHP->UpdateText(std::to_string((int32_t)m_HPPercent) + "%");

	printf("Base HP: %d\n", m_HP);
}

void Base::AdjustToView()
{
	destRect.x = static_cast<int32_t>(m_Pos.x - App::s_Camera.x);
	destRect.y = static_cast<int32_t>(m_Pos.y - App::s_Camera.y);
}