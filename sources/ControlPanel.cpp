#include "ControlPanel.h"
#include "BaseNode.h"
#include "Player.h"
#include "raymath.h"
#include <algorithm>
#include <cmath>

const float ControlPanel::NODE_UI_SIZE = 50.0f;
const float ControlPanel::NODE_INV_ITEM_HEIGHT = NODE_UI_SIZE + 10.0f;

const Color DARKSLATEBLUE = {72, 61, 139, 255};
const Color MIDNIGHTBLUE = {25, 25, 112, 255};
const Color STAT_ACTION_LINK_COLOR = {128, 0, 128, 255};
const Color POWER_LINK_COLOR = {50, 205, 50, 255};
const Color CYAN = {0, 255, 255, 255};

ControlPanel::ControlPanel()
    : isPanelOpen(false), inventoryScrollOffset(0.0f), draggingNodeIndex(-1),
      draggingFromInventory(false), connectingNodeFromId(-1) {}

void ControlPanel::Initialize(int screenWidth, int screenHeight) {
  panelArea = {screenWidth * 2.0f / 3.0f, 0, screenWidth / 3.0f,
               (float)screenHeight};
  panelInventoryArea = {panelArea.x + 10, panelArea.y + 40,
                        panelArea.width - 20, panelArea.height * 0.4f - 50};
  panelGridArea = {panelArea.x + 10, panelArea.y + panelArea.height * 0.4f + 10,
                   panelArea.width - 20, panelArea.height * 0.6f - 20};

  draggingNodeIndex = -1;
  connectingNodeFromId = -1;
  inventoryScrollOffset = 0.0f;
  panelCamera.target = {0.0f, 0.0f};
  panelCamera.offset = {panelGridArea.x + panelGridArea.width / 2.0f,
                        panelGridArea.y + panelGridArea.height / 2.0f};
  panelCamera.rotation = 0.0f;
  panelCamera.zoom = 1.0f;
}

void ControlPanel::Update(Player &player, float dt) {
  if (!isPanelOpen)
    return;

  UpdateInventoryScroll(player);
  UpdateGridCamera();
  UpdateNodeDragging(player);
  UpdateNodeConnections(player);
}

void ControlPanel::UpdateInventoryScroll(const Player &player) {
  Vector2 mousePosScreen = GetMousePosition();
  if (CheckCollisionPointRec(mousePosScreen, panelInventoryArea)) {
    inventoryScrollOffset -= GetMouseWheelMove() * NODE_INV_ITEM_HEIGHT * 0.5f;
    float maxScroll =
        fmaxf(0.0f, (player.inventoryNodes.size() * NODE_INV_ITEM_HEIGHT) -
                        panelInventoryArea.height);
    inventoryScrollOffset = Clamp(inventoryScrollOffset, 0.0f, maxScroll);
  }
}

void ControlPanel::UpdateGridCamera() {
  Vector2 mousePosScreen = GetMousePosition();
  if (CheckCollisionPointRec(mousePosScreen, panelGridArea)) {
    Vector2 mousePosWorldBeforeZoom =
        GetScreenToWorld2D(mousePosScreen, panelCamera);
    panelCamera.zoom += GetMouseWheelMove() * 0.05f * panelCamera.zoom;
    panelCamera.zoom = Clamp(panelCamera.zoom, 0.2f, 3.0f);
    Vector2 mousePosWorldAfterZoom =
        GetScreenToWorld2D(mousePosScreen, panelCamera);
    panelCamera.target =
        Vector2Add(panelCamera.target, Vector2Subtract(mousePosWorldBeforeZoom,
                                                       mousePosWorldAfterZoom));
  }
}

void ControlPanel::UpdateNodeDragging(Player &player) {
  Vector2 mousePosScreen = GetMousePosition();
  Vector2 mousePosPanelGridWorld =
      GetScreenToWorld2D(mousePosScreen, panelCamera);

  if (CheckCollisionPointRec(mousePosScreen, panelArea)) {
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
      draggingNodeIndex = -1;

      // Check inventory area for dragging
      if (CheckCollisionPointRec(mousePosScreen, panelInventoryArea)) {
        for (size_t i = 0; i < player.inventoryNodes.size(); ++i) {
          Rectangle nodeRect = {panelInventoryArea.x + 5,
                                panelInventoryArea.y + 5 +
                                    i * NODE_INV_ITEM_HEIGHT -
                                    inventoryScrollOffset,
                                panelInventoryArea.width - 10, NODE_UI_SIZE};
          if (CheckCollisionPointRec(mousePosScreen, nodeRect)) {
            draggingNodeIndex = (int)i;
            draggingFromInventory = true;
            break;
          }
        }
      }

      // Check grid area for dragging
      if (draggingNodeIndex == -1 &&
          CheckCollisionPointRec(mousePosScreen, panelGridArea)) {
        for (size_t i = 0; i < player.placedNodes.size(); ++i) {
          if (CheckCollisionPointCircle(mousePosPanelGridWorld,
                                        player.placedNodes[i]->panelPosition,
                                        NODE_UI_SIZE / 2.0f)) {
            draggingNodeIndex = (int)i;
            draggingFromInventory = false;
            break;
          }
        }
      }
    }

    if (IsMouseButtonDown(MOUSE_LEFT_BUTTON) && draggingNodeIndex != -1 &&
        !draggingFromInventory) {
      // Update dragged node position
      if (draggingNodeIndex < (int)player.placedNodes.size()) {
        player.placedNodes[draggingNodeIndex]->panelPosition =
            mousePosPanelGridWorld;
      }
    }

    if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON) && draggingNodeIndex != -1) {
      if (draggingFromInventory) {
        HandleNodePlacement(player);
      } else {
        HandleNodeRemoval(player);
      }
      draggingNodeIndex = -1;
    }
  } else {
    if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON) && draggingNodeIndex != -1)
      draggingNodeIndex = -1;
  }
}

void ControlPanel::UpdateNodeConnections(Player &player) {
  Vector2 mousePosScreen = GetMousePosition();
  Vector2 mousePosPanelGridWorld =
      GetScreenToWorld2D(mousePosScreen, panelCamera);

  BaseNode *toNodeForConnection = nullptr;
  if (CheckCollisionPointRec(mousePosScreen, panelGridArea)) {
    for (auto &node : player.placedNodes) {
      if (node &&
          CheckCollisionPointCircle(mousePosPanelGridWorld, node->panelPosition,
                                    NODE_UI_SIZE / 2.0f)) {
        toNodeForConnection = node.get();
        break;
      }
    }
  }

  // Start connection on right-click
  if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON)) {
    connectingNodeFromId = -1;
    if (toNodeForConnection) {
      connectingNodeFromId = toNodeForConnection->id;
    }
  }

  // Complete connection on right-click release
  if (IsMouseButtonReleased(MOUSE_RIGHT_BUTTON)) {
    if (connectingNodeFromId != -1 && toNodeForConnection) {
      BaseNode *fromNode = player.GetPlayerNodeById(connectingNodeFromId);
      if (fromNode && fromNode->id != toNodeForConnection->id) {
        bool connectionChanged = false;

        // CPU Core connection rules
        if (fromNode->getNodeType() == NodeType::CPU_CORE) {
          if (toNodeForConnection->getNodeKind() == NodeKind::STAT ||
              toNodeForConnection->getNodeKind() == NodeKind::POWER ||
              toNodeForConnection->getNodeKind() == NodeKind::ACTION) {

            // Special rule: CPU can only connect to one ACTION at a time
            if (toNodeForConnection->getNodeKind() == NodeKind::ACTION) {
              // Remove existing ACTION connections
              for (size_t i = 0; i < fromNode->connectedToNodeIDs.size();) {
                BaseNode *existingTarget =
                    player.GetPlayerNodeById(fromNode->connectedToNodeIDs[i]);
                if (existingTarget &&
                    existingTarget->getNodeKind() == NodeKind::ACTION &&
                    existingTarget->id != toNodeForConnection->id) {
                  // Remove bidirectional connection
                  existingTarget->connectedFromNodeIDs.erase(
                      std::remove(existingTarget->connectedFromNodeIDs.begin(),
                                  existingTarget->connectedFromNodeIDs.end(),
                                  fromNode->id),
                      existingTarget->connectedFromNodeIDs.end());
                  fromNode->connectedToNodeIDs.erase(
                      fromNode->connectedToNodeIDs.begin() + i);
                  connectionChanged = true;
                } else {
                  i++;
                }
              }
            }

            // Add new connection if not already connected
            if (std::find(fromNode->connectedToNodeIDs.begin(),
                          fromNode->connectedToNodeIDs.end(),
                          toNodeForConnection->id) ==
                fromNode->connectedToNodeIDs.end()) {
              fromNode->connectedToNodeIDs.push_back(toNodeForConnection->id);
              toNodeForConnection->connectedFromNodeIDs.push_back(fromNode->id);
              connectionChanged = true;
            }
          }
        }
        // ACTION to ACTION sequence connections
        else if (fromNode->getNodeKind() == NodeKind::ACTION &&
                 toNodeForConnection->getNodeKind() == NodeKind::ACTION) {
          // Remove existing ACTION connections from this node
          for (size_t i = 0; i < fromNode->connectedToNodeIDs.size();) {
            BaseNode *existingTarget =
                player.GetPlayerNodeById(fromNode->connectedToNodeIDs[i]);
            if (existingTarget &&
                existingTarget->getNodeKind() == NodeKind::ACTION &&
                existingTarget->id != toNodeForConnection->id) {
              existingTarget->connectedFromNodeIDs.erase(
                  std::remove(existingTarget->connectedFromNodeIDs.begin(),
                              existingTarget->connectedFromNodeIDs.end(),
                              fromNode->id),
                  existingTarget->connectedFromNodeIDs.end());
              fromNode->connectedToNodeIDs.erase(
                  fromNode->connectedToNodeIDs.begin() + i);
              connectionChanged = true;
            } else {
              i++;
            }
          }

          // Add new connection
          if (std::find(fromNode->connectedToNodeIDs.begin(),
                        fromNode->connectedToNodeIDs.end(),
                        toNodeForConnection->id) ==
              fromNode->connectedToNodeIDs.end()) {
            fromNode->connectedToNodeIDs.push_back(toNodeForConnection->id);
            toNodeForConnection->connectedFromNodeIDs.push_back(fromNode->id);
            connectionChanged = true;
          }
        }
        // STAT to ACTION buff connections
        else if (fromNode->getNodeKind() == NodeKind::STAT &&
                 toNodeForConnection->getNodeKind() == NodeKind::ACTION) {
          if (std::find(fromNode->connectedToNodeIDs.begin(),
                        fromNode->connectedToNodeIDs.end(),
                        toNodeForConnection->id) ==
              fromNode->connectedToNodeIDs.end()) {
            fromNode->connectedToNodeIDs.push_back(toNodeForConnection->id);
            toNodeForConnection->connectedFromNodeIDs.push_back(fromNode->id);
            connectionChanged = true;
          }
        }
        // POWER to STAT/ACTION modifier connections
        else if (fromNode->getNodeKind() == NodeKind::POWER &&
                 (toNodeForConnection->getNodeKind() == NodeKind::ACTION ||
                  toNodeForConnection->getNodeKind() == NodeKind::STAT)) {
          // Remove existing POWER connections (POWER nodes can only connect to
          // one target)
          for (size_t i = 0; i < fromNode->connectedToNodeIDs.size(); i++) {
            BaseNode *existingTarget =
                player.GetPlayerNodeById(fromNode->connectedToNodeIDs[i]);
            if (existingTarget) {
              existingTarget->connectedFromNodeIDs.erase(
                  std::remove(existingTarget->connectedFromNodeIDs.begin(),
                              existingTarget->connectedFromNodeIDs.end(),
                              fromNode->id),
                  existingTarget->connectedFromNodeIDs.end());
            }
          }
          fromNode->connectedToNodeIDs.clear();

          // Add new connection
          fromNode->connectedToNodeIDs.push_back(toNodeForConnection->id);
          toNodeForConnection->connectedFromNodeIDs.push_back(fromNode->id);
          connectionChanged = true;
        }

        // Update node activations if connections changed
        if (connectionChanged) {
          // Reset active action to restart sequences
          player.activeActionNodeId = -1;
        }
      }
    }
    connectingNodeFromId = -1;
  }
}

void ControlPanel::HandleNodePlacement(Player &player) {
  Vector2 mousePosScreen = GetMousePosition();
  if (CheckCollisionPointRec(mousePosScreen, panelGridArea) &&
      draggingNodeIndex >= 0 &&
      draggingNodeIndex < (int)player.inventoryNodes.size()) {
    Vector2 mousePosPanelGridWorld =
        GetScreenToWorld2D(mousePosScreen, panelCamera);

    // Move node from inventory to placed nodes
    auto nodeToPlace = std::move(player.inventoryNodes[draggingNodeIndex]);
    nodeToPlace->isPlaced = true;
    nodeToPlace->panelPosition = mousePosPanelGridWorld;

    // Add to placed nodes and remove from inventory
    player.placedNodes.push_back(std::move(nodeToPlace));
    player.inventoryNodes.erase(player.inventoryNodes.begin() +
                                draggingNodeIndex);

    // Update node activations after placement
    // We'll need to call the NodesController's UpdateNodeActivation
  }
}

void ControlPanel::HandleNodeRemoval(Player &player) {
  Vector2 mousePosScreen = GetMousePosition();
  if (CheckCollisionPointRec(mousePosScreen, panelInventoryArea) &&
      draggingNodeIndex >= 0 &&
      draggingNodeIndex < (int)player.placedNodes.size()) {

    // Don't allow removing the CPU core node
    if (player.placedNodes[draggingNodeIndex]->getNodeType() ==
        NodeType::CPU_CORE) {
      return;
    }

    // Move node from placed back to inventory
    auto nodeToRemove = std::move(player.placedNodes[draggingNodeIndex]);
    nodeToRemove->isPlaced = false;
    nodeToRemove->isActive = false;
    nodeToRemove->isCurrentlyActiveEffect = false;
    nodeToRemove->connectedToNodeIDs.clear();
    nodeToRemove->connectedFromNodeIDs.clear();

    // Remove connections from other nodes to this node
    int removedNodeId = nodeToRemove->id;
    for (auto &otherNode : player.placedNodes) {
      if (otherNode && otherNode->id != removedNodeId) {
        // Remove from connectedToNodeIDs
        otherNode->connectedToNodeIDs.erase(
            std::remove(otherNode->connectedToNodeIDs.begin(),
                        otherNode->connectedToNodeIDs.end(), removedNodeId),
            otherNode->connectedToNodeIDs.end());
      }
    }

    // Remove connections from remaining nodes' connectedFromNodeIDs
    for (int targetId : nodeToRemove->connectedToNodeIDs) {
      for (auto &targetNode : player.placedNodes) {
        if (targetNode && targetNode->id == targetId) {
          targetNode->connectedFromNodeIDs.erase(
              std::remove(targetNode->connectedFromNodeIDs.begin(),
                          targetNode->connectedFromNodeIDs.end(),
                          removedNodeId),
              targetNode->connectedFromNodeIDs.end());
        }
      }
    }

    // Add to inventory and remove from placed nodes
    player.inventoryNodes.push_back(std::move(nodeToRemove));
    player.placedNodes.erase(player.placedNodes.begin() + draggingNodeIndex);

    // Reset active action if it was this node
    if (removedNodeId == player.activeActionNodeId) {
      player.activeActionNodeId = -1;
      player.playerShieldIsActive = false;
    }

    // Update node activations after removal
    // We'll need to call the NodesController's UpdateNodeActivation
  }
}

void ControlPanel::Draw(const Player &player) {
  if (!isPanelOpen)
    return;

  DrawRectangleRec(panelArea, ColorAlpha(BLACK, 0.85f));
  DrawRectangleLinesEx(panelArea, 1, DARKGRAY);
  DrawText("CONTROL PANEL (TAB)", (int)(panelArea.x + 10),
           (int)(panelArea.y + 10), 20, WHITE);

  DrawInventoryArea(player);
  DrawGridArea(player);
}

void ControlPanel::DrawInventoryArea(const Player &player) {
  DrawText("Inventory:", (int)panelInventoryArea.x,
           (int)(panelInventoryArea.y - 25), 15, WHITE);
  DrawRectangleRec(panelInventoryArea, ColorAlpha(DARKSLATEBLUE, 0.5f));

  BeginScissorMode((int)panelInventoryArea.x, (int)panelInventoryArea.y,
                   (int)panelInventoryArea.width,
                   (int)panelInventoryArea.height);

  // Draw inventory nodes - add safety check
  if (!player.inventoryNodes.empty()) {
    for (size_t i = 0; i < player.inventoryNodes.size(); ++i) {
      if (!player.inventoryNodes[i])
        continue; // Safety check for null pointers

      Rectangle nodeRect = {panelInventoryArea.x + 5,
                            panelInventoryArea.y + 5 +
                                i * NODE_INV_ITEM_HEIGHT -
                                inventoryScrollOffset,
                            panelInventoryArea.width - 10, NODE_UI_SIZE};
      if (nodeRect.y + nodeRect.height > panelInventoryArea.y &&
          nodeRect.y < panelInventoryArea.y + panelInventoryArea.height) {
        bool isHovered = CheckCollisionPointRec(GetMousePosition(), nodeRect);
        DrawRectangleRec(nodeRect, player.inventoryNodes[i]->color);
        DrawRectangleLinesEx(nodeRect, 2, isHovered ? YELLOW : DARKGRAY);
        DrawText(player.inventoryNodes[i]->name.c_str(), (int)(nodeRect.x + 5),
                 (int)(nodeRect.y + 5), 10, BLACK);
        if (isHovered)
          DrawText(player.inventoryNodes[i]->description.c_str(),
                   (int)(GetMousePosition().x + 15),
                   (int)(GetMousePosition().y + 5), 10, WHITE);
      }
    }
  }

  EndScissorMode();

  DrawScrollbar(player);
}

void ControlPanel::DrawGridArea(const Player &player) {
  DrawText("System Grid:", (int)panelGridArea.x, (int)(panelGridArea.y - 25),
           15, WHITE);
  DrawRectangleRec(panelGridArea, ColorAlpha(MIDNIGHTBLUE, 0.5f));

  BeginScissorMode((int)panelGridArea.x, (int)panelGridArea.y,
                   (int)panelGridArea.width, (int)panelGridArea.height);
  BeginMode2D(panelCamera);

  DrawConnections(player);
  DrawNodes(player);

  EndMode2D();
  EndScissorMode();

  DrawDraggedNode(player);
  DrawTooltips(player);
}

void ControlPanel::DrawConnections(const Player &player) {
  // Draw connections between nodes
  for (const auto &fromNode : player.placedNodes) {
    if (!fromNode)
      continue;

    for (int targetNodeId : fromNode->connectedToNodeIDs) {
      const BaseNode *toNode = player.GetPlayerNodeById(targetNodeId);
      if (toNode) {
        Color lineColor = GRAY;
        bool isDirected = false;

        // Determine line color and style based on connection type
        if (fromNode->getNodeType() == NodeType::CPU_CORE &&
            (toNode->getNodeKind() == NodeKind::STAT ||
             toNode->getNodeKind() == NodeKind::POWER)) {
          lineColor = YELLOW; // CPU to STAT/POWER
        } else if (fromNode->getNodeType() == NodeType::CPU_CORE &&
                   toNode->getNodeKind() == NodeKind::ACTION) {
          lineColor = SKYBLUE; // CPU to ACTION
          isDirected = true;
        } else if (fromNode->getNodeKind() == NodeKind::ACTION &&
                   toNode->getNodeKind() == NodeKind::ACTION) {
          lineColor = CYAN; // ACTION to ACTION sequence
          isDirected = true;
        } else if (fromNode->getNodeKind() == NodeKind::STAT &&
                   toNode->getNodeKind() == NodeKind::ACTION) {
          lineColor = STAT_ACTION_LINK_COLOR; // STAT buffs ACTION
          isDirected = true;
        } else if (fromNode->getNodeKind() == NodeKind::POWER &&
                   (toNode->getNodeKind() == NodeKind::ACTION ||
                    toNode->getNodeKind() == NodeKind::STAT)) {
          lineColor = POWER_LINK_COLOR; // POWER modifies STAT/ACTION
          isDirected = true;
        }

        // Draw the connection line
        DrawLineEx(fromNode->panelPosition, toNode->panelPosition,
                   fmaxf(1.0f, 2.0f / panelCamera.zoom), lineColor);

        // Draw arrow for directed connections
        if (isDirected) {
          Vector2 dir = Vector2Normalize(
              Vector2Subtract(toNode->panelPosition, fromNode->panelPosition));
          if (Vector2LengthSqr(dir) > 0) {
            float arrowHeadOffset =
                (NODE_UI_SIZE / 2.0f) + 2.0f / panelCamera.zoom;
            Vector2 arrowEnd = Vector2Subtract(
                toNode->panelPosition, Vector2Scale(dir, arrowHeadOffset));

            Vector2 p1 = Vector2Subtract(
                arrowEnd, Vector2Scale(Vector2Rotate(dir, 30 * DEG2RAD),
                                       8.0f / panelCamera.zoom));
            Vector2 p2 = Vector2Subtract(
                arrowEnd, Vector2Scale(Vector2Rotate(dir, -30 * DEG2RAD),
                                       8.0f / panelCamera.zoom));
            DrawLineV(arrowEnd, p1, lineColor);
            DrawLineV(arrowEnd, p2, lineColor);
          }
        }
      }
    }
  }

  // Draw connection preview line while connecting
  if (connectingNodeFromId != -1) {
    const BaseNode *fromNode = player.GetPlayerNodeById(connectingNodeFromId);
    if (fromNode) {
      DrawLineEx(fromNode->panelPosition,
                 GetScreenToWorld2D(GetMousePosition(), panelCamera),
                 fmaxf(1.0f, 2.0f / panelCamera.zoom), LIGHTGRAY);
    }
  }
}

void ControlPanel::DrawNodes(const Player &player) {
  for (const auto &node : player.placedNodes) {
    if (!node)
      continue; // Safety check for null pointers
    Color nodeDrawColor = node->color;
    if (node->id == player.activeActionNodeId &&
        node->isCurrentlyActiveEffect) {
      float pulse = (sinf(GetTime() * 5.0f) + 1.0f) / 2.0f;
      nodeDrawColor.r =
          (unsigned char)Clamp(node->color.r + pulse * 50, 0, 255);
      nodeDrawColor.g =
          (unsigned char)Clamp(node->color.g + pulse * 50, 0, 255);
      nodeDrawColor.b =
          (unsigned char)Clamp(node->color.b + pulse * 50, 0, 255);
    }
    DrawCircleV(node->panelPosition, NODE_UI_SIZE / 2.0f, nodeDrawColor);

    Color borderColor = DARKGRAY;
    if (node->isActive && (node->getNodeKind() == NodeKind::STAT ||
                           node->getNodeKind() == NodeKind::POWER ||
                           node->getNodeType() == NodeType::CPU_CORE))
      borderColor = YELLOW;
    else if (node->id == player.activeActionNodeId &&
             node->isCurrentlyActiveEffect)
      borderColor = WHITE;
    else if (node->getNodeKind() == NodeKind::ACTION &&
             !node->isCurrentlyActiveEffect &&
             player.activeActionNodeId == node->id)
      borderColor = LIGHTGRAY;

    DrawCircleLines((int)node->panelPosition.x, (int)node->panelPosition.y,
                    NODE_UI_SIZE / 2.0f, borderColor);

    float textSize = fmaxf(4.0f, 10.0f / panelCamera.zoom);
    Vector2 textSzMeasure =
        MeasureTextEx(GetFontDefault(), node->name.c_str(), textSize, 1.0f);
    DrawTextEx(GetFontDefault(), node->name.c_str(),
               {node->panelPosition.x - textSzMeasure.x / 2.0f,
                node->panelPosition.y - textSzMeasure.y / 2.0f},
               textSize, 1.0f, BLACK);
  }
}

void ControlPanel::DrawDraggedNode(const Player &player) {
  if (draggingNodeIndex != -1 && draggingFromInventory &&
      draggingNodeIndex < (int)player.inventoryNodes.size()) {
    const auto &node = player.inventoryNodes[draggingNodeIndex];
    if (node) {
      DrawCircleV(GetMousePosition(), NODE_UI_SIZE / 2.0f,
                  ColorAlpha(node->color, 0.7f));
      DrawText(node->name.c_str(),
               (int)(GetMousePosition().x -
                     MeasureText(node->name.c_str(), 10) / 2.0f),
               (int)(GetMousePosition().y - 5), 10, BLACK);
    }
  }
}

void ControlPanel::DrawTooltips(const Player &player) {
  Vector2 mousePosScreen = GetMousePosition();
  if (CheckCollisionPointRec(mousePosScreen, panelGridArea)) {
    Vector2 worldMouse = GetScreenToWorld2D(mousePosScreen, panelCamera);
    for (const auto &node : player.placedNodes) {
      if (!node)
        continue;

      if (CheckCollisionPointCircle(worldMouse, node->panelPosition,
                                    NODE_UI_SIZE / 2.0f)) {
        int yOff = 5;
        Vector2 tooltipPos = {mousePosScreen.x + 10,
                              mousePosScreen.y + yOff - 2};

        std::string descText = node->description;
        std::string line2Text =
            TextFormat("ID:%d Val:%.1f", node->id, node->value);
        std::string line3Text;

        if (node->getNodeKind() == NodeKind::ACTION) {
          line3Text = TextFormat("Dur:%.1fs Eff:%s", node->duration,
                                 node->isCurrentlyActiveEffect ? "ON" : "OFF");
        } else if (node->getNodeKind() == NodeKind::STAT ||
                   node->getNodeKind() == NodeKind::POWER) {
          line3Text = TextFormat("Active:%s", node->isActive ? "YES" : "NO");
        }

        float maxWidth = 0;
        maxWidth = fmaxf(maxWidth, (float)MeasureText(descText.c_str(), 10));
        maxWidth = fmaxf(maxWidth, (float)MeasureText(line2Text.c_str(), 10));
        if (!line3Text.empty())
          maxWidth = fmaxf(maxWidth, (float)MeasureText(line3Text.c_str(), 10));
        maxWidth = fmaxf(120.0f, maxWidth + 10); // Min width + Padding

        float tooltipHeight = 12 + (line3Text.empty() ? 0 : 12) + 12 + 4;

        DrawRectangle((int)tooltipPos.x, (int)tooltipPos.y, (int)maxWidth,
                      (int)tooltipHeight, ColorAlpha(BLACK, 0.8f));
        DrawText(descText.c_str(), (int)(tooltipPos.x + 5),
                 (int)(tooltipPos.y + 2), 10, WHITE);
        yOff += 12;
        DrawText(line2Text.c_str(), (int)(tooltipPos.x + 5),
                 (int)(tooltipPos.y + yOff - 3 + 2), 10, WHITE);
        yOff += 12;
        if (!line3Text.empty())
          DrawText(line3Text.c_str(), (int)(tooltipPos.x + 5),
                   (int)(tooltipPos.y + yOff - 3 + 2), 10, WHITE);
        break;
      }
    }
  }
}

void ControlPanel::DrawScrollbar(const Player &player) {
  if (player.inventoryNodes.size() * NODE_INV_ITEM_HEIGHT >
      panelInventoryArea.height) {
    float contentHeight = player.inventoryNodes.size() * NODE_INV_ITEM_HEIGHT;
    float viewableRatio = panelInventoryArea.height / contentHeight;
    float scrollbarHeight = panelInventoryArea.height * viewableRatio;
    float scrollRatio = (contentHeight > panelInventoryArea.height &&
                         panelInventoryArea.height > 0)
                            ? (inventoryScrollOffset /
                               (contentHeight - panelInventoryArea.height))
                            : 0;
    float scrollbarY =
        panelInventoryArea.y +
        scrollRatio * (panelInventoryArea.height - scrollbarHeight);
    DrawRectangle((int)(panelInventoryArea.x + panelInventoryArea.width - 8),
                  (int)scrollbarY, 5, (int)fmaxf(10.0f, scrollbarHeight),
                  LIGHTGRAY);
  }
}
