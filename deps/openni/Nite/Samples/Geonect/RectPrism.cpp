#include "RectPrism.h"
#include <XnCppWrapper.h>

#if (XN_PLATFORM == XN_PLATFORM_MACOSX)
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#include "StdAfx.h"
#endif



RectPrism::RectPrism(float x, float y, float z, float xRot, float yRot, float width, float height, float depth, XnBoundingBox3D& boundingBox)
{
	this->xPos = x;
	this->yPos = y;
	this->zPos = z;
	this->xRot = xRot;
	this->yRot = yRot;
	this->width = width;
	this->height = height;
	this->depth = depth;
    this->boundingBox = boundingBox;
}

void RectPrism::draw(DrawState drawState)
{
	glPushMatrix();
	glTranslatef(0.0f,0.0f,-10.0f);
	glTranslatef(this->xPos,this->yPos,this->zPos);
	glRotatef(this->xRot, 1.0f, 0.0f, 0.0f);
	glRotatef(this->yRot, 0.0f, 1.0f, 0.0f);
	drawBox(drawState);
	glPopMatrix();
}

void RectPrism::rotate(float xRot,float yRot)
{
	this->xRot = xRot;
	this->yRot = yRot;
}
void RectPrism::displace(float x, float y, float z)
{
	this->xPos += x;
	this->yPos += y;
	this->zPos += z;
}
void RectPrism::resize(float width, float height, float depth)
{
	this->width += width;
	this->height += height;
	this->depth += depth;
}
float RectPrism::getDist(float a, float b, float c)
{
	return (this->xPos - a)*(this->xPos - a)+(this->yPos -b)*(this->yPos -b)+(this->zPos - c)*(this->zPos - c);
}
void RectPrism::drawBox(DrawState drawState)
{
	glBegin(GL_QUADS);

    switch(drawState) {
        case UNSELECTED:
            glColor3f(1.0f, 0.0f, 0.0f);
            break;
        case HOVER:
            glColor3f(1.0f, 0.5f, 1.0f);
            break;
        case SELECTED:
            glColor3f(1.0f, 1.0f, 0.0f);
            break;            
    }
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

		glColor3f(0.0f, 1.0f, 0.0f);
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

		glColor3f(0.0f, 0.0f, 1.0f);
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
