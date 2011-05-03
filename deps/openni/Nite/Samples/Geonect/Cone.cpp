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


Cone::Cone(float x, float y, float z, float xRot, float yRot, float radius, float length, XnBoundingBox3D& boundingBox)
{
	this->xPos = x;
	this->yPos = y;
	this->zPos = z;
	this->xRot = xRot;
	this->yRot = yRot;
	this->length = length;
	this->radius = radius;
    this->boundingBox = boundingBox;
	quadric = gluNewQuadric();
}

void Cone::draw(DrawState drawState)
{
	glPushMatrix();
	glTranslatef(0.0f,0.0f,-10.0f);
	glTranslatef(this->xPos,this->yPos,this->zPos);
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
void Cone::rotate(float xRot,float yRot)
{
	this->xRot += xRot;
	this->yRot += yRot;
}
void Cone::displace(float x, float y, float z)
{
	this->xPos += x;
	this->yPos += y;
	this->zPos += z;
}
void Cone::resize(float x, float y, float z)
{
    // keep it a Cone
	this->radius += x;
	this->length += y;
}
float Cone::getCenterDistSq(float a, float b, float c)
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
Cone::~Cone(void){}