//  Copyright 2016 Martin Fracker, Jr.
//  All Rights Reserved.
// 
//  This project is free software, released under the terms
//  of the GNU General Public License v3. Please see the
//  file LICENSE in the root directory or visit
//  www.gnu.org/licenses/gpl-3.0.en.html for license terms.

#include "clip/rectangle.hh"
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

#define ImageW 800
#define ImageH 600

enum class Mode { Scanning, Clipping } mode;

float framebuffer[ImageH][ImageW][3];
std::vector<Vector2> pointsBuffer;
std::vector<Polygon> polygons;
std::default_random_engine generator;
std::uniform_real_distribution<float> distribution(0.0, 1.0);
Rectangle clippingRect;
bool clip = false;
bool drawingClippingRect = false;

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

void clipPolygon(Polygon polygon, Rectangle rect) {
  
}

void clipPolygons() {
  if (clip) {
    for (auto& polygon : polygons)
      clipPolygon(polygon, clippingRect);
  }
}

void drawClippingRect() {
  if (!drawingClippingRect)
    return;
  glBegin(GL_LINE_LOOP);
  glColor3f(1.0, 1.0, 1.0);
  glVertex2i(clippingRect.p1.x, clippingRect.p1.y);
  glVertex2i(clippingRect.p2.x, clippingRect.p1.y);
  glVertex2i(clippingRect.p2.x, clippingRect.p2.y);
  glVertex2i(clippingRect.p1.x, clippingRect.p2.y);
  glEnd();
}

void display(void) {
  clearFramebuffer();
  glClear(GL_COLOR_BUFFER_BIT);
  clipPolygons();
  for (auto polygon : polygons)
    scanfill(polygon.points, polygon.color);
  glDrawPixels(ImageW, ImageH, GL_RGB, GL_FLOAT, framebuffer);
  drawPointsBuffer();
  drawClippingRect();
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
  mode = Mode::Scanning;
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

bool settingClippingRectP2 = false;
void mouse(int button, int status, int x, int y) {
  switch(button) {
  case GLUT_LEFT_BUTTON:
    if (status != GLUT_UP) {
      if (mode == Mode::Scanning)
        addToPointsBuffer({ x, y });
      else
        if (!settingClippingRectP2 && !clip) {
          settingClippingRectP2 = true;
          clippingRect.p1 = { x, y };
        }
    } else {
      if (drawingClippingRect) {
        drawingClippingRect = false;
        settingClippingRectP2 = false;
        clip = true;
      }
    }
    break;
  case GLUT_RIGHT_BUTTON:
    if (status == GLUT_UP)
      break;
    if (mode == Mode::Scanning && pointsBuffer.size() > 1) {
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

void motion(int x, int y) {
  if (settingClippingRectP2 && !clip) {
    clippingRect.p2 = { x, y };
    drawingClippingRect = true;
    glutPostRedisplay();
  }
}

void keyboard(unsigned char c, int x, int y) {
  if (c == 'c')
    mode = Mode::Clipping;
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
  glutMotionFunc(motion);
  glutKeyboardFunc(keyboard);
  glutMainLoop();
  return 0;
}
