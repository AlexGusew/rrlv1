#include "NodeTypes.h"

// CPUCoreNode
CPUCoreNode::CPUCoreNode(int _id, float _val, float _dur)
    : BaseNode(_id, NodeType::CPU_CORE, "CPU Core", _val, _dur) {
  setTypeSpecificProperties();
}

void CPUCoreNode::setTypeSpecificProperties() {
  color = GOLD;
  description = "CPU. Powers STAT & POWER nodes. Starts ACTION sequences.";
}

// HealthStatNode
HealthStatNode::HealthStatNode(int _id, float _val, float _dur)
    : BaseNode(_id, NodeType::STAT_HEALTH, "Health Chip", _val, _dur) {
  setTypeSpecificProperties();
}

void HealthStatNode::setTypeSpecificProperties() {
  color = GREEN;
  description = "Health Chip: +Max Health. Buffs Shield Duration.";
}

// SpeedStatNode
SpeedStatNode::SpeedStatNode(int _id, float _val, float _dur)
    : BaseNode(_id, NodeType::STAT_SPEED, "Speed Chip", _val, _dur) {
  setTypeSpecificProperties();
}

void SpeedStatNode::setTypeSpecificProperties() {
  color = BLUE;
  description = "Speed Chip: +Move Speed. Buffs Shift Potency.";
}

// DamageStatNode
DamageStatNode::DamageStatNode(int _id, float _val, float _dur)
    : BaseNode(_id, NodeType::STAT_DAMAGE, "Damage Chip", _val, _dur) {
  setTypeSpecificProperties();
}

void DamageStatNode::setTypeSpecificProperties() {
  color = RED;
  description = "Damage Chip: +Bullet Dmg. Buffs Fire Dmg.";
}

// FireRateStatNode
FireRateStatNode::FireRateStatNode(int _id, float _val, float _dur)
    : BaseNode(_id, NodeType::STAT_FIRE_RATE, "FireRate Chip", _val, _dur) {
  setTypeSpecificProperties();
}

void FireRateStatNode::setTypeSpecificProperties() {
  color = SKYBLUE;
  description = "FireRate Chip: +Fire Rate.";
}

// FireActionNode
FireActionNode::FireActionNode(int _id, float _val, float _dur)
    : BaseNode(_id, NodeType::ACTION_FIRE, "Fire Blast", _val, _dur) {
  setTypeSpecificProperties();
}

void FireActionNode::setTypeSpecificProperties() {
  color = ORANGE;
  description = "Action: Fire Blast.";
}

// ShieldActionNode
ShieldActionNode::ShieldActionNode(int _id, float _val, float _dur)
    : BaseNode(_id, NodeType::ACTION_SHIELD, "Energy Shield", _val, _dur) {
  setTypeSpecificProperties();
}

void ShieldActionNode::setTypeSpecificProperties() {
  color = DARKPURPLE;
  description = "Action: Energy Shield.";
}

// ShiftActionNode
ShiftActionNode::ShiftActionNode(int _id, float _val, float _dur)
    : BaseNode(_id, NodeType::ACTION_SHIFT, "Phase Shift", _val, _dur) {
  setTypeSpecificProperties();
}

void ShiftActionNode::setTypeSpecificProperties() {
  color = PINK;
  description = "Action: Phase Shift.";
}

// DurationReducePowerNode
DurationReducePowerNode::DurationReducePowerNode(int _id, float _val, float _dur)
    : BaseNode(_id, NodeType::POWER_DURATION_REDUCE, "Duration Mod", _val, _dur) {
  setTypeSpecificProperties();
}

void DurationReducePowerNode::setTypeSpecificProperties() {
  const Color TEAL_CUSTOM = {0, 128, 128, 255};
  color = TEAL_CUSTOM;
  description = "Power: Modify connected Action duration. Value is seconds change (+/-).";
}

// ValueAddPowerNode
ValueAddPowerNode::ValueAddPowerNode(int _id, float _val, float _dur)
    : BaseNode(_id, NodeType::POWER_VALUE_ADD, "Value Mod", _val, _dur) {
  setTypeSpecificProperties();
}

void ValueAddPowerNode::setTypeSpecificProperties() {
  const Color VIOLET_CUSTOM = {238, 130, 238, 255};
  color = VIOLET_CUSTOM;
  description = "Power: Modify connected Stat/Action value. Value is amount to add (+/-).";
}