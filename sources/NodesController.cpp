#include "NodesController.h"
#include "BaseNode.h"
#include "NodeTypes.h"
#include "raymath.h"
#include <cmath>

struct Bullet {
  Vector2 position;
  Vector2 velocity;
  bool active;
  Color color;
  int damage;
  bool fromPlayer;
};

const int BULLET_SPEED = 800;
const int MAX_BULLETS = 50;

NodesController::NodesController() : nextNodeId(0) {}

void NodesController::Initialize() {
  InitNodeTemplates();
  nextNodeId = 0;
}

void NodesController::InitNodeTemplates() {
  nodeTemplates.clear();
  nodeTemplates[NodeType::CPU_CORE] = std::make_unique<CPUCoreNode>(0);
  nodeTemplates[NodeType::STAT_HEALTH] = std::make_unique<HealthStatNode>(0);
  nodeTemplates[NodeType::STAT_SPEED] = std::make_unique<SpeedStatNode>(0);
  nodeTemplates[NodeType::STAT_DAMAGE] = std::make_unique<DamageStatNode>(0);
  nodeTemplates[NodeType::STAT_FIRE_RATE] =
      std::make_unique<FireRateStatNode>(0);
  nodeTemplates[NodeType::ACTION_FIRE] = std::make_unique<FireActionNode>(0);
  nodeTemplates[NodeType::ACTION_SHIELD] =
      std::make_unique<ShieldActionNode>(0);
  nodeTemplates[NodeType::ACTION_SHIFT] = std::make_unique<ShiftActionNode>(0);
  nodeTemplates[NodeType::POWER_DURATION_REDUCE] =
      std::make_unique<DurationReducePowerNode>(0);
  nodeTemplates[NodeType::POWER_VALUE_ADD] =
      std::make_unique<ValueAddPowerNode>(0);
}

std::unique_ptr<BaseNode>
NodesController::CreateNodeFromTemplate(NodeType type) {
  if (nodeTemplates.count(type)) {
    switch (type) {
    case NodeType::CPU_CORE:
      return std::make_unique<CPUCoreNode>(nextNodeId++);
    case NodeType::STAT_HEALTH:
      return std::make_unique<HealthStatNode>(nextNodeId++);
    case NodeType::STAT_SPEED:
      return std::make_unique<SpeedStatNode>(nextNodeId++);
    case NodeType::STAT_DAMAGE:
      return std::make_unique<DamageStatNode>(nextNodeId++);
    case NodeType::STAT_FIRE_RATE:
      return std::make_unique<FireRateStatNode>(nextNodeId++);
    case NodeType::ACTION_FIRE:
      return std::make_unique<FireActionNode>(nextNodeId++);
    case NodeType::ACTION_SHIELD:
      return std::make_unique<ShieldActionNode>(nextNodeId++);
    case NodeType::ACTION_SHIFT:
      return std::make_unique<ShiftActionNode>(nextNodeId++);
    case NodeType::POWER_DURATION_REDUCE:
      return std::make_unique<DurationReducePowerNode>(nextNodeId++);
    case NodeType::POWER_VALUE_ADD:
      return std::make_unique<ValueAddPowerNode>(nextNodeId++);
    default:
      break;
    }
  }
  return std::make_unique<CPUCoreNode>(nextNodeId++); // Default fallback
}

BaseNode *
NodesController::GetNodeById(int id,
                             std::vector<std::unique_ptr<BaseNode>> &nodes) {
  for (auto &node : nodes) {
    if (node && node->id == id)
      return node.get();
  }
  return nullptr;
}

void NodesController::UpdateNodeActivation(Player &player) {
  // Reset all non-CPU nodes to inactive
  for (auto &node : player.placedNodes) {
    if (node->getNodeType() == NodeType::CPU_CORE)
      node->isActive = true;
    else if (node->getNodeKind() == NodeKind::STAT ||
             node->getNodeKind() == NodeKind::POWER)
      node->isActive = false;
  }

  // Activate nodes connected to CPU
  BaseNode *cpuNode = player.GetPlayerNodeById(0);
  if (cpuNode && cpuNode->getNodeType() == NodeType::CPU_CORE) {
    for (int connectedId : cpuNode->connectedToNodeIDs) {
      BaseNode *targetNode = player.GetPlayerNodeById(connectedId);
      if (targetNode && (targetNode->getNodeKind() == NodeKind::STAT ||
                         targetNode->getNodeKind() == NodeKind::POWER)) {
        targetNode->isActive = true;
      }
    }
  }

  player.applyNodeEffects();
}

void NodesController::UpdateActionSystem(Player &player, Bullet bullets[],
                                         float dt) {
  if (player.activeActionNodeId == -1) {
    // Start new action sequence
    BaseNode *cpuNode = player.GetPlayerNodeById(0);
    if (cpuNode && cpuNode->getNodeType() == NodeType::CPU_CORE) {
      for (int connectedId : cpuNode->connectedToNodeIDs) {
        BaseNode *potentialStartNode = player.GetPlayerNodeById(connectedId);
        if (potentialStartNode &&
            potentialStartNode->getNodeKind() == NodeKind::ACTION) {
          player.activeActionNodeId = potentialStartNode->id;
          StartNewAction(player, bullets, potentialStartNode);
          break;
        }
      }
    }
  } else {
    ProcessActionSequence(player, bullets, dt);
  }
}

void NodesController::ProcessActionSequence(Player &player, Bullet bullets[],
                                            float dt) {
  BaseNode *currentActionNode =
      player.GetPlayerNodeById(player.activeActionNodeId);
  if (currentActionNode) {
    if (currentActionNode->currentActiveTimer > 0) {
      currentActionNode->currentActiveTimer -= dt;
      if (currentActionNode->getNodeType() == NodeType::ACTION_SHIELD)
        player.playerShieldIsActive = true;
    } else {
      // Action finished, find next action or restart
      currentActionNode->isCurrentlyActiveEffect = false;
      if (currentActionNode->getNodeType() == NodeType::ACTION_SHIELD)
        player.playerShieldIsActive = false;

      int nextNodeInSequenceId = -1;
      for (int connectedId : currentActionNode->connectedToNodeIDs) {
        BaseNode *potentialNextNode = player.GetPlayerNodeById(connectedId);
        if (potentialNextNode &&
            potentialNextNode->getNodeKind() == NodeKind::ACTION) {
          nextNodeInSequenceId = potentialNextNode->id;
          break;
        }
      }

      player.activeActionNodeId = (nextNodeInSequenceId == -1)
                                      ? currentActionNode->id
                                      : nextNodeInSequenceId;

      BaseNode *newActionNode =
          player.GetPlayerNodeById(player.activeActionNodeId);
      if (newActionNode) {
        StartNewAction(player, bullets, newActionNode);
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

void NodesController::StartNewAction(Player &player, Bullet bullets[],
                                     BaseNode *actionNode) {
  float effectiveDuration = CalculateEffectiveDuration(actionNode, player);
  actionNode->currentActiveTimer = fmaxf(0.1f, effectiveDuration);
  actionNode->isCurrentlyActiveEffect = true;

  if (actionNode->getNodeType() == NodeType::ACTION_FIRE)
    FireActionBullet(player, bullets, *actionNode);
  else if (actionNode->getNodeType() == NodeType::ACTION_SHIELD)
    player.playerShieldIsActive = true;
}

float NodesController::CalculateEffectiveDuration(const BaseNode *actionNode,
                                                  const Player &player) {
  float effectiveDuration = actionNode->duration;
  for (int fromId : actionNode->connectedFromNodeIDs) {
    BaseNode *modNode = player.GetPlayerNodeById(fromId);
    if (modNode && modNode->isActive) {
      if (modNode->getNodeType() == NodeType::POWER_DURATION_REDUCE)
        effectiveDuration += modNode->value;
      else if (actionNode->getNodeType() == NodeType::ACTION_SHIELD &&
               modNode->getNodeType() == NodeType::STAT_HEALTH)
        effectiveDuration += modNode->value / 50.0f;
    }
  }
  return effectiveDuration;
}

float NodesController::CalculateEffectiveValue(const BaseNode *node,
                                               const Player &player) {
  float effectiveValue = node->value;
  for (int fromId : node->connectedFromNodeIDs) {
    BaseNode *powerNode = player.GetPlayerNodeById(fromId);
    if (powerNode && powerNode->getNodeType() == NodeType::POWER_VALUE_ADD &&
        powerNode->isActive) {
      effectiveValue += powerNode->value;
    }
  }
  return effectiveValue;
}

void NodesController::FireActionBullet(Player &player, Bullet bullets[],
                                       const BaseNode &fireActionNode) {
  float effectiveDamage = CalculateEffectiveValue(&fireActionNode, player);

  // Add damage stat bonuses
  for (int fromId : fireActionNode.connectedFromNodeIDs) {
    BaseNode *modifierNode = player.GetPlayerNodeById(fromId);
    if (modifierNode && modifierNode->isActive) {
      if (modifierNode->getNodeType() == NodeType::STAT_DAMAGE) {
        effectiveDamage += modifierNode->value;
      }
    }
  }

  for (int i = 0; i < MAX_BULLETS; i++) {
    if (!bullets[i].active) {
      bullets[i].active = true;
      bullets[i].fromPlayer = true;
      bullets[i].position = player.position;
      Vector2 mousePos = GetMousePosition();
      Vector2 direction =
          Vector2Normalize(Vector2Subtract(mousePos, player.position));
      if (Vector2LengthSqr(direction) == 0)
        direction = {0, -1};
      bullets[i].velocity = Vector2Scale(direction, BULLET_SPEED * 1.1f);
      bullets[i].color = ORANGE;
      bullets[i].damage = player.currentDamage + (int)effectiveDamage;
      break;
    }
  }
}
