#include "projectile.h"
#include "attacker.h"
#include "../textureManager.h"
#include "../level.h"
#include "../app.h"

constexpr float baseVelocity = 1.0f;

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
		printf("Destroyed0\n");
		Destroy();
		return;
	}

	if (m_Pos.x == m_Destination.x && m_Pos.y == m_Destination.y)
	{
		printf("Destroyed\n");
		Destroy();
		return;
	}

	uint16_t scaledTileSize = App::s_CurrentLevel->m_ScaledTileSize;

	m_Destination.x = m_Target->GetPos().x * scaledTileSize;
	m_Destination.y = m_Target->GetPos().y * scaledTileSize - round(float(scaledTileSize / 2.0f));

	if (m_Destination.x < m_Pos.x)
	{
		m_Velocity.x = -baseVelocity;
	}
	else if (m_Destination.x > m_Pos.x)
	{
		m_Velocity.x = baseVelocity;
	}

	if (m_Destination.y < m_Pos.y)
	{
		m_Velocity.y = -baseVelocity;
	}
	else if (m_Destination.y > m_Pos.y)
	{
		m_Velocity.y = baseVelocity;
	}

	m_Pos.x += m_Velocity.x;
	m_Pos.y += m_Velocity.y;

	destRect.x = (int32_t)m_Pos.x + scaledTileSize / 2;
	destRect.y = (int32_t)m_Pos.y;
}

void Projectile::Draw()
{
	TextureManager::DrawTexture(m_Texture, srcRect, destRect);
}