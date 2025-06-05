#pragma once
#include "Player.h"
#include "raylib.h"

class Player;
class BaseNode;

class ControlPanel {
public:
  ControlPanel(Player &player);
  ~ControlPanel() = default;

  void Initialize(int screenWidth, int screenHeight);
  void Update(Player &player, float dt);
  void Draw(const Player &player);

  bool IsOpen() const { return isPanelOpen; }
  void SetOpen(bool open) { isPanelOpen = open; }
  Rectangle GetPanelArea() const { return panelArea; }

private:
  bool isPanelOpen;
  Rectangle panelArea;
  Rectangle panelInventoryArea;
  Rectangle panelGridArea;
  Camera2D panelCamera;
  float inventoryScrollOffset;

  int draggingNodeIndex;
  bool draggingFromInventory;
  int connectingNodeFromId;
  Vector2 startCameraDraggingPos;
  Player &player;

  static const float NODE_UI_SIZE;
  static const float NODE_INV_ITEM_HEIGHT;

  void UpdateInventoryScroll();
  void UpdateGridCamera();
  void UpdateNodeDragging();
  void UpdateNodeConnections();
  void HandleNodePlacement();
  void HandleNodeRemoval();
  void HandleCameraMove();

  void DrawInventoryArea();
  void DrawGridArea();
  void DrawConnections();
  void DrawNodes();
  void DrawDraggedNode();
  void DrawTooltips();
  void DrawScrollbar();
};
