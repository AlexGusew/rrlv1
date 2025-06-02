#pragma once
#include "BaseNode.h"
#include "Player.h"
#include <map>
#include <memory>

struct Bullet;

class NodesController {
public:
  NodesController();
  ~NodesController() = default;

  void Initialize();
  void UpdateNodeActivation(Player &player);
  void UpdateActionSystem(Player &player, Bullet bullets[], float dt);

  std::unique_ptr<BaseNode> CreateNodeFromTemplate(NodeType type);
  BaseNode *GetNodeById(int id, std::vector<std::unique_ptr<BaseNode>> &nodes);

  void FireActionBullet(Player &player, Bullet bullets[],
                        const BaseNode &fireActionNode);

  int GetNextNodeId() { return nextNodeId++; }

private:
  std::map<NodeType, std::unique_ptr<BaseNode>> nodeTemplates;
  int nextNodeId;

  void InitNodeTemplates();
  void ProcessActionSequence(Player &player, Bullet bullets[], float dt);
  void StartNewAction(Player &player, Bullet bullets[], BaseNode *actionNode);
  float CalculateEffectiveDuration(const BaseNode *actionNode,
                                   const Player &player);
  float CalculateEffectiveValue(const BaseNode *node, const Player &player);
};
