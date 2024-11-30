#pragma once
enum class ProjectileType
{
	arrow = 0,
	size
};

enum class EnemyType
{
	elf = 0,
	goblinWarrior,
	dwarfSoldier,
	dwarfKing,
	size
};

enum class AttackerType
{
	archer = 0,
	hunter,
	musketeer,
	size
};

// This enum is adjusted to layers
// 0 suits to all tiles placed at first place
// 1 suits to all additional stuff, but without collision
// 2 suits to layer with collisions (e.g. trees) including spawners
// special is an exception
enum class TileType
{
	regular = 0,
	additional,
	spawner,
	special
};