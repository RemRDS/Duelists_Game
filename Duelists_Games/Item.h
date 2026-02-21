#pragma once
#include <string>
#include "raylib.h"

enum class ItemType
{
	HEALTH_POTION,
	DAMAGE_BOOST,
	STAMINA_RESTORE
};

class Item
{
public:
	Item(ItemType type, int power, std::string name, Color displayColor);

	ItemType GetType() const { return type; }
	int GetPower() const { return power; }
	std::string GetName() const { return name; }
	Color GetDisplayColor() const { return displayColor; }
	std::string GetDescription() const;

private:
	ItemType type;
	int power;
	std::string name;
	Color displayColor;
};
