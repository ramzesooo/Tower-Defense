#pragma once
#include "entity/label.h"
#include "entity/entity.h"
#include "entity/tile.h"
#include "entity/attacker.h"
#include "entity/enemy.h"
#include "entity/projectile.h"
#include "level.h"
#include "textureManager.h"
#include "logger.h"

#include "SDL.h"
#include "SDL_image.h"
#include "SDL_ttf.h"
#include <memory>

class App
{
// MAIN SECTION
public:
	App();
	~App();

	void EventHandler();
	void Update(float elapsedTime);
	void Render();

	void UpdateCamera();

	void OnResolutionChange();

	bool IsRunning() const { return m_IsRunning; }

	static int32_t WINDOW_WIDTH;
	static int32_t WINDOW_HEIGHT;

	static std::unique_ptr<TextureManager> s_Textures;
	static std::unique_ptr<Logger> s_Logger;
	static std::unique_ptr<Manager> s_Manager;

	static SDL_Renderer* s_Renderer;
	static SDL_Event s_Event;
	static SDL_FRect s_Camera;

	static class Level* s_CurrentLevel;
	static uint16_t s_TowerRange;
	static float s_ElapsedTime;
private:
	bool m_IsRunning = false;
	bool m_IsFullscreen = false;
	SDL_Window* m_Window = nullptr;
// MAIN SECTION END

public:
	// NOTE: this method should do all job for starting the level (e.g. creating enemies and whatever feature added in future)
	void LoadLevel(uint32_t baseX, uint32_t baseY);

	static inline std::string_view TextureOf(AttackerType type)
	{
		switch (type)
		{
		case AttackerType::archer:
			return "attackerArcher";
		}
		return "";
	}

	static inline std::string_view TextureOf(EnemyType type)
	{
		switch (type)
		{
		case EnemyType::elf:
			return "enemyElf";
		}
		return "";
	}

	static inline std::string_view TextureOf(ProjectileType type)
	{
		switch (type)
		{
		case ProjectileType::arrow:
			return "projectileArrow";
		}
		return "";
	}
private:
	std::vector<std::unique_ptr<Level>> levels;
};