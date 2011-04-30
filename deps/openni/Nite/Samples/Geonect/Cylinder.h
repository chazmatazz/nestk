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

class Cylinder :public Shape
{
public:
	Cylinder(float x, float y, float z, float xRot, float yRot, float radius1, float radius2, float length, XnBoundingBox3D& boundingBox);
	~Cylinder(void);
	void draw(DrawState drawState);
	void rotate(float xRot,float yRot);
	void displace(float x, float y, float z);
	void resize(float width, float height, float depth);
	float getDist(float a, float b, float c);
    float getXPos(void) { return xPos;}
    float getYPos(void) { return yPos;}
    float getZPos(void) { return zPos;}
private:
	GLUquadricObj *quadric;
	float xPos,yPos,zPos;
	float xRot,yRot;
	float radius1,radius2,length;
    XnBoundingBox3D boundingBox;
	void drawCyl(DrawState drawState);
};
