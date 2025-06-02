#pragma once
#include "BaseNode.h"

class CPUCoreNode : public BaseNode {
public:
  CPUCoreNode(int _id = -1, float _val = 0.0f, float _dur = 0.0f);
  NodeType getType() const override { return NodeType::CPU_CORE; }

protected:
  void setTypeSpecificProperties() override;
};

class HealthStatNode : public BaseNode {
public:
  HealthStatNode(int _id = -1, float _val = 25.0f, float _dur = 0.0f);
  NodeType getType() const override { return NodeType::STAT_HEALTH; }

protected:
  void setTypeSpecificProperties() override;
};

class SpeedStatNode : public BaseNode {
public:
  SpeedStatNode(int _id = -1, float _val = 50.0f, float _dur = 0.0f);
  NodeType getType() const override { return NodeType::STAT_SPEED; }

protected:
  void setTypeSpecificProperties() override;
};

class DamageStatNode : public BaseNode {
public:
  DamageStatNode(int _id = -1, float _val = 5.0f, float _dur = 0.0f);
  NodeType getType() const override { return NodeType::STAT_DAMAGE; }

protected:
  void setTypeSpecificProperties() override;
};

class FireRateStatNode : public BaseNode {
public:
  FireRateStatNode(int _id = -1, float _val = 0.1f, float _dur = 0.0f);
  NodeType getType() const override { return NodeType::STAT_FIRE_RATE; }

protected:
  void setTypeSpecificProperties() override;
};

class FireActionNode : public BaseNode {
public:
  FireActionNode(int _id = -1, float _val = 20.0f, float _dur = 2.0f);
  NodeType getType() const override { return NodeType::ACTION_FIRE; }

protected:
  void setTypeSpecificProperties() override;
};

class ShieldActionNode : public BaseNode {
public:
  ShieldActionNode(int _id = -1, float _val = 0.0f, float _dur = 3.0f);
  NodeType getType() const override { return NodeType::ACTION_SHIELD; }

protected:
  void setTypeSpecificProperties() override;
};

class ShiftActionNode : public BaseNode {
public:
  ShiftActionNode(int _id = -1, float _val = 75.0f, float _dur = 1.5f);
  NodeType getType() const override { return NodeType::ACTION_SHIFT; }

protected:
  void setTypeSpecificProperties() override;
};

class DurationReducePowerNode : public BaseNode {
public:
  DurationReducePowerNode(int _id = -1, float _val = -0.5f, float _dur = 0.0f);
  NodeType getType() const override { return NodeType::POWER_DURATION_REDUCE; }

protected:
  void setTypeSpecificProperties() override;
};

class ValueAddPowerNode : public BaseNode {
public:
  ValueAddPowerNode(int _id = -1, float _val = 10.0f, float _dur = 0.0f);
  NodeType getType() const override { return NodeType::POWER_VALUE_ADD; }

protected:
  void setTypeSpecificProperties() override;
};