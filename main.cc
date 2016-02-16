//  Copyright 2016 Martin Fracker, Jr.
//  All Rights Reserved.
// 
//  This project is free software, released under the terms
//  of the GNU General Public License v3. Please see the
//  file LICENSE in the root directory or visit
//  www.gnu.org/licenses/gpl-3.0.en.html for license terms.

#include "util/vector2.hh"

#include<GL/glut.h>

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

struct Color {
  float r, g, b;    // Color (R, G, B values)
};


// Draws the scene
void drawit(void) {
  glDrawPixels(ImageW, ImageH, GL_RGB, GL_FLOAT, framebuffer);
  glFlush();
}

// Clears framebuffer to black
void clearFramebuffer() {
  int i, j;

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
  color = clampColorValues(color);
  // changes the origin from the lower-left corner to the upper-left corner
  position.y = ImageH - 1 - position.y;

  framebuffer[position.y][position.x][0] = color.r;
  framebuffer[position.y][position.x][1] = color.g;
  framebuffer[position.y][position.x][2] = color.b;
}

void display(void) {
  //The next two lines of code are for demonstration only.
  //They show how to draw a line from (0, 0) to (100, 100)
  int i;
  for(i=0;i<=100;i++) setFramebuffer({i, i}, {1.0, 1.0, 1.0});

  drawit();
}

void init(void) {
  clearFramebuffer();
}

int main(int argc,  char** argv) {
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_SINGLE|GLUT_RGB);
  glutInitWindowSize(ImageW, ImageH);
  glutInitWindowPosition(100, 100);
  glutCreateWindow("Martin Fracker - Homework 2");
  init();  
  glutDisplayFunc(display);
  glutMainLoop();
  return 0;
}
