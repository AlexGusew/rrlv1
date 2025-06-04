// main.cpp - Refactored with OOP principles
#include "ControlPanel.h"
#include "HUD.h"
#include "NodesController.h"
#include "Player.h"
#include "raylib.h"
#include "raymath.h"
#include <cmath>
#include <iostream>
#include <memory>
#include <ostream>
#include <vector>

//------------------------------------------------------------------------------------
// Constants
//------------------------------------------------------------------------------------
#define VIRTUAL_WIDTH 800
#define VIRTUAL_HEIGHT 600

// #define VIRTUAL_WIDTH 1600
// #define VIRTUAL_HEIGHT 1200
const int PLAYER_SIZE = 20;
const int ENEMY_SIZE = 25;
const int BULLET_SPEED = 800;
const int BULLET_RADIUS = 5;
const int MAX_BULLETS = 50;
const int MAX_ENEMIES = 10;

// Custom Colors
const Color CYAN = {0, 255, 255, 255};
const Color STAT_ACTION_LINK_COLOR = {128, 0, 128, 255};
const Color POWER_LINK_COLOR = {50, 205, 50, 255};
const Color DARKSLATEBLUE = {72, 61, 139, 255};
const Color MIDNIGHTBLUE = {25, 25, 112, 255};
const Color TEAL_CUSTOM = {0, 128, 128, 255};
const Color VIOLET_CUSTOM = {238, 130, 238, 255};

//------------------------------------------------------------------------------------
// Enums and Structs
//------------------------------------------------------------------------------------
enum class GameState { MAIN_MENU, GAMEPLAY, GAME_OVER };

struct Bullet {
  Vector2 position;
  Vector2 velocity;
  bool active;
  Color color;
  int damage;
  bool fromPlayer;
};

struct Enemy {
  Vector2 position;
  int health;
  float speed;
  bool active;
  float shootCooldown;
  float currentShootTimer;
};

class Window {
public:
  int width = VIRTUAL_WIDTH;
  int height = VIRTUAL_HEIGHT;

  void setDimensions() {
    width = GetRenderWidth();
    height = GetRenderHeight();
  };
};
//------------------------------------------------------------------------------------
// Global Variables
//------------------------------------------------------------------------------------
GameState currentGameState = GameState::MAIN_MENU;
Player player;
Bullet bullets[MAX_BULLETS];
Enemy enemies[MAX_ENEMIES];

NodesController nodesController;
ControlPanel controlPanel;
Window window;
HUD hud;

//------------------------------------------------------------------------------------
// Function Declarations
//------------------------------------------------------------------------------------
void InitGame();
void UpdateGame(float dt);
void DrawGame();
void UpdateDrawFrame();
void SpawnEnemy();
bool AreColorsEqual(Color c1, Color c2);

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------

bool firstRender = false;

int main() {
  SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_WINDOW_HIGHDPI);

  InitWindow(VIRTUAL_WIDTH, VIRTUAL_HEIGHT, "rrl - v0.0.3");
  SetTargetFPS(60);

  InitGame();
  while (!WindowShouldClose()) {
    UpdateDrawFrame();
  }
  CloseWindow();
  return 0;
}

//------------------------------------------------------------------------------------
// Module Functions Definitions
//------------------------------------------------------------------------------------
bool AreColorsEqual(Color c1, Color c2) {
  return (c1.r == c2.r && c1.g == c2.g && c1.b == c2.b && c1.a == c2.a);
}

void InitGame() {
  nodesController.Initialize();
  player = Player();

  controlPanel.Initialize(window.width, window.height);

  // Create initial CPU core node
  auto coreNode = nodesController.CreateNodeFromTemplate(NodeType::CPU_CORE);
  coreNode->panelPosition = {0, 0};
  coreNode->isPlaced = true;
  coreNode->isActive = true;
  player.placedNodes.push_back(std::move(coreNode));

  // Add initial inventory nodes
  player.inventoryNodes.push_back(
      nodesController.CreateNodeFromTemplate(NodeType::STAT_HEALTH));
  player.inventoryNodes.push_back(
      nodesController.CreateNodeFromTemplate(NodeType::POWER_VALUE_ADD));
  player.inventoryNodes.push_back(
      nodesController.CreateNodeFromTemplate(NodeType::ACTION_FIRE));
  player.inventoryNodes.push_back(
      nodesController.CreateNodeFromTemplate(NodeType::STAT_DAMAGE));
  player.inventoryNodes.push_back(
      nodesController.CreateNodeFromTemplate(NodeType::POWER_DURATION_REDUCE));
  player.inventoryNodes.push_back(
      nodesController.CreateNodeFromTemplate(NodeType::ACTION_SHIELD));

  for (int i = 0; i < MAX_BULLETS; i++)
    bullets[i].active = false;
  for (int i = 0; i < MAX_ENEMIES; i++)
    enemies[i].active = false;

  SpawnEnemy();
  SpawnEnemy();

  nodesController.UpdateNodeActivation(player);
  player.currentHealth = player.maxHealth;
  controlPanel.SetOpen(false);
}

void SpawnEnemy() {
  for (int i = 0; i < MAX_ENEMIES; i++) {
    if (!enemies[i].active) {
      enemies[i].active = true;
      enemies[i].position = {(float)GetRandomValue(50, window.width - 50),
                             (float)GetRandomValue(50, 150)};
      enemies[i].health = 30;
      enemies[i].speed = 100.0f;
      enemies[i].shootCooldown = 2.0f;
      enemies[i].currentShootTimer = (float)GetRandomValue(0, 200) / 100.0f;
      return;
    }
  }
}

void UpdateGame(float gameDt) {
  // Player movement
  Vector2 moveDir = {0.0f, 0.0f};

  if (IsKeyDown(KEY_W))
    moveDir.y -= 1.0f;
  if (IsKeyDown(KEY_S))
    moveDir.y += 1.0f;
  if (IsKeyDown(KEY_A))
    moveDir.x -= 1.0f;
  if (IsKeyDown(KEY_D))
    moveDir.x += 1.0f;

  if (moveDir.x != 0.0f || moveDir.y != 0.0f) {
    float length = sqrtf(moveDir.x * moveDir.x + moveDir.y * moveDir.y);
    moveDir.x /= length;
    moveDir.y /= length;
  }

  player.position.x += moveDir.x * player.currentSpeed * gameDt;
  player.position.y += moveDir.y * player.currentSpeed * gameDt;

  player.position.x = Clamp(player.position.x, (float)PLAYER_SIZE,
                            (float)window.width - PLAYER_SIZE);
  player.position.y = Clamp(player.position.y, (float)PLAYER_SIZE,
                            (float)window.height - PLAYER_SIZE);

  // Player manual shooting
  player.fireCooldownTimer -= gameDt;
  if (IsMouseButtonDown(MOUSE_LEFT_BUTTON) && player.fireCooldownTimer <= 0) {
    Vector2 mousePos = GetMousePosition();
    if (mousePos.x < controlPanel.GetPanelArea().x || !controlPanel.IsOpen()) {
      for (int i = 0; i < MAX_BULLETS; i++) {
        if (!bullets[i].active) {
          bullets[i].active = true;
          bullets[i].fromPlayer = true;
          bullets[i].position = player.position;
          Vector2 direction =
              Vector2Normalize(Vector2Subtract(mousePos, player.position));
          if (Vector2LengthSqr(direction) == 0)
            direction = {0, -1};
          bullets[i].velocity = Vector2Scale(direction, BULLET_SPEED);
          bullets[i].color = YELLOW;
          bullets[i].damage = player.currentDamage;
          player.fireCooldownTimer = player.currentFireRate;
          break;
        }
      }
    }
  }

  // Update node system
  nodesController.UpdateActionSystem(player, bullets, gameDt);
  player.applyNodeEffects();

  // Update bullets
  for (int i = 0; i < MAX_BULLETS; i++) {
    if (bullets[i].active) {
      bullets[i].position = Vector2Add(
          bullets[i].position, Vector2Scale(bullets[i].velocity, gameDt));
      if (bullets[i].position.x < 0 || bullets[i].position.x > window.width ||
          bullets[i].position.y < 0 || bullets[i].position.y > window.height) {
        bullets[i].active = false;
      }
    }
  }

  // Update enemies
  int activeEnemies = 0;
  for (int i = 0; i < MAX_ENEMIES; i++) {
    if (enemies[i].active) {
      activeEnemies++;
      Vector2 directionToPlayer = Vector2Normalize(
          Vector2Subtract(player.position, enemies[i].position));
      if (Vector2Distance(player.position, enemies[i].position) >
          PLAYER_SIZE + ENEMY_SIZE) {
        enemies[i].position = Vector2Add(
            enemies[i].position,
            Vector2Scale(directionToPlayer, enemies[i].speed * gameDt));
      }

      // Bullet vs enemy collision
      for (int b = 0; b < MAX_BULLETS; b++) {
        if (bullets[b].active && bullets[b].fromPlayer) {
          if (CheckCollisionCircles(enemies[i].position, ENEMY_SIZE,
                                    bullets[b].position, BULLET_RADIUS)) {
            enemies[i].health -= bullets[b].damage;
            bullets[b].active = false;
            if (enemies[i].health <= 0) {
              enemies[i].active = false;
              NodeType typesToDrop[] = {
                  NodeType::STAT_HEALTH,    NodeType::STAT_SPEED,
                  NodeType::STAT_DAMAGE,    NodeType::ACTION_FIRE,
                  NodeType::ACTION_SHIELD,  NodeType::ACTION_SHIFT,
                  NodeType::STAT_FIRE_RATE, NodeType::POWER_DURATION_REDUCE,
                  NodeType::POWER_VALUE_ADD};
              player.inventoryNodes.push_back(
                  nodesController.CreateNodeFromTemplate(
                      typesToDrop[GetRandomValue(
                          0, sizeof(typesToDrop) / sizeof(NodeType) - 1)]));
              break;
            }
          }
        }
      }

      // Player vs enemy collision
      if (CheckCollisionCircles(player.position, PLAYER_SIZE,
                                enemies[i].position, ENEMY_SIZE)) {
        if (!player.playerShieldIsActive) {
          player.currentHealth -= 10;
          if (player.currentHealth <= 0)
            currentGameState = GameState::GAME_OVER;
        }
      }
    }
  }

  if (activeEnemies == 0 && currentGameState == GameState::GAMEPLAY) {
    SpawnEnemy();
    SpawnEnemy();
  }
}

void DrawGame() {
  // Draw bullets
  for (int i = 0; i < MAX_BULLETS; i++)
    if (bullets[i].active)
      DrawCircleV(bullets[i].position, BULLET_RADIUS, bullets[i].color);

  // Draw enemies
  for (int i = 0; i < MAX_ENEMIES; i++) {
    if (enemies[i].active) {
      DrawCircleV(enemies[i].position, ENEMY_SIZE, MAROON);
      DrawText(
          TextFormat("%d", enemies[i].health),
          (int)(enemies[i].position.x -
                MeasureText(TextFormat("%d", enemies[i].health), 10) / 2.0f),
          (int)(enemies[i].position.y - ENEMY_SIZE - 12), 10, WHITE);
    }
  }

  // Draw player
  DrawCircleV(player.position, PLAYER_SIZE,
              player.playerShieldIsActive ? SKYBLUE : LIME);
  if (player.playerShieldIsActive)
    DrawCircleLines((int)player.position.x, (int)player.position.y,
                    PLAYER_SIZE + 3, ColorAlpha(BLUE, 0.5f));

  // Draw HUD
  hud.DrawGameHUD(player, controlPanel.IsOpen(), window.width, window.height);
}

void UpdateDrawFrame() {
  float dt = GetFrameTime();

  if (IsWindowResized()) {
    window.setDimensions();
  }

  if (IsKeyPressed(KEY_TAB)) {
    controlPanel.SetOpen(!controlPanel.IsOpen());
    if (controlPanel.IsOpen())
      controlPanel.Initialize(window.width, window.height);
    else
      player.applyNodeEffects();
  }

  if (currentGameState == GameState::GAMEPLAY) {
    UpdateGame(dt);
    if (controlPanel.IsOpen()) {
      controlPanel.Update(player, dt);
      // Update node activations after any drag and drop operations
      nodesController.UpdateNodeActivation(player);
    }
  } else if (currentGameState == GameState::MAIN_MENU ||
             currentGameState == GameState::GAME_OVER) {
    if (IsKeyPressed(KEY_ENTER)) {
      InitGame();
      currentGameState = GameState::GAMEPLAY;
      controlPanel.SetOpen(false);
    }
  }

  BeginDrawing();
  ClearBackground(BLACK);

  if (currentGameState == GameState::GAMEPLAY) {
    DrawGame();
    if (controlPanel.IsOpen())
      controlPanel.Draw(player);
  } else if (currentGameState == GameState::MAIN_MENU) {
    hud.DrawMainMenu(window.width, window.height);
  } else if (currentGameState == GameState::GAME_OVER) {
    DrawGame();
    hud.DrawGameOver(window.width, window.height);
  }

  EndDrawing();
}
