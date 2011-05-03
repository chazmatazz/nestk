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

#include "Cone.h"


Cone::Cone(XnPoint3D center, XnFloat xRot, XnFloat yRot, XnFloat radius, XnFloat length, XnBoundingBox3D& boundingBox)
{
	this->center = center;
	this->xRot = xRot;
	this->yRot = yRot;
	this->length = length;
	this->radius = radius;
    this->boundingBox = boundingBox;
	quadric = gluNewQuadric();
}

void Cone::draw(DrawState drawState)
{
    if(drawState == DRAWSTATE_HOVER) {
        drawBoundingBox(getBoundingBox());
    }
	glPushMatrix();
	glTranslatef(0.0f,0.0f,-10.0f);
	glTranslatef(this->center.X,this->center.Y,this->center.Z);
	glRotatef(this->xRot, 1.0f, 0.0f, 0.0f);
	glRotatef(this->yRot, 0.0f, 1.0f, 0.0f);
	glTranslatef(0.0f,0.0f,-this->length/2);
    setglColor(drawState, SIDE1); // side
	gluCylinder(quadric,this->radius,0,length,32,16);
    setglColor(drawState, SIDE2); // Top
	//gluQuadricOrientation(quadric,GLU_OUTSIDE);
	gluDisk( quadric, 0.0, radius, 32, 16);
	glTranslatef( 0,0,this->length );
    setglColor(drawState, SIDE2); // bottom
	//gluQuadricOrientation(quadric,GLU_INSIDE);
	gluDisk( quadric, 0.0, 0, 32, 16);
	glTranslatef( 0,0,this->length );
    glPopMatrix();
}
void Cone::rotate(XnPoint3D d)
{
    // intentionally swapped
	this->xRot += d.Y;
	this->yRot += d.X;
}
void Cone::displace(XnPoint3D d)
{
	this->center.X += d.X;
	this->center.Y += d.Y;
	this->center.Z += d.Z;
    clamp_point(center, boundingBox);
}
void Cone::resize(XnPoint3D d)
{
	this->radius += d.X;
	this->length += d.Y;
    clamp_val(radius, SIZE_MIN_DIM/2, SIZE_MAX_DIM/2);
    clamp_val(length, SIZE_MIN_DIM, SIZE_MAX_DIM);
}

XnPoint3D Cone::getCenter() {
    return center;
}
XnPoint3D Cone::getBoundingBoxSize() {
    XnFloat max;
    if(length > radius*2) {
        max = length;
    } else {
        max = radius*2;
    }
    max *= SQRT_2 * SHRINK;
    XnPoint3D size;
    size.X = max;
    size.Y = max;
    size.Z = max;
    return size;
}
XnBoundingBox3D Cone::getBoundingBox() {
    return calcBoundingBox(getCenter(), getBoundingBoxSize());
}
XnFloat Cone::getCenterDistSq(XnPoint3D p)
{
    return dist_sq(p, getCenter());
}
XnBool Cone::isInside(XnPoint3D p) {
    return inBoundingBox(p, getBoundingBox());
}
Cone::~Cone(void){}
