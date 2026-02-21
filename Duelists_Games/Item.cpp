#include "Item.h"

Item::Item(ItemType type, int power, std::string name, Color displayColor)
	: type(type), power(power), name(name), displayColor(displayColor)
{
}

std::string Item::GetDescription() const
{
	switch (type)
	{
	case ItemType::HEALTH_POTION:
		return "Restores " + std::to_string(power) + " health";
	case ItemType::DAMAGE_BOOST:
		return "Increases attack by " + std::to_string(power) + " for 2 rounds";
	case ItemType::STAMINA_RESTORE:
		return "Fully restores stamina";
	default:
		return "Unknown item";
	}
}
