#ifdef USE_GLUT
#if (XN_PLATFORM == XN_PLATFORM_MACOSX)
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#include "StdAfx.h"
#endif
#else
#include "opengles.h"
#endif

#include "Shape.h"

void setglColor(DrawState d, ObjectFace f) {
  switch(d) {
  case DRAWSTATE_UNSELECTED:
    // blue
    switch(f) {
    case SIDE1:
      glColor3f(0.0f, 0.0f, 1.0f);
      break;
    case SIDE2:
      glColor3f(0.0f, 0.0f, 0.7f);
      break;
    case SIDE3:
      glColor3f(0.0f, 0.0f, 0.3f);
      break;
    }
    break;
        case DRAWSTATE_HOVER:
          // red
          switch(f) {
          case SIDE1:
            glColor3f(1.0f, 0.0f, 0.0f);
            break;
          case SIDE2:
            glColor3f(0.7f, 0.0f, 0.0f);
            break;
          case SIDE3:
            glColor3f(0.3f, 0.0f, 0.0f);
            break;
          }
          break;
  case DRAWSTATE_SELECTED:
    // green
    switch(f) {
    case SIDE1:
      glColor3f(0.0f, 1.0f, 0.0f);
      break;
    case SIDE2:
      glColor3f(0.0f, 0.7f, 0.0f);
      break;
    case SIDE3:
      glColor3f(0.0f, 0.3f, 0.0f);
      break;
    }
    break;
  }
}

XnBoundingBox3D calcBoundingBox(XnPoint3D center, XnPoint3D size) {
    XnBoundingBox3D b;
    b.LeftBottomNear.X = center.X - size.X/2;
    b.LeftBottomNear.Y = center.Y + size.Y/2;
    b.LeftBottomNear.Z = center.Z - size.Z/2;
    b.RightTopFar.X = center.X + size.X/2;
    b.RightTopFar.Y = center.Y - size.Y/2;
    b.RightTopFar.Z = center.Z + size.Z/2;
    return b;
}

XnBool inBoundingBox(XnPoint3D p, XnBoundingBox3D b) {
    return p.X > b.LeftBottomNear.X 
    && p.Y < b.LeftBottomNear.Y 
    && p.Z > b.LeftBottomNear.Z 
    && p.X < b.RightTopFar.X
    && p.Y > b.RightTopFar.Y
    && p.Z < b.RightTopFar.Z;
}

void clamp_point(XnPoint3D& p, XnBoundingBox3D b) {
    if(p.X < b.LeftBottomNear.X) {
        p.X = b.LeftBottomNear.X;
    }
    if(p.Y > b.LeftBottomNear.Y) {
        p.Y = b.LeftBottomNear.Y;
    }
    if(p.Z < b.LeftBottomNear.Z) {
        p.Z = b.LeftBottomNear.Z;
    }
    
    
    if(p.X > b.RightTopFar.X) {
        p.X = b.RightTopFar.X;
    }
    if(p.Y < b.RightTopFar.Y) {
        p.Y = b.RightTopFar.Y;
    }
    if(p.Z > b.RightTopFar.Z) {
        p.Z = b.RightTopFar.Z;
    }
}

void clamp_val(XnFloat& v, XnFloat min, XnFloat max) {
    if (v < min) {
        v = min;
    } else if (v > max) {
        v = max;
    }
}

void drawBoundingBox(XnBoundingBox3D b) {
    XnPoint3D size;
    size.X = b.RightTopFar.X - b.LeftBottomNear.X;
    size.Y = b.LeftBottomNear.Y - b.RightTopFar.Y;
    size.Z = b.RightTopFar.Z - b.LeftBottomNear.Z;
    glPushMatrix();
    glColor3f(1.0f, 1.0f, 1.0f);
    glTranslatef(b.RightTopFar.X - size.X/2, b.LeftBottomNear.Y - size.Y/2, b.RightTopFar.Z - size.Z/2);
    glScalef(size.X, size.Y, size.Z);
    glutWireCube(1);
    glPopMatrix();
}

Shape::Shape(void){}
Shape::~Shape(void){}
