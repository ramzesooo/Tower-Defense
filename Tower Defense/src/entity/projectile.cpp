#include "projectile.h"
#include "attacker.h"
#include "../textureManager.h"
#include "../level.h"
#include "../app.h"

constexpr float baseVelocity = 100.0f;

Projectile::Projectile(ProjectileType type, Attacker* owner, Enemy* target)
	: m_Type(type), m_Owner(owner), m_Target(target),
	m_Destination(target->GetPos().x * App::s_CurrentLevel->m_ScaledTileSize, target->GetPos().y * App::s_CurrentLevel->m_ScaledTileSize), 
	m_Texture(App::s_Textures->GetTexture(App::TextureOf(type))), m_Pos(owner->GetPos())
{
	destRect.x = (int32_t)m_Pos.x + App::s_CurrentLevel->m_ScaledTileSize / 2;
	destRect.y = (int32_t)m_Pos.y;
}

void Projectile::Update()
{
	if (!m_Owner || !m_Target || !m_Owner->IsActive() || !m_Target->IsActive())
	{
		Destroy();
		return;
	}

	if (m_Pos.x >= m_Destination.x - (m_Velocity.x * App::s_ElapsedTime) && m_Pos.x <= m_Destination.x + (m_Velocity.x * App::s_ElapsedTime)
		&& m_Pos.y >= m_Destination.y - (m_Velocity.y * App::s_ElapsedTime) && m_Pos.y <= m_Destination.y + (m_Velocity.y * App::s_ElapsedTime))
	{
		Destroy();
		return;
	}

	uint16_t scaledTileSize = App::s_CurrentLevel->m_ScaledTileSize;

	m_Destination.x = m_Target->GetPos().x * scaledTileSize - float(scaledTileSize) / 2.0f;
	m_Destination.y = m_Target->GetPos().y * scaledTileSize - float(scaledTileSize) / 2.0f;

	if (m_Destination.x < trunc(m_Pos.x))
	{
		m_Velocity.x = -baseVelocity;
	}
	else if (m_Destination.x > trunc(m_Pos.x))
	{
		m_Velocity.x = baseVelocity;
	}
	else
	{
		m_Velocity.x = 0.0f;
	}

	if (m_Destination.y < trunc(m_Pos.y))
	{
		m_Velocity.y = -baseVelocity;
	}
	else if (m_Destination.y > trunc(m_Pos.y))
	{
		m_Velocity.y = baseVelocity;
	}
	else
	{
		m_Velocity.y = 0.0f;
	}

	// TODO:
	// Change the way velocity and texture drawing has been done
	// Remove all those if statements and replace it with correct math
	if (m_Velocity.y == 0.0f && m_Velocity.x == 0.0f)
	{
		Destroy();
		return;
	}

	double dx = m_Destination.x - m_Pos.x;
	double dy = m_Destination.y - m_Pos.y;
	
	// If angle is equal to zero, then it supposed to be directed into right
	// std::atan2(dy, dx) returns radian
	// To convert it to degrees it has to be multiplied by 180 and divided by PI
	double angle = std::atan2(dy, dx) * 180.0f / M_PI;

	/*if (angle < 0)
	{
		angle += 360;
		m_TextureFlip = SDL_FLIP_VERTICAL;
	}
	else
	{
		m_TextureFlip = SDL_FLIP_NONE;
	}*/
	
	m_Angle = angle;

	m_Pos.x += m_Velocity.x * App::s_ElapsedTime;
	m_Pos.y += m_Velocity.y * App::s_ElapsedTime;

	destRect.x = (int32_t)m_Pos.x + scaledTileSize / 2;
	destRect.y = (int32_t)m_Pos.y;
}

void Projectile::Draw()
{
	TextureManager::DrawTexture(m_Texture, srcRect, destRect, m_Angle, m_TextureFlip);
}