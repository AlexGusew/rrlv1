#pragma once
#include "raylib.h"
#include <string>
#include <vector>

enum class EditMode {
  MOVE,
  RESIZE_TOP_LEFT,
  RESIZE_TOP_RIGHT,
  RESIZE_BOTTOM_LEFT,
  RESIZE_BOTTOM_RIGHT,
  RESIZE_LEFT,
  RESIZE_RIGHT,
  RESIZE_TOP,
  RESIZE_BOTTOM
};

struct SpriteCollider {
  std::string name;
  Texture2D sprite;
  Rectangle collisionRect;
  Vector2 spritePosition;
  float spriteScale;

  SpriteCollider(const std::string &spriteName, Texture2D spriteTexture)
      : name(spriteName), sprite(spriteTexture), spriteScale(2.0f) {
    spritePosition = {400, 300}; // Center of screen roughly
    // Default collision rect to full sprite size
    collisionRect = {0, 0, static_cast<float>(sprite.width),
                     static_cast<float>(sprite.height)};
  }
};

class CollisionEditor {
private:
  std::vector<SpriteCollider> sprites;
  int selectedSpriteIndex;
  bool isActive;
  bool isDragging;
  EditMode currentEditMode;
  Vector2 dragStartPos;
  Rectangle originalRect;

  // UI properties
  const float HANDLE_SIZE = 8.0f;
  const Color COLLISION_COLOR = {255, 0, 0, 100};
  const Color HANDLE_COLOR = {255, 255, 0, 200};
  const Color SELECTED_COLOR = {0, 255, 0, 150};

  // Helper methods
  Rectangle GetWorldCollisionRect(int spriteIndex) const;
  EditMode GetEditModeAtPosition(Vector2 mousePos,
                                 const Rectangle &worldRect) const;
  void UpdateCollisionRect(Vector2 currentMousePos);
  void DrawCollisionHandles(const Rectangle &worldRect) const;
  void DrawUI() const;

public:
  CollisionEditor();
  ~CollisionEditor();

  // Main interface
  void AddSprite(const std::string &name, const std::string &texturePath);
  void AddSprite(const std::string &name, Texture2D texture);
  void Toggle() { isActive = !isActive; }
  bool IsActive() const { return isActive; }

  // Main loop methods
  void Update();
  void Draw() const;

  // Get collision data
  Rectangle GetCollisionRect(int index) const;
  Rectangle GetCollisionRect(const std::string &name) const;
  int GetSpriteCount() const { return static_cast<int>(sprites.size()); }
  void Step();
};
