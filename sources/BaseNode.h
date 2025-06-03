#pragma once
#include "raylib.h"
#include <string>
#include <vector>

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

class BaseNode {
public:
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

  BaseNode(int _id = -1, NodeType _type = NodeType::NONE,
           std::string _name = "N/A", float _val = 0.0f, float _dur = 0.0f);

  virtual ~BaseNode() = default;

  virtual NodeType getType() const = 0;
  bool isActionType() const;
  bool isStatType() const;
  bool isPowerType() const;

protected:
  virtual void setTypeSpecificProperties() = 0;
};
