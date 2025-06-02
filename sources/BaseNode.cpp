#include "BaseNode.h"

BaseNode::BaseNode(int _id, NodeType _type, std::string _name, float _val, float _dur)
    : id(_id), type(_type), name(_name), value(_val), duration(_dur),
      isPlaced(false), isActive(false), currentActiveTimer(0.0f),
      isCurrentlyActiveEffect(false) {
  // setTypeSpecificProperties(); // Moved to derived constructors to avoid pure virtual call
}

bool BaseNode::isActionType() const {
  return type == NodeType::ACTION_FIRE || type == NodeType::ACTION_SHIELD ||
         type == NodeType::ACTION_SHIFT;
}

bool BaseNode::isStatType() const {
  return type == NodeType::STAT_HEALTH || type == NodeType::STAT_SPEED ||
         type == NodeType::STAT_DAMAGE || type == NodeType::STAT_FIRE_RATE;
}

bool BaseNode::isPowerType() const {
  return type == NodeType::POWER_DURATION_REDUCE ||
         type == NodeType::POWER_VALUE_ADD;
}