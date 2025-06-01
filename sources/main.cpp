// main.cpp
#include "raylib.h"
#include <vector>
#include <string>
#include <cmath>
#include <map>
#include <algorithm> // For std::remove_if, std::find_if

//------------------------------------------------------------------------------------
// Constants
//------------------------------------------------------------------------------------
const int SCREEN_WIDTH = 1000;
const int SCREEN_HEIGHT = 700;
const int PLAYER_SIZE = 20;
const int ENEMY_SIZE = 25;
const int BULLET_SPEED = 800;
const int BULLET_RADIUS = 5;
const int MAX_BULLETS = 50;
const int MAX_ENEMIES = 10;
const int NODE_UI_SIZE = 50; // Visual size of a node in the panel

//------------------------------------------------------------------------------------
// Enums and Structs
//------------------------------------------------------------------------------------
enum class GameState {
    MAIN_MENU,
    GAMEPLAY,
    CONTROL_PANEL,
    GAME_OVER
};

enum class NodeType {
    NONE,
    CPU_CORE, // Central connection point
    STAT_HEALTH,
    STAT_SPEED,
    STAT_DAMAGE,
    STAT_FIRE_RATE
    // Add more types: PIERCE_SHOTS, MULTI_SHOT, SHIELD etc.
};

struct Node {
    int id;
    NodeType type;
    std::string name;
    std::string description;
    float value; // e.g., +10 health, +50 speed, -0.1 fire rate cooldown
    Color color;

    Vector2 panelPosition; // Position on the control panel screen
    bool isPlaced;         // Is it on the panel grid?
    bool isActive;         // Is it powered and providing benefits?
    std::vector<int> connectedToNodeIDs; // IDs of nodes this one is connected TO (for CPU_CORE)
    std::vector<int> connectedFromNodeIDs; // IDs of nodes connected FROM this one (for other nodes)

    Node(int _id = -1, NodeType _type = NodeType::NONE, std::string _name = "N/A", float _val = 0.0f)
        : id(_id), type(_type), name(_name), value(_val), isPlaced(false), isActive(false) {
        switch (type) {
            case NodeType::CPU_CORE: color = GOLD; description="Central Processing Unit. Connect other nodes to it."; break;
            case NodeType::STAT_HEALTH: color = GREEN; description="Increases max health."; break;
            case NodeType::STAT_SPEED: color = BLUE; description="Increases movement speed."; break;
            case NodeType::STAT_DAMAGE: color = RED; description="Increases bullet damage."; break;
            case NodeType::STAT_FIRE_RATE: color = SKYBLUE; description="Increases fire rate."; break;
            default: color = GRAY; break;
        }
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
    float baseFireRate, currentFireRate; // Lower is faster (cooldown)
    float fireCooldownTimer;
    int baseDamage, currentDamage;

    std::vector<Node> inventoryNodes; // Nodes player has collected but not placed
    std::vector<Node> placedNodes;    // Nodes on the control panel

    Player() {
        position = { (float)SCREEN_WIDTH / 2, (float)SCREEN_HEIGHT / 2 };
        baseSpeed = 200.0f;
        baseHealth = 100;
        baseFireRate = 0.5f; // 2 shots per second
        baseDamage = 10;
        resetStats();
    }

    void resetStats() {
        currentSpeed = baseSpeed;
        maxHealth = baseHealth;
        // currentHealth should not reset to max unless specified (e.g. new level)
        // but ensure it's not above new maxHealth
        currentHealth = std::min(currentHealth, maxHealth);
        if (currentHealth <= 0) currentHealth = baseHealth; // Initial setup
        currentFireRate = baseFireRate;
        currentDamage = baseDamage;
    }

    void applyNodeEffects() {
        resetStats(); // Reset to base before applying
        for (Node& node : placedNodes) {
            if (node.isActive) {
                switch (node.type) {
                    case NodeType::STAT_HEALTH:
                        maxHealth += (int)node.value;
                        currentHealth += (int)node.value; // Can also heal
                        break;
                    case NodeType::STAT_SPEED:
                        currentSpeed += node.value;
                        break;
                    case NodeType::STAT_DAMAGE:
                        currentDamage += (int)node.value;
                        break;
                    case NodeType::STAT_FIRE_RATE:
                        currentFireRate -= node.value; // Reduce cooldown
                        if (currentFireRate < 0.05f) currentFireRate = 0.05f; // Cap
                        break;
                    default: break;
                }
            }
        }
        currentHealth = std::min(currentHealth, maxHealth); // Ensure health doesn't exceed new max
    }
};

//------------------------------------------------------------------------------------
// Global Variables
//------------------------------------------------------------------------------------
GameState currentGameState = GameState::MAIN_MENU;
Player player;
Bullet bullets[MAX_BULLETS];
Enemy enemies[MAX_ENEMIES];
int nextNodeId = 0; // For unique node IDs

// Control Panel State
int draggingNodeIndex = -1; // Index in inventoryNodes or placedNodes
bool draggingFromInventory = false;
int connectingNodeFromId = -1; // ID of the node we are starting a connection from
Vector2 mouseUIPosition = {0,0}; // Mouse position relative to UI panel origin

// Node templates
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
void UpdateNodeActivation(); // Crucial for making connections work

Node CreateNodeFromTemplate(NodeType type);
void SpawnEnemy();

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main() {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Robo-Roguelike");
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
void InitNodeTemplates() {
    nodeTemplates[NodeType::CPU_CORE] = Node(0, NodeType::CPU_CORE, "CPU Core", 0);
    nodeTemplates[NodeType::STAT_HEALTH] = Node(0, NodeType::STAT_HEALTH, "Health Chip", 25);
    nodeTemplates[NodeType::STAT_SPEED] = Node(0, NodeType::STAT_SPEED, "Speed Chip", 50);
    nodeTemplates[NodeType::STAT_DAMAGE] = Node(0, NodeType::STAT_DAMAGE, "Damage Chip", 5);
    nodeTemplates[NodeType::STAT_FIRE_RATE] = Node(0, NodeType::STAT_FIRE_RATE, "FireRate Chip", 0.1f); // reduces cooldown
}

Node CreateNodeFromTemplate(NodeType type) {
    if (nodeTemplates.count(type)) {
        Node newNode = nodeTemplates[type]; // Copy template
        newNode.id = nextNodeId++;          // Assign unique ID
        return newNode;
    }
    return Node(nextNodeId++, NodeType::NONE, "ERROR", 0); // Should not happen
}


void InitGame() {
    InitNodeTemplates();
    player = Player(); // Re-initialize player

    // Add a CPU Core to placed nodes by default
    Node coreNode = CreateNodeFromTemplate(NodeType::CPU_CORE);
    coreNode.panelPosition = {(float)SCREEN_WIDTH / 2 - 100, (float)SCREEN_HEIGHT / 2};
    coreNode.isPlaced = true;
    coreNode.isActive = true; // Core is always active if placed
    player.placedNodes.push_back(coreNode);


    // Give player some starting nodes in inventory
    player.inventoryNodes.push_back(CreateNodeFromTemplate(NodeType::STAT_HEALTH));
    player.inventoryNodes.push_back(CreateNodeFromTemplate(NodeType::STAT_SPEED));
    player.inventoryNodes.push_back(CreateNodeFromTemplate(NodeType::STAT_DAMAGE));


    for (int i = 0; i < MAX_BULLETS; i++) bullets[i].active = false;
    for (int i = 0; i < MAX_ENEMIES; i++) enemies[i].active = false;

    SpawnEnemy();
    SpawnEnemy();

    player.applyNodeEffects(); // Apply effects from any initially active nodes
    player.currentHealth = player.maxHealth; // Start with full health
    currentGameState = GameState::MAIN_MENU;
}

void SpawnEnemy() {
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (!enemies[i].active) {
            enemies[i].active = true;
            enemies[i].position = {(float)GetRandomValue(50, SCREEN_WIDTH - 50), (float)GetRandomValue(50, 150)};
            enemies[i].health = 30;
            enemies[i].speed = 100.0f;
            enemies[i].shootCooldown = 2.0f;
            enemies[i].currentShootTimer = (float)GetRandomValue(0, 200)/100.0f; // Stagger shooting
            return;
        }
    }
}

void UpdateGame(float dt) {
    // Player movement
    if (IsKeyDown(KEY_W)) player.position.y -= player.currentSpeed * dt;
    if (IsKeyDown(KEY_S)) player.position.y += player.currentSpeed * dt;
    if (IsKeyDown(KEY_A)) player.position.x -= player.currentSpeed * dt;
    if (IsKeyDown(KEY_D)) player.position.x += player.currentSpeed * dt;

    // Keep player on screen
    if (player.position.x - PLAYER_SIZE < 0) player.position.x = PLAYER_SIZE;
    if (player.position.x + PLAYER_SIZE > SCREEN_WIDTH) player.position.x = SCREEN_WIDTH - PLAYER_SIZE;
    if (player.position.y - PLAYER_SIZE < 0) player.position.y = PLAYER_SIZE;
    if (player.position.y + PLAYER_SIZE > SCREEN_HEIGHT) player.position.y = SCREEN_HEIGHT - PLAYER_SIZE;


    // Player shooting
    player.fireCooldownTimer -= dt;
    if (IsMouseButtonDown(MOUSE_LEFT_BUTTON) && player.fireCooldownTimer <= 0) {
        for (int i = 0; i < MAX_BULLETS; i++) {
            if (!bullets[i].active) {
                bullets[i].active = true;
                bullets[i].fromPlayer = true;
                bullets[i].position = player.position;
                Vector2 mousePos = GetMousePosition();
                Vector2 direction = {mousePos.x - player.position.x, mousePos.y - player.position.y};
                float length = sqrt(direction.x * direction.x + direction.y * direction.y);
                if (length > 0) { // Avoid division by zero
                    direction.x /= length;
                    direction.y /= length;
                }
                bullets[i].velocity = {direction.x * BULLET_SPEED, direction.y * BULLET_SPEED};
                bullets[i].color = YELLOW;
                bullets[i].damage = player.currentDamage;
                player.fireCooldownTimer = player.currentFireRate;
                break;
            }
        }
    }

    // Bullets update
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (bullets[i].active) {
            bullets[i].position.x += bullets[i].velocity.x * dt;
            bullets[i].position.y += bullets[i].velocity.y * dt;

            // Deactivate bullets if off-screen
            if (bullets[i].position.x < 0 || bullets[i].position.x > SCREEN_WIDTH ||
                bullets[i].position.y < 0 || bullets[i].position.y > SCREEN_HEIGHT) {
                bullets[i].active = false;
            }
        }
    }

    // Enemies update
    int activeEnemies = 0;
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (enemies[i].active) {
            activeEnemies++;
            // Simple AI: move towards player
            Vector2 direction = {player.position.x - enemies[i].position.x, player.position.y - enemies[i].position.y};
            float length = sqrt(direction.x * direction.x + direction.y * direction.y);
            if (length > PLAYER_SIZE + ENEMY_SIZE) { // Don't move if very close
                 if (length > 0) {
                    direction.x /= length;
                    direction.y /= length;
                }
                enemies[i].position.x += direction.x * enemies[i].speed * dt;
                enemies[i].position.y += direction.y * enemies[i].speed * dt;
            }


            // Collision: Player bullets with enemies
            for (int b = 0; b < MAX_BULLETS; b++) {
                if (bullets[b].active && bullets[b].fromPlayer) {
                    if (CheckCollisionCircles(enemies[i].position, ENEMY_SIZE, bullets[b].position, BULLET_RADIUS)) {
                        enemies[i].health -= bullets[b].damage;
                        bullets[b].active = false; // Bullet hits
                        if (enemies[i].health <= 0) {
                            enemies[i].active = false;
                            // Enemy drops a random node
                            int randNode = GetRandomValue(1, 4); // Assuming CPU_CORE is 0, other stats are 1-4
                            NodeType typeToDrop = NodeType::NONE;
                            switch(randNode) {
                                case 1: typeToDrop = NodeType::STAT_HEALTH; break;
                                case 2: typeToDrop = NodeType::STAT_SPEED; break;
                                case 3: typeToDrop = NodeType::STAT_DAMAGE; break;
                                case 4: typeToDrop = NodeType::STAT_FIRE_RATE; break;
                            }
                            if(typeToDrop != NodeType::NONE) {
                                player.inventoryNodes.push_back(CreateNodeFromTemplate(typeToDrop));
                            }
                            break; // Stop checking bullets for this dead enemy
                        }
                    }
                }
            }
             // Collision: Enemy with Player
            if (CheckCollisionCircles(player.position, PLAYER_SIZE, enemies[i].position, ENEMY_SIZE)) {
                player.currentHealth -= 10; // Simple damage
                // Add a small knockback or invulnerability window later
                if (player.currentHealth <= 0) {
                    currentGameState = GameState::GAME_OVER;
                }
            }
        }
    }
    if (activeEnemies == 0) {
        // Spawn new wave or go to next level (for now, just spawn more)
        SpawnEnemy(); SpawnEnemy(); SpawnEnemy();
    }


    // Open Control Panel
    if (IsKeyPressed(KEY_C)) {
        currentGameState = GameState::CONTROL_PANEL;
        // When opening panel, ensure panel state is reset
        draggingNodeIndex = -1;
        connectingNodeFromId = -1;
        InitControlPanel(); // Recalculate node activation on open
    }
}

void DrawGame() {
    ClearBackground(DARKGRAY);

    // Draw Bullets
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (bullets[i].active) {
            DrawCircleV(bullets[i].position, BULLET_RADIUS, bullets[i].color);
        }
    }

    // Draw Enemies
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (enemies[i].active) {
            DrawCircleV(enemies[i].position, ENEMY_SIZE, MAROON);
            DrawText(TextFormat("%d", enemies[i].health), enemies[i].position.x - 10, enemies[i].position.y - ENEMY_SIZE -15, 10, WHITE);
        }
    }

    // Draw Player
    DrawCircleV(player.position, PLAYER_SIZE, LIME);

    // Draw UI
    DrawText(TextFormat("Health: %d/%d", player.currentHealth, player.maxHealth), 10, 10, 20, RAYWHITE);
    DrawText(TextFormat("Speed: %.0f", player.currentSpeed), 10, 30, 20, RAYWHITE);
    DrawText(TextFormat("Damage: %d", player.currentDamage), 10, 50, 20, RAYWHITE);
    DrawText(TextFormat("Fire Rate CD: %.2fs", player.currentFireRate), 10, 70, 20, RAYWHITE);
    DrawText("Press C for Control Panel", 10, SCREEN_HEIGHT - 30, 20, RAYWHITE);
    DrawText(TextFormat("Nodes in Inv: %zu", player.inventoryNodes.size()), SCREEN_WIDTH - 250, 10, 20, RAYWHITE);

}


// Traverses connections from the CPU_CORE to activate nodes
void UpdateNodeActivation() {
    // 1. Deactivate all non-CPU nodes first
    for (Node& node : player.placedNodes) {
        if (node.type != NodeType::CPU_CORE) {
            node.isActive = false;
        } else {
            node.isActive = true; // CPU Core is always active if placed
        }
    }

    // 2. Activate nodes connected to the CPU_CORE
    // This is a simple one-level activation. For multi-level, you'd need a graph traversal (BFS/DFS)
    Node* cpuNode = nullptr;
    for (Node& node : player.placedNodes) {
        if (node.type == NodeType::CPU_CORE) {
            cpuNode = &node;
            break;
        }
    }

    if (cpuNode) {
        for (int connectedId : cpuNode->connectedToNodeIDs) {
            for (Node& targetNode : player.placedNodes) {
                if (targetNode.id == connectedId) {
                    targetNode.isActive = true;
                    break; 
                }
            }
        }
    }
    player.applyNodeEffects(); // Re-calculate player stats based on new active nodes
}


void InitControlPanel() {
    // This function is called when entering the control panel state.
    // Reset any temporary panel interaction states.
    draggingNodeIndex = -1;
    connectingNodeFromId = -1;
    UpdateNodeActivation(); // Important to check activation status
}

void UpdateControlPanel(float dt) {
    Vector2 mousePos = GetMousePosition();

    if (IsKeyPressed(KEY_C) || IsKeyPressed(KEY_ESCAPE)) {
        currentGameState = GameState::GAMEPLAY;
        draggingNodeIndex = -1; // Ensure no lingering drag state
        connectingNodeFromId = -1;
        player.applyNodeEffects(); // Apply changes before returning to game
        return;
    }

    // --- Node Dragging Logic ---
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        // Check inventory nodes
        for (int i = 0; i < player.inventoryNodes.size(); ++i) {
            Rectangle nodeRect = { 20.0f, 50.0f + i * (NODE_UI_SIZE + 10), (float)NODE_UI_SIZE * 2, (float)NODE_UI_SIZE };
            if (CheckCollisionPointRec(mousePos, nodeRect)) {
                draggingNodeIndex = i;
                draggingFromInventory = true;
                break;
            }
        }
        // Check placed nodes if not dragging from inventory
        if (draggingNodeIndex == -1) {
            for (int i = 0; i < player.placedNodes.size(); ++i) {
                if (CheckCollisionPointCircle(mousePos, player.placedNodes[i].panelPosition, NODE_UI_SIZE / 2.0f)) {
                    draggingNodeIndex = i;
                    draggingFromInventory = false;
                    break;
                }
            }
        }
    }

    if (IsMouseButtonDown(MOUSE_LEFT_BUTTON) && draggingNodeIndex != -1) {
        if (draggingFromInventory) {
            // Node being dragged from inventory, just visually follow mouse
        } else {
            player.placedNodes[draggingNodeIndex].panelPosition = mousePos;
        }
    }

    if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON) && draggingNodeIndex != -1) {
        if (draggingFromInventory) {
            // Try to place node from inventory onto panel
            // For simplicity, just add to placedNodes and remove from inventory
            Node nodeToPlace = player.inventoryNodes[draggingNodeIndex];
            nodeToPlace.isPlaced = true;
            nodeToPlace.panelPosition = mousePos;
            player.placedNodes.push_back(nodeToPlace);
            player.inventoryNodes.erase(player.inventoryNodes.begin() + draggingNodeIndex);
        } else {
            // Node was already placed, just update its position (already done by dragging)
            // Potentially check if dropped back into inventory area
             Rectangle inventoryArea = {0, 0, 200, (float)SCREEN_HEIGHT};
             if (CheckCollisionPointRec(mousePos, inventoryArea)) {
                 // If not CPU core, move back to inventory
                 if(player.placedNodes[draggingNodeIndex].type != NodeType::CPU_CORE){
                    Node nodeToStore = player.placedNodes[draggingNodeIndex];
                    nodeToStore.isPlaced = false;
                    nodeToStore.isActive = false; // Deactivate when moved to inventory
                    // Clear its connections
                    for(Node& otherNode : player.placedNodes) {
                        otherNode.connectedToNodeIDs.erase(std::remove(otherNode.connectedToNodeIDs.begin(), otherNode.connectedToNodeIDs.end(), nodeToStore.id), otherNode.connectedToNodeIDs.end());
                    }
                    nodeToStore.connectedToNodeIDs.clear();
                    nodeToStore.connectedFromNodeIDs.clear();

                    player.inventoryNodes.push_back(nodeToStore);
                    player.placedNodes.erase(player.placedNodes.begin() + draggingNodeIndex);
                 } else {
                    // Snap CPU core back if dragged to inventory (or disallow)
                    player.placedNodes[draggingNodeIndex].panelPosition = {(float)SCREEN_WIDTH / 2 - 100, (float)SCREEN_HEIGHT / 2}; // Default pos
                 }
             }
        }
        draggingNodeIndex = -1;
        UpdateNodeActivation(); // Recalculate active nodes after any move
    }


    // --- Node Connecting Logic (Simplified: Connect TO CPU Core) ---
    if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON)) {
        // Find if clicked on a placed node's "connector" (center for now)
        for (const auto& node : player.placedNodes) {
            if (CheckCollisionPointCircle(mousePos, node.panelPosition, NODE_UI_SIZE / 2.0f)) {
                if (connectingNodeFromId == -1) { // Start a new connection
                    // We only allow connecting FROM CPU_CORE TO other nodes in this simplified model
                    if (node.type == NodeType::CPU_CORE) {
                        connectingNodeFromId = node.id;
                    } else {
                        // Or from another node TO CPU_CORE (bidirectional visually but logic is CPU->Other)
                        // For now, let's keep it simple: only CPU can initiate visually
                        // Or, if clicking a non-CPU node, try to connect IT TO the CPU
                        Node* cpuNode = nullptr;
                        for(Node& n : player.placedNodes) if(n.type == NodeType::CPU_CORE) cpuNode = &n;

                        if (cpuNode) {
                            // Check if already connected
                            bool alreadyConnected = false;
                            for(int connectedId : cpuNode->connectedToNodeIDs) {
                                if (connectedId == node.id) {
                                    alreadyConnected = true;
                                    // If already connected, remove connection
                                    cpuNode->connectedToNodeIDs.erase(std::remove(cpuNode->connectedToNodeIDs.begin(), cpuNode->connectedToNodeIDs.end(), node.id), cpuNode->connectedToNodeIDs.end());
                                    // Also remove from the target node's perspective if you track that
                                    // For Node A (CPU) connected TO Node B:
                                    // NodeB.connectedFromNodeIDs.erase(NodeA.id)
                                    break;
                                }
                            }
                            if (!alreadyConnected) {
                                cpuNode->connectedToNodeIDs.push_back(node.id);
                                // For Node A (CPU) connected TO Node B:
                                // NodeB.connectedFromNodeIDs.push_back(NodeA.id)
                            }
                            UpdateNodeActivation();
                        }
                    }
                } else { // Complete a connection
                    Node* fromNode = nullptr;
                    for (Node& n : player.placedNodes) if (n.id == connectingNodeFromId) fromNode = &n;
                    
                    if (fromNode && fromNode->id != node.id) { // Cannot connect to self
                        // Add connection if not already present
                        if (std::find(fromNode->connectedToNodeIDs.begin(), fromNode->connectedToNodeIDs.end(), node.id) == fromNode->connectedToNodeIDs.end()) {
                            fromNode->connectedToNodeIDs.push_back(node.id);
                            // If you want to track incoming connections on 'node':
                            // node.connectedFromNodeIDs.push_back(fromNode->id);
                        }
                    }
                    connectingNodeFromId = -1; // Reset connection state
                    UpdateNodeActivation();
                }
                break; // Found a node, stop checking
            }
        }
        // If clicked on empty space and was connecting, cancel connection
        if (connectingNodeFromId != -1 && !IsMouseButtonDown(MOUSE_RIGHT_BUTTON)) { // Check if a node was actually clicked
            bool clickedOnNode = false;
             for (const auto& node : player.placedNodes) {
                if (CheckCollisionPointCircle(mousePos, node.panelPosition, NODE_UI_SIZE / 2.0f)) {
                    clickedOnNode = true; break;
                }
            }
            if (!clickedOnNode) connectingNodeFromId = -1;
        }
    }
}


void DrawControlPanel() {
    ClearBackground(DARKBLUE);
    DrawText("CONTROL PANEL (Robot Configuration)", SCREEN_WIDTH / 2 - MeasureText("CONTROL PANEL (Robot Configuration)", 30)/2, 10, 30, WHITE);
    DrawText("Press C or ESC to return to Game", 10, SCREEN_HEIGHT - 30, 20, WHITE);
    DrawText("Left-Click: Drag nodes. Right-Click on Node: Toggle connection to/from CPU.", 10, SCREEN_HEIGHT - 60, 15, LIGHTGRAY);


    // --- Draw Inventory Nodes ---
    DrawText("Inventory:", 20, 20, 20, WHITE);
    for (int i = 0; i < player.inventoryNodes.size(); ++i) {
        Rectangle nodeRect = { 20.0f, 50.0f + i * (NODE_UI_SIZE + 10), (float)NODE_UI_SIZE * 2, (float)NODE_UI_SIZE };
        bool isHovered = CheckCollisionPointRec(GetMousePosition(), nodeRect);
        DrawRectangleRec(nodeRect, player.inventoryNodes[i].color);
        DrawRectangleLinesEx(nodeRect, 2, isHovered ? YELLOW : DARKGRAY);
        DrawText(player.inventoryNodes[i].name.c_str(), nodeRect.x + 5, nodeRect.y + 5, 10, BLACK);
        if (isHovered) {
             DrawText(player.inventoryNodes[i].description.c_str(), GetMousePosition().x +15, GetMousePosition().y+5, 10, WHITE);
        }
    }

    // --- Draw Placed Nodes and Connections ---
    DrawText("System Grid:", 250, 20, 20, WHITE);
    // Draw connections first so nodes are on top
    Node* cpuNodePtr = nullptr;
    for(Node& n : player.placedNodes) if(n.type == NodeType::CPU_CORE) cpuNodePtr = &n;

    if (cpuNodePtr) {
        for (int targetNodeId : cpuNodePtr->connectedToNodeIDs) {
            Node* targetNodePtr = nullptr;
            for (Node& tn : player.placedNodes) if (tn.id == targetNodeId) targetNodePtr = &tn;
            
            if (targetNodePtr) {
                DrawLineEx(cpuNodePtr->panelPosition, targetNodePtr->panelPosition, 3.0f, YELLOW);
            }
        }
    }
    // If currently making a connection from CPU
    if (connectingNodeFromId != -1) {
        Node* fromNode = nullptr;
        for(Node& n : player.placedNodes) if(n.id == connectingNodeFromId) fromNode = &n;
        if(fromNode) DrawLineEx(fromNode->panelPosition, GetMousePosition(), 2.0f, LIGHTGRAY);
    }


    for (int i = 0; i < player.placedNodes.size(); ++i) {
        const auto& node = player.placedNodes[i];
        bool isHovered = CheckCollisionPointCircle(GetMousePosition(), node.panelPosition, NODE_UI_SIZE / 2.0f);

        DrawCircleV(node.panelPosition, NODE_UI_SIZE / 2.0f, node.color);
        DrawCircleLines(node.panelPosition.x, node.panelPosition.y, NODE_UI_SIZE / 2.0f, node.isActive ? YELLOW : DARKGRAY);
        if (isHovered) DrawCircleLines(node.panelPosition.x, node.panelPosition.y, NODE_UI_SIZE / 2.0f + 2, WHITE);

        DrawText(node.name.c_str(), node.panelPosition.x - MeasureText(node.name.c_str(), 10)/2, node.panelPosition.y - 5, 10, BLACK);
         if (isHovered) {
             DrawText(node.description.c_str(), GetMousePosition().x +15, GetMousePosition().y+5, 10, WHITE);
             DrawText(TextFormat("Active: %s", node.isActive ? "YES" : "NO"), GetMousePosition().x +15, GetMousePosition().y+20, 10, node.isActive ? GREEN : RED);
        }
    }

    // --- Draw Node being dragged from inventory ---
    if (draggingNodeIndex != -1 && draggingFromInventory) {
        const auto& node = player.inventoryNodes[draggingNodeIndex];
        DrawCircleV(GetMousePosition(), NODE_UI_SIZE / 2.0f, ColorAlpha(node.color, 0.7f));
        DrawText(node.name.c_str(), GetMousePosition().x - MeasureText(node.name.c_str(), 10)/2, GetMousePosition().y - 5, 10, BLACK);
    }
}


void UpdateDrawFrame() {
    float dt = GetFrameTime();

    switch (currentGameState) {
        case GameState::MAIN_MENU:
            if (IsKeyPressed(KEY_ENTER)) {
                InitGame(); // Reset game state for a new game
                currentGameState = GameState::GAMEPLAY;
            }
            BeginDrawing();
            ClearBackground(BLACK);
            DrawText("ROBO ROGUELIKE", SCREEN_WIDTH / 2 - MeasureText("ROBO ROGUELIKE", 40) / 2, SCREEN_HEIGHT / 2 - 40, 40, WHITE);
            DrawText("Press ENTER to Start", SCREEN_WIDTH / 2 - MeasureText("Press ENTER to Start", 20) / 2, SCREEN_HEIGHT / 2 + 20, 20, LIGHTGRAY);
            EndDrawing();
            break;

        case GameState::GAMEPLAY:
            UpdateGame(dt);
            BeginDrawing();
            DrawGame();
            EndDrawing();
            break;

        case GameState::CONTROL_PANEL:
            UpdateControlPanel(dt);
            BeginDrawing();
            DrawControlPanel();
            EndDrawing();
            break;

        case GameState::GAME_OVER:
            if (IsKeyPressed(KEY_ENTER)) {
                currentGameState = GameState::MAIN_MENU;
            }
            BeginDrawing();
            ClearBackground(BLACK);
            DrawText("GAME OVER", SCREEN_WIDTH / 2 - MeasureText("GAME OVER", 40) / 2, SCREEN_HEIGHT / 2 - 40, 40, RED);
            DrawText("Press ENTER to return to Main Menu", SCREEN_WIDTH / 2 - MeasureText("Press ENTER to return to Main Menu", 20) / 2, SCREEN_HEIGHT / 2 + 20, 20, LIGHTGRAY);
            EndDrawing();
            break;
    }
}
