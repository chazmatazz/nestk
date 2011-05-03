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

#include "Cylinder.h"


Cylinder::Cylinder(XnPoint3D center, XnFloat xRot, XnFloat yRot, XnFloat radius, XnFloat length, XnBoundingBox3D& boundingBox)
{
	this->center = center;
    
	this->xRot = xRot;
	this->yRot = yRot;
    
	this->length = length;
	this->radius = radius;
    this->boundingBox = boundingBox;
	quadric = gluNewQuadric();
}

void Cylinder::draw(DrawState drawState)
{
    //if(drawState == DRAWSTATE_HOVER) {
    //    drawBoundingBox(getBoundingBox());
    //}
	glPushMatrix();
    glTranslatef(this->center.X,this->center.Y,this->center.Z);
	glTranslatef(0.0f,0.0f,-10.0f);
	glRotatef(this->xRot, 1.0f, 0.0f, 0.0f);
	glRotatef(this->yRot, 0.0f, 1.0f, 0.0f);
	glTranslatef(0.0f,0.0f,-this->length/2);
    setglColor(drawState, SIDE1); // side
	gluCylinder(quadric,this->radius,this->radius,length,32,16);
    setglColor(drawState, SIDE2); // Top
	//gluQuadricOrientation(quadric,GLU_OUTSIDE);
	gluDisk( quadric, 0.0, radius, 32, 16);
	glTranslatef( 0,0,this->length );
    setglColor(drawState, SIDE2); // bottom
	//gluQuadricOrientation(quadric,GLU_INSIDE);
	gluDisk( quadric, 0.0, radius, 32, 16);
    glPopMatrix();
}
void Cylinder::rotate(XnPoint3D d)
{
    // intentionally swapped
	this->xRot += d.Y;
	this->yRot += d.X;
}
void Cylinder::displace(XnPoint3D d)
{
	this->center.X += d.X;
	this->center.Y += d.Y;
	this->center.Z += d.Z;
    clamp_point(center, boundingBox);
}
void Cylinder::resize(XnPoint3D d)
{
	this->length += d.X;
	this->radius += d.Y;
    clamp_val(length, SIZE_MIN_DIM, SIZE_MAX_DIM);
    clamp_val(radius, SIZE_MIN_DIM/2, SIZE_MAX_DIM/2);
}
XnPoint3D Cylinder::getCenter() {
    return center;
}
XnPoint3D Cylinder::getBoundingBoxSize() {
    XnFloat max;
    if(length > radius*2) {
        max = length;
    } else {
        max = radius*2;
    }
    max *= SQRT_2 * SHRINK;
    XnPoint3D s;
    s.X = max;
    s.Y = max;
    s.Z = max;
    return s;
}
XnBoundingBox3D Cylinder::getBoundingBox() {
    return calcBoundingBox(getCenter(), getBoundingBoxSize());
}

XnFloat Cylinder::getCenterDistSq(XnPoint3D p)
{
    return dist_sq(p, getCenter());
}
XnBool Cylinder::isInside(XnPoint3D p) {
    return inBoundingBox(p, getBoundingBox());
}
Cylinder::~Cylinder(void){}
