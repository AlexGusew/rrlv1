#pragma once
#include "raylib.h"

enum class SpriteType {
  CPU,
  ACTION_CHIP,
  RED_CYLINDER,
  GREEN_CYLINDER,
  BROWN_CYLINDER,
  BLUE_LAMP,
  RED_LAMP,
  VACUUM,
  PLAYER,
  COUNT // Used to get the total count
};

class SpriteManager {
private:
  static constexpr int SPRITE_COUNT = static_cast<int>(SpriteType::COUNT);
  Texture2D spritesheet;
  Rectangle sprites[SPRITE_COUNT];
  Rectangle collisions[SPRITE_COUNT];
  NPatchInfo nPatchInfo[SPRITE_COUNT];

public:
  // Constructor
  SpriteManager();

  // Destructor
  ~SpriteManager();

  // Disable copy and move operations
  SpriteManager(const SpriteManager &other) = delete;
  SpriteManager &operator=(const SpriteManager &other) = delete;
  SpriteManager(SpriteManager &&other) = delete;
  SpriteManager &operator=(SpriteManager &&other) = delete;

  void Init(const char *spritesheetPath, const Rectangle *collisions,
            const NPatchInfo *nPatchInfo);

  // Main drawing function
  // To draw Nodes, user Draw from BaseNode
  void DrawSprite(SpriteType type, Vector2 position, float scale = 1.0f) const;

  // Debug function to display all sprites
  void DrawDebugSprites() const;

  // Utility functions
  bool IsLoaded() const { return spritesheet.id != 0; }
  int GetSpriteCount() const { return SPRITE_COUNT; }
  const Rectangle *GetCollisions() const { return collisions; }
  const NPatchInfo *GetNPatchInfo() const { return nPatchInfo; }
  const Rectangle *GetSprites() const { return sprites; }
  Texture2D &GetSpritesheet() { return spritesheet; }
};
