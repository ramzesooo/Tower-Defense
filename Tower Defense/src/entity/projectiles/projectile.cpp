#include "projectile.h"
#include "../attackers/attacker.h"
#include "../enemy.h"
#include "../../textureManager.h"
#include "../../level.h"
#include "../../app.h"

Projectile::Projectile(ProjectileType type, Attacker *owner, Enemy *target)
	: m_Type(type), m_Owner(owner), m_Target(target),
	m_Destination(target->GetScaledPos()), 
	m_Texture(App::s_Textures.GetTextureOf(type)), m_Pos(owner->GetPos())
{}

void Projectile::Destroy()
{
	m_IsActive = false;

	if (m_Owner)
		std::erase(m_Owner->m_OwnedProjectiles, this);

	App::s_Manager.m_EntitiesToDestroy = true;
}

void Projectile::Update()
{
	if (!m_Owner || !m_Target)
	{
		Destroy();
		return;
	}
}

void Projectile::Draw()
{
	TextureManager::DrawTextureF(m_Texture, srcRect, destRect, m_Angle, SDL_FLIP_NONE);
}

void Projectile::AdjustToView()
{
	if (m_Type == ProjectileType::thunder)
	{
		destRect.x -= CameraMovement::realVelocity.x;
		destRect.y -= CameraMovement::realVelocity.y;
	}
}