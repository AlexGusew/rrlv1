#pragma once
#include "BaseNode.h"

class CPUCoreNode : public BaseNode {
public:
  CPUCoreNode(int _id = -1, float _val = 0.0f, float _dur = 0.0f);
  void Draw(SpriteManager &spriteManager, Rectangle dest,
            float rotation) override;
};

class HealthStatNode : public BaseNode {
public:
  HealthStatNode(int _id = -1, float _val = 25.0f, float _dur = 0.0f);
  void Draw(SpriteManager &spriteManager, Rectangle dest,
            float rotation) override;
};

class SpeedStatNode : public BaseNode {
public:
  SpeedStatNode(int _id = -1, float _val = 50.0f, float _dur = 0.0f);
  void Draw(SpriteManager &spriteManager, Rectangle dest,
            float rotation) override;
};

class DamageStatNode : public BaseNode {
public:
  DamageStatNode(int _id = -1, float _val = 5.0f, float _dur = 0.0f);
  void Draw(SpriteManager &spriteManager, Rectangle dest,
            float rotation) override;
};

class FireRateStatNode : public BaseNode {
public:
  FireRateStatNode(int _id = -1, float _val = 0.1f, float _dur = 0.0f);
  void Draw(SpriteManager &spriteManager, Rectangle dest,
            float rotation) override;
};

class FireActionNode : public BaseNode {
public:
  FireActionNode(int _id = -1, float _val = 20.0f, float _dur = 2.0f);
  void Draw(SpriteManager &spriteManager, Rectangle dest,
            float rotation) override;
};

class ShieldActionNode : public BaseNode {
public:
  ShieldActionNode(int _id = -1, float _val = 0.0f, float _dur = 3.0f);
  void Draw(SpriteManager &spriteManager, Rectangle dest,
            float rotation) override;
};

class ShiftActionNode : public BaseNode {
public:
  ShiftActionNode(int _id = -1, float _val = 75.0f, float _dur = 1.5f);
  void Draw(SpriteManager &spriteManager, Rectangle dest,
            float rotation) override;
};

class DurationReducePowerNode : public BaseNode {
public:
  DurationReducePowerNode(int _id = -1, float _val = -0.5f, float _dur = 0.0f);
  void Draw(SpriteManager &spriteManager, Rectangle dest,
            float rotation) override;
};

class ValueAddPowerNode : public BaseNode {
public:
  ValueAddPowerNode(int _id = -1, float _val = 10.0f, float _dur = 0.0f);
  void Draw(SpriteManager &spriteManager, Rectangle dest,
            float rotation) override;
};
