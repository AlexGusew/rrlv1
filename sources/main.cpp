// main.cpp
#include "raylib.h"
#include "raymath.h"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <map>
#include <string>
#include <vector>

//------------------------------------------------------------------------------------
// Constants
//------------------------------------------------------------------------------------
const int SCREEN_WIDTH = 1200;
const int SCREEN_HEIGHT = 700;
const int PLAYER_SIZE = 20;
const int ENEMY_SIZE = 25;
const int BULLET_SPEED = 800;
const int BULLET_RADIUS = 5;
const int MAX_BULLETS = 50;
const int MAX_ENEMIES = 10;
const int NODE_UI_SIZE = 50;

// Custom Colors
const Color CYAN = {0, 255, 255, 255};
const Color STAT_ACTION_LINK_COLOR = {128, 0, 128, 255};
const Color POWER_LINK_COLOR = {50, 205, 50, 255}; // LimeGreen like
const Color DARKSLATEBLUE = {72, 61, 139, 255};
const Color MIDNIGHTBLUE = {25, 25, 112, 255};
const Color TEAL_CUSTOM = {0, 128, 128, 255};
const Color VIOLET_CUSTOM = {238, 130, 238, 255};

//------------------------------------------------------------------------------------
// Enums and Structs
//------------------------------------------------------------------------------------
enum class GameState { MAIN_MENU, GAMEPLAY, GAME_OVER };

enum class NodeType {
  NONE,
  CPU_CORE,
  STAT_HEALTH,
  STAT_SPEED,
  STAT_DAMAGE,
  STAT_FIRE_RATE,
  ACTION_FIRE,
  ACTION_SHIELD,
  ACTION_SHIFT,
  POWER_DURATION_REDUCE,
  POWER_VALUE_ADD
};

struct Node {
  int id;
  NodeType type;
  std::string name;
  std::string description;
  float value;
  Color color;

  Vector2 panelPosition;
  bool isPlaced;
  bool isActive;
  std::vector<int> connectedToNodeIDs;
  std::vector<int> connectedFromNodeIDs;

  float duration;
  float currentActiveTimer;
  bool isCurrentlyActiveEffect;

  Node(int _id = -1, NodeType _type = NodeType::NONE, std::string _name = "N/A",
       float _val = 0.0f, float _dur = 0.0f)
      : id(_id), type(_type), name(_name), value(_val), duration(_dur),
        isPlaced(false), isActive(false), currentActiveTimer(0.0f),
        isCurrentlyActiveEffect(false) {
    switch (type) {
    case NodeType::CPU_CORE:
      color = GOLD;
      description = "CPU. Powers STAT & POWER nodes. Starts ACTION sequences.";
      break;
    case NodeType::STAT_HEALTH:
      color = GREEN;
      description = "Health Chip: +Max Health. Buffs Shield Duration.";
      break;
    case NodeType::STAT_SPEED:
      color = BLUE;
      description = "Speed Chip: +Move Speed. Buffs Shift Potency.";
      break;
    case NodeType::STAT_DAMAGE:
      color = RED;
      description = "Damage Chip: +Bullet Dmg. Buffs Fire Dmg.";
      break;
    case NodeType::STAT_FIRE_RATE:
      color = SKYBLUE;
      description = "FireRate Chip: +Fire Rate.";
      break;
    case NodeType::ACTION_FIRE:
      color = ORANGE;
      description = "Action: Fire Blast.";
      break;
    case NodeType::ACTION_SHIELD:
      color = DARKPURPLE;
      description = "Action: Energy Shield.";
      break;
    case NodeType::ACTION_SHIFT:
      color = PINK;
      description = "Action: Phase Shift.";
      break;
    case NodeType::POWER_DURATION_REDUCE:
      color = TEAL_CUSTOM;
      description = "Power: Modify connected Action duration. Value is seconds "
                    "change (+/-).";
      break;
    case NodeType::POWER_VALUE_ADD:
      color = VIOLET_CUSTOM;
      description = "Power: Modify connected Stat/Action value. Value is "
                    "amount to add (+/-).";
      break;
    default:
      color = GRAY;
      break;
    }
  }

  bool isActionType() const {
    return type == NodeType::ACTION_FIRE || type == NodeType::ACTION_SHIELD ||
           type == NodeType::ACTION_SHIFT;
  }
  bool isStatType() const {
    return type == NodeType::STAT_HEALTH || type == NodeType::STAT_SPEED ||
           type == NodeType::STAT_DAMAGE || type == NodeType::STAT_FIRE_RATE;
  }
  bool isPowerType() const {
    return type == NodeType::POWER_DURATION_REDUCE ||
           type == NodeType::POWER_VALUE_ADD;
  }
};

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

struct Player {
  Vector2 position;
  float baseSpeed, currentSpeed;
  int baseHealth, currentHealth, maxHealth;
  float baseFireRate, currentFireRate;
  float fireCooldownTimer;
  int baseDamage, currentDamage;

  std::vector<Node> inventoryNodes;
  std::vector<Node> placedNodes;

  int activeActionNodeId;
  bool playerShieldIsActive;

  Node *GetPlayerNodeById(int id);

  Player() {
    position = {(float)SCREEN_WIDTH / 2.0f, (float)SCREEN_HEIGHT / 2.0f};
    baseSpeed = 200.0f;
    baseHealth = 100;
    baseFireRate = 2.5f;
    baseDamage = 10;
    activeActionNodeId = -1;
    playerShieldIsActive = false;
    fireCooldownTimer = 0.0f;

    resetStats();
  }

  void resetStats() {
    currentSpeed = baseSpeed;
    maxHealth = baseHealth;
    currentHealth = std::min(currentHealth, maxHealth);
    if (currentHealth <= 0 && baseHealth > 0)
      currentHealth = baseHealth;
    currentFireRate = baseFireRate;
    currentDamage = baseDamage;
  }

  void applyNodeEffects() {
    resetStats();
    for (Node &node : placedNodes) {
      if (node.isActive && node.isStatType()) {
        float effectiveStatValue = node.value;
        for (int fromId : node.connectedFromNodeIDs) {
          Node *powerNode = GetPlayerNodeById(fromId);
          if (powerNode && powerNode->type == NodeType::POWER_VALUE_ADD &&
              powerNode->isActive) {
            effectiveStatValue += powerNode->value;
          }
        }
        switch (node.type) {
        case NodeType::STAT_HEALTH:
          maxHealth += (int)effectiveStatValue;
          break;
        case NodeType::STAT_SPEED:
          currentSpeed += effectiveStatValue;
          break;
        case NodeType::STAT_DAMAGE:
          currentDamage += (int)effectiveStatValue;
          break;
        case NodeType::STAT_FIRE_RATE:
          currentFireRate -= effectiveStatValue;
          if (currentFireRate < 0.05f)
            currentFireRate = 0.05f;
          break;
        default:
          break;
        }
      }
    }
    if (activeActionNodeId != -1) {
      Node *currentAction = GetPlayerNodeById(activeActionNodeId);
      if (currentAction && currentAction->type == NodeType::ACTION_SHIFT &&
          currentAction->isCurrentlyActiveEffect) {
        float shiftEffectiveValue = currentAction->value;
        for (int fromId : currentAction->connectedFromNodeIDs) {
          Node *modNode = GetPlayerNodeById(fromId);
          if (modNode && modNode->isActive) {
            if (modNode->type == NodeType::POWER_VALUE_ADD) {
              shiftEffectiveValue += modNode->value;
            } else if (modNode->type == NodeType::STAT_SPEED) {
              shiftEffectiveValue += modNode->value;
            }
          }
        }
        currentSpeed += shiftEffectiveValue;
      }
    }
    currentHealth = std::min(currentHealth, maxHealth);
    if (currentHealth > maxHealth)
      currentHealth = maxHealth;
  }
};

//------------------------------------------------------------------------------------
// Global Variables
//------------------------------------------------------------------------------------
GameState currentGameState = GameState::MAIN_MENU;
Player player;
Bullet bullets[MAX_BULLETS];
Enemy enemies[MAX_ENEMIES];
int nextNodeId = 0;

bool isPanelOpen = false;
Rectangle panelArea;
Rectangle panelInventoryArea;
Rectangle panelGridArea;
Camera2D panelCamera;
float inventoryScrollOffset = 0.0f;
const float NODE_INV_ITEM_HEIGHT = NODE_UI_SIZE + 10.0f;

int draggingNodeIndex = -1;
bool draggingFromInventory = false;
int connectingNodeFromId = -1;

std::map<NodeType, Node> nodeTemplates;

//------------------------------------------------------------------------------------
// Function Declarations
//------------------------------------------------------------------------------------
void InitGame();
void UpdateGame(float dt);
void DrawGame();
void UpdateDrawFrame();
void InitControlPanel();
void UpdateControlPanel(float dt);
void DrawControlPanel();
void UpdateNodeActivation();
Node CreateNodeFromTemplate(NodeType type);
void SpawnEnemy();
Node *GetNodeById(int id, std::vector<Node> &nodes);
void FireActionBullet(Player &p, Bullet bulletsArray[],
                      const Node &fireActionNode);
bool AreColorsEqual(Color c1, Color c2);

Node *Player::GetPlayerNodeById(int id) { return GetNodeById(id, placedNodes); }
//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main() {
  InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Robo-Roguelike - Power Nodes v3");
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
Node *GetNodeById(int id, std::vector<Node> &nodes) {
  for (Node &node : nodes) {
    if (node.id == id)
      return &node;
  }
  return nullptr;
}
bool AreColorsEqual(Color c1, Color c2) {
  return (c1.r == c2.r && c1.g == c2.g && c1.b == c2.b && c1.a == c2.a);
}
void InitNodeTemplates() {
  nodeTemplates.clear();
  nodeTemplates[NodeType::CPU_CORE] =
      Node(0, NodeType::CPU_CORE, "CPU Core", 0);
  nodeTemplates[NodeType::STAT_HEALTH] =
      Node(0, NodeType::STAT_HEALTH, "Health Chip", 25);
  nodeTemplates[NodeType::STAT_SPEED] =
      Node(0, NodeType::STAT_SPEED, "Speed Chip", 50);
  nodeTemplates[NodeType::STAT_DAMAGE] =
      Node(0, NodeType::STAT_DAMAGE, "Damage Chip", 5);
  nodeTemplates[NodeType::STAT_FIRE_RATE] =
      Node(0, NodeType::STAT_FIRE_RATE, "FireRate Chip", 0.1f);
  nodeTemplates[NodeType::ACTION_FIRE] =
      Node(0, NodeType::ACTION_FIRE, "Fire Blast", 20, 2.0f);
  nodeTemplates[NodeType::ACTION_SHIELD] =
      Node(0, NodeType::ACTION_SHIELD, "Energy Shield", 0, 3.0f);
  nodeTemplates[NodeType::ACTION_SHIFT] =
      Node(0, NodeType::ACTION_SHIFT, "Phase Shift", 75.0f, 1.5f);
  nodeTemplates[NodeType::POWER_DURATION_REDUCE] =
      Node(0, NodeType::POWER_DURATION_REDUCE, "Duration Mod", -0.5f);
  nodeTemplates[NodeType::POWER_VALUE_ADD] =
      Node(0, NodeType::POWER_VALUE_ADD, "Value Mod", 10.0f);
}
Node CreateNodeFromTemplate(NodeType type) {
  if (nodeTemplates.count(type)) {
    Node newNode = nodeTemplates[type];
    newNode.id = nextNodeId++;
    return newNode;
  }
  return Node(nextNodeId++, NodeType::NONE, "ERROR NODE", 0);
}

void InitGame() {
  InitNodeTemplates();
  player = Player();
  nextNodeId = 0;

  panelArea = {SCREEN_WIDTH * 2.0f / 3.0f, 0, SCREEN_WIDTH / 3.0f,
               (float)SCREEN_HEIGHT};
  panelInventoryArea = {panelArea.x + 10, panelArea.y + 40,
                        panelArea.width - 20, panelArea.height * 0.4f - 50};
  panelGridArea = {panelArea.x + 10, panelArea.y + panelArea.height * 0.4f + 10,
                   panelArea.width - 20, panelArea.height * 0.6f - 20};

  Node coreNode = CreateNodeFromTemplate(NodeType::CPU_CORE);
  coreNode.panelPosition = {0, 0};
  coreNode.isPlaced = true;
  coreNode.isActive = true;
  player.placedNodes.push_back(coreNode);

  player.inventoryNodes.push_back(
      CreateNodeFromTemplate(NodeType::STAT_HEALTH));
  player.inventoryNodes.push_back(
      CreateNodeFromTemplate(NodeType::POWER_VALUE_ADD));
  player.inventoryNodes.push_back(
      CreateNodeFromTemplate(NodeType::ACTION_FIRE));
  player.inventoryNodes.push_back(
      CreateNodeFromTemplate(NodeType::STAT_DAMAGE));
  player.inventoryNodes.push_back(
      CreateNodeFromTemplate(NodeType::POWER_DURATION_REDUCE));
  player.inventoryNodes.push_back(
      CreateNodeFromTemplate(NodeType::ACTION_SHIELD));

  for (int i = 0; i < MAX_BULLETS; i++)
    bullets[i].active = false;
  for (int i = 0; i < MAX_ENEMIES; i++)
    enemies[i].active = false;
  SpawnEnemy();
  SpawnEnemy();
  player.applyNodeEffects();
  player.currentHealth = player.maxHealth;
  isPanelOpen = false;
  InitControlPanel();
}

void SpawnEnemy() {
  for (int i = 0; i < MAX_ENEMIES; i++) {
    if (!enemies[i].active) {
      enemies[i].active = true;
      enemies[i].position = {(float)GetRandomValue(50, SCREEN_WIDTH - 50),
                             (float)GetRandomValue(50, 150)};
      enemies[i].health = 30;
      enemies[i].speed = 100.0f;
      enemies[i].shootCooldown = 2.0f;
      enemies[i].currentShootTimer = (float)GetRandomValue(0, 200) / 100.0f;
      return;
    }
  }
}

void FireActionBullet(Player &p, Bullet bulletsArray[],
                      const Node &fireActionNode) {
  float effectiveDamage = fireActionNode.value;
  for (int fromId : fireActionNode.connectedFromNodeIDs) {
    Node *modifierNode = p.GetPlayerNodeById(fromId);
    if (modifierNode && modifierNode->isActive) {
      if (modifierNode->type == NodeType::STAT_DAMAGE ||
          modifierNode->type == NodeType::POWER_VALUE_ADD) {
        effectiveDamage += modifierNode->value;
      }
    }
  }
  for (int i = 0; i < MAX_BULLETS; i++) {
    if (!bulletsArray[i].active) {
      bulletsArray[i].active = true;
      bulletsArray[i].fromPlayer = true;
      bulletsArray[i].position = p.position;
      Vector2 mousePos = GetMousePosition();
      Vector2 direction =
          Vector2Normalize(Vector2Subtract(mousePos, p.position));
      if (Vector2LengthSqr(direction) == 0)
        direction = {0, -1};
      bulletsArray[i].velocity = Vector2Scale(direction, BULLET_SPEED * 1.1f);
      bulletsArray[i].color = ORANGE;
      bulletsArray[i].damage = p.currentDamage + (int)effectiveDamage;
      break;
    }
  }
}

void UpdateGame(float gameDt) {
  // Player movement always available
  if (IsKeyDown(KEY_W))
    player.position.y -= player.currentSpeed * gameDt;
  if (IsKeyDown(KEY_S))
    player.position.y += player.currentSpeed * gameDt;
  if (IsKeyDown(KEY_A))
    player.position.x -= player.currentSpeed * gameDt;
  if (IsKeyDown(KEY_D))
    player.position.x += player.currentSpeed * gameDt;
  player.position.x = Clamp(player.position.x, (float)PLAYER_SIZE,
                            (float)SCREEN_WIDTH - PLAYER_SIZE);
  player.position.y = Clamp(player.position.y, (float)PLAYER_SIZE,
                            (float)SCREEN_HEIGHT - PLAYER_SIZE);

  // Player manual shooting
  player.fireCooldownTimer -= gameDt;
  std::cout << IsMouseButtonDown(MOUSE_LEFT_BUTTON) << ", "
            << player.fireCooldownTimer << ", " << player.currentFireRate
            << std::endl;
  if (IsMouseButtonDown(MOUSE_LEFT_BUTTON) && player.fireCooldownTimer <= 0) {
    Vector2 mousePos = GetMousePosition();
    std::cout << mousePos.x << ", " << panelArea.x << ", " << isPanelOpen
              << std::endl;
    if (mousePos.x < panelArea.x || !isPanelOpen) {
      for (int i = 0; i < MAX_BULLETS; i++) {
        if (!bullets[i].active) {
          bullets[i].active = true;
          bullets[i].fromPlayer = true;
          bullets[i].position = player.position;
          Vector2 direction =
              Vector2Normalize(Vector2Subtract(mousePos, player.position));
          if (Vector2LengthSqr(direction) == 0)
            direction = {0, -1}; // Default shoot upwards if mouse on player
          bullets[i].velocity = Vector2Scale(direction, BULLET_SPEED);
          bullets[i].color = YELLOW;
          bullets[i].damage = player.currentDamage;
          player.fireCooldownTimer = player.currentFireRate;
          break;
        }
      }
    }
  }

  // Action Node System Update
  if (player.activeActionNodeId == -1) {
    Node *cpuNode = player.GetPlayerNodeById(0);
    if (cpuNode && cpuNode->type == NodeType::CPU_CORE) {
      for (int connectedId : cpuNode->connectedToNodeIDs) {
        Node *potentialStartNode = player.GetPlayerNodeById(connectedId);
        if (potentialStartNode && potentialStartNode->isActionType()) {
          player.activeActionNodeId = potentialStartNode->id;
          float effectiveDuration = potentialStartNode->duration;
          for (int fromId : potentialStartNode->connectedFromNodeIDs) {
            Node *modNode = player.GetPlayerNodeById(fromId);
            if (modNode && modNode->isActive) {
              if (modNode->type == NodeType::POWER_DURATION_REDUCE)
                effectiveDuration += modNode->value;
              else if (potentialStartNode->type == NodeType::ACTION_SHIELD &&
                       modNode->type == NodeType::STAT_HEALTH)
                effectiveDuration += modNode->value / 50.0f;
            }
          }
          potentialStartNode->currentActiveTimer =
              fmaxf(0.1f, effectiveDuration);
          potentialStartNode->isCurrentlyActiveEffect = true;
          if (potentialStartNode->type == NodeType::ACTION_FIRE)
            FireActionBullet(player, bullets, *potentialStartNode);
          else if (potentialStartNode->type == NodeType::ACTION_SHIELD)
            player.playerShieldIsActive = true;
          break;
        }
      }
    }
  } else {
    Node *currentActionNode =
        player.GetPlayerNodeById(player.activeActionNodeId);
    if (currentActionNode) {
      if (currentActionNode->currentActiveTimer > 0) {
        currentActionNode->currentActiveTimer -= gameDt;
        if (currentActionNode->type == NodeType::ACTION_SHIELD)
          player.playerShieldIsActive = true;
      } else {
        currentActionNode->isCurrentlyActiveEffect = false;
        if (currentActionNode->type == NodeType::ACTION_SHIELD)
          player.playerShieldIsActive = false;
        int nextNodeInSequenceId = -1;
        for (int connectedId : currentActionNode->connectedToNodeIDs) {
          Node *potentialNextNode = player.GetPlayerNodeById(connectedId);
          if (potentialNextNode && potentialNextNode->isActionType()) {
            nextNodeInSequenceId = potentialNextNode->id;
            break;
          }
        }
        player.activeActionNodeId = (nextNodeInSequenceId == -1)
                                        ? currentActionNode->id
                                        : nextNodeInSequenceId;
        Node *newActionNode =
            player.GetPlayerNodeById(player.activeActionNodeId);
        if (newActionNode) {
          float effectiveDuration = newActionNode->duration;
          for (int fromId : newActionNode->connectedFromNodeIDs) {
            Node *modNode = player.GetPlayerNodeById(fromId);
            if (modNode && modNode->isActive) {
              if (modNode->type == NodeType::POWER_DURATION_REDUCE)
                effectiveDuration += modNode->value;
              else if (newActionNode->type == NodeType::ACTION_SHIELD &&
                       modNode->type == NodeType::STAT_HEALTH)
                effectiveDuration += modNode->value / 50.0f;
            }
          }
          newActionNode->currentActiveTimer = fmaxf(0.1f, effectiveDuration);
          newActionNode->isCurrentlyActiveEffect = true;
          if (newActionNode->type == NodeType::ACTION_FIRE)
            FireActionBullet(player, bullets, *newActionNode);
          else if (newActionNode->type == NodeType::ACTION_SHIELD)
            player.playerShieldIsActive = true;
        } else {
          player.activeActionNodeId = -1;
          player.playerShieldIsActive = false;
        }
      }
    } else {
      player.activeActionNodeId = -1;
      player.playerShieldIsActive = false;
    }
  }
  player.applyNodeEffects();

  for (int i = 0; i < MAX_BULLETS; i++) {
    if (bullets[i].active) {
      bullets[i].position = Vector2Add(
          bullets[i].position, Vector2Scale(bullets[i].velocity, gameDt));
      if (bullets[i].position.x < 0 || bullets[i].position.x > SCREEN_WIDTH ||
          bullets[i].position.y < 0 || bullets[i].position.y > SCREEN_HEIGHT) {
        bullets[i].active = false;
      }
    }
  }

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
                  CreateNodeFromTemplate(typesToDrop[GetRandomValue(
                      0, sizeof(typesToDrop) / sizeof(NodeType) - 1)]));
              break;
            }
          }
        }
      }
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
  for (int i = 0; i < MAX_BULLETS; i++)
    if (bullets[i].active)
      DrawCircleV(bullets[i].position, BULLET_RADIUS, bullets[i].color);
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
  DrawCircleV(player.position, PLAYER_SIZE,
              player.playerShieldIsActive ? SKYBLUE : LIME);
  if (player.playerShieldIsActive)
    DrawCircleLines((int)player.position.x, (int)player.position.y,
                    PLAYER_SIZE + 3, ColorAlpha(BLUE, 0.5f));

  DrawText(TextFormat("Health: %d/%d", player.currentHealth, player.maxHealth),
           10, 10, 20, RAYWHITE);
  DrawText(TextFormat("Speed: %.0f", player.currentSpeed), 10, 30, 20,
           RAYWHITE);
  DrawText(TextFormat("Damage: %d", player.currentDamage), 10, 50, 20,
           RAYWHITE);
  DrawText(TextFormat("Fire Rate CD: %.2fs", player.currentFireRate), 10, 70,
           20, RAYWHITE);

  if (player.activeActionNodeId != -1) {
    Node *activeNode = player.GetPlayerNodeById(player.activeActionNodeId);
    if (activeNode && activeNode->isCurrentlyActiveEffect) {
      const char *text =
          TextFormat("ACTION: %s (%.1fs)", activeNode->name.c_str(),
                     fmaxf(0.0f, activeNode->currentActiveTimer));
      int textWidth = MeasureText(text, 20);
      float gameAreaReferenceWidth = SCREEN_WIDTH;
      if (isPanelOpen)
        gameAreaReferenceWidth = SCREEN_WIDTH * 2.0f / 3.0f;
      DrawText(text, (int)(gameAreaReferenceWidth / 2.0f - textWidth / 2.0f),
               10, 20, activeNode->color);
    }
  }
  DrawText("TAB for Panel", 10, SCREEN_HEIGHT - 30, 20, RAYWHITE);
}

void UpdateNodeActivation() {
  for (Node &node : player.placedNodes) {
    if (node.type == NodeType::CPU_CORE)
      node.isActive = true;
    else if (node.isStatType() || node.isPowerType())
      node.isActive = false;
  }
  Node *cpuNode = player.GetPlayerNodeById(0);
  if (cpuNode && cpuNode->type == NodeType::CPU_CORE) {
    for (int connectedId : cpuNode->connectedToNodeIDs) {
      Node *targetNode = player.GetPlayerNodeById(connectedId);
      if (targetNode &&
          (targetNode->isStatType() || targetNode->isPowerType())) {
        targetNode->isActive = true;
      }
    }
  }
  player.applyNodeEffects();
}

void InitControlPanel() {
  draggingNodeIndex = -1;
  connectingNodeFromId = -1;
  inventoryScrollOffset = 0.0f;
  panelCamera.target = {0.0f, 0.0f};
  panelCamera.offset = {panelGridArea.x + panelGridArea.width / 2.0f,
                        panelGridArea.y + panelGridArea.height / 2.0f};
  panelCamera.rotation = 0.0f;
  panelCamera.zoom = 1.0f;
  UpdateNodeActivation();
}

void UpdateControlPanel(float dt) {
  Vector2 mousePosScreen = GetMousePosition();
  if (CheckCollisionPointRec(mousePosScreen, panelInventoryArea)) {
    inventoryScrollOffset -= GetMouseWheelMove() * NODE_INV_ITEM_HEIGHT * 0.5f;
    float maxScroll =
        fmaxf(0.0f, (player.inventoryNodes.size() * NODE_INV_ITEM_HEIGHT) -
                        panelInventoryArea.height);
    inventoryScrollOffset = Clamp(inventoryScrollOffset, 0.0f, maxScroll);
  } else if (CheckCollisionPointRec(mousePosScreen, panelGridArea)) {
    Vector2 mousePosWorldBeforeZoom =
        GetScreenToWorld2D(mousePosScreen, panelCamera);
    panelCamera.zoom += GetMouseWheelMove() * 0.05f * panelCamera.zoom;
    panelCamera.zoom = Clamp(panelCamera.zoom, 0.2f, 3.0f);
    Vector2 mousePosWorldAfterZoom =
        GetScreenToWorld2D(mousePosScreen, panelCamera);
    panelCamera.target =
        Vector2Add(panelCamera.target, Vector2Subtract(mousePosWorldBeforeZoom,
                                                       mousePosWorldAfterZoom));
  }
  Vector2 mousePosPanelGridWorld =
      GetScreenToWorld2D(mousePosScreen, panelCamera);

  if (CheckCollisionPointRec(mousePosScreen, panelArea)) {
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
      draggingNodeIndex = -1;
      if (CheckCollisionPointRec(mousePosScreen, panelInventoryArea)) {
        for (size_t i = 0; i < player.inventoryNodes.size(); ++i) {
          Rectangle nodeRect = {
              panelInventoryArea.x + 5,
              panelInventoryArea.y + 5 + i * NODE_INV_ITEM_HEIGHT -
                  inventoryScrollOffset,
              panelInventoryArea.width - 10, (float)NODE_UI_SIZE};
          if (CheckCollisionPointRec(mousePosScreen, nodeRect)) {
            draggingNodeIndex = (int)i;
            draggingFromInventory = true;
            break;
          }
        }
      }
      if (draggingNodeIndex == -1 &&
          CheckCollisionPointRec(mousePosScreen, panelGridArea)) {
        for (size_t i = 0; i < player.placedNodes.size(); ++i) {
          if (CheckCollisionPointCircle(mousePosPanelGridWorld,
                                        player.placedNodes[i].panelPosition,
                                        NODE_UI_SIZE / 2.0f)) {
            draggingNodeIndex = (int)i;
            draggingFromInventory = false;
            break;
          }
        }
      }
    }
    if (IsMouseButtonDown(MOUSE_LEFT_BUTTON) && draggingNodeIndex != -1 &&
        !draggingFromInventory) {
      player.placedNodes[draggingNodeIndex].panelPosition =
          mousePosPanelGridWorld;
    }
    if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON) && draggingNodeIndex != -1) {
      if (draggingFromInventory) {
        if (CheckCollisionPointRec(mousePosScreen, panelGridArea)) {
          Node nodeToPlace = player.inventoryNodes[draggingNodeIndex];
          nodeToPlace.isPlaced = true;
          nodeToPlace.panelPosition = mousePosPanelGridWorld;
          player.placedNodes.push_back(nodeToPlace);
          player.inventoryNodes.erase(player.inventoryNodes.begin() +
                                      draggingNodeIndex);
        }
      } else {
        if (CheckCollisionPointRec(mousePosScreen, panelInventoryArea) &&
            player.placedNodes[draggingNodeIndex].type != NodeType::CPU_CORE) {
          Node &nodeToRemoveRef = player.placedNodes[draggingNodeIndex];
          int removedNodeId = nodeToRemoveRef.id;
          for (Node &otherNode : player.placedNodes)
            if (otherNode.id != removedNodeId)
              otherNode.connectedToNodeIDs.erase(
                  std::remove(otherNode.connectedToNodeIDs.begin(),
                              otherNode.connectedToNodeIDs.end(),
                              removedNodeId),
                  otherNode.connectedToNodeIDs.end());
          for (int targetId : nodeToRemoveRef.connectedToNodeIDs)
            if (Node *target = player.GetPlayerNodeById(targetId))
              target->connectedFromNodeIDs.erase(
                  std::remove(target->connectedFromNodeIDs.begin(),
                              target->connectedFromNodeIDs.end(),
                              removedNodeId),
                  target->connectedFromNodeIDs.end());
          Node nodeToStoreCopy = nodeToRemoveRef;
          nodeToStoreCopy.isPlaced = false;
          nodeToStoreCopy.isActive = false;
          nodeToStoreCopy.isCurrentlyActiveEffect = false;
          nodeToStoreCopy.connectedToNodeIDs.clear();
          nodeToStoreCopy.connectedFromNodeIDs.clear();
          player.inventoryNodes.push_back(nodeToStoreCopy);
          player.placedNodes.erase(player.placedNodes.begin() +
                                   draggingNodeIndex);
          if (removedNodeId == player.activeActionNodeId) {
            player.activeActionNodeId = -1;
            player.playerShieldIsActive = false;
          }
        }
      }
      draggingNodeIndex = -1;
      UpdateNodeActivation();
      if (player.activeActionNodeId == -1 && isPanelOpen)
        player.applyNodeEffects();
    }

    Node *toNodeForConnection = nullptr;
    if (CheckCollisionPointRec(mousePosScreen, panelGridArea)) {
      for (Node &n_to : player.placedNodes) {
        if (CheckCollisionPointCircle(mousePosPanelGridWorld,
                                      n_to.panelPosition,
                                      NODE_UI_SIZE / 2.0f)) {
          toNodeForConnection = &n_to;
          break;
        }
      }
    }
    if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON)) {
      connectingNodeFromId = -1;
      if (toNodeForConnection) {
        connectingNodeFromId = toNodeForConnection->id;
      }
    }
    if (IsMouseButtonReleased(MOUSE_RIGHT_BUTTON)) {
      if (connectingNodeFromId != -1 && toNodeForConnection) {
        Node *fromNode = player.GetPlayerNodeById(connectingNodeFromId);
        if (fromNode && fromNode->id != toNodeForConnection->id) {
          bool connectionChanged = false;
          if (fromNode->type == NodeType::CPU_CORE) {
            if (toNodeForConnection->isStatType() ||
                toNodeForConnection->isPowerType() ||
                toNodeForConnection->isActionType()) {
              if (toNodeForConnection->isActionType()) {
                for (size_t i = 0; i < fromNode->connectedToNodeIDs.size();) {
                  Node *prev =
                      player.GetPlayerNodeById(fromNode->connectedToNodeIDs[i]);
                  if (prev && prev->isActionType() &&
                      prev->id != toNodeForConnection->id) {
                    prev->connectedFromNodeIDs.erase(
                        std::remove(prev->connectedFromNodeIDs.begin(),
                                    prev->connectedFromNodeIDs.end(),
                                    fromNode->id),
                        prev->connectedFromNodeIDs.end());
                    fromNode->connectedToNodeIDs.erase(
                        fromNode->connectedToNodeIDs.begin() + i);
                    connectionChanged = true;
                  } else
                    i++;
                }
              }
              if (std::find(fromNode->connectedToNodeIDs.begin(),
                            fromNode->connectedToNodeIDs.end(),
                            toNodeForConnection->id) ==
                  fromNode->connectedToNodeIDs.end()) {
                fromNode->connectedToNodeIDs.push_back(toNodeForConnection->id);
                toNodeForConnection->connectedFromNodeIDs.push_back(
                    fromNode->id);
                connectionChanged = true;
              }
            }
          } else if (fromNode->isActionType() &&
                     toNodeForConnection->isActionType()) {
            for (size_t i = 0; i < fromNode->connectedToNodeIDs.size();) {
              Node *prev =
                  player.GetPlayerNodeById(fromNode->connectedToNodeIDs[i]);
              if (prev && prev->isActionType() &&
                  prev->id != toNodeForConnection->id) {
                prev->connectedFromNodeIDs.erase(
                    std::remove(prev->connectedFromNodeIDs.begin(),
                                prev->connectedFromNodeIDs.end(), fromNode->id),
                    prev->connectedFromNodeIDs.end());
                fromNode->connectedToNodeIDs.erase(
                    fromNode->connectedToNodeIDs.begin() + i);
                connectionChanged = true;
              } else
                i++;
            }
            if (std::find(fromNode->connectedToNodeIDs.begin(),
                          fromNode->connectedToNodeIDs.end(),
                          toNodeForConnection->id) ==
                fromNode->connectedToNodeIDs.end()) {
              fromNode->connectedToNodeIDs.push_back(toNodeForConnection->id);
              toNodeForConnection->connectedFromNodeIDs.push_back(fromNode->id);
              connectionChanged = true;
            }
          } else if (fromNode->isStatType() &&
                     toNodeForConnection->isActionType()) {
            if (std::find(fromNode->connectedToNodeIDs.begin(),
                          fromNode->connectedToNodeIDs.end(),
                          toNodeForConnection->id) ==
                fromNode->connectedToNodeIDs.end()) {
              fromNode->connectedToNodeIDs.push_back(toNodeForConnection->id);
              toNodeForConnection->connectedFromNodeIDs.push_back(fromNode->id);
              connectionChanged = true;
            }
          } else if (fromNode->isPowerType() &&
                     (toNodeForConnection->isActionType() ||
                      toNodeForConnection->isStatType())) {
            for (size_t i = 0; i < fromNode->connectedToNodeIDs.size();) {
              Node *prevTarget =
                  player.GetPlayerNodeById(fromNode->connectedToNodeIDs[i]);
              if (prevTarget)
                prevTarget->connectedFromNodeIDs.erase(
                    std::remove(prevTarget->connectedFromNodeIDs.begin(),
                                prevTarget->connectedFromNodeIDs.end(),
                                fromNode->id),
                    prevTarget->connectedFromNodeIDs.end());
              fromNode->connectedToNodeIDs.erase(
                  fromNode->connectedToNodeIDs.begin() + i);
              connectionChanged = true;
            }
            fromNode->connectedToNodeIDs.push_back(toNodeForConnection->id);
            toNodeForConnection->connectedFromNodeIDs.push_back(fromNode->id);
            connectionChanged = true;
          }
          if (connectionChanged) {
            UpdateNodeActivation();
            player.activeActionNodeId = -1;
          }
        }
      }
      connectingNodeFromId = -1;
    }
  } else {
    if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON) && draggingNodeIndex != -1)
      draggingNodeIndex = -1;
    if (IsMouseButtonReleased(MOUSE_RIGHT_BUTTON) && connectingNodeFromId != -1)
      connectingNodeFromId = -1;
  }
}

void DrawControlPanel() {
  DrawRectangleRec(panelArea, ColorAlpha(BLACK, 0.85f));
  DrawRectangleLinesEx(panelArea, 1, DARKGRAY);
  DrawText("CONTROL PANEL (TAB)", (int)(panelArea.x + 10),
           (int)(panelArea.y + 10), 20, WHITE);

  DrawText("Inventory:", (int)panelInventoryArea.x,
           (int)(panelInventoryArea.y - 25), 15, WHITE);
  DrawRectangleRec(panelInventoryArea, ColorAlpha(DARKSLATEBLUE, 0.5f));
  BeginScissorMode((int)panelInventoryArea.x, (int)panelInventoryArea.y,
                   (int)panelInventoryArea.width,
                   (int)panelInventoryArea.height);
  for (size_t i = 0; i < player.inventoryNodes.size(); ++i) {
    Rectangle nodeRect = {panelInventoryArea.x + 5,
                          panelInventoryArea.y + 5 + i * NODE_INV_ITEM_HEIGHT -
                              inventoryScrollOffset,
                          panelInventoryArea.width - 10, (float)NODE_UI_SIZE};
    if (nodeRect.y + nodeRect.height > panelInventoryArea.y &&
        nodeRect.y < panelInventoryArea.y + panelInventoryArea.height) {
      bool isHovered = CheckCollisionPointRec(GetMousePosition(), nodeRect);
      DrawRectangleRec(nodeRect, player.inventoryNodes[i].color);
      DrawRectangleLinesEx(nodeRect, 2, isHovered ? YELLOW : DARKGRAY);
      DrawText(player.inventoryNodes[i].name.c_str(), (int)(nodeRect.x + 5),
               (int)(nodeRect.y + 5), 10, BLACK);
      if (isHovered)
        DrawText(player.inventoryNodes[i].description.c_str(),
                 (int)(GetMousePosition().x + 15),
                 (int)(GetMousePosition().y + 5), 10, WHITE);
    }
  }
  EndScissorMode();
  if (player.inventoryNodes.size() * NODE_INV_ITEM_HEIGHT >
      panelInventoryArea.height) {
    float contentHeight = player.inventoryNodes.size() * NODE_INV_ITEM_HEIGHT;
    float viewableRatio = panelInventoryArea.height / contentHeight;
    float scrollbarHeight = panelInventoryArea.height * viewableRatio;
    float scrollRatio = (contentHeight > panelInventoryArea.height &&
                         panelInventoryArea.height > 0)
                            ? (inventoryScrollOffset /
                               (contentHeight - panelInventoryArea.height))
                            : 0; // Avoid div by zero
    float scrollbarY =
        panelInventoryArea.y +
        scrollRatio * (panelInventoryArea.height - scrollbarHeight);
    DrawRectangle((int)(panelInventoryArea.x + panelInventoryArea.width - 8),
                  (int)scrollbarY, 5, (int)fmaxf(10.0f, scrollbarHeight),
                  LIGHTGRAY);
  }

  DrawText("System Grid:", (int)panelGridArea.x, (int)(panelGridArea.y - 25),
           15, WHITE);
  DrawRectangleRec(panelGridArea, ColorAlpha(MIDNIGHTBLUE, 0.5f));
  BeginScissorMode((int)panelGridArea.x, (int)panelGridArea.y,
                   (int)panelGridArea.width, (int)panelGridArea.height);
  BeginMode2D(panelCamera);
  for (const auto &fromNodeRef : player.placedNodes) {
    for (int targetNodeId : fromNodeRef.connectedToNodeIDs) {
      const Node *toNodePtr = player.GetPlayerNodeById(targetNodeId);
      if (toNodePtr) {
        Color lineColor = GRAY;
        bool isDirected = false;
        if (fromNodeRef.type == NodeType::CPU_CORE &&
            (toNodePtr->isStatType() || toNodePtr->isPowerType()))
          lineColor = YELLOW;
        else if (fromNodeRef.type == NodeType::CPU_CORE &&
                 toNodePtr->isActionType()) {
          lineColor = SKYBLUE;
          isDirected = true;
        } else if (fromNodeRef.isActionType() && toNodePtr->isActionType()) {
          lineColor = CYAN;
          isDirected = true;
        } else if (fromNodeRef.isStatType() && toNodePtr->isActionType()) {
          lineColor = STAT_ACTION_LINK_COLOR;
          isDirected = true;
        } else if (fromNodeRef.isPowerType() &&
                   (toNodePtr->isActionType() || toNodePtr->isStatType())) {
          lineColor = POWER_LINK_COLOR;
          isDirected = true;
        }

        DrawLineEx(fromNodeRef.panelPosition, toNodePtr->panelPosition,
                   fmaxf(1.0f, 2.0f / panelCamera.zoom), lineColor);
        if (isDirected) {
          Vector2 dir = Vector2Normalize(Vector2Subtract(
              toNodePtr->panelPosition, fromNodeRef.panelPosition));
          if (Vector2LengthSqr(dir) == 0)
            dir = {0, -1};
          float arrowHeadOffset =
              (NODE_UI_SIZE / 2.0f) + 2.0f / panelCamera.zoom;
          Vector2 arrowEnd = Vector2Subtract(
              toNodePtr->panelPosition, Vector2Scale(dir, arrowHeadOffset));
          Vector2 p1 = Vector2Subtract(
              arrowEnd, Vector2Scale(Vector2Rotate(dir, 30 * DEG2RAD),
                                     8.0f / panelCamera.zoom));
          Vector2 p2 = Vector2Subtract(
              arrowEnd, Vector2Scale(Vector2Rotate(dir, -30 * DEG2RAD),
                                     8.0f / panelCamera.zoom));
          DrawLineV(arrowEnd, p1, lineColor);
          DrawLineV(arrowEnd, p2, lineColor);
        }
      }
    }
  }
  if (connectingNodeFromId != -1) {
    Node *fromNodePtr = player.GetPlayerNodeById(connectingNodeFromId);
    if (fromNodePtr)
      DrawLineEx(fromNodePtr->panelPosition,
                 GetScreenToWorld2D(GetMousePosition(), panelCamera),
                 fmaxf(1.0f, 2.0f / panelCamera.zoom), LIGHTGRAY);
  }
  for (const auto &node : player.placedNodes) {
    Color nodeDrawColor = node.color;
    if (node.id == player.activeActionNodeId && node.isCurrentlyActiveEffect) {
      float pulse = (sinf(GetTime() * 5.0f) + 1.0f) / 2.0f;
      nodeDrawColor.r = (unsigned char)Clamp(node.color.r + pulse * 50, 0, 255);
      nodeDrawColor.g = (unsigned char)Clamp(node.color.g + pulse * 50, 0, 255);
      nodeDrawColor.b = (unsigned char)Clamp(node.color.b + pulse * 50, 0, 255);
    }
    DrawCircleV(node.panelPosition, NODE_UI_SIZE / 2.0f, nodeDrawColor);
    Color borderColor = DARKGRAY;
    if (node.isActive && (node.isStatType() || node.isPowerType() ||
                          node.type == NodeType::CPU_CORE))
      borderColor = YELLOW;
    else if (node.id == player.activeActionNodeId &&
             node.isCurrentlyActiveEffect)
      borderColor = WHITE;
    else if (node.isActionType() && !node.isCurrentlyActiveEffect &&
             player.activeActionNodeId == node.id)
      borderColor = LIGHTGRAY;
    DrawCircleLines((int)node.panelPosition.x, (int)node.panelPosition.y,
                    NODE_UI_SIZE / 2.0f, borderColor);
    float textSize = fmaxf(4.0f, 10.0f / panelCamera.zoom);
    Vector2 textSzMeasure =
        MeasureTextEx(GetFontDefault(), node.name.c_str(), textSize, 1.0f);
    DrawTextEx(GetFontDefault(), node.name.c_str(),
               {node.panelPosition.x - textSzMeasure.x / 2.0f,
                node.panelPosition.y - textSzMeasure.y / 2.0f},
               textSize, 1.0f, BLACK);
  }
  EndMode2D();
  EndScissorMode();

  if (draggingNodeIndex != -1 && draggingFromInventory) {
    const auto &node = player.inventoryNodes[draggingNodeIndex];
    DrawCircleV(GetMousePosition(), NODE_UI_SIZE / 2.0f,
                ColorAlpha(node.color, 0.7f));
    DrawText(
        node.name.c_str(),
        (int)(GetMousePosition().x - MeasureText(node.name.c_str(), 10) / 2.0f),
        (int)(GetMousePosition().y - 5), 10, BLACK);
  }
  if (CheckCollisionPointRec(GetMousePosition(), panelGridArea)) {
    Vector2 worldMouse = GetScreenToWorld2D(GetMousePosition(), panelCamera);
    for (const auto &node : player.placedNodes) {
      if (CheckCollisionPointCircle(worldMouse, node.panelPosition,
                                    NODE_UI_SIZE / 2.0f)) {
        int yOff = 5;
        Vector2 tooltipPos = {GetMousePosition().x + 10,
                              GetMousePosition().y + (float)yOff - 2};
        std::string descText = node.description;
        std::string line2Text =
            TextFormat("ID:%d Val:%.1f", node.id, node.value);
        std::string line3Text;
        if (node.isActionType())
          line3Text = TextFormat("Dur:%.1fs Eff:%s", node.duration,
                                 node.isCurrentlyActiveEffect ? "ON" : "OFF");
        else if (node.isStatType() || node.isPowerType())
          line3Text = TextFormat("Active:%s", node.isActive ? "YES" : "NO");

        float maxWidth = 0;
        maxWidth = fmaxf(maxWidth, (float)MeasureText(descText.c_str(), 10));
        maxWidth = fmaxf(maxWidth, (float)MeasureText(line2Text.c_str(), 10));
        if (!line3Text.empty())
          maxWidth = fmaxf(maxWidth, (float)MeasureText(line3Text.c_str(), 10));
        maxWidth = fmaxf(120.0f, maxWidth + 10); // Min width + Padding
        float tooltipHeight = 12 + (line3Text.empty() ? 0 : 12) + 12 + 4;

        DrawRectangle((int)tooltipPos.x, (int)tooltipPos.y, (int)maxWidth,
                      (int)tooltipHeight, ColorAlpha(BLACK, 0.8f));
        DrawText(descText.c_str(), (int)(tooltipPos.x + 5),
                 (int)(tooltipPos.y + 2), 10, WHITE);
        yOff += 12;
        DrawText(line2Text.c_str(), (int)(tooltipPos.x + 5),
                 (int)(tooltipPos.y + yOff - 3 + 2), 10, WHITE);
        yOff += 12;
        if (!line3Text.empty())
          DrawText(line3Text.c_str(), (int)(tooltipPos.x + 5),
                   (int)(tooltipPos.y + yOff - 3 + 2), 10, WHITE);
        break;
      }
    }
  }
}

void UpdateDrawFrame() {
  float dt = GetFrameTime();
  if (IsKeyPressed(KEY_TAB)) {
    isPanelOpen = !isPanelOpen;
    if (isPanelOpen)
      InitControlPanel();
    else
      player.applyNodeEffects();
  }
  if (currentGameState == GameState::GAMEPLAY) {
    UpdateGame(dt);
    if (isPanelOpen)
      UpdateControlPanel(dt);
  } else if (currentGameState == GameState::MAIN_MENU ||
             currentGameState == GameState::GAME_OVER) {
    if (IsKeyPressed(KEY_ENTER)) {
      InitGame();
      currentGameState = GameState::GAMEPLAY;
      isPanelOpen = false;
    }
  }

  BeginDrawing();
  ClearBackground(BLACK);
  if (currentGameState == GameState::GAMEPLAY) {
    DrawGame();
    if (isPanelOpen)
      DrawControlPanel();
  } else if (currentGameState == GameState::MAIN_MENU) {
    DrawText("ROBO ROGUELIKE: POWER NODES v2",
             (int)(SCREEN_WIDTH / 2.0f -
                   MeasureText("ROBO ROGUELIKE: POWER NODES v2", 30) / 2.0f),
             (int)(SCREEN_HEIGHT / 2.0f - 40), 30, WHITE);
    DrawText("Press ENTER to Start",
             (int)(SCREEN_WIDTH / 2.0f -
                   MeasureText("Press ENTER to Start", 20) / 2.0f),
             (int)(SCREEN_HEIGHT / 2.0f + 20), 20, LIGHTGRAY);
  } else if (currentGameState == GameState::GAME_OVER) {
    DrawGame();
    DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, ColorAlpha(BLACK, 0.7f));
    DrawText("GAME OVER",
             (int)(SCREEN_WIDTH / 2.0f - MeasureText("GAME OVER", 40) / 2.0f),
             (int)(SCREEN_HEIGHT / 2.0f - 40), 40, RED);
    DrawText("Press ENTER to Main Menu",
             (int)(SCREEN_WIDTH / 2.0f -
                   MeasureText("Press ENTER to Main Menu", 20) / 2.0f),
             (int)(SCREEN_HEIGHT / 2.0f + 20), 20, LIGHTGRAY);
  }
  EndDrawing();
}
