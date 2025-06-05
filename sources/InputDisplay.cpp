#include "InputDisplay.h"

InputDisplay::InputDisplay() : active(false), lastCharPressed(0) {}

InputDisplay::~InputDisplay() {}

void InputDisplay::Update() {
  // Toggle display with F2
  if (IsKeyPressed(KEY_F2)) {
    active = !active;
  }

  if (active) {
    int key = 0;
    // GetCharPressed returns 0 if no key is pressed
    while ((key = GetCharPressed()) != 0) {
      lastCharPressed = key;
      // Only get the most recent character per frame
    }
  }
}

void InputDisplay::Draw() const {
  if (!active)
    return;

  if (lastCharPressed > 0) {
    const char *buf = TextFormat("Last char: '%c' (U+%04X)", lastCharPressed,
                                 lastCharPressed);
    DrawText(buf, 10, 200, 14, RAYWHITE);
  } else {
    DrawText("Press any key...", 10, 200, 14, RAYWHITE);
  }
}
