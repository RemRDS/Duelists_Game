#include "Player.h"
#include "CombatLog.h"
#include <iostream>
#include "raylib.h"

// Passes all arguments to Character Constructor
Player::Player(int MaxHealth, int AtkPower, int Armor, int MaxStamina, std::string Name) : Character(MaxHealth, AtkPower, Armor, MaxStamina, Name)
{

}

Action Player::ChooseAction()
{
	return GetActionFromInput(3);
}

void Player::GainExperience(int amount)
{
	experience += amount;
	CombatLog::AddMessage(("Gained " + std::to_string(amount) + " XP!").c_str(), GOLD, 3.0f);

	// Check for level up
	while (experience >= GetExperienceToNextLevel())
	{
		LevelUp();
	}
}

int Player::GetExperienceToNextLevel() const
{
	// Progressive XP curve: 100, 250, 450, 700, etc.
	return 100 + (level - 1) * 150;
}

void Player::LevelUp()
{
	level++;

	// Increase stats on level up
	MaxHealth += 2;
	Health = MaxHealth; // Restore to full health on level up
	AtkPower += 1;
	MaxStamina += 1;
	Stamina = MaxStamina; // Restore stamina on level up

	CombatLog::AddMessage(("LEVEL UP! Now level " + std::to_string(level) + "!").c_str(), GOLD, 4.0f);
	CombatLog::AddMessage("Health, Attack, and Stamina increased!", GREEN, 4.0f);
}

void Player::AddItem(const Item& item)
{
	if (inventory.size() < maxInventorySize)
	{
		inventory.push_back(item);
		CombatLog::AddMessage(("Found: " + item.GetName() + "!").c_str(), item.GetDisplayColor(), 3.0f);
	}
	else
	{
		CombatLog::AddMessage("Inventory full! Item lost.", RED, 3.0f);
	}
}

bool Player::UseItem(int inventoryIndex)
{
	if (inventoryIndex < 0 || inventoryIndex >= inventory.size())
	{
		return false;
	}

	Item& item = inventory[inventoryIndex];

	switch (item.GetType())
	{
	case ItemType::HEALTH_POTION:
		UpdateHealth(item.GetPower());
		CombatLog::AddMessage(("Used " + item.GetName() + "! Restored " + std::to_string(item.GetPower()) + " health!").c_str(), GREEN, 3.0f);
		break;

	case ItemType::DAMAGE_BOOST:
		damageBoostAmount = item.GetPower();
		damageBoostRoundsLeft = 2;
		CombatLog::AddMessage(("Used " + item.GetName() + "! Attack boosted for 2 rounds!").c_str(), ORANGE, 3.0f);
		break;

	case ItemType::STAMINA_RESTORE:
		Stamina = MaxStamina;
		CombatLog::AddMessage(("Used " + item.GetName() + "! Stamina fully restored!").c_str(), BLUE, 3.0f);
		break;
	}

	// Remove used item from inventory
	inventory.erase(inventory.begin() + inventoryIndex);
	return true;
}

int Player::GetEffectiveAttack() const
{
	return AtkPower + damageBoostAmount;
}

void Player::UpdateBuffs()
{
	if (damageBoostRoundsLeft > 0)
	{
		damageBoostRoundsLeft--;
		if (damageBoostRoundsLeft == 0)
		{
			damageBoostAmount = 0;
			CombatLog::AddMessage("Damage boost has worn off.", LIGHTGRAY, 2.0f);
		}
	}
}

void Player::ActivateCharge()
{
	isCharged = true;
	CombatLog::AddMessage("You focus your energy! Next attack will be enhanced!", GOLD, 3.0f);
}
