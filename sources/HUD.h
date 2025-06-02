#pragma once
#include "raylib.h"
#include "Player.h"
#include <string>

class HUD {
public:
  HUD();
  ~HUD() = default;

  void DrawGameHUD(const Player& player, bool isPanelOpen, int screenWidth, int screenHeight);
  void DrawMainMenu(int screenWidth, int screenHeight);
  void DrawGameOver(int screenWidth, int screenHeight);

private:
  void DrawPlayerStats(const Player& player);
  void DrawActiveAction(const Player& player, bool isPanelOpen, int screenWidth);
  void DrawInstructions(int screenHeight);
};