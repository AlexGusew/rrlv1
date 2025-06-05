#pragma once
#include <raylib.h>

class InputDisplay {
public:
  InputDisplay();
  ~InputDisplay();

  void Update();
  void Draw() const;

private:
  bool active;
  int lastCharPressed; // Unicode codepoint (int)
};
