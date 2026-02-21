#pragma once
#include "Character.h"
#include "Item.h"
#include <random>
#include <string>
#include <optional>

class Enemy : public Character
{
public:
	Enemy(int MaxHealth, int AtkPower, int Armor, int MaxStamina, std::string Name);

	Action ChooseAction() override;
	void IncreaseDifficulty(int RoundNumber);
	void PrepareAction();
	std::string GetTelegraph();
	Action GetCurrentAction() const { return currentAction; }
	void ResetAction() { currentAction = NONE; }

	// Loot system
	std::optional<Item> GenerateLoot();
	int GetExperienceReward() const { return experienceReward; }

private:

	std::mt19937 Generator;
	std::uniform_int_distribution<> Distribution;
	Action currentAction = NONE;
	int experienceReward = 50;
};


/*
Public members of Character remain public in Player.
Protected members of Character remain protected in Player.
Private members of Character remain inaccessible in Player (though Player can access them indirectly via public or protected methods in Character).
*/