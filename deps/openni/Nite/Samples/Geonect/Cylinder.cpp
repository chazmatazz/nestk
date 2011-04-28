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


Cylinder::Cylinder(float x, float y, float z, float xRot, float yRot, float radius1, float radius2, float length)
{
	this->xPos = x;
	this->yPos = y;
	this->zPos = z;
	this->xRot = xRot;
	this->yRot = yRot;
	this->length = length;
	this->radius1 = radius1;
	this->radius2 = radius2;
	quadric = gluNewQuadric();
}
void Cylinder::draw(void)
{
	glLoadIdentity();
	glTranslatef(0.0f,0.0f,-10.0f);
	glTranslatef(this->xPos,this->yPos,this->zPos);
	glRotatef(this->xRot, 1.0f, 0.0f, 0.0f);
	glRotatef(this->yRot, 0.0f, 1.0f, 0.0f);
	glTranslatef(0.0f,0.0f,-this->length/2);
	glColor3f(0.0f, 0.0f, 1.0f);
	gluCylinder(quadric,this->radius1,this->radius2,length,32,16);
	glColor3f(1.0f, 0.0f, 0.0f);
	gluQuadricOrientation(quadric,GLU_INSIDE);
	gluDisk( quadric, 0.0, radius1, 32, 16);
	glTranslatef( 0,0,this->length );
	glColor3f(0.0f, 1.0f, 0.0f);
	gluQuadricOrientation(quadric,GLU_OUTSIDE);
	gluDisk( quadric, 0.0, radius2, 32, 16);
	glTranslatef( 0,0,this->length );
}
void Cylinder::drawHighlighted(void)
{
	glPushMatrix();
	glTranslatef(0.0f,0.0f,-10.0f);
	glTranslatef(this->xPos,this->yPos,this->zPos);
	glRotatef(this->xRot, 1.0f, 0.0f, 0.0f);
	glRotatef(this->yRot, 0.0f, 1.0f, 0.0f);
	glTranslatef(0.0f,0.0f,-this->length/2);
	glColor3f(1.0f, 0.5f, 1.0f);
	gluCylinder(quadric,this->radius1,this->radius2,length,32,16);
	gluQuadricOrientation(quadric,GLU_INSIDE);
	gluDisk( quadric, 0.0, radius1, 32, 16);
	glTranslatef( 0,0,this->length );
	gluQuadricOrientation(quadric,GLU_OUTSIDE);
	gluDisk( quadric, 0.0, radius2, 32, 16);
	glTranslatef( 0,0,this->length );
	glPopMatrix();
}
void Cylinder::rotate(float xRot,float yRot)
{
	this->xRot = xRot;
	this->yRot = yRot;
}
void Cylinder::displace(float x, float y, float z)
{
	this->xPos += x;
	this->yPos += y;
	this->zPos += z;
}
void Cylinder::resize(float radius1, float radius2, float length)
{
	this->radius1 += radius1;
	this->radius2 += radius2;
	this->length += length;
}
float Cylinder::getDist(float a, float b, float c)
{
	return (this->xPos - a)*(this->xPos - a)+(this->yPos -b)*(this->yPos -b)+(this->zPos - c)*(this->zPos - c);
}
Cylinder::~Cylinder(void){}