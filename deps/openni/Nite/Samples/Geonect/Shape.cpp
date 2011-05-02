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



Shape::Shape(void){}
Shape::~Shape(void){}