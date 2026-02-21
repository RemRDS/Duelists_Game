#include "Enemy.h"


// Passes all arguments to Character Constructor
Enemy::Enemy(int MaxHealth, int AtkPower, int Armor, int MaxStamina, std::string Name) : Character(MaxHealth, AtkPower, Armor, MaxStamina, Name), Generator(std::random_device{}()), Distribution(0, 2)
{

}

Action Enemy::ChooseAction()
{
	int Input = Distribution(Generator);
	if (Stamina == 0)
	{
		std::uniform_int_distribution<> LimitedDistribution(0, 2);
		Input = LimitedDistribution(Generator) == 1 ? 0 : 2; // Implement Enemy being exhausted / Dazed 
	}
	SpriteC.CurrentSprite = Input + 1;
	return GetActionFromInput(Input);
}

void Enemy::IncreaseDifficulty(int RoundNumber)
{
	static std::vector<const char*> OgreTextures =
	{
		"../SourceArt/Characters/Ogre/Ogre_IdleBlinking_Sprite.png",
		"../SourceArt/Characters/Ogre/Ogre_Attacking_Sprite.png",
		"../SourceArt/Characters/Ogre/Ogre_Parry_Sprite.png",
		"../SourceArt/Characters/Ogre/Ogre_Defend_Sprite.png"
	};

	static std::vector<const char*> SkeletonTextures =
	{
		"../SourceArt/Characters/Skeleton/Skeleton_Idle_Sprite.png",
		"../SourceArt/Characters/Skeleton/Skeleton_Attacking_Sprite.png",
		"../SourceArt/Characters/Skeleton/Skeleton_Parry_Sprite.png",
		"../SourceArt/Characters/Skeleton/Skeleton_Defend_Sprite.png"
	};

	static std::vector<const char*> CyclopTextures =
	{
		"../SourceArt/Characters/Cyclop/Cyclop_IdleBlinking_Sprite.png",
		"../SourceArt/Characters/Cyclop/Cyclop_Attacking_Sprite.png",
		"../SourceArt/Characters/Cyclop/Cyclop_Parry_Sprite.png",
		"../SourceArt/Characters/Cyclop/Cyclop_Defend_Sprite.png"
	};


	switch (RoundNumber)
	{
	case 1:
		return;
	case 2:
		MaxHealth += RoundNumber + 1;
		AtkPower += 1;
		experienceReward = 75;
		Name = "Ogre";
		SwapTextureSet(OgreTextures);
		InitStats();
		return;
	case 3:
		MaxHealth += RoundNumber + 1;
		AtkPower += 2;
		Armor += RoundNumber / 2;
		experienceReward = 100;
		Name = "Skeleton";
		SwapTextureSet(SkeletonTextures);
		InitStats();
		return;
	case 4:
		MaxHealth += RoundNumber + 3;
		AtkPower += 3;
		Armor += RoundNumber / 2;
		experienceReward = 150;
		Name = "Cyclop";
		SwapTextureSet(CyclopTextures);
		InitStats();
		return;
	}

}

void Enemy::PrepareAction()
{
	currentAction = ChooseAction();
}

std::string Enemy::GetTelegraph()
{
	std::uniform_int_distribution<> accuracyRoll(1, 10);
	int roll = accuracyRoll(Generator);

	Action telegraphedAction = currentAction;

	// 10% chance for unclear telegraph
	if (roll == 1)
	{
		return "The " + Name + "'s intentions are unclear...";
	}
	// 10% chance for feint (wrong info)
	else if (roll == 2)
	{
		std::uniform_int_distribution<> feintRoll(0, 2);
		int feintAction = feintRoll(Generator);
		// Make sure feint is different from actual action
		while (feintAction == static_cast<int>(currentAction))
		{
			feintAction = feintRoll(Generator);
		}
		telegraphedAction = static_cast<Action>(feintAction);
	}
	// 80% chance for accurate telegraph (rolls 3-10)

	// Generate message based on telegraphed action
	switch (telegraphedAction)
	{
	case ATTACK:
		return "The " + Name + "'s muscles tense, readying for a strike!";
	case PARRY:
		return "The " + Name + " watches your movements intently, ready to counter!";
	case DEFEND:
		return "The " + Name + " shifts into a defensive stance...";
	default:
		return "The " + Name + " prepares for combat...";
	}
}

std::optional<Item> Enemy::GenerateLoot()
{
	std::uniform_int_distribution<> dropChance(1, 100);
	int roll = dropChance(Generator);

	// 60% chance to drop an item
	if (roll > 60)
	{
		return std::nullopt;
	}

	// Determine item type
	std::uniform_int_distribution<> itemTypeRoll(1, 3);
	int itemType = itemTypeRoll(Generator);

	// Generate random item properties
	std::uniform_int_distribution<> powerRoll(1, 3);

	switch (itemType)
	{
	case 1: // Health Potion
	{
		int healAmount = 2 + powerRoll(Generator);
		std::string potionName = "Health Potion";
		if (healAmount >= 4) potionName = "Greater Health Potion";
		return Item(ItemType::HEALTH_POTION, healAmount, potionName, GREEN);
	}
	case 2: // Damage Boost
	{
		int boostAmount = 1 + (powerRoll(Generator) / 2);
		std::string boostName = "Strength Elixir";
		if (boostAmount >= 2) boostName = "Berserker Brew";
		return Item(ItemType::DAMAGE_BOOST, boostAmount, boostName, ORANGE);
	}
	case 3: // Stamina Restore
	{
		return Item(ItemType::STAMINA_RESTORE, 0, "Stamina Tonic", SKYBLUE);
	}
	}

	return std::nullopt;
}
