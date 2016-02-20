#pragma once

#include <vector>

#include "color.hh"
#include "util/vector2.hh"

struct Polygon {
  Polygon(std::vector<Vector2> points, Color color)
    : points(points), color(color) { }

  std::vector<Vector2> points;
  Color color;
};
