#include "CollisionEditor.h"
#include "raylib.h"
#include <string>
#include <vector>

// Implementation
CollisionEditor::CollisionEditor()
    : selectedSpriteIndex(0), isActive(false), isDragging(false),
      currentEditMode(EditMode::MOVE), dragStartPos{0, 0},
      originalRect{0, 0, 0, 0} {}

CollisionEditor::~CollisionEditor() {
  // Note: If textures were loaded by this class, we'd unload them here
  // But since they might be managed elsewhere, we don't unload
}

void CollisionEditor::AddSprite(const std::string &name,
                                const std::string &texturePath) {
  Texture2D texture = LoadTexture(texturePath.c_str());
  if (texture.id != 0) {
    sprites.emplace_back(name, texture);
  }
}

void CollisionEditor::AddSprite(const std::string &name, Texture2D texture) {
  if (texture.id != 0) {
    sprites.emplace_back(name, texture);
  }
}

Rectangle CollisionEditor::GetWorldCollisionRect(int spriteIndex) const {
  if (spriteIndex < 0 || spriteIndex >= sprites.size()) {
    return {0, 0, 0, 0};
  }

  const auto &sprite = sprites[spriteIndex];
  return {sprite.spritePosition.x + sprite.collisionRect.x * sprite.spriteScale,
          sprite.spritePosition.y + sprite.collisionRect.y * sprite.spriteScale,
          sprite.collisionRect.width * sprite.spriteScale,
          sprite.collisionRect.height * sprite.spriteScale};
}

EditMode
CollisionEditor::GetEditModeAtPosition(Vector2 mousePos,
                                       const Rectangle &worldRect) const {
  const float h = HANDLE_SIZE;

  // Check corner handles first (higher priority)
  if (CheckCollisionPointRec(mousePos,
                             {worldRect.x - h / 2, worldRect.y - h / 2, h, h}))
    return EditMode::RESIZE_TOP_LEFT;
  if (CheckCollisionPointRec(mousePos, {worldRect.x + worldRect.width - h / 2,
                                        worldRect.y - h / 2, h, h}))
    return EditMode::RESIZE_TOP_RIGHT;
  if (CheckCollisionPointRec(
          mousePos,
          {worldRect.x - h / 2, worldRect.y + worldRect.height - h / 2, h, h}))
    return EditMode::RESIZE_BOTTOM_LEFT;
  if (CheckCollisionPointRec(mousePos,
                             {worldRect.x + worldRect.width - h / 2,
                              worldRect.y + worldRect.height - h / 2, h, h}))
    return EditMode::RESIZE_BOTTOM_RIGHT;

  // Check edge handles
  if (CheckCollisionPointRec(
          mousePos, {worldRect.x - h / 2,
                     worldRect.y + worldRect.height / 2 - h / 2, h, h}))
    return EditMode::RESIZE_LEFT;
  if (CheckCollisionPointRec(
          mousePos, {worldRect.x + worldRect.width - h / 2,
                     worldRect.y + worldRect.height / 2 - h / 2, h, h}))
    return EditMode::RESIZE_RIGHT;
  if (CheckCollisionPointRec(mousePos,
                             {worldRect.x + worldRect.width / 2 - h / 2,
                              worldRect.y - h / 2, h, h}))
    return EditMode::RESIZE_TOP;
  if (CheckCollisionPointRec(mousePos,
                             {worldRect.x + worldRect.width / 2 - h / 2,
                              worldRect.y + worldRect.height - h / 2, h, h}))
    return EditMode::RESIZE_BOTTOM;

  // Check if inside rectangle for moving
  if (CheckCollisionPointRec(mousePos, worldRect))
    return EditMode::MOVE;

  return EditMode::MOVE; // Default
}

void CollisionEditor::UpdateCollisionRect(Vector2 currentMousePos) {
  if (selectedSpriteIndex < 0 || selectedSpriteIndex >= sprites.size())
    return;

  auto &sprite = sprites[selectedSpriteIndex];
  Vector2 delta = {currentMousePos.x - dragStartPos.x,
                   currentMousePos.y - dragStartPos.y};

  // Convert delta to sprite-local coordinates
  delta.x /= sprite.spriteScale;
  delta.y /= sprite.spriteScale;

  Rectangle newRect = originalRect;

  switch (currentEditMode) {
  case EditMode::MOVE:
    sprite.spritePosition.x += (currentMousePos.x - dragStartPos.x);
    sprite.spritePosition.y += (currentMousePos.y - dragStartPos.y);
    dragStartPos = currentMousePos;
    return;

  case EditMode::RESIZE_TOP_LEFT:
    newRect.x += delta.x;
    newRect.y += delta.y;
    newRect.width -= delta.x;
    newRect.height -= delta.y;
    break;

  case EditMode::RESIZE_TOP_RIGHT:
    newRect.y += delta.y;
    newRect.width += delta.x;
    newRect.height -= delta.y;
    break;

  case EditMode::RESIZE_BOTTOM_LEFT:
    newRect.x += delta.x;
    newRect.width -= delta.x;
    newRect.height += delta.y;
    break;

  case EditMode::RESIZE_BOTTOM_RIGHT:
    newRect.width += delta.x;
    newRect.height += delta.y;
    break;

  case EditMode::RESIZE_LEFT:
    newRect.x += delta.x;
    newRect.width -= delta.x;
    break;

  case EditMode::RESIZE_RIGHT:
    newRect.width += delta.x;
    break;

  case EditMode::RESIZE_TOP:
    newRect.y += delta.y;
    newRect.height -= delta.y;
    break;

  case EditMode::RESIZE_BOTTOM:
    newRect.height += delta.y;
    break;
  }

  // Ensure minimum size
  if (newRect.width < 4)
    newRect.width = 4;
  if (newRect.height < 4)
    newRect.height = 4;

  // Keep within sprite bounds
  if (newRect.x < 0) {
    newRect.width += newRect.x;
    newRect.x = 0;
  }
  if (newRect.y < 0) {
    newRect.height += newRect.y;
    newRect.y = 0;
  }
  if (newRect.x + newRect.width > sprite.sprite.width)
    newRect.width = sprite.sprite.width - newRect.x;
  if (newRect.y + newRect.height > sprite.sprite.height)
    newRect.height = sprite.sprite.height - newRect.y;

  sprite.collisionRect = newRect;
}

void CollisionEditor::Update() {
  if (!isActive || sprites.empty())
    return;

  Vector2 mousePos = GetMousePosition();

  // Handle sprite selection with number keys
  for (int i = 0; i < sprites.size() && i < 9; i++) {
    if (IsKeyPressed(KEY_ONE + i)) {
      selectedSpriteIndex = i;
    }
  }

  // Clamp selected index
  if (selectedSpriteIndex >= sprites.size()) {
    selectedSpriteIndex = sprites.size() - 1;
  }

  if (selectedSpriteIndex < 0)
    return;

  const int keyPressed = GetCharPressed();
  if (keyPressed == '+') {
    sprites[selectedSpriteIndex].spriteScale *= 1.2;
  }

  if (keyPressed == '-') {
    sprites[selectedSpriteIndex].spriteScale *= 0.8;
  }

  Rectangle worldRect = GetWorldCollisionRect(selectedSpriteIndex);

  if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
    currentEditMode = GetEditModeAtPosition(mousePos, worldRect);
    isDragging = true;
    dragStartPos = mousePos;
    originalRect = sprites[selectedSpriteIndex].collisionRect;
  }

  if (isDragging && IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
    UpdateCollisionRect(mousePos);
  }

  if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
    isDragging = false;
  }

  // Reset collision rect with R key
  if (IsKeyPressed(KEY_R)) {
    auto &sprite = sprites[selectedSpriteIndex];
    sprite.collisionRect = {0, 0, static_cast<float>(sprite.sprite.width),
                            static_cast<float>(sprite.sprite.height)};
  }
}

void CollisionEditor::DrawCollisionHandles(const Rectangle &worldRect) const {
  const float h = HANDLE_SIZE;

  // Corner handles
  DrawRectangle(worldRect.x - h / 2, worldRect.y - h / 2, h, h, HANDLE_COLOR);
  DrawRectangle(worldRect.x + worldRect.width - h / 2, worldRect.y - h / 2, h,
                h, HANDLE_COLOR);
  DrawRectangle(worldRect.x - h / 2, worldRect.y + worldRect.height - h / 2, h,
                h, HANDLE_COLOR);
  DrawRectangle(worldRect.x + worldRect.width - h / 2,
                worldRect.y + worldRect.height - h / 2, h, h, HANDLE_COLOR);

  // Edge handles
  DrawRectangle(worldRect.x - h / 2, worldRect.y + worldRect.height / 2 - h / 2,
                h, h, HANDLE_COLOR);
  DrawRectangle(worldRect.x + worldRect.width - h / 2,
                worldRect.y + worldRect.height / 2 - h / 2, h, h, HANDLE_COLOR);
  DrawRectangle(worldRect.x + worldRect.width / 2 - h / 2, worldRect.y - h / 2,
                h, h, HANDLE_COLOR);
  DrawRectangle(worldRect.x + worldRect.width / 2 - h / 2,
                worldRect.y + worldRect.height - h / 2, h, h, HANDLE_COLOR);
}

void CollisionEditor::DrawUI() const {
  if (sprites.empty()) {
    DrawText("No sprites loaded", 10, 10, 20, WHITE);
    return;
  }

  // Instructions
  DrawText("Collision Editor", 10, 10, 24, WHITE);
  DrawText("Keys 1-9: Select sprite", 10, 40, 16, LIGHTGRAY);
  DrawText("R: Reset collision rect", 10, 60, 16, LIGHTGRAY);
  DrawText("ESC: Close editor", 10, 80, 16, LIGHTGRAY);

  // Current sprite info
  if (selectedSpriteIndex >= 0 && selectedSpriteIndex < sprites.size()) {
    const auto &sprite = sprites[selectedSpriteIndex];
    std::string info = "Selected: " + sprite.name + " (" +
                       std::to_string(selectedSpriteIndex + 1) + ")";
    DrawText(info.c_str(), 10, 110, 18, YELLOW);

    // Collision rect info
    char rectInfo[256];
    snprintf(rectInfo, sizeof(rectInfo), "Collision: (%.0f, %.0f, %.0f, %.0f)",
             sprite.collisionRect.x, sprite.collisionRect.y,
             sprite.collisionRect.width, sprite.collisionRect.height);
    DrawText(rectInfo, 10, 130, 16, WHITE);
  }

  // Sprite list
  int yOffset = 160;
  for (size_t i = 0; i < sprites.size(); i++) {
    Color textColor = (i == selectedSpriteIndex) ? YELLOW : LIGHTGRAY;
    std::string spriteInfo = std::to_string(i + 1) + ": " + sprites[i].name;
    DrawText(spriteInfo.c_str(), 10, yOffset, 14, textColor);
    yOffset += 20;
  }
}

void CollisionEditor::Draw() const {
  if (!isActive)
    return;

  // Semi-transparent background
  DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), {0, 0, 0, 128});

  // Draw all sprites
  for (size_t i = 0; i < sprites.size(); i++) {
    const auto &sprite = sprites[i];

    // Draw sprite
    Rectangle destRect = {sprite.spritePosition.x, sprite.spritePosition.y,
                          sprite.sprite.width * sprite.spriteScale,
                          sprite.sprite.height * sprite.spriteScale};
    DrawTexturePro(sprite.sprite,
                   {0, 0, static_cast<float>(sprite.sprite.width),
                    static_cast<float>(sprite.sprite.height)},
                   destRect, {0, 0}, 0, WHITE);

    // Draw collision rectangle
    Rectangle worldRect = GetWorldCollisionRect(i);
    Color collisionColor =
        (i == selectedSpriteIndex) ? SELECTED_COLOR : COLLISION_COLOR;
    DrawRectangleRec(worldRect, collisionColor);
    DrawRectangleLinesEx(worldRect, 2, RED);

    // Draw handles for selected sprite
    if (i == selectedSpriteIndex) {
      DrawCollisionHandles(worldRect);
    }
  }

  DrawUI();
}

Rectangle CollisionEditor::GetCollisionRect(int index) const {
  if (index >= 0 && index < sprites.size()) {
    return sprites[index].collisionRect;
  }
  return {0, 0, 0, 0};
}

Rectangle CollisionEditor::GetCollisionRect(const std::string &name) const {
  for (const auto &sprite : sprites) {
    if (sprite.name == name) {
      return sprite.collisionRect;
    }
  }
  return {0, 0, 0, 0};
}

void CollisionEditor::Step() {
  // Toggle editor with F1
  if (IsKeyPressed(KEY_F1)) {
    Toggle();
  }

  // Close editor with ESC
  if (IsActive() && IsKeyPressed(KEY_ESCAPE)) {
    Toggle();
  }

  Update();
}
