#include "RectPrism.h"
#include <XnCppWrapper.h>

#if (XN_PLATFORM == XN_PLATFORM_MACOSX)
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#include "StdAfx.h"
#endif



RectPrism::RectPrism(XnPoint3D center, XnFloat xRot, XnFloat yRot, XnFloat width, XnFloat height, XnFloat depth, XnBoundingBox3D& boundingBox)
{
    this->center = center;
	this->xRot = xRot;
	this->yRot = yRot;
	this->width = width;
	this->height = height;
	this->depth = depth;
    this->boundingBox = boundingBox;
}

void RectPrism::draw(DrawState drawState)
{
    if(drawState == DRAWSTATE_HOVER) {
        drawBoundingBox(getBoundingBox());
    }
	glPushMatrix();
	glTranslatef(0.0f,0.0f,-10.0f);
	glTranslatef(this->center.X,this->center.Y,this->center.Z);
	glRotatef(this->xRot, 1.0f, 0.0f, 0.0f);
	glRotatef(this->yRot, 0.0f, 1.0f, 0.0f);
	drawBox(drawState);
	glPopMatrix();
}

void RectPrism::rotate(XnPoint3D d)
{
    // intentionally swapped
	this->xRot += d.Y;
	this->yRot += d.X;
}
void RectPrism::displace(XnPoint3D d)
{
	this->center.X += d.X;
	this->center.Y += d.Y;
	this->center.Z += d.Z;
    clamp_point(center, boundingBox);
}
void RectPrism::resize(XnPoint3D d)
{
	this->width += d.X;
	this->height += d.Y;
	this->depth += d.Z;
    clamp_val(width, SIZE_MIN_DIM/2, SIZE_MAX_DIM/2);
    clamp_val(height, SIZE_MIN_DIM/2, SIZE_MAX_DIM/2);
    clamp_val(height, SIZE_MIN_DIM/2, SIZE_MAX_DIM/2);
}
XnPoint3D RectPrism::getCenter() {
    return center;
}
XnPoint3D RectPrism::getBoundingBoxSize() {
    XnFloat max;
    if(width > height) {
        max = width;
    } else {
        max = height;
    }
    if(depth > max) {
        max = depth;
    }
    max *= SQRT_2 * SHRINK;
    XnPoint3D size;
    size.X = max;
    size.Y = max;
    size.Z = max;
    return size;
}
XnBoundingBox3D RectPrism::getBoundingBox() {
    return calcBoundingBox(getCenter(), getBoundingBoxSize());
}
float RectPrism::getCenterDistSq(XnPoint3D p)
{
    return dist_sq(p, getCenter());
}
XnBool RectPrism::isInside(XnPoint3D p) {
    return inBoundingBox(p, getBoundingBox());
}

void RectPrism::drawBox(DrawState drawState)
{
	glBegin(GL_QUADS);

        setglColor(drawState, SIDE1);
		// FRONT
        glVertex3f(-width/2, -height/2, depth/2);
		glVertex3f( width/2, -height/2, depth/2);
		glVertex3f( width/2, height/2, depth/2);
		glVertex3f(-width/2, height/2, depth/2);
		// BACK
		glVertex3f(-width/2, -height/2, -depth/2);
		glVertex3f(-width/2, height/2, -depth/2);
		glVertex3f( width/2, height/2, -depth/2);
		glVertex3f( width/2, -height/2, -depth/2);

        setglColor(drawState, SIDE2);
		// LEFT
		glVertex3f(-width/2, -height/2, depth/2);
		glVertex3f(-width/2, height/2, depth/2);
		glVertex3f(-width/2, height/2, -depth/2);
		glVertex3f(-width/2, -height/2, -depth/2);
		// RIGHT
		glVertex3f( width/2, -height/2, -depth/2);
		glVertex3f( width/2, height/2, -depth/2);
		glVertex3f( width/2, height/2, depth/2);
		glVertex3f( width/2, -height/2, depth/2);

		 setglColor(drawState, SIDE3);
		// TOP
		glVertex3f(-width/2, height/2, depth/2);
		glVertex3f( width/2, height/2, depth/2);
		glVertex3f( width/2, height/2, -depth/2);
		glVertex3f(-width/2, height/2, -depth/2);
		// BOTTOM
		glVertex3f(-width/2, -height/2, depth/2);
		glVertex3f(-width/2, -height/2, -depth/2);
		glVertex3f( width/2, -height/2, -depth/2);
		glVertex3f( width/2, -height/2, depth/2);

	glEnd();

}
RectPrism::~RectPrism(void) {}
