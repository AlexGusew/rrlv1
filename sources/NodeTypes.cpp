#include "NodeTypes.h"
#include "BaseNode.h"

// CPUCoreNode
CPUCoreNode::CPUCoreNode(int _id, float _val, float _dur)
    : BaseNode(_id, NodeType::CPU_CORE, NodeKind::CPU, "CPU Core", _val, _dur) {
  color = GOLD;
  description = "CPU. Powers STAT & POWER nodes. Starts ACTION sequences.";
}

// HealthStatNode
HealthStatNode::HealthStatNode(int _id, float _val, float _dur)
    : BaseNode(_id, NodeType::STAT_HEALTH, NodeKind::STAT, "Health Chip", _val,
               _dur) {
  color = GREEN;
  description = "Health Chip: +Max Health. Buffs Shield Duration.";
}

// SpeedStatNode
SpeedStatNode::SpeedStatNode(int _id, float _val, float _dur)
    : BaseNode(_id, NodeType::STAT_SPEED, NodeKind::STAT, "Speed Chip", _val,
               _dur) {
  color = BLUE;
  description = "Speed Chip: +Move Speed. Buffs Shift Potency.";
}

// DamageStatNode
DamageStatNode::DamageStatNode(int _id, float _val, float _dur)
    : BaseNode(_id, NodeType::STAT_DAMAGE, NodeKind::STAT, "Damage Chip", _val,
               _dur) {
  color = RED;
  description = "Damage Chip: +Bullet Dmg. Buffs Fire Dmg.";
}

// FireRateStatNode
FireRateStatNode::FireRateStatNode(int _id, float _val, float _dur)
    : BaseNode(_id, NodeType::STAT_FIRE_RATE, NodeKind::STAT, "FireRate Chip",
               _val, _dur) {
  color = SKYBLUE;
  description = "FireRate Chip: +Fire Rate.";
}

// FireActionNode
FireActionNode::FireActionNode(int _id, float _val, float _dur)
    : BaseNode(_id, NodeType::ACTION_FIRE, NodeKind::ACTION, "Fire Blast", _val,
               _dur) {
  color = ORANGE;
  description = "Action: Fire Blast.";
}

// ShieldActionNode
ShieldActionNode::ShieldActionNode(int _id, float _val, float _dur)
    : BaseNode(_id, NodeType::ACTION_SHIELD, NodeKind::ACTION, "Energy Shield",
               _val, _dur) {
  color = DARKPURPLE;
  description = "Action: Energy Shield.";
}

// ShiftActionNode
ShiftActionNode::ShiftActionNode(int _id, float _val, float _dur)
    : BaseNode(_id, NodeType::ACTION_SHIFT, NodeKind::ACTION, "Phase Shift",
               _val, _dur) {
  color = PINK;
  description = "Action: Phase Shift.";
}

// DurationReducePowerNode
DurationReducePowerNode::DurationReducePowerNode(int _id, float _val,
                                                 float _dur)
    : BaseNode(_id, NodeType::POWER_DURATION_REDUCE, NodeKind::POWER,
               "Duration Mod", _val, _dur) {
  const Color TEAL_CUSTOM = {0, 128, 128, 255};
  color = TEAL_CUSTOM;
  description =
      "Power: Modify connected Action duration. Value is seconds change (+/-).";
}

// ValueAddPowerNode
ValueAddPowerNode::ValueAddPowerNode(int _id, float _val, float _dur)
    : BaseNode(_id, NodeType::POWER_VALUE_ADD, NodeKind::POWER, "Value Mod",
               _val, _dur) {
  const Color VIOLET_CUSTOM = {238, 130, 238, 255};
  color = VIOLET_CUSTOM;
  description = "Power: Modify connected Stat/Action value. Value is amount to "
                "add (+/-).";
}
