//  Copyright 2016 Martin Fracker, Jr.
//  All Rights Reserved.
// 
//  This project is free software, released under the terms
//  of the GNU General Public License v3. Please see the
//  file LICENSE in the root directory or visit
//  www.gnu.org/licenses/gpl-3.0.en.html for license terms.

#pragma once

#include "util/vector2.hh"

// A rectangle specified by two corner points opposite each other
struct Rectangle {
  Vector2 p1;
  Vector2 p2;
};
