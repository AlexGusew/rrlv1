#pragma once
#include "raylib.h"

class Player;
class BaseNode;

class ControlPanel {
public:
  ControlPanel();
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

  static const float NODE_UI_SIZE;
  static const float NODE_INV_ITEM_HEIGHT;

  void UpdateInventoryScroll(const Player &player);
  void UpdateGridCamera();
  void UpdateNodeDragging(Player &player);
  void UpdateNodeConnections(Player &player);
  void HandleNodePlacement(Player &player);
  void HandleNodeRemoval(Player &player);
  void HandleCameraMove(Player &player);

  void DrawInventoryArea(const Player &player);
  void DrawGridArea(const Player &player);
  void DrawConnections(const Player &player);
  void DrawNodes(const Player &player);
  void DrawDraggedNode(const Player &player);
  void DrawTooltips(const Player &player);
  void DrawScrollbar(const Player &player);
};
