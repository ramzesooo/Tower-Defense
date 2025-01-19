#include "textureManager.h"
#include "app.h"
#include "logger.h"

#include "SDL_image.h"

#include <format>
#include <string>

static constexpr char assetsPathEntry[] = "assets/";
static constexpr char soundPathEntry[] = "assets/sounds/";
static constexpr char fontPathEntry[] = "assets/fonts/";

struct AssetData
{
	std::string_view id, path;
};

// TEXTURES

static constexpr AssetData texturesToLoad[]
{
	{ "mapSheet",					"tileset.png" },
	{ "szpaku",						"szpaku.jpg" },

	{ "base",						"base.png" },
	{ "square",						"square_32x32.png" },
	{ "green",						"green_32x32.png" },
	{ "transparent",				"transparent.png" },
	{ "grayArrow",					"grayArrow_32x32.png" },

	{ "buttonUI",					"ui/ui_button.png" },
	{ "checkedButtonUI",			"ui/ui_button_checked.png" },
	{ "uncheckedButtonUI",			"ui/ui_button_unchecked.png" },
	{ "hoveredButtonUI",			"ui/ui_button_hovered.png" },
	{ "checkedHoveredButtonUI",		"ui/ui_button_hovered_checked.png" },
	{ "uncheckedHoveredButtonUI",	"ui/ui_button_hovered_unchecked.png" },
	{ "canBuild",					"ui/tile_CanBuild.png" },
	{ "cantBuild",					"ui/tile_CantBuild.png" },
	//{ "upgradeTower",				"ui/tile_Upgrade.png" },
	{ "highlightTowerRange",		"ui/highlightRange.png" },
	{ "elementUI",					"ui/ui_element.png" },
	{ "coinUI",						"ui/coin.png" },
	{ "heartUI",					"ui/heart.png" },
	{ "timerUI",					"ui/timer.png" },
	{ "buildHammer",				"ui/hammer_1.png" },
	{ "buildHammerGreen",			"ui/hammer_2.png" },
	{ "upgradeIcon",				"ui/upgrade.png" },
	{ "sellIcon",					"ui/sell.png" },
	{ "transparentGreen",			"ui/green_34x34.png" },

	{ TextureManager::IconOf(TowerType::classic),			"ui/tower_0.png"},
	{ TextureManager::IconOf(TowerType::dark),				"ui/tower_1.png"},

	{ TextureManager::TextureOf(TowerType::classic),		"towers/classic/tower.png"},
	{ TextureManager::TextureOf(TowerType::dark),			"towers/dark/DarkTower-Sheet.png"},
	  
	{ TextureManager::TextureOf(ProjectileType::arrow),	"projectiles/arrow_16x16.png" },
	{ TextureManager::TextureOf(ProjectileType::thunder),	"projectiles/thunder.png" },
	  
	{ TextureManager::TextureOf(AttackerType::archer),		"entities/friendly/attackerArcher.png" },
	{ TextureManager::TextureOf(AttackerType::hunter),		"entities/friendly/attackerHunter.png"},
	{ TextureManager::TextureOf(AttackerType::musketeer),	"entities/friendly/attackerMusketeer.png" },
	  
	{ TextureManager::TextureOf(EnemyType::elf),			"entities/enemy/enemyElf.png" },
	{ TextureManager::TextureOf(EnemyType::goblinWarrior),	"entities/enemy/enemyGoblinWarrior.png" },
	{ TextureManager::TextureOf(EnemyType::dwarfSoldier),	"entities/enemy/enemyDwarfSoldier.png" },
	{ TextureManager::TextureOf(EnemyType::dwarfKing),		"entities/enemy/enemyDwarfKing.png" }
};

// TEXTURES

// FONTS

struct FontData
{
	AssetData assetData{};
	uint16_t fontSize = 0u;
};

static constexpr FontData fontsToLoad[]
{
	{ "default",		"F25_Bank_Printer.ttf",	15u },
	{ "enemyHealth",	"Rostack.otf",			13u }
};

// FONTS

// SOUNDS

static constexpr AssetData soundsToLoad[]
{
	{ "hoverButton",	"hover_button.wav" },
	{ "selectButton",	"select_button.wav" },
	{ "thunderAttack",	"thunder_attack.wav" },
	{ "arrowAttack",	"arrow_attack.wav" },
	{ "hurt",			"hurt.wav" },
	{ "finishBuild",	"finish_build.wav" }
};

// SOUNDS

TextureManager::~TextureManager()
{
	for (const auto &font : fonts)
	{
		TTF_CloseFont(font.second);
	}

	for (const auto &texture : textures)
	{
		SDL_DestroyTexture(texture.second);
	}

	for (const auto &sound : sounds)
	{
		Mix_FreeChunk(sound.second);
	}

	TTF_Quit();

	IF_DEBUG(App::s_Logger.AddInstantLog(std::string_view("TextureManager::~TextureManager: Destroyed all assets and triggered TTF_Quit()")););
}

void TextureManager::LoadAssets() // Initialize
{
	for (const auto &[id, path] : texturesToLoad)
	{
		AddTexture(std::string(id), (assetsPathEntry + std::string(path)).c_str());
	}

	for (const auto &[data, size] : fontsToLoad)
	{
		AddFont(std::string(data.id), (fontPathEntry + std::string(data.path)).c_str(), size);
	}

	for (const auto &[id, path] : soundsToLoad)
	{
		LoadSound(std::string(id), (soundPathEntry + std::string(path)).c_str());
	}
}

void TextureManager::DrawTexture(SDL_Texture *texture, const SDL_Rect &src, const SDL_Rect &dest, double angle, SDL_RendererFlip flip)
{
	SDL_RenderCopyEx(App::s_Renderer, texture, &src, &dest, angle, nullptr, flip);
}

void TextureManager::DrawTextureF(SDL_Texture *texture, const SDL_Rect &src, const SDL_FRect &dest, double angle, SDL_RendererFlip flip)
{
	SDL_RenderCopyExF(App::s_Renderer, texture, &src, &dest, angle, nullptr, flip);
	// Basically could be if (SDL_RenderCopyExF(...) != 0) to print errors as well as TextureManager::DrawTexture
	// But already GetTexture() throws log about missing texture or whatever
}

// TEXTURES

void TextureManager::AddTexture(const std::string &textureID, const char *path)
{
	SDL_Surface *tempSurface = IMG_Load(path);
	if (!tempSurface)
	{
		IF_DEBUG(App::s_Logger.AddInstantLog(std::string_view(SDL_GetError())););
		IF_NDEBUG(App::s_Logger.AddLog(std::string_view(SDL_GetError())););
		return;
	}

	SDL_Texture *texture = SDL_CreateTextureFromSurface(App::s_Renderer, tempSurface);
	if (!texture)
	{
		IF_DEBUG(App::s_Logger.AddInstantLog(std::string_view(SDL_GetError())););
		IF_NDEBUG(App::s_Logger.AddLog(std::string_view(SDL_GetError())););
		SDL_FreeSurface(tempSurface);
		return;
	}
	SDL_FreeSurface(tempSurface);

	IF_DEBUG(App::s_Logger.AddInstantLog(std::format("Loaded texture \"{}\" from \"{}\"", textureID, path)););

	textures.emplace(textureID, texture);
}

SDL_Texture* TextureManager::GetTexture(std::string_view textureID) const
{
	auto it = textures.find(textureID);
	if (it == textures.end())
	{
		App::s_Logger.AddLog(std::string_view("TextureManager::GetTexture: Missing texture "), false);
		App::s_Logger.AddLog(textureID);
		return nullptr;
	}

	return it->second;
}

// TEXTURES

// FONTS

void TextureManager::AddFont(std::string_view fontID, const char *path, uint16_t fontSize)
{
	TTF_Font *font = TTF_OpenFont(path, fontSize);

	if (!font)
	{
		IF_DEBUG(App::s_Logger.AddInstantLog(std::format("TextureManager::GetFont: Couldn't open font {}", path)););
		IF_NDEBUG(App::s_Logger.AddLog(std::format("TextureManager::GetFont: Couldn't open font {}", path)););
		return;
	}

	IF_DEBUG(App::s_Logger.AddInstantLog(std::format("Loaded font \"{}\" (size: {}) from \"{}\"", fontID, fontSize, path)););

	fonts.emplace(fontID, font);
}

TTF_Font* TextureManager::GetFont(std::string_view fontID) const
{
	auto it = fonts.find(fontID);
	if (it == fonts.end())
	{
		App::s_Logger.AddLog(std::string_view("TextureManager::GetFont: Missing font "), false);
		App::s_Logger.AddLog(fontID);
		return nullptr;
	}

	return it->second;
}

// FONTS

// SOUNDS

void TextureManager::LoadSound(const std::string &soundID, const char *path)
{
	Mix_Chunk *sound = Mix_LoadWAV(path);
	if (!sound)
	{
		IF_DEBUG(App::s_Logger.AddInstantLog(std::format("TextureManager::LoadSound: Couldn't load sound from \"{}\"", path)););
		IF_NDEBUG(App::s_Logger.AddLog(std::format("TextureManager::LoadSound: Couldn't load sound from \"{}\"", path)););
		return;
	}

	IF_DEBUG(App::s_Logger.AddInstantLog(std::format("Loaded sound \"{}\" from \"{}\"", soundID, path)););

	sounds.emplace(soundID, sound);
}

Mix_Chunk *TextureManager::GetSound(std::string_view soundID)
{
	auto it = sounds.find(soundID);
	if (it == sounds.end())
	{
		App::s_Logger.AddLog(std::string_view("TextureManager::GetSound: Missing sound "), false);
		App::s_Logger.AddLog(soundID);
		return nullptr;
	}

	return it->second;
}

// SOUNDS