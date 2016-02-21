#pragma once

struct Color {
  float r, g, b;    // Color (R, G, B values)
};

bool operator==(Color lhs, Color rhs) {
  return lhs.r == rhs.r && lhs.g == rhs.g && lhs.b == rhs.b;
}
