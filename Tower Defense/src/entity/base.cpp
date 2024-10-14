#include "base.h"
#include "../app.h"
#include "../textureManager.h"

#include <cmath>

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
		m_RectHP.labelHP->Destroy();
		return;
	}
	else
	{
		m_HP -= dmg;
	}

	m_HPPercent = float(m_HP) / float(m_MaxHP) * 100.0f;

	//m_RectHP.barRect = m_RectHP.squareRect;
	m_RectHP.barRect.w = std::fabs(m_RectHP.squareRect.w / 100 * (-m_HPPercent));

	m_RectHP.labelHP->UpdateText(std::to_string((int32_t)m_HPPercent) + "%");
	//float HPBarX = m_RectHP.squareRect.x + (m_RectHP.squareRect.w / 2.0f) - (float)m_RectHP.labelHP->GetRect().w / 2.0f;
	//m_RectHP.labelHP->UpdatePos(Vector2D(HPBarX, m_RectHP.barRect.y + (m_RectHP.barRect.h / 4.0f)));

#ifdef DEBUG
	App::s_Logger.AddLog("Base has taken damage, current HP: " + std::to_string(m_HP) + " (" + std::to_string((int32_t)m_HPPercent) + "%)");
#endif
}

void Base::AdjustToView()
{
	destRect.x = static_cast<int32_t>(m_Pos.x - App::s_Camera.x);
	destRect.y = static_cast<int32_t>(m_Pos.y - App::s_Camera.y);

	m_RectHP.squareRect.x = App::s_Camera.w / 3;
	m_RectHP.squareRect.y = App::s_Camera.h / 24;
	m_RectHP.squareRect.w = App::s_Camera.w / 3;
	m_RectHP.squareRect.h = App::s_Camera.h / 18;

	m_RectHP.barRect = m_RectHP.squareRect;
	m_RectHP.barRect.w = std::fabs(m_RectHP.squareRect.w / 100 * (-m_HPPercent));

	float HPBarX = m_RectHP.squareRect.x + (m_RectHP.squareRect.w / 2.0f) - (float)m_RectHP.labelHP->GetRect().w / 2.0f;
	float HPBarY = m_RectHP.squareRect.y + m_RectHP.squareRect.h / 2.0f - (float)m_RectHP.labelHP->GetRect().h / 2.0f;
	m_RectHP.labelHP->UpdatePos({ HPBarX, HPBarY });
}