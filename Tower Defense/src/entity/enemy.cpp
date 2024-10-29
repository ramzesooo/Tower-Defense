#include "enemy.h"
#include "tower.h"
#include "../app.h"
#include "../level.h"

#include <cmath>

extern std::vector<Entity*> &g_Towers;
#ifdef DEBUG
extern std::vector<Entity*> &g_Enemies;
#endif

SDL_Texture *Enemy::s_ArrowTexture = nullptr;

Enemy::Enemy(float posX, float posY, EnemyType type, SDL_Texture* texture, uint16_t scale)
	: m_Pos(posX, posY), m_Type(type), m_Texture(texture), m_Scale(scale), m_Destination(m_Pos),
	m_ScaledPos(m_Pos.x * App::s_CurrentLevel->m_ScaledTileSize, m_Pos.y * App::s_CurrentLevel->m_ScaledTileSize)
{
	destRect.w = Enemy::s_EnemyWidth * m_Scale;
	destRect.h = Enemy::s_EnemyHeight * m_Scale;

	destRect.x = static_cast<int32_t>(m_ScaledPos.x - App::s_Camera.x) - destRect.w / 8;
	destRect.y = static_cast<int32_t>(m_ScaledPos.y - App::s_Camera.y) - destRect.h / 8;

	m_RectHP.squareRect.w = float(App::s_CurrentLevel->m_ScaledTileSize);
	m_RectHP.squareRect.h = float(App::s_CurrentLevel->m_ScaledTileSize) / 4.0f;

	m_OccupiedTile = App::s_CurrentLevel->GetTileFrom((uint32_t)m_Pos.x, (uint32_t)m_Pos.y);

	animations.emplace("Idle", Animation("Idle", 0, 2, 500));
	animations.emplace("Walk", Animation("Walk", 1, 3, 100)); // walk down
	animations.emplace("WalkRight", Animation("WalkRight", 2, 3, 100));
	animations.emplace("WalkUp", Animation("WalkUp", 3, 3, 100));
	animations.emplace("WalkLeft", Animation("WalkLeft", 4, 3, 100));

	switch (type)
	{
	case EnemyType::elf:
		m_HP = m_MaxHP = 50;
		m_MovementSpeed = 1.5f;
		break;
	case EnemyType::goblinWarrior:
		m_HP = m_MaxHP = 85;
		m_MovementSpeed = 1.35f;
		break;
	case EnemyType::dwarfSoldier:
		m_HP = m_MaxHP = 120;
		m_MovementSpeed = 1.3f;
		break;
	case EnemyType::dwarfKing:
		m_HP = m_MaxHP = 130;
		m_MovementSpeed = 1.4f;
		break;
	}

	PlayAnim("Idle");

	m_RectHP.labelHP = App::s_Manager.NewLabel(0, 0, "-0", App::s_Textures.GetFont("enemyHealth"), SDL_Color(255, 255, 255, 255), this);

	m_RectHP.squareRect.x = float(destRect.x) + float(destRect.w) / 8.0f;
	m_RectHP.squareRect.y = float(destRect.y) - float(destRect.h) / 12.0f;

	m_HPPercent = float(m_HP) / float(m_MaxHP) * 100.0f;

	m_RectHP.barRect = m_RectHP.squareRect;
	m_RectHP.barRect.w = std::fabs(m_RectHP.squareRect.w / 100 * (-m_HPPercent));

	float HPBarX = m_RectHP.barRect.x + (m_RectHP.squareRect.w / 3.0f);
	m_RectHP.labelHP->UpdatePos(Vector2D(HPBarX, m_RectHP.barRect.y + (m_RectHP.barRect.h / 4.0f)));
	m_RectHP.labelHP->UpdateText(std::to_string((int32_t)m_HPPercent) + "%");
}

Enemy::~Enemy()
{
	if (m_RectHP.labelHP)
	{
		m_RectHP.labelHP->m_AttachedTo = nullptr;
		m_RectHP.labelHP->Destroy();
	}
}

void Enemy::Destroy()
{
	m_IsActive = false;

	if (m_OccupiedTile)
	{
		m_OccupiedTile->SetOccupyingEntity(nullptr);
		m_OccupiedTile = nullptr;
	}

	for (const auto &a : m_Attackers)
	{
		a->StopAttacking(false);
	}

	for (const auto &p : App::s_Manager.GetGroup(EntityGroup::projectile))
	{
		if (static_cast<Projectile*>(p)->GetTarget() == this)
			static_cast<Projectile*>(p)->SetTarget(nullptr);
	}

#ifdef DEBUG
	App::s_EnemiesAmountLabel->UpdateText("Enemies: " + std::to_string(g_Enemies.size() - 1));
#endif
}

void Enemy::Update()
{
	UpdateMovement();

	srcRect.x = srcRect.w * static_cast<int>((SDL_GetTicks() / m_CurrentAnim.speed) % m_CurrentAnim.frames);
	srcRect.y = m_CurrentAnim.index * Enemy::s_EnemyHeight;

	Attacker* attacker = nullptr;
	for (const auto &tower : g_Towers)
	{
		attacker = static_cast<Tower*>(tower)->GetAttacker();

		if (!attacker)
			continue;

		if (attacker->GetTarget() == this)
		{
			if (!IsTowerInRange((Tower*)tower, App::s_TowerRange))
				attacker->StopAttacking();
		}
		else if (!attacker->IsAttacking() && m_IsActive && IsTowerInRange((Tower*)tower, App::s_TowerRange))
		{
			attacker->InitAttack(this);
		}
	}
}

void Enemy::Draw()
{
	// If enemy should be drawn because its position is in camera
	if (destRect.x + destRect.w > 0 && destRect.x < App::s_Camera.w
		&& destRect.y + destRect.h > 0 && destRect.y < App::s_Camera.h)
	{
		TextureManager::DrawTexture(m_Texture, srcRect, destRect);

		TextureManager::DrawTextureF(App::s_GreenTex, RectHP::srcRect, m_RectHP.barRect);
		TextureManager::DrawTextureF(App::s_Square, RectHP::srcRect, m_RectHP.squareRect);
		m_RectHP.labelHP->Draw();
		return;
	}

	double angle = 0;

	// If enemy shouldn't be drawn, then draw the arrow pointing to the enemy
	if (destRect.x + destRect.w < 0)
	{
		m_PointingArrowDest.x = 0;

		angle = 270;
	}
	else if (destRect.x > App::s_Camera.w)
	{
		m_PointingArrowDest.x = (int32_t)App::s_Camera.w - m_PointingArrowDest.w;

		angle = 90;
	}
	else
	{
		m_PointingArrowDest.x = destRect.x;
	}

	if (destRect.y + destRect.h < 0)
	{
		m_PointingArrowDest.y = 0;

		angle = 0;
	}
	else if (destRect.y > App::s_Camera.h)
	{
		m_PointingArrowDest.y = (int32_t)App::s_Camera.h - m_PointingArrowDest.h;

		angle = 180;
	}
	else
	{
		m_PointingArrowDest.y = destRect.y;
	}

	SDL_RenderCopyEx(App::s_Renderer, Enemy::s_ArrowTexture, nullptr, &m_PointingArrowDest, angle, NULL, SDL_FLIP_NONE);
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
	
	m_Destination = Vector2D(m_Pos).Add(destination);
	/*m_Destination.x = m_Pos.x + destination.x;
	m_Destination.y = m_Pos.y + destination.y;*/

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
		m_Velocity.x = -m_MovementSpeed;
	}
	else if (destination.x > 0.0f)
	{
		m_Velocity.x = m_MovementSpeed;
	}
	else
	{
		m_Velocity.x = 0.0f;
	}

	if (destination.y < 0.0f)
	{
		m_Velocity.y = -m_MovementSpeed;
	}
	else if (destination.y > 0.0f)
	{
		m_Velocity.y = m_MovementSpeed;
	}
	else
	{
		m_Velocity.y = 0.0f;
	}
}

void Enemy::Move(float destinationX, float destinationY)
{
	Move(Vector2D(destinationX, destinationY));
}

void Enemy::UpdateMovement()
{
	Tile *nextTile = App::s_CurrentLevel->GetTileFrom(uint32_t(m_Pos.x + (m_Velocity.x * App::s_ElapsedTime)), uint32_t(m_Pos.y + (m_Velocity.y * App::s_ElapsedTime)));

	if (!nextTile)
	{
		App::s_Logger.AddLog("Enemy tried to walk into a non-exising tile, enemy has been destroyed!\n");
		Destroy();
		return;
	}

	if (nextTile != m_OccupiedTile)
	{
		if (nextTile->GetOccupyingEntity() && nextTile->GetOccupyingEntity() != this)
			return;

		m_OccupiedTile->SetOccupyingEntity(nullptr);
		m_OccupiedTile = nextTile;
		m_OccupiedTile->SetOccupyingEntity(this);
	}

	m_Pos += Vector2D(m_Velocity) * App::s_ElapsedTime;

	if (std::fabs(m_Pos.x - m_Destination.x) < m_MovementSpeed * App::s_ElapsedTime)
		m_Velocity.x = 0.0f;

	if (std::fabs(m_Pos.y - m_Destination.y) < m_MovementSpeed * App::s_ElapsedTime)
		m_Velocity.y = 0.0f;

	// The direction of walk animation doesn't really matter in the game, so it can be done in the easiest possible way
	if (m_Velocity.x > 0)
		PlayAnim("WalkRight");
	else if (m_Velocity.x < 0)
		PlayAnim("WalkLeft");
	else if (m_Velocity.y > 0)
		PlayAnim("Walk");
	else if (m_Velocity.y < 0)
		PlayAnim("WalkUp");

	m_ScaledPos = Vector2D(m_Pos) * App::s_CurrentLevel->m_ScaledTileSize;

	destRect.x = static_cast<int32_t>(m_ScaledPos.x - App::s_Camera.x) - destRect.w / 8;
	destRect.y = static_cast<int32_t>(m_ScaledPos.y - App::s_Camera.y) - destRect.h / 8;

	const SDL_Rect &rectOfBase = App::s_CurrentLevel->GetBase()->GetRect();
	if (destRect.x + destRect.w / 2 >= rectOfBase.x && destRect.x - destRect.w / 2 <= rectOfBase.x
		&& destRect.y + destRect.h / 2 >= rectOfBase.y && destRect.y - destRect.h / 2 <= rectOfBase.y)
	{
		App::TakeLifes();
		Destroy();
		return;
	}

	UpdateHealthBar();
}

void Enemy::UpdateHealthBar()
{
	static float onePercent = m_RectHP.squareRect.w / 100; // references to width of 1% hp

	m_RectHP.squareRect.x = m_ScaledPos.x - App::s_Camera.x;
	m_RectHP.squareRect.y = float(destRect.y) - float(destRect.h) / 12.0f;

	m_RectHP.barRect.x = m_RectHP.squareRect.x;
	m_RectHP.barRect.y = m_RectHP.squareRect.y;
	m_RectHP.barRect.w = std::fabs(onePercent * (-m_HPPercent));

	m_RectHP.labelHP->UpdatePos(int32_t(m_RectHP.barRect.x + (m_RectHP.squareRect.w / 3.0f)), int32_t(m_RectHP.barRect.y + (m_RectHP.barRect.h / 4.0f)));
}

void Enemy::AdjustToView()
{
	m_ScaledPos = Vector2D(m_Pos) * App::s_CurrentLevel->m_ScaledTileSize;

	destRect.x = static_cast<int32_t>(m_ScaledPos.x - App::s_Camera.x) - destRect.w / 8;
	destRect.y = static_cast<int32_t>(m_ScaledPos.y - App::s_Camera.y) - destRect.h / 8;

	UpdateHealthBar();
}

void Enemy::OnHit(Projectile* projectile, uint16_t dmg)
{
	if (m_HP > dmg)
	{
		m_HP -= dmg;
	}
	else
	{
		m_HP = 0;
		Destroy();
		return;
	}

	m_HPPercent = float(m_HP) / float(m_MaxHP) * 100.0f;
	
	m_RectHP.labelHP->UpdateText(std::to_string((int32_t)m_HPPercent) + "%");

	UpdateHealthBar();

	projectile->Destroy();
}

bool Enemy::IsTowerInRange(Tower* tower, uint16_t range) const
{
	int32_t posX = static_cast<int32_t>(tower->GetPos().x / App::s_CurrentLevel->m_ScaledTileSize);
	int32_t posY = static_cast<int32_t>(tower->GetPos().y / App::s_CurrentLevel->m_ScaledTileSize);
	int32_t enemyX = (int32_t)m_Pos.x;
	int32_t enemyY = (int32_t)m_Pos.y;

	// Tower's position is based on left-upper tile occupied by the tower
	for (auto i = 0; i < 4; i++)
	{
		auto x = i % 2;
		auto y = i / 2;

		if ((posX + x) - range <= enemyX && (posY + y) - range <= enemyY
			&& (posX + x) + range >= enemyX && (posY + y) + range >= enemyY)
		{
			return true;
		}
	}

	return false;
}