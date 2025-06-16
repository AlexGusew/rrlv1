#pragma once
#include "SpriteManager.h"
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

enum class NodeKind { CPU, ACTION, STAT, POWER };

class BaseNode {
public:
  int id;
  NodeType type;
  NodeKind kind;
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
           NodeKind _kind = NodeKind::ACTION, std::string _name = "N/A",
           float _val = 0.0f, float _dur = 0.0f);

  virtual ~BaseNode() = default;

  virtual NodeType getNodeType() const { return type; }
  virtual NodeKind getNodeKind() const { return kind; }
  virtual void Draw(SpriteManager &spriteManager, Rectangle dest,
                    float rotation) = 0;
};
