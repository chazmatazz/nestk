#pragma once
#include "Shape.h"

#if (XN_PLATFORM == XN_PLATFORM_MACOSX)
#include <GLUT/glut.h>
#else
#include "StdAfx.h"
#include <GL/glut.h>
#endif

#include <XnOpenNI.h>
#include <XnCppWrapper.h>

class Cone :public Shape
{
public:
	Cone(float x, float y, float z, float xRot, float yRot, float radius, float length, XnBoundingBox3D& boundingBox);
	~Cone(void);
	void draw(DrawState drawState);
	void rotate(float xRot,float yRot);
	void displace(float x, float y, float z);
	void resize(float width, float height, float depth);
	float getCenterDistSq(float a, float b, float c);
    float getXPos(void) { return xPos;}
    float getYPos(void) { return yPos;}
    float getZPos(void) { return zPos;}
private:
	GLUquadricObj *quadric;
	float xPos,yPos,zPos;
	float xRot,yRot;
	float radius,length;
    XnBoundingBox3D boundingBox;
	void drawCyl(DrawState drawState);
};
