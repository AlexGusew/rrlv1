#include "BaseNode.h"

BaseNode::BaseNode(int _id, NodeType _type, NodeKind _kind, std::string _name,
                   float _val, float _dur)
    : id(_id), type(_type), kind(_kind), name(_name), value(_val),
      duration(_dur), isPlaced(false), isActive(false),
      currentActiveTimer(0.0f), isCurrentlyActiveEffect(false) {}
