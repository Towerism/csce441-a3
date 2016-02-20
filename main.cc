//  Copyright 2016 Martin Fracker, Jr.
//  All Rights Reserved.
// 
//  This project is free software, released under the terms
//  of the GNU General Public License v3. Please see the
//  file LICENSE in the root directory or visit
//  www.gnu.org/licenses/gpl-3.0.en.html for license terms.

#include "scan/activeEdgeList.hh"
#include "scan/activeEdgeTable.hh"
#include "scan/color.hh"
#include "scan/edge.hh"
#include "scan/polygon.hh"
#include "util/vector2.hh"

#include <algorithm>
#include <GL/glut.h>
#include <iostream>
#include <list>
#include <limits>
#include <random>
#include <utility>
#include <vector>

/******************************************************************
  Notes:
  Image size is 400 by 400 by default.  You may adjust this if
    you want to.
  You can assume the window will NOT be resized.
  Call clearFramebuffer to clear the entire framebuffer.
  Call setFramebuffer to set a pixel.  This should be the only
    routine you use to set the color (other than clearing the
    entire framebuffer).  drawit() will cause the current
    framebuffer to be displayed.
  As is,  your scan conversion should probably be called from
    within the display function.  There is a very short sample
    of code there now.
  You may add code to any of the subroutines here,   You probably
    want to leave the drawit,  clearFramebuffer,  and
    setFramebuffer commands alone,  though.
  *****************************************************************/

#define ImageW 400
#define ImageH 400

float framebuffer[ImageH][ImageW][3];
std::vector<Vector2> pointsBuffer;
std::vector<Polygon> polygons;
std::default_random_engine generator;
std::uniform_real_distribution<float> distribution(0.0, 1.0);

// Clears framebuffer to black
void clearFramebuffer() {
  std::size_t i, j;

  for(i=0;i<ImageH;i++) {
    for (j=0;j<ImageW;j++) {
      framebuffer[i][j][0] = 0.0;
      framebuffer[i][j][1] = 0.0;
      framebuffer[i][j][2] = 0.0;
    }
  }
}

float clampFloat(float f) {
  if (f < 0.0)
    return 0.0;
  if (f > 1.0)
    return 1.0;
  return f;
}

// make sure color values are between 0 and 1
Color clampColorValues(Color color) {
  return { clampFloat(color.r), clampFloat(color.g), clampFloat(color.b) };
}

// Sets pixel x, y to the color RGB
// I've made a small change to this function to make the pixels match
// those returned by the glutMouseFunc exactly - Scott Schaefer 
// Made the function less ugly - Martin Fracker
void setFramebuffer(Vector2 position, Color color) {
  // changes the origin from the lower-left corner to the upper-left corner
  position.y = ImageH - 1 - position.y;

  color = clampColorValues(color);
  if (position.y < 0 || position.y > ImageH ||
      position.x < 0 || position.x > ImageW)
    return;
  framebuffer[position.y][position.x][0] = color.r;
  framebuffer[position.y][position.x][1] = color.g;
  framebuffer[position.y][position.x][2] = color.b;
}

void drawScanLine(int y, int startX, int endX, Color color) {
  for (int x = startX; x < endX - 1; ++x) {
    setFramebuffer({x, y}, color);
  }
}

void scanfill(std::vector<Vector2> points, Color color) {
  std::list<Edge> edges = makeEdges(points);
  ActiveEdgeTable edgeTable = makeActiveEdgeTable(edges);
  ActiveEdgeList edgeList(findMinYFromEdges(edges));
  for (auto list : edgeTable) {
    edgeList.add(list);
    for (std::size_t i = 0; i < edgeList.size(); i += 2) {
      drawScanLine(edgeList.getCurrentY(),
                   edgeList[i].currentX,
                   edgeList[i + 1].currentX,
                   color);
    }
  }
}

void drawPointsBuffer() {
  glBegin(GL_LINE_STRIP);
  glColor3f(1.0, 1.0, 1.0);
  for (auto point : pointsBuffer) {
    glVertex2i(point.x, point.y);
  }
  glEnd();
}

void display(void) {
  clearFramebuffer();
  glClear(GL_COLOR_BUFFER_BIT);
  for (auto polygon : polygons)
    scanfill(polygon.points, polygon.color);
  glDrawPixels(ImageW, ImageH, GL_RGB, GL_FLOAT, framebuffer);
  drawPointsBuffer();
  glFlush();
}

void init(void) {
  clearFramebuffer();
  glViewport(0, 0, (GLsizei)ImageW, (GLsizei)ImageH);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluOrtho2D(0.0, ImageW, ImageH, 0.0);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
}

bool addToPointsBuffer(Vector2 point) {
  auto it = std::find_if(pointsBuffer.begin(), pointsBuffer.end(),
                         [point](Vector2 pt) { return point == pt; });
  if (it == pointsBuffer.end()) {
    pointsBuffer.push_back(point);
    return true;
  }
  return false;
}

Color randomColor() {
  Color color;
  color.r = distribution(generator);
  color.g = distribution(generator);
  color.b = distribution(generator);
  return color;
}

void mouse(int button, int status, int x, int y) {
  switch(button) {
  case GLUT_LEFT_BUTTON:
    if (status == GLUT_UP)
      break;
    addToPointsBuffer({ x, y });
    break;
  case GLUT_RIGHT_BUTTON:
    if (status == GLUT_UP)
      break;
    if (pointsBuffer.size() > 1) {
      if (addToPointsBuffer({ x, y })) {
        Color color = randomColor();
        polygons.emplace_back(pointsBuffer, color);
        pointsBuffer.clear();
      }
    }
    break;
  }
  glutPostRedisplay();
}

int main(int argc,  char** argv) {
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_SINGLE|GLUT_RGB);
  glutInitWindowSize(ImageW, ImageH);
  glutInitWindowPosition(100, 100);
  glutCreateWindow("Martin Fracker - Homework 2");
  init();  
  glutDisplayFunc(display);
  glutMouseFunc(mouse);
  glutMainLoop();
  return 0;
}
