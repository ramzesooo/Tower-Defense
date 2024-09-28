#include "enemy.h"
#include "tower.h"
#include "../app.h"
#include "../level.h"

#include <cmath>

SDL_Texture* Enemy::s_Square = nullptr;
SDL_Texture* Enemy::s_GreenTex = nullptr;

Enemy::Enemy(float posX, float posY, EnemyType type, SDL_Texture* texture, uint16_t scale)
	: m_Pos(posX, posY), m_Type(type), m_Texture(texture), m_Scale(scale), m_Destination(m_Pos),
	m_ScaledPos(m_Pos.x * App::s_CurrentLevel->m_ScaledTileSize, m_Pos.y * App::s_CurrentLevel->m_ScaledTileSize),
	towers(App::s_Manager.GetGroup(EntityGroup::tower))
{
	//destRect.x = (int32_t)posX * App::s_CurrentLevel->m_ScaledTileSize;
	//destRect.y = (int32_t)posY * App::s_CurrentLevel->m_ScaledTileSize;
	destRect.w = Enemy::s_EnemyWidth * m_Scale;
	destRect.h = Enemy::s_EnemyHeight * m_Scale;

	destRect.x = static_cast<int32_t>(m_ScaledPos.x - App::s_Camera.x) - destRect.w / 8;
	destRect.y = static_cast<int32_t>(m_ScaledPos.y - App::s_Camera.y) - destRect.h / 8;

	Enemy::s_Square = App::s_Textures.GetTexture("square");
	Enemy::s_GreenTex = App::s_Textures.GetTexture("green");

	m_RectHP.squareRect.w = float(App::s_CurrentLevel->m_ScaledTileSize);
	m_RectHP.squareRect.h = float(App::s_CurrentLevel->m_ScaledTileSize) / 4.0f;

	m_OccupiedTile = App::s_CurrentLevel->GetTileFrom((uint32_t)m_Pos.x, (uint32_t)m_Pos.y);

	//Animation idle = Animation(0, 2, 500);
	animations.emplace("Idle", Animation("Idle", 0, 2, 500));

	switch (type)
	{
	case EnemyType::elf:
		{
			//Animation walk = Animation(1, 3, 100);
			animations.emplace("Walk", Animation("Walk", 1, 3, 100));

			m_HP = m_MaxHP = 200;
		}
		break;
	default:
		break;
	}

	PlayAnim("Idle");

	m_AttachedLabel = App::s_Manager.NewEntity<Label>(0, 0, "-0", App::s_Textures.GetFont("hpBar"), SDL_Color(255, 255, 255, 255), this);
	m_AttachedLabel->AddGroup(EntityGroup::label);

	m_RectHP.squareRect.x = float(destRect.x) + float(destRect.w) / 8.0f;
	m_RectHP.squareRect.y = float(destRect.y) - float(destRect.h) / 12.0f;

	/*if (m_RectHP.squareRect.x < 0.5f)
	{
		m_RectHP.squareRect.x = 0.5f;
	}

	if (m_RectHP.squareRect.y < 0.5f)
	{
		m_RectHP.squareRect.y = 0.5f;
	}*/

	m_HPPercent = float(m_HP) / float(m_MaxHP) * 100.0f;

	m_RectHP.barRect = m_RectHP.squareRect;
	m_RectHP.barRect.w = std::fabs(m_RectHP.squareRect.w / 100 * (-m_HPPercent));

	float HPBarX = m_RectHP.barRect.x + (m_RectHP.squareRect.w / 3.0f);
	m_AttachedLabel->UpdatePos(Vector2D(HPBarX, m_RectHP.barRect.y + (m_RectHP.barRect.h / 4.0f)));
	m_AttachedLabel->UpdateText(std::to_string((int32_t)m_HPPercent) + "%");

	//UpdateHealthBar();
}

void Enemy::Destroy()
{
	m_IsActive = false;

	if (m_AttachedLabel)
	{
		m_AttachedLabel->m_AttachedTo = nullptr;
		m_AttachedLabel->Destroy();
	}

	auto& attackers = App::s_Manager.GetGroup(EntityGroup::attacker);
	auto& projectiles = App::s_Manager.GetGroup(EntityGroup::projectile);

	{
		Attacker* attacker = nullptr;

		for (const auto& a : attackers)
		{
			attacker = static_cast<Attacker*>(a);

			if (attacker->GetTarget() != this)
			{
				continue;
			}

			attacker->StopAttacking();
		}
	}

	{
		Projectile* projectile = nullptr;
		for (const auto& p : projectiles)
		{
			projectile = static_cast<Projectile*>(p);

			if (projectile->GetTarget() != this)
			{
				continue;
			}

			p->Destroy();
		}
	}
}

void Enemy::Update()
{
	if (IsMoving())
		UpdateMovement();
	
	srcRect.x = srcRect.w * static_cast<int>((SDL_GetTicks() / m_CurrentAnim.speed) % m_CurrentAnim.frames);
	srcRect.y = m_CurrentAnim.index * Enemy::s_EnemyHeight;

	{
		Attacker* attacker = nullptr;
		for (const auto& tower : towers)
		{
			attacker = static_cast<Tower*>(tower)->GetAttacker();
			
			if (attacker->GetTarget() && attacker->GetTarget() != this)
			{
				continue;
			}

			if (attacker->GetTarget() == this)
			{
				if (!IsTowerInRange((Tower*)tower, App::s_TowerRange))
				{
					attacker->StopAttacking();
				}

				continue;
			}

			// if attacker doesn't have any target right now
			if (IsTowerInRange((Tower*)tower, App::s_TowerRange))
			{
				attacker->InitAttack(this);
			}

			/*if (!IsTowerInRange((Tower*)tower, App::s_TowerRange))
			{
				if (attacker->GetTarget() == this)
				{
					attacker->StopAttacking();
				}

				continue;
			}

			attacker->InitAttack(this);*/
		}
	}
}

void Enemy::Draw()
{
	TextureManager::DrawTexture(m_Texture, srcRect, destRect);
	TextureManager::DrawTextureF(Enemy::s_GreenTex, RectHP::srcRect, m_RectHP.barRect);
	TextureManager::DrawTextureF(Enemy::s_Square, RectHP::srcRect, m_RectHP.squareRect);
}

void Enemy::PlayAnim(std::string_view animID)
{
	auto it = animations.find(animID);
	if (it == animations.end())
	{
		App::s_Logger.AddLog("Couldn't find animation called " + std::string(animID));
		return;
	}

	if (m_CurrentAnim.id != it->second.id)
	{
		m_CurrentAnim = it->second;
	}
}

void Enemy::Move(Vector2D destination)
{
	if (IsMoving())
		return;

	// destination should be rounded in case it will get some digits after a comma
	// to avoid improper rendering like between of 2 tiles
	destination.Roundf();
	
	m_Destination.x = m_Pos.x + destination.x;
	m_Destination.y = m_Pos.y + destination.y;

	// block moving outside of map
	if (m_Destination.x < 0.0f)
	{
		m_Destination.x = 0.0f;
		destination.x = 0.0f;
	}
	if (m_Destination.y < 0.0f)
	{
		m_Destination.y = 0.0f;
		destination.y = 0.0f;
	}

	if (destination.x < 0.0f)
	{
		m_Velocity.x = -s_MovementSpeed;
	}
	else if (destination.x > 0.0f)
	{
		m_Velocity.x = s_MovementSpeed;
	}
	else
	{
		m_Velocity.x = 0.0f;
	}

	if (destination.y < 0.0f)
	{
		m_Velocity.y = -s_MovementSpeed;
	}
	else if (destination.y > 0.0f)
	{
		m_Velocity.y = s_MovementSpeed;
	}
	else
	{
		m_Velocity.y = 0.0f;
	}

	// TODO:
	// Make more needed animations for walking and code it right for every direction
	if (IsMoving())
	{
		PlayAnim("Walk");
	}
}

void Enemy::Move(float destinationX, float destinationY)
{
	Move(Vector2D(destinationX, destinationY));
}

void Enemy::UpdateMovement()
{
	//m_Pos += m_Velocity * App::s_ElapsedTime;
	m_Pos.x += m_Velocity.x * App::s_ElapsedTime;
	m_Pos.y += m_Velocity.y * App::s_ElapsedTime;

	if (std::fabs(m_Pos.x - m_Destination.x) < s_MovementSpeed * App::s_ElapsedTime)
	{
		m_Velocity.x = 0.0f;
	}

	if (std::fabs(m_Pos.y - m_Destination.y) < s_MovementSpeed * App::s_ElapsedTime)
	{
		m_Velocity.y = 0.0f;
	}

	if (!IsMoving())
	{
		/*PlayAnim("Idle");
		m_Pos.Roundf();
		m_Destination.Roundf();*/

		App::s_CurrentLevel->GetBase()->TakeDamage(1);
		Destroy();
	}

	Tile* newOccupiedTile = App::s_CurrentLevel->GetTileFrom((uint32_t)m_Pos.x, (uint32_t)m_Pos.y);

	if (newOccupiedTile && newOccupiedTile != m_OccupiedTile)
	{
		m_OccupiedTile->SetOccupyingEntity(nullptr);

		m_OccupiedTile = newOccupiedTile;
		newOccupiedTile->SetOccupyingEntity(this);
	}

	m_ScaledPos = Vector2D(m_Pos) * App::s_CurrentLevel->m_ScaledTileSize;

	destRect.x = static_cast<int32_t>(m_ScaledPos.x - App::s_Camera.x) - destRect.w / 8;
	destRect.y = static_cast<int32_t>(m_ScaledPos.y - App::s_Camera.y) - destRect.h / 8;

	UpdateHealthBar();
}

void Enemy::UpdateHealthBar()
{
	m_RectHP.squareRect.x = m_ScaledPos.x - App::s_Camera.x;
	m_RectHP.squareRect.y = float(destRect.y) - float(destRect.h) / 12.0f;

	m_RectHP.barRect = m_RectHP.squareRect;
	m_RectHP.barRect.w = std::fabs(m_RectHP.squareRect.w / 100 * (-m_HPPercent));

	float HPBarX = m_RectHP.barRect.x + (m_RectHP.squareRect.w / 3.0f);
	m_AttachedLabel->UpdatePos(Vector2D(HPBarX, m_RectHP.barRect.y + (m_RectHP.barRect.h / 4.0f)));
}

void Enemy::AdjustToView()
{
	destRect.x = static_cast<int32_t>(m_ScaledPos.x - App::s_Camera.x) - destRect.w / 8;
	destRect.y = static_cast<int32_t>(m_ScaledPos.y - App::s_Camera.y) - destRect.h / 8;

	UpdateHealthBar();
}

bool Enemy::IsTowerInRange(Tower* tower, uint16_t range) const
{
	if (range < 0)
	{
		range = 0;
	}

	if (range >= App::s_CurrentLevel->m_MapSizeX)
	{
		range = App::s_CurrentLevel->m_MapSizeX - 1;
	}

	int32_t posX = static_cast<int32_t>(tower->GetPos().x / App::s_CurrentLevel->m_ScaledTileSize);
	int32_t posY = static_cast<int32_t>(tower->GetPos().y / App::s_CurrentLevel->m_ScaledTileSize);
	int32_t enemyX = (int32_t)m_Pos.x;
	int32_t enemyY = (int32_t)m_Pos.y;

	int x, y;
	// Tower's position is based on left-upper tile occupied by the tower
	for (auto i = 0; i < 4; i++)
	{
		x = i % 2;
		y = i / 2;

		if ((posX + x) - range <= enemyX && (posY + y) - range <= enemyY
			&& (posX + x) + range >= enemyX && (posY + y) + range >= enemyY)
		{
			return true;
		}
	}

	return false;
}

void Enemy::AddProjectile(ProjectileType type, Attacker* projectileOwner)
{
	Projectile* projectile = App::s_Manager.NewEntity<Projectile>(type, projectileOwner, this);
	projectile->AddGroup(EntityGroup::projectile);
}

void Enemy::DelProjectile(Projectile* projectile, bool IsHit)
{
	if (IsHit)
	{
		uint16_t dmg = App::GetDamageOf(projectile->GetType());

		if (m_HP > dmg)
		{
			m_HP -= dmg;
		}
		else
		{
			m_HP = 0;
			Destroy();
		}

		m_HPPercent = float(m_HP) / float(m_MaxHP) * 100.0f;

		m_AttachedLabel->UpdateText(std::to_string((int32_t)m_HPPercent) + "%");

		UpdateHealthBar();
	}

	projectile->Destroy();
}