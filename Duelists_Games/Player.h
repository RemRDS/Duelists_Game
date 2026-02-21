#pragma once
#include "Character.h"
#include "Item.h"
#include <vector>

class Player : public Character
{
public:
	Player(int MaxHealth, int AtkPower, int Armor, int MaxStamina, std::string Name);
	Action ChooseAction() override;

	// Experience and leveling
	void GainExperience(int amount);
	int GetExperience() const { return experience; }
	int GetLevel() const { return level; }
	int GetExperienceToNextLevel() const;

	// Inventory
	void AddItem(const Item& item);
	bool UseItem(int inventoryIndex);
	const std::vector<Item>& GetInventory() const { return inventory; }

	// Buff system
	int GetEffectiveAttack() const;
	void UpdateBuffs();
	bool HasDamageBoost() const { return damageBoostRoundsLeft > 0; }
	int GetDamageBoostRounds() const { return damageBoostRoundsLeft; }

	// Charge Up system
	void ActivateCharge();
	bool IsCharged() const { return isCharged; }
	void ConsumeCharge() { isCharged = false; }

private:
	void LevelUp();

	// Leveling
	int level = 1;
	int experience = 0;

	// Inventory
	std::vector<Item> inventory;
	const int maxInventorySize = 5;

	// Active buffs
	int damageBoostRoundsLeft = 0;
	int damageBoostAmount = 0;

	// Charge Up state
	bool isCharged = false;
};


/*
Public members of Character remain public in Player.
Protected members of Character remain protected in Player.
Private members of Character remain inaccessible in Player (though Player can access them indirectly via public or protected methods in Character).
*/