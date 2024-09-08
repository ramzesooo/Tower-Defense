#include "enemy.h"
#include "tower.h"
#include "../app.h"
#include "../level.h"


Enemy::Enemy(float posX, float posY, EnemyType type, SDL_Texture* texture, uint16_t scale)
	: m_Pos(posX, posY), m_Type(type), m_Texture(texture), m_Scale(scale), m_Destination(m_Pos), towers(App::s_Manager->GetGroup(EntityGroup::tower))
{
	destRect.x = (int32_t)posX * App::s_CurrentLevel->m_ScaledTileSize;
	destRect.y = (int32_t)posY * App::s_CurrentLevel->m_ScaledTileSize;
	destRect.w = Enemy::s_EnemyWidth * m_Scale;
	destRect.h = Enemy::s_EnemyHeight * m_Scale;

	m_OccupiedTile = App::s_CurrentLevel->GetTileFrom((uint32_t)m_Pos.x, (uint32_t)m_Pos.y);

	Animation idle = Animation(0, 2, 500);
	animations.emplace("Idle", idle);

	switch (type)
	{
	case EnemyType::elf:
		{
			Animation walk = Animation(1, 3, 100);
			animations.emplace("Walk", walk);
		}
		break;
	default:
		break;
	}

	PlayAnim("Idle");
}

void Enemy::Update()
{
	srcRect.x = srcRect.w * static_cast<int>((SDL_GetTicks() / m_AnimSpeed) % m_AnimFrames);
	srcRect.y = m_AnimIndex * Enemy::s_EnemyHeight;

	UpdateMovement();

	Tile* newOccupiedTile = App::s_CurrentLevel->GetTileFrom((uint32_t)m_Pos.x, (uint32_t)m_Pos.y);

	if (newOccupiedTile && newOccupiedTile != m_OccupiedTile)
	{
		m_OccupiedTile->SetOccupyingEntity(nullptr);

		m_OccupiedTile = newOccupiedTile;
		newOccupiedTile->SetOccupyingEntity(this);
	}

	for (const auto& tower : towers)
	{
		if (!IsTowerInRange((Tower*)tower, App::s_TowerRange))
		{
			continue;
		}

		Attacker* attacker = static_cast<Tower*>(tower)->GetAttacker();
		attacker->InitAttack(this);
	}

	destRect.x = static_cast<int32_t>(m_Pos.x * App::s_CurrentLevel->m_ScaledTileSize - App::s_Camera.x) - destRect.w / 8;
	destRect.y = static_cast<int32_t>(m_Pos.y * App::s_CurrentLevel->m_ScaledTileSize - App::s_Camera.y) - destRect.h / 8;
}

void Enemy::Draw()
{
	TextureManager::DrawTexture(m_Texture, srcRect, destRect);
}

void Enemy::PlayAnim(std::string_view animID)
{
	auto it = animations.find(animID);
	if (it == animations.end())
	{
		App::s_Logger->AddLog("Couldn't find animation called " + std::string(animID));
		return;
	}

	m_AnimFrames = it->second.frames;
	m_AnimIndex = it->second.index;
	m_AnimSpeed = it->second.speed;
}

void Enemy::Move(Vector2D destination)
{
	// Wait for current movement to finish, before making another one
	if (IsMoving())
	{
		return;
	}

	PlayAnim("Walk");

	if (destination.x < 0.0f)
	{
		m_Velocity.x = -s_MovementSpeed;
	}
	else if (destination.x > 0.0f)
	{
		m_Velocity.x = s_MovementSpeed;
	}

	if (destination.y < 0.0f)
	{
		m_Velocity.y = -s_MovementSpeed;
	}
	else if (destination.y > 0.0f)
	{
		m_Velocity.y = s_MovementSpeed;
	}

	m_Destination.x = m_Pos.x + destination.x;
	m_Destination.y = m_Pos.y + destination.y;

	if (m_Destination.x < 0.0f)
	{
		m_Destination.x = 0.0f;
		m_Velocity.x = 0.0f;
	}

	if (m_Destination.y < 0.0f)
	{
		m_Destination.y = 0.0f;
		m_Velocity.y = 0.0f;
	}
}

void Enemy::Move(float destinationX, float destinationY)
{
	Move(Vector2D(destinationX, destinationY));
}

void Enemy::UpdateMovement()
{
	// Movement should be updated only if velocity is different than 0
	if (!IsMoving())
	{
		return;
	}

	if (m_Velocity.x < 0.0f)
	{
		m_Pos.x = trunc((m_Pos.x + m_Velocity.x) * 100) / 100;

		if (m_Destination.x == m_Pos.x)
		{
			m_Velocity.x = 0.0f;
		}
	}
	else
	{
		m_Pos.x += m_Velocity.x;

		if (m_Destination.x == trunc(m_Pos.x))
		{
			m_Velocity.x = 0.0f;
		}
	}
	
	if (m_Velocity.y < 0.0f)
	{
		m_Pos.y = trunc((m_Pos.y + m_Velocity.y) * 100) / 100;

		if (m_Destination.y == m_Pos.y)
		{
			m_Velocity.y = 0.0f;
		}
	}
	else
	{
		m_Pos.y += m_Velocity.y;

		if (m_Destination.y == trunc(m_Pos.y))
		{
			m_Velocity.y = 0.0f;
		}
	}

	if (m_Destination.x < 0.0f)
	{
		m_Destination.x = 0.0f;
		m_Velocity.x = 0.0f;
	}

	if (m_Destination.y < 0.0f)
	{
		m_Destination.y = 0.0f;
		m_Velocity.y = 0.0f;
	}

	if (m_Velocity.x == 0.0f && m_Velocity.y == 0.0f)
	{
		PlayAnim("Idle");
		m_Pos.x = trunc(m_Pos.x);
		m_Pos.y = trunc(m_Pos.y);
		m_Destination.x = trunc(m_Destination.x);
		m_Destination.y = trunc(m_Destination.y);
	}
}

bool Enemy::IsTowerInRange(Tower* tower, int32_t range) const
{
	if (range < 0)
	{
		range = 0;
	}

	if (range >= App::s_CurrentLevel->m_MapSizeX)
	{
		range = App::s_CurrentLevel->m_MapSizeX - 1;
	}

	int32_t posX = static_cast<int32_t>(tower->GetPos().y / App::s_CurrentLevel->m_ScaledTileSize);
	int32_t posY = static_cast<int32_t>(tower->GetPos().x / App::s_CurrentLevel->m_ScaledTileSize);
	int32_t enemyX = (int32_t)m_Pos.x;
	int32_t enemyY = (int32_t)m_Pos.y;

	// Tower's position is based on left-upper tile occupied by the tower

	for (int y = 0; y <= 1; y++)
	{
		for (int x = 0; x <= 1; x++)
		{
			if ((posX + x) - range <= enemyX && (posY + y) - range <= enemyY
				&& (posX + x) + range >= enemyX && (posY + y) + range >= enemyY)
			{
				return true;
			}
		}
	}

	return false;
}