#include "Player.h"
#include <algorithm>

const int SCREEN_WIDTH = 1200;
const int SCREEN_HEIGHT = 700;

Player::Player() {
  position = {(float)SCREEN_WIDTH / 2.0f, (float)SCREEN_HEIGHT / 2.0f};
  baseSpeed = 200.0f;
  baseHealth = 100;
  baseFireRate = 0.5f;
  baseDamage = 10;
  activeActionNodeId = -1;
  playerShieldIsActive = false;
  fireCooldownTimer = 0.0f;
  resetStats();
}

BaseNode *Player::GetPlayerNodeById(int id) const {
  for (const auto &node : placedNodes) {
    if (node->id == id)
      return node.get();
  }
  return nullptr;
}

void Player::resetStats() {
  currentSpeed = baseSpeed;
  maxHealth = baseHealth;
  currentHealth = std::min(currentHealth, maxHealth);
  if (currentHealth <= 0 && baseHealth > 0)
    currentHealth = baseHealth;
  currentFireRate = baseFireRate;
  currentDamage = baseDamage;
}

void Player::applyNodeEffects() {
  resetStats();
  for (auto &node : placedNodes) {
    if (node->isActive && node->isStatType()) {
      float effectiveStatValue = node->value;
      for (int fromId : node->connectedFromNodeIDs) {
        BaseNode *powerNode = GetPlayerNodeById(fromId);
        if (powerNode && powerNode->getType() == NodeType::POWER_VALUE_ADD &&
            powerNode->isActive) {
          effectiveStatValue += powerNode->value;
        }
      }
      switch (node->getType()) {
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

  // Handle shift action effects
  if (activeActionNodeId != -1) {
    BaseNode *currentAction = GetPlayerNodeById(activeActionNodeId);
    if (currentAction && currentAction->getType() == NodeType::ACTION_SHIFT &&
        currentAction->isCurrentlyActiveEffect) {
      float shiftEffectiveValue = currentAction->value;
      for (int fromId : currentAction->connectedFromNodeIDs) {
        BaseNode *modNode = GetPlayerNodeById(fromId);
        if (modNode && modNode->isActive) {
          if (modNode->getType() == NodeType::POWER_VALUE_ADD) {
            shiftEffectiveValue += modNode->value;
          } else if (modNode->getType() == NodeType::STAT_SPEED) {
            shiftEffectiveValue += modNode->value;
          }
        }
      }
      currentSpeed += shiftEffectiveValue;
    }
  }

  if (currentHealth > maxHealth)
    currentHealth = maxHealth;
}

std::vector<BaseNode *> Player::GetInventoryNodePtrs() const {
  std::vector<BaseNode *> ptrs;
  for (const auto &node : inventoryNodes) {
    ptrs.push_back(node.get());
  }
  return ptrs;
}

std::vector<BaseNode *> Player::GetPlacedNodePtrs() const {
  std::vector<BaseNode *> ptrs;
  for (const auto &node : placedNodes) {
    ptrs.push_back(node.get());
  }
  return ptrs;
}
