#include "base.h"
#include "../app.h"
#include "../textureManager.h"

void Base::Draw() const
{
	TextureManager::DrawFullTexture(m_Texture, destRect);
}

void Base::AdjustToView()
{
	destRect.x = static_cast<int32_t>(m_Pos.x - App::s_Camera.x);
	destRect.y = static_cast<int32_t>(m_Pos.y - App::s_Camera.y);
}