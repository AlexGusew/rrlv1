#pragma once
#include "raylib.h"
#include "BaseNode.h"
#include <vector>
#include <memory>

class Player {
public:
  Vector2 position;
  float baseSpeed, currentSpeed;
  int baseHealth, currentHealth, maxHealth;
  float baseFireRate, currentFireRate;
  float fireCooldownTimer;
  int baseDamage, currentDamage;

  std::vector<std::unique_ptr<BaseNode>> inventoryNodes;
  std::vector<std::unique_ptr<BaseNode>> placedNodes;

  int activeActionNodeId;
  bool playerShieldIsActive;

  Player();

  BaseNode* GetPlayerNodeById(int id) const;
  void resetStats();
  void applyNodeEffects();

  // Helper methods for getting raw pointers for the UI classes
  std::vector<BaseNode*> GetInventoryNodePtrs() const;
  std::vector<BaseNode*> GetPlacedNodePtrs() const;
};