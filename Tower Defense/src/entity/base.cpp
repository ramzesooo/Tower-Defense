#include "base.h"
#include "../app.h"
#include "../textureManager.h"

#include <cmath>

extern std::vector<Entity*> &g_Enemies;

void Base::Draw()
{
#ifdef DEBUG
	m_AttachedLabel->Draw();
#endif

	if (!m_IsActive)
		return;

	TextureManager::DrawTexture(m_Texture, srcRect, destRect);
}

void Base::AdjustToView()
{
	destRect.x = static_cast<int32_t>(m_Pos.x - App::s_Camera.x);
	destRect.y = static_cast<int32_t>(m_Pos.y - App::s_Camera.y);
}