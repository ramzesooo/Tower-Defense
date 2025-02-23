#include "enemy.h"
#include "attackers/attacker.h"
#include "towers/tower.h"
#include "../app.h"
#include "../level.h"

#include <cmath>
#include <format>

extern uint32_t g_PausedTicks;

extern std::vector<Entity*> &g_Projectiles;
extern std::vector<Entity*> &g_Towers;
IF_DEBUG(extern std::vector<Entity*> &g_Enemies;);

SDL_Texture *Enemy::s_ArrowTexture = nullptr;

DamageInfo::DamageInfo(const SDL_Rect& enemyHP, uint32_t dmg, uint32_t currentTicks)
	: updateTicks(currentTicks), lifespanTicks(currentTicks + DamageInfo::lifespan)
{
	static TTF_Font* defaultFont = App::s_Textures.GetFont("default");
	static constexpr SDL_Color takenDamageColor{ 255, 50, 50, 255 };

	this->label = Label(static_cast<int32_t>(enemyHP.x), static_cast<int32_t>(enemyHP.y), std::format("-{}", dmg), defaultFont, takenDamageColor, nullptr, true);
	const SDL_Rect& rect = this->label.GetRect();
	this->label.UpdatePos(rect.x - rect.w / 2, rect.y);
}

Enemy::Enemy(float posX, float posY, EnemyType type, uint16_t scale)
	: m_Pos(posX, posY), m_Type(type), m_Texture(App::s_Textures.GetTextureOf(type)), m_Scale(scale), m_Destination(m_Pos),
	m_ScaledPos(m_Pos * App::s_CurrentLevel->m_ScaledTileSize)
	IF_DEBUG(, m_Speedy(App::s_Speedy))
{
	static TTF_Font *healthFont = App::s_Textures.GetFont("enemyHealth");

	destRect.w = Enemy::s_EnemyWidth * m_Scale;
	destRect.h = Enemy::s_EnemyHeight * m_Scale;

	destRect.x = static_cast<int32_t>(m_ScaledPos.x - App::s_Camera.x) - destRect.w / 8;
	destRect.y = static_cast<int32_t>(m_ScaledPos.y - App::s_Camera.y) - destRect.h / 8;

	m_RectHP.squareRect.x = static_cast<float>(destRect.x) + static_cast<float>(destRect.w) / 8.0f;
	m_RectHP.squareRect.y = static_cast<float>(destRect.y) - static_cast<float>(destRect.h) / 12.0f;

	m_RectHP.squareRect.w = static_cast<float>(App::s_CurrentLevel->m_ScaledTileSize);
	m_RectHP.squareRect.h = static_cast<float>(App::s_CurrentLevel->m_ScaledTileSize) / 4.0f;

	m_RectHP.onePercent = m_RectHP.squareRect.w / 100.0f;

	m_OccupiedTile = App::s_CurrentLevel->GetTileFrom(m_Pos.x, m_Pos.y);

	animations.emplace("Idle", Animation("Idle", 0, 2, 500));
	animations.emplace("Walk", Animation("Walk", 1, 3, 100)); // walk down
	animations.emplace("WalkRight", Animation("WalkRight", 2, 3, 100));
	animations.emplace("WalkUp", Animation("WalkUp", 3, 3, 100));
	animations.emplace("WalkLeft", Animation("WalkLeft", 4, 3, 100));

	switch (type)
	{
	case EnemyType::elf:
		m_HP = m_MaxHP = 75;
		m_MovementSpeed = 1.5f;
		m_Coins = 1;
		break;
	case EnemyType::goblinWarrior:
		m_HP = m_MaxHP = 90;
		m_MovementSpeed = 1.35f;
		m_Coins = 1;
		break;
	case EnemyType::dwarfSoldier:
		m_HP = m_MaxHP = 120;
		m_MovementSpeed = 1.3f;
		m_Coins = 2;
		break;
	case EnemyType::dwarfKing:
		m_HP = m_MaxHP = 130;
		m_MovementSpeed = 1.4f;
		m_Coins = 4;
		break;
	}

	m_MovementSpeed *= App::s_CurrentLevel->m_MovementSpeedRate;
	IF_DEBUG(m_MovementDebugSpeed = m_MovementSpeed;);

	IF_DEBUG(
		if (m_Speedy == EnemyDebugSpeed::faster)
			m_MovementSpeed *= 2.0f;
	);

	PlayAnim("Idle");

	m_RectHP.labelHP = Label(0, 0, "-0", healthFont, SDL_Color(255, 255, 255, 255), this, true);

	m_HPPercent = std::ceilf(static_cast<float>(m_HP) / static_cast<float>(m_MaxHP) * 100.0f);

	m_RectHP.barRect = m_RectHP.squareRect;
	m_RectHP.barRect.w = std::fabsf(m_RectHP.squareRect.w / 100.0f * (-m_HPPercent));

	m_RectHP.labelHP.UpdatePos(
		{
			m_RectHP.barRect.x + (m_RectHP.squareRect.w / 3.0f),
			m_RectHP.barRect.y + (m_RectHP.squareRect.h / 6.0f)
		}
	);
	m_RectHP.labelHP.UpdateText(std::format("{}%", m_HPPercent));
}

Enemy::~Enemy()
{
	m_RectHP.labelHP.m_AttachedTo = nullptr;
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

	for (const auto &p : g_Projectiles)
	{
		if (dynamic_cast<Projectile*>(p)->GetTarget() == this)
			dynamic_cast<Projectile*>(p)->SetTarget(nullptr);
	}

	IF_DEBUG(
		App::s_EnemiesAmountLabel.UpdateText(std::format("Enemies: {}", g_Enemies.size() - 1));
	);

	App::s_Manager.m_EntitiesToDestroy = true;
}

void Enemy::Update()
{
#ifdef DEBUG
	if (m_Speedy != EnemyDebugSpeed::stay)
	{
		// Check if the enemy has stopped
		if (m_Velocity.IsEqualZero())
		{
			// Check if the enemy has already done its last step (has reached destination) and destroy it if true
			if (m_MoveCount == m_Path.size())
			{
				Destroy();
				App::TakeLifes();
				return;
			}

			// Look for next step to destination if not reached
			Move();
			m_MoveCount++;
		}

		UpdateMovement();
	}
#else
	// Check if the enemy has stopped
	if (m_Velocity.IsEqualZero())
	{
		// Check if the enemy has already done its last step (has reached destination) and destroy it if true
		if (m_MoveCount == m_Path.size())
		{
			Destroy();
			App::TakeLifes();
			return;
		}

		// Look for next step to destination if not reached
		Move();
		m_MoveCount++;
	}

	UpdateMovement();
#endif

	// Animation
	srcRect.x = srcRect.w * static_cast<int32_t>((SDL_GetTicks() / m_CurrentAnim.speed) % m_CurrentAnim.frames);
	srcRect.y = m_CurrentAnim.index * Enemy::s_EnemyHeight;

	bool erasedDmgInfo = false;

	// Iterate through all info about damage associated with the enemy
	for (auto it = m_TakenDamages.begin(); it != m_TakenDamages.end();)
	{
		uint32_t currentTicks = SDL_GetTicks() - g_PausedTicks; // g_PausedTicks referees to spent ticks on paused state

		// Check if the damage info should be already deleted
		if (SDL_TICKS_PASSED(currentTicks, (*it).lifespanTicks))
		{
			// Erase from vector and skip the rest
			it = m_TakenDamages.erase(it);
			erasedDmgInfo = true;
			continue;
		}

		// Check if height of the info should be already decreased (should go more to top)
		if (SDL_TICKS_PASSED(currentTicks, (*it).updateTicks + DamageInfo::updatePosTime))
		{
			(*it).updatedPosY--;
			(*it).updateTicks = currentTicks;
		}

		const SDL_Rect &rect = m_RectHP.labelHP.GetRect();
		(*it).label.UpdatePos(rect.x - rect.w / 2, rect.y + (*it).updatedPosY);

		it++;
	}

	if (erasedDmgInfo)
	{
		m_TakenDamages.shrink_to_fit();
	}
}

void Enemy::Draw()
{
	// If enemy should be drawn because its position is in camera
	if (destRect.x + destRect.w > 0 && destRect.x < App::s_Camera.w
		&& destRect.y + destRect.h > 0 && destRect.y < App::s_Camera.h)
	{
		TextureManager::DrawTexture(m_Texture, srcRect, destRect);

		TextureManager::DrawFullTextureF(App::s_GreenTex, m_RectHP.barRect);
		TextureManager::DrawFullTextureF(App::s_Square, m_RectHP.squareRect);
		m_RectHP.labelHP.Draw();
		
		for (auto &dmg : m_TakenDamages)
		{
			dmg.label.Draw();
		}

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
		m_PointingArrowDest.x = static_cast<int32_t>(App::s_Camera.w) - m_PointingArrowDest.w;

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
		m_PointingArrowDest.y = static_cast<int32_t>(App::s_Camera.h) - m_PointingArrowDest.h;

		angle = 180;
	}
	else
	{
		m_PointingArrowDest.y = destRect.y;
	}

	TextureManager::DrawFullTexture(Enemy::s_ArrowTexture, m_PointingArrowDest, angle);
	//SDL_RenderCopyEx(App::s_Renderer, Enemy::s_ArrowTexture, nullptr, &m_PointingArrowDest, angle, NULL, SDL_FLIP_NONE);
}

void Enemy::PlayAnim(std::string_view animID)
{
	auto it = animations.find(animID);
	if (it == animations.end())
	{
		App::s_Logger.AddLog(std::string_view("Couldn't find animation called "));
		App::s_Logger.AddLog(animID);
		return;
	}

	m_CurrentAnim = it->second;
}

void Enemy::UpdateMovement()
{
	Tile *nextTile = App::s_CurrentLevel->GetTileFrom(uint32_t(m_Pos.x + (m_Velocity.x * App::s_ElapsedTime)), uint32_t(m_Pos.y + (m_Velocity.y * App::s_ElapsedTime)));

	// Check if the enemy is about to reach next tile
	if (nextTile != m_OccupiedTile)
	{
		// Check if the next tile doesn't exist
		if (!nextTile)
		{
			App::s_Logger.AddLog(std::string_view("Enemy tried to walk into a non-existing tile, enemy has been destroyed!"));
			Destroy();
			return;
		}

		// TODO: Maybe enemies' path should be reseted every time they are walking in the same tile at the same time with another one
		// This way, attackers could also check for enemies on specific tiles in their range what might have great impact on performance

		Enemy *occupyingEnemy = dynamic_cast<Enemy*>(nextTile->GetOccupyingEntity());
		if (occupyingEnemy && occupyingEnemy != this)
		{
			Vector2D scaledPos = occupyingEnemy->GetScaledPos();
			Vector2D tilePos = nextTile->GetPos();
			if (scaledPos.x >= tilePos.x && scaledPos.x <= tilePos.x + nextTile->GetWidth()
				&& scaledPos.y >= tilePos.y && scaledPos.y <= tilePos.y + nextTile->GetHeight())
			{
				nextTile->SetOccupyingEntity(nullptr);
			}
			else
			{
				return;
			}
		}

		m_OccupiedTile->SetOccupyingEntity(nullptr);
		m_OccupiedTile = nextTile;
		m_OccupiedTile->SetOccupyingEntity(this);

		// NOTE: It should work fine without ValidAttacker(), but also ValidAttacker()
		// is less expensive than ValidTarget() for attackers
		// in case of a lot of enemies attacking
		// but as well, the game is more likely designed for small amount of enemies per wave
		
		// Probably it's not needed anymore since attackers check by themselves if they should switch the target
		//ValidAttacker();
	}

	// Updated position and scale it to tiles' size
	m_Pos += m_Velocity * App::s_ElapsedTime;
	m_ScaledPos = m_Pos * App::s_CurrentLevel->m_ScaledTileSize;

	destRect.x = static_cast<int32_t>(m_ScaledPos.x - App::s_Camera.x) - destRect.w / 8;
	destRect.y = static_cast<int32_t>(m_ScaledPos.y - App::s_Camera.y) - destRect.h / 8;

	// Check if distance between current position and next step's (tile's) position is less than enemy's movement speed
	// Basically just predict if in next frame the position will be equal to destination (next tile)
	if (std::fabsf(m_Pos.x - m_Destination.x) < m_MovementSpeed * App::s_ElapsedTime)
		m_Velocity.x = 0.0f;
	if (std::fabsf(m_Pos.y - m_Destination.y) < m_MovementSpeed * App::s_ElapsedTime)
		m_Velocity.y = 0.0f;

	if (m_Velocity.x > 0.0f)
		PlayAnim("WalkRight");
	else if (m_Velocity.x < 0.0f)
		PlayAnim("WalkLeft");
	else if (m_Velocity.y > 0.0f)
		PlayAnim("Walk");
	else if (m_Velocity.y < 0.0f)
		PlayAnim("WalkUp");

	UpdateHealthBar();
}

void Enemy::Move()
{
	// Assign position of next tile to m_Destination to let enemy reach it
	m_Destination = m_Path[m_MoveCount];

	// Check the direction
	if (m_Destination.x > m_Pos.x)
	{
		m_Velocity.x = m_MovementSpeed;
	}
	else if (m_Destination.x < m_Pos.x)
	{
		m_Velocity.x = -m_MovementSpeed;
	}
	else
	{
		m_Velocity.x = 0.0f;
	}

	if (m_Destination.y > m_Pos.y)
	{
		m_Velocity.y = m_MovementSpeed;
	}
	else if (m_Destination.y < m_Pos.y)
	{
		m_Velocity.y = -m_MovementSpeed;
	}
	else
	{
		m_Velocity.y = 0.0f;
	}
}

void Enemy::UpdateHealthBar()
{
	m_RectHP.squareRect.x = m_RectHP.barRect.x = m_ScaledPos.x - App::s_Camera.x;
	m_RectHP.squareRect.y = m_RectHP.barRect.y = static_cast<float>(destRect.y) - static_cast<float>(destRect.h) / 12.0f;

	//m_RectHP.barRect.x = m_RectHP.squareRect.x;
	//m_RectHP.barRect.y = m_RectHP.squareRect.y;
	m_RectHP.barRect.w = std::fabsf(m_RectHP.onePercent * (-m_HPPercent));
	
	m_RectHP.labelHP.UpdatePos(
		m_RectHP.barRect.x + (m_RectHP.squareRect.w / 3.0f),
		m_RectHP.barRect.y + (m_RectHP.squareRect.h / 6.0f)
	);
	
}

void Enemy::AdjustToView()
{
	m_ScaledPos = m_Pos * App::s_CurrentLevel->m_ScaledTileSize;

	destRect.x = static_cast<int32_t>(m_ScaledPos.x - App::s_Camera.x) - destRect.w / 8;
	destRect.y = static_cast<int32_t>(m_ScaledPos.y - App::s_Camera.y) - destRect.h / 8;

	UpdateHealthBar();
}

void Enemy::OnHit(uint16_t dmg)
{
	//static TTF_Font* defaultFont = App::s_Textures.GetFont("default");
	//static constexpr SDL_Color takenDamageColor{ 255, 50, 50, 255 };

	if (m_HP <= dmg)
	{
		App::Instance().AddCoins(m_Coins);

		m_HP = 0u;
		Destroy();
		return;
	}

	m_HP -= dmg;

	/*
	const SDL_Rect &labelRect = m_RectHP.labelHP.GetRect();

	DamageInfo newTakenDamage;

	newTakenDamage.label = Label(static_cast<int32_t>(labelRect.x), static_cast<int32_t>(labelRect.y), std::format("-{}", dmg), defaultFont, takenDamageColor);
	const SDL_Rect &rect = newTakenDamage.label.GetRect();
	newTakenDamage.label.UpdatePos(rect.x - rect.w / 2, rect.y);

	newTakenDamage.updateTicks = SDL_GetTicks() - g_PausedTicks;
	newTakenDamage.lifespanTicks = newTakenDamage.updateTicks + newTakenDamage.lifespan;

	m_TakenDamages.emplace_back(newTakenDamage);
	*/

	m_TakenDamages.reserve(m_TakenDamages.size() + 1);
	m_TakenDamages.emplace_back(m_RectHP.labelHP.GetRect(), dmg, SDL_GetTicks() - g_PausedTicks);

	m_HPPercent = std::ceilf(static_cast<float>(m_HP) / static_cast<float>(m_MaxHP) * 100.0f);
	
	m_RectHP.labelHP.UpdateText(std::format("{}%", m_HPPercent));

	UpdateHealthBar();
}

/*
void Enemy::ValidAttacker()
{
	if (!m_IsActive)
		return;

	Attacker *attacker = nullptr;
	Tower *tower = nullptr;
	for (const auto &t : g_Towers)
	{
		tower = dynamic_cast<Tower*>(t);
		attacker = tower->GetAttacker();

		if (!attacker)
			continue;

		if (attacker->GetTarget() == this && !IsTowerInRange(tower))
		{
			attacker->StopAttacking();
		}
		else if (!attacker->IsAttacking() && IsTowerInRange(tower))
		{
			attacker->InitAttack(this);
		}
	}
}
*/

bool Enemy::IsTowerInRange(Tower *tower) const
{
	auto &tilesInRange = tower->GetTilesInRange();

	for (const auto &tile : tilesInRange)
	{
		if (tile != m_OccupiedTile)
			continue;

		return true;
	}

	return false;
}

IF_DEBUG(
void Enemy::DebugSpeed()
{
	m_Speedy = App::s_Speedy;

	switch (m_Speedy)
	{
	case EnemyDebugSpeed::none:
		m_MovementSpeed = m_MovementDebugSpeed;
		Move();
		return;
	case EnemyDebugSpeed::faster:
		m_MovementSpeed = m_MovementDebugSpeed * 2;
		Move();
		return;
	case EnemyDebugSpeed::stay: // Do nothing
	default:
		return;
	}
}
);