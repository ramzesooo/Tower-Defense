#pragma once
#include "entity/label.h"
#include "entity/entity.h"
#include "entity/tile.h"
#include "entity/attacker.h"
#include "entity/enemy.h"
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
	void Update();
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
private:
	bool m_IsRunning = false;
	bool m_IsFullscreen = false;
	SDL_Window* m_Window = nullptr;
// MAIN SECTION END
// 
// LEVEL SECTION
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

	// probably don't need this since tiles are added directly in class Level in Setup()
	Tile* AddTile(int srcX, int srcY, int posX, int posY, int tileSize, int tileScale, std::string_view textureID);
private:
	std::vector<std::unique_ptr<Level>> levels;
// LEVEL SECTION END
};