#include <iostream>
#include "raylib.h"
#include "CombatLog.h"
#include "Player.h"
#include "Enemy.h"
#include <map>
#include <functional>

float SHORT_MESSAGE_DURATION = 3.0f;
float LONG_MESSAGE_DURATION = 6.0f;
bool CAN_INPUT = true;
float INPUT_COOLDOWN = 2.9f;
float TIME_SINCE_LAST_INPUT = 0.0f;
bool INVENTORY_OPEN = false;


enum GameState {WAITING_FOR_INPUT, PROCESSING, GAME_OVER};
void DrawOutcome(Player& MainPlayer, Enemy& MainEnemy, Action PlayerAction, GameState& State, int& RoundNumber);
void ProcessOutcome(Player& Mainplayer, Enemy& MainEnemy, Action PlayerAction);
void DrawWaitForInput(Action& PlayerAction, GameState& State, Player& MainPlayer, Enemy& MainEnemy);
void DrawInventoryUI(Player& MainPlayer, Action& PlayerAction, GameState& State);


struct CombatOutcome 
{
	std::string OutcomeText;
	Color TextColor;
	std::function<void(Player&, Enemy&)> GameplayResult = [](Player&, Enemy&) {};
};

int main(void)
{
	// Raylib Init
	const int screenWidth = 800;
	const int screenHeight = 600;
	InitWindow(screenWidth, screenHeight, "Dueling Game");


	// Game Init
	Texture2D Background = LoadTexture("../SourceArt/Arena.png");
	int FramesCounter = 0;
	int FramesSpeed = 6;
	int CurrentFrame = 0;
	SetTargetFPS(60);              

	// Player Init
	Player MainPlayer(5, 2, 2, 2, "Hero");
	Vector2 PlayerPosition = { 255.0f, 150.0f };
	
	MainPlayer.AddTextureSprite("../SourceArt/Characters/Knight/Knight_IdleBlinking_Sprite.png");
	MainPlayer.AddTextureSprite("../SourceArt/Characters/Knight/Knight_Attacking_Sprite.png");
	MainPlayer.AddTextureSprite("../SourceArt/Characters/Knight/Knight_Parry_Sprite.png");
	MainPlayer.AddTextureSprite("../SourceArt/Characters/Knight/Knight_Defend_Sprite.png");

	Rectangle PlayerRect = { 0.0f, 0.0f, static_cast<float>(MainPlayer.SpriteC.TextureArray[MainPlayer.SpriteC.CurrentSprite].width / 4), static_cast<float>(MainPlayer.SpriteC.TextureArray[MainPlayer.SpriteC.CurrentSprite].height / 3) };


	// Enemy Init
	Enemy MainEnemy(2, 1, 0, 1, "Goblin");
	Vector2 EnemyPosition = { 380.0f, 160.0f };

	MainEnemy.AddTextureSprite("../SourceArt/Characters/Goblin/Goblin_IdleBlinking_Sprite.png");
	MainEnemy.AddTextureSprite("../SourceArt/Characters/Goblin/Goblin_Attacking_Sprite.png");
	MainEnemy.AddTextureSprite("../SourceArt/Characters/Goblin/Goblin_Parry_Sprite.png");
	MainEnemy.AddTextureSprite("../SourceArt/Characters/Goblin/Goblin_Defend_Sprite.png");
	
	Rectangle EnemyRect = { 0.0f, 0.0f, static_cast<float>(MainEnemy.SpriteC.TextureArray[MainEnemy.SpriteC.CurrentSprite].width / 4), static_cast<float>(MainEnemy.SpriteC.TextureArray[MainEnemy.SpriteC.CurrentSprite].height / 3) };
	EnemyRect.width = -EnemyRect.width; //Flip image
	
	int RoundNumber = 1;
	bool GameOver = false;

	GameState State = WAITING_FOR_INPUT;
	Action PlayerAction = NONE;

	while (!WindowShouldClose())
	{

		TIME_SINCE_LAST_INPUT += GetFrameTime();

		// Player Sprite Test
		FramesCounter++;
		if (FramesCounter >= (60/FramesSpeed))
		{
		
			FramesCounter = 0;
			CurrentFrame++;

			if (CurrentFrame > 3)
			{
				CurrentFrame = 0;
			}

			PlayerRect.x = static_cast<float>(CurrentFrame) * MainPlayer.GetCurrentTexture().width / 4; 
			PlayerRect.y = static_cast<float>(CurrentFrame) * MainPlayer.GetCurrentTexture().height/ 3;

			EnemyRect.x = static_cast<float>(CurrentFrame) * MainEnemy.GetCurrentTexture().width / 4;
			EnemyRect.y = static_cast<float>(CurrentFrame) * MainEnemy.GetCurrentTexture().height / 3;
		
		}


		BeginDrawing(); // Begin frame rendering
		ClearBackground(BLACK);
		DrawTexture(Background, 0, 0, WHITE);

		// Player Sprite Test
		// Apply color tint when charged to make weapons glow
		Color spriteTint = WHITE;
		if (MainPlayer.IsCharged())
		{
			// Pulsing color intensity for weapon glow
			static float weaponGlowTimer = 0.0f;
			weaponGlowTimer += GetFrameTime() * 4.0f;
			float glowPulse = (sin(weaponGlowTimer) + 1.0f) / 2.0f; // 0-1

			// Blend red (for sword/attack power) and cyan/blue (for shield/defense)
			unsigned char redIntensity = static_cast<unsigned char>(255);
			unsigned char greenIntensity = static_cast<unsigned char>(180 + (glowPulse * 75));
			unsigned char blueIntensity = static_cast<unsigned char>(100 + (glowPulse * 155));

			spriteTint = Color{ redIntensity, greenIntensity, blueIntensity, 255 };
		}
		DrawTextureRec(MainPlayer.GetCurrentTexture(), PlayerRect, PlayerPosition, spriteTint);
		DrawTextureRec(MainEnemy.GetCurrentTexture(), EnemyRect, EnemyPosition, WHITE);

		// Charge Up visual effect
		if (MainPlayer.IsCharged())
		{
			// Pulsing glow effect
			static float glowTimer = 0.0f;
			glowTimer += GetFrameTime() * 3.0f;
			float pulseIntensity = (sin(glowTimer) + 1.0f) / 2.0f; // Oscillates 0-1

			// Character center position
			float charCenterX = PlayerPosition.x + (MainPlayer.GetCurrentTexture().width / 4) / 2;
			float charCenterY = PlayerPosition.y + (MainPlayer.GetCurrentTexture().height / 3) / 2;

			// Draw outer glow (gold/yellow energy)
			Color outerGlow = ColorAlpha(GOLD, 0.3f + (pulseIntensity * 0.2f));
			DrawCircle(static_cast<int>(charCenterX), static_cast<int>(charCenterY), 45 + (pulseIntensity * 10), outerGlow);

			// Draw middle glow (brighter)
			Color middleGlow = ColorAlpha(YELLOW, 0.4f + (pulseIntensity * 0.3f));
			DrawCircle(static_cast<int>(charCenterX), static_cast<int>(charCenterY), 35 + (pulseIntensity * 8), middleGlow);

			// Draw inner glow (brightest)
			Color innerGlow = ColorAlpha(WHITE, 0.2f + (pulseIntensity * 0.3f));
			DrawCircle(static_cast<int>(charCenterX), static_cast<int>(charCenterY), 25 + (pulseIntensity * 5), innerGlow);

			// Draw red energy particles (sword/attack side - left side)
			for (int i = 0; i < 4; i++)
			{
				float angle = (glowTimer + (i * 90) + 90) * DEG2RAD; // Left side
				float orbitRadius = 35 + (pulseIntensity * 8);
				float particleX = charCenterX + cos(angle) * orbitRadius;
				float particleY = charCenterY + sin(angle) * orbitRadius;

				Color particleColor = ColorAlpha(RED, 0.7f + (pulseIntensity * 0.3f));
				DrawCircle(static_cast<int>(particleX), static_cast<int>(particleY), 4, particleColor);

				// Smaller trailing particles
				float trailX = charCenterX + cos(angle) * (orbitRadius - 10);
				float trailY = charCenterY + sin(angle) * (orbitRadius - 10);
				DrawCircle(static_cast<int>(trailX), static_cast<int>(trailY), 2, ColorAlpha(ORANGE, 0.5f));
			}

			// Draw green/cyan energy particles (shield/defense side - right side)
			for (int i = 0; i < 4; i++)
			{
				float angle = (glowTimer + (i * 90) - 90) * DEG2RAD; // Right side
				float orbitRadius = 35 + (pulseIntensity * 8);
				float particleX = charCenterX + cos(angle) * orbitRadius;
				float particleY = charCenterY + sin(angle) * orbitRadius;

				Color particleColor = ColorAlpha(LIME, 0.7f + (pulseIntensity * 0.3f));
				DrawCircle(static_cast<int>(particleX), static_cast<int>(particleY), 4, particleColor);

				// Smaller trailing particles
				float trailX = charCenterX + cos(angle) * (orbitRadius - 10);
				float trailY = charCenterY + sin(angle) * (orbitRadius - 10);
				DrawCircle(static_cast<int>(trailX), static_cast<int>(trailY), 2, ColorAlpha(SKYBLUE, 0.5f));
			}
		}

		// Player Health Bar
		int playerHealthBarX = 50;
		int playerHealthBarY = 350;
		int healthBarWidth = 150;
		int healthBarHeight = 20;

		float playerHealthPercent = static_cast<float>(MainPlayer.GetHealth()) / static_cast<float>(MainPlayer.GetMaxHealth());
		DrawRectangle(playerHealthBarX, playerHealthBarY, healthBarWidth, healthBarHeight, DARKGRAY);
		DrawRectangle(playerHealthBarX, playerHealthBarY, static_cast<int>(healthBarWidth * playerHealthPercent), healthBarHeight, RED);
		DrawRectangleLines(playerHealthBarX, playerHealthBarY, healthBarWidth, healthBarHeight, WHITE);

		std::string playerHpText = "HP: " + std::to_string(MainPlayer.GetHealth()) + "/" + std::to_string(MainPlayer.GetMaxHealth());
		DrawText(playerHpText.c_str(), playerHealthBarX, playerHealthBarY - 20, 16, WHITE);

		// Player Stamina Indicator
		int staminaY = playerHealthBarY + 30;
		std::string staminaText = "Stamina: " + std::to_string(MainPlayer.GetStamina()) + "/" + std::to_string(MainPlayer.GetMaxStamina());
		DrawText(staminaText.c_str(), playerHealthBarX, staminaY, 14, SKYBLUE);

		// Draw stamina circles
		for (int i = 0; i < MainPlayer.GetMaxStamina(); i++)
		{
			Color staminaColor = (i < MainPlayer.GetStamina()) ? SKYBLUE : DARKGRAY;
			DrawCircle(playerHealthBarX + (i * 25), staminaY + 20, 8, staminaColor);
			DrawCircleLines(playerHealthBarX + (i * 25), staminaY + 20, 8, WHITE);
		}

		// Player Attack Display
		int playerAtkDisplay = MainPlayer.GetEffectiveAttack();
		std::string playerAtkText = "ATK: " + std::to_string(playerAtkDisplay);
		DrawText(playerAtkText.c_str(), playerHealthBarX, staminaY + 45, 14, ORANGE);

		// Enemy Health Bar
		int enemyHealthBarX = 550;
		int enemyHealthBarY = 350;

		float enemyHealthPercent = static_cast<float>(MainEnemy.GetHealth()) / static_cast<float>(MainEnemy.GetMaxHealth());
		DrawRectangle(enemyHealthBarX, enemyHealthBarY, healthBarWidth, healthBarHeight, DARKGRAY);
		DrawRectangle(enemyHealthBarX, enemyHealthBarY, static_cast<int>(healthBarWidth * enemyHealthPercent), healthBarHeight, RED);
		DrawRectangleLines(enemyHealthBarX, enemyHealthBarY, healthBarWidth, healthBarHeight, WHITE);

		std::string enemyHpText = MainEnemy.GetName() + " HP: " + std::to_string(MainEnemy.GetHealth()) + "/" + std::to_string(MainEnemy.GetMaxHealth());
		DrawText(enemyHpText.c_str(), enemyHealthBarX, enemyHealthBarY - 20, 16, WHITE);

		// Enemy Stats Display
		std::string enemyAtkText = "ATK: " + std::to_string(MainEnemy.GetAtkPower());
		DrawText(enemyAtkText.c_str(), enemyHealthBarX, enemyHealthBarY + 25, 14, ORANGE);

		CombatLog::DrawMessages();

		if (State == GAME_OVER)
		{
			DrawText("Game Over. Press ESC to exit.", 200, 200, 20, RED);
			EndDrawing(); // Finalize frame rendering
			continue;
		}

		DrawText(("Round " + std::to_string(RoundNumber)).c_str(), 10, 10, 20, LIGHTGRAY);

		// Display player level and XP
		std::string levelText = "Level: " + std::to_string(MainPlayer.GetLevel());
		DrawText(levelText.c_str(), 650, 10, 18, GOLD);

		std::string xpText = "XP: " + std::to_string(MainPlayer.GetExperience()) + "/" + std::to_string(MainPlayer.GetExperienceToNextLevel());
		DrawText(xpText.c_str(), 650, 30, 14, YELLOW);

		// Display active buffs
		int buffY = 50;
		if (MainPlayer.HasDamageBoost())
		{
			std::string buffText = "ATK BOOST! (" + std::to_string(MainPlayer.GetDamageBoostRounds()) + " rounds)";
			DrawText(buffText.c_str(), 650, buffY, 14, ORANGE);
			buffY += 20;
		}

		// Display charged state
		if (MainPlayer.IsCharged())
		{
			DrawText("CHARGED! Next move enhanced!", 650, buffY, 16, GOLD);
		}

		if (State == WAITING_FOR_INPUT)
		{
			DrawWaitForInput(PlayerAction, State, MainPlayer, MainEnemy);
		}
		else if (State == PROCESSING)
		{
			DrawOutcome(MainPlayer, MainEnemy, PlayerAction, State, RoundNumber);
		}

		// Draw inventory UI overlay if open
		if (INVENTORY_OPEN && State == WAITING_FOR_INPUT)
		{
			DrawInventoryUI(MainPlayer, PlayerAction, State);
		}

		// Reset animations to Idle
		if (CAN_INPUT)
		{
			MainPlayer.SpriteC.CurrentSprite = 0;
			MainEnemy.SpriteC.CurrentSprite = 0;
		}

		EndDrawing();
	}

	UnloadTexture(Background);
	MainPlayer.UnloadAllTextures();
	MainEnemy.UnloadAllTextures();
	CloseWindow();
	return 0;
}


void DrawWaitForInput(Action& PlayerAction, GameState& State, Player& MainPlayer, Enemy& MainEnemy)
{


	if (State != WAITING_FOR_INPUT) return;

	// Prepare enemy action once per round
	if (MainEnemy.GetCurrentAction() == NONE)
	{
		MainEnemy.PrepareAction();
	}

	// Display telegraph
	std::string telegraph = MainEnemy.GetTelegraph();
	DrawText(telegraph.c_str(), 10, 50, 18, ORANGE);

	std::string actionText = "Choose an Action - (1: Attack, 2: Parry, 3: Defend, 4: Charge Up)";
	if (MainPlayer.IsCharged())
	{
		actionText = "CHARGED! (1: Heavy Attack, 2: Enhanced Parry, 3: Bulwark, 4: Charge Up)";
	}
	DrawText(actionText.c_str(), 10, 30, 18, YELLOW);

	// Display inventory hint
	const auto& inventory = MainPlayer.GetInventory();
	if (!inventory.empty())
	{
		DrawText("Press TAB or I to open Inventory", 10, 70, 16, SKYBLUE);
	}

	if(CAN_INPUT)
	{
		int keyPressed = GetKeyPressed();

		// Toggle inventory with Tab or I
		if (keyPressed == KEY_TAB || keyPressed == KEY_I)
		{
			INVENTORY_OPEN = !INVENTORY_OPEN;
		}

		switch (keyPressed)
		{

		case KEY_ONE:
			PlayerAction = ATTACK;
			State = PROCESSING;
			CAN_INPUT = false;
			TIME_SINCE_LAST_INPUT = 0.0f;
			MainPlayer.SpriteC.CurrentSprite = 1;
			break;

		case KEY_TWO:
			if (MainPlayer.GetStamina() > 0)
			{
				MainPlayer.UpdateStamina(false);
				PlayerAction = PARRY;
				State = PROCESSING;
				CAN_INPUT = false;
				TIME_SINCE_LAST_INPUT = 0.0f;
				MainPlayer.SpriteC.CurrentSprite = 2;

				break;
			}
			else
			{
				CombatLog::AddMessage("You are exhausted: You cannot Parry until you Defend!", RED, SHORT_MESSAGE_DURATION);
				CAN_INPUT = false;
				TIME_SINCE_LAST_INPUT = 0.0f;
				break;
			}

		case KEY_THREE:
			PlayerAction = DEFEND;
			State = PROCESSING;
			MainPlayer.UpdateStamina(true);
			CAN_INPUT = false;
			TIME_SINCE_LAST_INPUT = 0.0f;
			MainPlayer.SpriteC.CurrentSprite = 3;
			break;

		case KEY_FOUR:
			PlayerAction = CHARGE_UP;
			State = PROCESSING;
			CAN_INPUT = false;
			TIME_SINCE_LAST_INPUT = 0.0f;
			MainPlayer.SpriteC.CurrentSprite = 0; // Keep idle animation
			break;

		default:
			break;

		}

	}

	if (TIME_SINCE_LAST_INPUT > INPUT_COOLDOWN)
	{
		CAN_INPUT = true;
	}

}


void DrawOutcome(Player& MainPlayer, Enemy& MainEnemy, Action PlayerAction, GameState& State, int& RoundNumber)
{

	ProcessOutcome(MainPlayer, MainEnemy, PlayerAction);
	MainEnemy.ResetAction(); // Reset for next round
	State = WAITING_FOR_INPUT;

	if (!MainEnemy.GetIsAlive())
	{
		// Grant XP
		MainPlayer.GainExperience(MainEnemy.GetExperienceReward());

		// Generate loot
		auto loot = MainEnemy.GenerateLoot();
		if (loot.has_value())
		{
			MainPlayer.AddItem(loot.value());
		}

		RoundNumber++;

		if (RoundNumber > 5)
		{
			CombatLog::AddMessage("Our Hero has defeated all his opponents and is now champion of the arena !!!", GREEN, SHORT_MESSAGE_DURATION);
			State = GAME_OVER;
		}

		else
		{
			CombatLog::AddMessage((MainEnemy.GetName() + " has been defeated by our Hero! A new stronger enemy appears").c_str(), LIGHTGRAY, SHORT_MESSAGE_DURATION);
			MainEnemy.IncreaseDifficulty(RoundNumber);
			CombatLog::AddMessage((MainEnemy.GetName() + " enters the fray!").c_str(), YELLOW, SHORT_MESSAGE_DURATION);
			MainPlayer.InitStats();
			CombatLog::AddMessage("Our Hero Recovers his stamina and energy ready for a new round!", LIGHTGRAY, SHORT_MESSAGE_DURATION);
		}

	}

	if (!MainPlayer.GetIsAlive())
	{
		CombatLog::AddMessage("Our Hero has perished! You must try again!", RED, 15.5f);
		State = GAME_OVER;
	}

}


void ProcessOutcome(Player& MainPlayer, Enemy& MainEnemy, Action PlayerAction)
{
	// Update player buffs each round
	MainPlayer.UpdateBuffs();

	// Process the round logic based on actions
	Action enemyAction = MainEnemy.GetCurrentAction();

	// Handle Charge Up action
	if (PlayerAction == CHARGE_UP)
	{
		MainPlayer.ActivateCharge();

		// Enemy still acts while player charges
		switch (enemyAction)
		{
		case ATTACK:
			MainPlayer.UpdateHealth(-(MainEnemy.GetAtkPower()));
			CombatLog::AddMessage("Enemy attacks while you charge up!", RED, SHORT_MESSAGE_DURATION);
			break;
		case PARRY:
			CombatLog::AddMessage("Enemy readies a parry while you charge!", LIGHTGRAY, SHORT_MESSAGE_DURATION);
			break;
		case DEFEND:
			CombatLog::AddMessage("Enemy defends while you charge!", LIGHTGRAY, SHORT_MESSAGE_DURATION);
			break;
		default:
			break;
		}
		return;
	}

	// If player used an item, enemy still acts
	if (PlayerAction == NONE)
	{
		switch (enemyAction)
		{
		case ATTACK:
			MainPlayer.UpdateHealth(-(MainEnemy.GetAtkPower()));
			CombatLog::AddMessage("Enemy attacks while you use an item!", RED, SHORT_MESSAGE_DURATION);
			break;
		case PARRY:
			CombatLog::AddMessage("Enemy readies a parry, but you're using an item!", LIGHTGRAY, SHORT_MESSAGE_DURATION);
			break;
		case DEFEND:
			CombatLog::AddMessage("Enemy defends while you use an item!", LIGHTGRAY, SHORT_MESSAGE_DURATION);
			break;
		default:
			break;
		}
		return;
	}

	// Check if player is charged and enhance their action
	bool isCharged = MainPlayer.IsCharged();

	// Lambda for mapping Action to String
	auto GetActionString = [isCharged](Action action) -> std::string
	{
		switch (action)
		{
			case ATTACK: return isCharged ? "Heavy Attack" : "Attack";
			case DEFEND: return isCharged ? "Bulwark" : "Defend";
			case PARRY: return isCharged ? "Enhanced Parry" : "Parry";
			case CHARGE_UP: return "Charge Up";
			default: return "";
		}
	};

	std::string playerActionStr = GetActionString(PlayerAction);
	std::string enemyActionStr = GetActionString(enemyAction);

	// Display player and enemy actions
	CombatLog::AddMessage(("Player uses " + playerActionStr + "!").c_str(), GOLD, SHORT_MESSAGE_DURATION);
	CombatLog::AddMessage(("Enemy " + enemyActionStr + "s").c_str(), LIGHTGRAY, SHORT_MESSAGE_DURATION);

	// Handle combat outcomes based on charged state
	if (isCharged)
	{
		// Enhanced outcomes when charged
		switch (PlayerAction)
		{
		case ATTACK: // Heavy Attack
			switch (enemyAction)
			{
			case ATTACK:
				CombatLog::AddMessage("Heavy clash! Both combatants stagger!", ORANGE, SHORT_MESSAGE_DURATION);
				MainEnemy.UpdateHealth(-(MainPlayer.GetEffectiveAttack()));
				break;
			case PARRY:
				CombatLog::AddMessage("Enemy parries the heavy blow, but still takes damage!", ORANGE, SHORT_MESSAGE_DURATION);
				MainPlayer.UpdateHealth(-(MainEnemy.GetAtkPower()));
				MainEnemy.UpdateHealth(-(MainPlayer.GetEffectiveAttack()));
				break;
			case DEFEND:
				CombatLog::AddMessage("Heavy attack shatters the defense! Full damage!", GOLD, SHORT_MESSAGE_DURATION);
				MainEnemy.UpdateHealth(-(MainPlayer.GetEffectiveAttack() * 2));
				break;
			default:
				break;
			}
			break;

		case PARRY: // Enhanced Parry
			switch (enemyAction)
			{
			case ATTACK:
				CombatLog::AddMessage("Perfect enhanced parry! Enemy takes massive damage!", GOLD, SHORT_MESSAGE_DURATION);
				MainEnemy.UpdateHealth(-(MainPlayer.GetEffectiveAttack() * 4));
				break;
			case PARRY:
				CombatLog::AddMessage("Both combatants ready their parries!", DARKGRAY, SHORT_MESSAGE_DURATION);
				break;
			case DEFEND:
				CombatLog::AddMessage("Enhanced parry finds no target!", DARKGRAY, SHORT_MESSAGE_DURATION);
				break;
			default:
				break;
			}
			break;

		case DEFEND: // Bulwark
			switch (enemyAction)
			{
			case ATTACK:
				CombatLog::AddMessage("Bulwark stance! Attack is completely blocked!", BLUE, SHORT_MESSAGE_DURATION);
				// No damage taken!
				break;
			case PARRY:
				CombatLog::AddMessage("Bulwark stance active! Enemy watches carefully!", LIGHTGRAY, SHORT_MESSAGE_DURATION);
				break;
			case DEFEND:
				CombatLog::AddMessage("Both take impenetrable stances!", DARKGRAY, SHORT_MESSAGE_DURATION);
				break;
			default:
				break;
			}
			break;
		default:
			break;
		}

		MainPlayer.ConsumeCharge();
	}
	else
	{
		// Normal outcomes
		static std::map<std::pair<Action, Action>, CombatOutcome> outcomeMap =
		{
		{{ATTACK, ATTACK}, {"It's a clash! The weapons ring as they hit each other!", DARKGRAY,	[](Player& player, Enemy& enemy) {}}},
		{{ATTACK, PARRY}, {"Enemy parries the attack! Oh no, our hero is struck!", RED,[](Player& player, Enemy& enemy) {player.UpdateHealth(-(enemy.GetAtkPower() * 2)); }}},
		{{ATTACK, DEFEND}, {"Enemy is defending! The attack is less successful!", LIGHTGRAY,[](Player& player, Enemy& enemy) {enemy.UpdateHealth(-(player.GetEffectiveAttack() / 2)); }}},
		{{PARRY, ATTACK}, {"Our hero masterfully parries! Enemy takes double damage!", GREEN,	[](Player& player, Enemy& enemy) {enemy.UpdateHealth(-(player.GetEffectiveAttack() * 2)); }}},
		{{PARRY, PARRY}, {"Both combatants drain their stamina!", DARKGRAY,	[](Player& player, Enemy& enemy) {}}},
		{{PARRY, DEFEND}, {"The enemy defends cautiously! Our hero's parry finds no target!", DARKGRAY,	[](Player& player, Enemy& enemy) {}}},
		{{DEFEND, ATTACK}, {"Our hero is defending! Enemy's attack is less successful!", BLUE,[](Player& player, Enemy& enemy) {	player.UpdateHealth(-(enemy.GetAtkPower() / 2));}}},
		{{DEFEND, PARRY}, {"The enemy misreads the tell and drains his stamina, our hero recovers!", LIGHTGRAY,	[](Player& player, Enemy& enemy) {		}}},
		{{DEFEND, DEFEND}, {"Both combatants are recovering energy!", DARKGRAY,	[](Player& player, Enemy& enemy) {}}}
		};

		CombatOutcome outcome = outcomeMap[{PlayerAction, enemyAction}];

		// Display the outcome text
		CombatLog::AddMessage(outcome.OutcomeText.c_str(), outcome.TextColor, SHORT_MESSAGE_DURATION);
		outcome.GameplayResult(MainPlayer, MainEnemy);
	}

}

void DrawInventoryUI(Player& MainPlayer, Action& PlayerAction, GameState& State)
{
	// Draw semi-transparent background
	DrawRectangle(0, 0, 800, 600, ColorAlpha(BLACK, 0.8f));

	// Draw inventory panel
	int panelX = 200;
	int panelY = 150;
	int panelWidth = 400;
	int panelHeight = 300;

	DrawRectangle(panelX, panelY, panelWidth, panelHeight, DARKGRAY);
	DrawRectangleLines(panelX, panelY, panelWidth, panelHeight, GOLD);

	DrawText("INVENTORY", panelX + 120, panelY + 20, 24, GOLD);
	DrawText("Press TAB or I to close", panelX + 90, panelY + 50, 16, LIGHTGRAY);

	const auto& inventory = MainPlayer.GetInventory();

	if (inventory.empty())
	{
		DrawText("No items in inventory", panelX + 100, panelY + 120, 18, LIGHTGRAY);
	}
	else
	{
		DrawText("Select item to use:", panelX + 20, panelY + 80, 18, WHITE);

		for (size_t i = 0; i < inventory.size(); i++)
		{
			int itemY = panelY + 110 + (i * 30);
			std::string itemText = std::to_string(i + 1) + ": " + inventory[i].GetName() + " - " + inventory[i].GetDescription();
			DrawText(itemText.c_str(), panelX + 20, itemY, 16, inventory[i].GetDisplayColor());
		}

		// Handle item usage
		if (CAN_INPUT)
		{
			int keyPressed = GetKeyPressed();

			// Check for number keys 1-5
			if (keyPressed >= KEY_ONE && keyPressed <= KEY_FIVE)
			{
				int inventoryIndex = keyPressed - KEY_ONE;
				if (MainPlayer.UseItem(inventoryIndex))
				{
					PlayerAction = NONE;
					State = PROCESSING;
					CAN_INPUT = false;
					TIME_SINCE_LAST_INPUT = 0.0f;
					INVENTORY_OPEN = false;
				}
			}
		}
	}
}