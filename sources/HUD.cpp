#include "HUD.h"
#include <cmath>

HUD::HUD() {}

void HUD::DrawGameHUD(const Player &player, bool isPanelOpen, int screenWidth,
                      int screenHeight) {
  DrawPlayerStats(player);
  DrawActiveAction(player, isPanelOpen, screenWidth);
  DrawInstructions(screenHeight);
}

void HUD::DrawPlayerStats(const Player &player) {
  DrawText(TextFormat("Health: %d/%d", player.currentHealth, player.maxHealth),
           10, 10, 20, RAYWHITE);
  DrawText(TextFormat("Speed: %.0f", player.currentSpeed), 10, 30, 20,
           RAYWHITE);
  DrawText(TextFormat("Damage: %d", player.currentDamage), 10, 50, 20,
           RAYWHITE);
  DrawText(TextFormat("Fire Rate CD: %.2fs", player.currentFireRate), 10, 70,
           20, RAYWHITE);
}

void HUD::DrawActiveAction(const Player &player, bool isPanelOpen,
                           int screenWidth) {
  if (player.activeActionNodeId != -1) {
    BaseNode *activeNode = player.GetPlayerNodeById(player.activeActionNodeId);
    if (activeNode && activeNode->isCurrentlyActiveEffect) {
      const char *text =
          TextFormat("ACTION: %s (%.1fs)", activeNode->name.c_str(),
                     fmaxf(0.0f, activeNode->currentActiveTimer));
      int textWidth = MeasureText(text, 20);
      float gameAreaReferenceWidth = screenWidth;
      if (isPanelOpen)
        gameAreaReferenceWidth = screenWidth * 2.0f / 3.0f;
      DrawText(text, (int)(gameAreaReferenceWidth / 2.0f - textWidth / 2.0f),
               10, 20, activeNode->color);
    }
  }
}

void HUD::DrawInstructions(int screenHeight) {
  DrawText("TAB for Panel", 10, screenHeight - 30, 20, RAYWHITE);
}

void HUD::DrawMainMenu(int screenWidth, int screenHeight) {
  DrawText("ROBO ROGUELIKE: POWER NODES v2",
           (int)(screenWidth / 2.0f -
                 MeasureText("ROBO ROGUELIKE: POWER NODES v2", 30) / 2.0f),
           (int)(screenHeight / 2.0f - 40), 30, WHITE);
  DrawText("Press ENTER to Start",
           (int)(screenWidth / 2.0f -
                 MeasureText("Press ENTER to Start", 20) / 2.0f),
           (int)(screenHeight / 2.0f + 20), 20, LIGHTGRAY);
}

void HUD::DrawGameOver(int screenWidth, int screenHeight) {
  DrawRectangle(0, 0, screenWidth, screenHeight, ColorAlpha(BLACK, 0.7f));
  DrawText("GAME OVER",
           (int)(screenWidth / 2.0f - MeasureText("GAME OVER", 40) / 2.0f),
           (int)(screenHeight / 2.0f - 40), 40, RED);
  DrawText("Press ENTER to Main Menu",
           (int)(screenWidth / 2.0f -
                 MeasureText("Press ENTER to Main Menu", 20) / 2.0f),
           (int)(screenHeight / 2.0f + 20), 20, LIGHTGRAY);
}
