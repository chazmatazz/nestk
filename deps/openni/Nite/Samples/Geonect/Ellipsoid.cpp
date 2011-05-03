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

#include "Ellipsoid.h"


Ellipsoid::Ellipsoid(float x, float y, float z, float xRot, float yRot, float radius1, float radius2, float radius3, XnBoundingBox3D& boundingBox)
{
	this->xPos = x;
	this->yPos = y;
	this->zPos = z;
	this->xRot = xRot;
	this->yRot = yRot;
    this->radius1 = radius1;
	this->radius2 = radius2;
    this->radius3 = radius3;
    this->boundingBox = boundingBox;
}

void Ellipsoid::draw(DrawState drawState)
{
	glPushMatrix();
	glTranslatef(0.0f,0.0f,-10.0f);
	glTranslatef(this->xPos,this->yPos,this->zPos);
	glRotatef(this->xRot, 1.0f, 0.0f, 0.0f);
	glRotatef(this->yRot, 0.0f, 1.0f, 0.0f);
    glScalef(this->radius1, this->radius2, this->radius3);
    setglColor(drawState, SIDE1); // side
    glutSolidSphere(1, 8, 8);
    glPopMatrix();
}
void Ellipsoid::rotate(float xRot,float yRot)
{
	this->xRot += xRot;
	this->yRot += yRot;
}
void Ellipsoid::displace(float x, float y, float z)
{
	this->xPos += x;
	this->yPos += y;
	this->zPos += z;
}
void Ellipsoid::resize(float x, float y, float z)
{
	this->radius1 += x;
	this->radius2 += y;
    this->radius3 += z;
}
float Ellipsoid::getCenterDistSq(float a, float b, float c)
{
    XnPoint3D pt1;
    pt1.X = a;
    pt1.Y = b;
    pt1.Z = c;
    XnPoint3D pt2;
    pt2.X = this->xPos;
    pt2.Y = this->yPos;
    pt2.Z = this->zPos;
    return dist_sq(pt1, pt2);
}
Ellipsoid::~Ellipsoid(void){}