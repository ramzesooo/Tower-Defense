#pragma once
#include "common.h"
#include "entity/typesEnums.h"

#include "SDL_rect.h"
#include "SDL_render.h"
#include "SDL_ttf.h"
#include "SDL_mixer.h"

#include <unordered_map>
#include <string>

class TextureManager // wrong name, but too lazy for change it everywhere
{
public:
	TextureManager() = default;
	TextureManager(const TextureManager &) = delete;
	~TextureManager();

	TextureManager &operator=(const TextureManager &) = delete;

	void LoadAssets();

	// DrawTexture is responsible for drawing rectangles with integer values
	// angle is NULL and flip is SDL_FLIP_NONE by default
	static void DrawTexture(SDL_Texture *texture, const SDL_Rect &src, const SDL_Rect &dest, double angle = 0, SDL_RendererFlip flip = SDL_FLIP_NONE);

	// DrawTextureF is responsible for drawing rectangles with float values
	// angle is NULL and flip is SDL_FLIP_NONE by default
	static void DrawTextureF(SDL_Texture* texture, const SDL_Rect& src, const SDL_FRect& dest, double angle = 0, SDL_RendererFlip flip = SDL_FLIP_NONE);

	static constexpr inline std::string_view TextureOf(AttackerType type)
	{
		switch (type)
		{
		case AttackerType::archer:
			return "attackerArcher";
		case AttackerType::hunter:
			return "attackerHunter";
		case AttackerType::musketeer:
			return "attackerMusketeer";
		case AttackerType::darkTower:
			return "";
		}
		return "";
	}

	static constexpr inline std::string_view TextureOf(EnemyType type)
	{
		switch (type)
		{
		case EnemyType::elf:
			return "enemyElf";
		case EnemyType::goblinWarrior:
			return "enemyGoblinWarrior";
		case EnemyType::dwarfSoldier:
			return "enemyDwarfSoldier";
		case EnemyType::dwarfKing:
			return "enemyDwarfKing";
		}
		return "";
	}

	static constexpr inline std::string_view TextureOf(ProjectileType type)
	{
		switch (type)
		{
		case ProjectileType::arrow:
			return "projectileArrow";
		case ProjectileType::thunder:
			return "projectileDarkTower";
		}
		return "";
	}

	static constexpr inline std::string_view IconOf(TowerType type)
	{
		switch (type)
		{
		case TowerType::classic:
			return "classicTowerIcon";
		case TowerType::dark:
			return "darkTowerIcon";
		}
		return "";
	}

	static constexpr inline std::string_view TextureOf(TowerType type)
	{
		switch (type)
		{
		case TowerType::classic:
			return "classicTower";
		case TowerType::dark:
			return "darkTower";
		}
		return "";
	}

	SDL_Texture *GetTextureOf(TowerType type) const { return GetTexture(TextureOf(type)); }
	SDL_Texture *GetIconOf(TowerType type) const { return GetTexture(IconOf(type)); }
	SDL_Texture *GetTextureOf(ProjectileType type) const { return GetTexture(TextureOf(type)); }
	SDL_Texture *GetTextureOf(EnemyType type) const { return GetTexture(TextureOf(type)); }
	SDL_Texture *GetTextureOf(AttackerType type) const
	{ 
		if (type == AttackerType::darkTower)
			return nullptr;

		return GetTexture(TextureOf(type)); 
	}

	void AddTexture(const std::string &textureID, const char *path);
	SDL_Texture* GetTexture(std::string_view textureID) const;

	void AddFont(std::string_view fontID, const char* path, uint16_t fontSize);
	TTF_Font *GetFont(std::string_view fontID) const;

	void LoadSound(const std::string &soundID, const char *path);
	Mix_Chunk *GetSound(std::string_view soundID);
private:
	std::unordered_map<std::string, SDL_Texture*, proxy_hash, std::equal_to<void>> textures;
	std::unordered_map<std::string, TTF_Font*, proxy_hash, std::equal_to<void>> fonts;
	std::unordered_map<std::string, Mix_Chunk*, proxy_hash, std::equal_to<void>> sounds;
};