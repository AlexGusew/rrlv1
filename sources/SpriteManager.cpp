#include "SpriteManager.h"
#include "raylib.h"

SpriteManager::SpriteManager() : spritesheet{0} {}

void SpriteManager::Init(const char *spritesheetPath,
                         const Rectangle *collisions) {
  spritesheet = LoadTexture(spritesheetPath);
  // Initialize sprite rectangles (assuming horizontal layout)
  for (int i = 0; i < SPRITE_COUNT; i++) {
    sprites[i] = Rectangle{static_cast<float>(32 * i), 0.0f, 32.0f, 32.0f};
    this->collisions[i] = collisions[i];
  }
}

SpriteManager::~SpriteManager() {
  if (spritesheet.id != 0) {
    UnloadTexture(spritesheet);
  }
}

void SpriteManager::DrawSprite(SpriteType type, Vector2 position,
                               float scale) const {
  int index = static_cast<int>(type);
  if (index < 0 || index >= SPRITE_COUNT) {
    return; // Invalid sprite type
  }

  Rectangle destRect = {position.x, position.y, 32.0f * scale, 32.0f * scale};

  DrawTexturePro(spritesheet, sprites[index], destRect, Vector2{0.0f, 0.0f},
                 0.0f, WHITE);
}

void SpriteManager::DrawDebugSprites() const {
  int total = 0;
  for (int row = 0; total < SPRITE_COUNT; row++) {
    for (int col = 0; col < 5 && total < SPRITE_COUNT; col++) {
      DrawSprite(
          static_cast<SpriteType>(total),
          Vector2{static_cast<float>(col * 100), static_cast<float>(row * 100)},
          2.0f);
      total++;
    }
  }
}
