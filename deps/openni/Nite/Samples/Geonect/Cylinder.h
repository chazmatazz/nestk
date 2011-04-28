#pragma once
#include "Shape.h"

#if (XN_PLATFORM == XN_PLATFORM_MACOSX)
#include <GLUT/glut.h>
#else
#include "StdAfx.h"
#include <GL/glut.h>
#endif


class Cylinder :public Shape
{
public:
	Cylinder(float x, float y, float z, float xRot, float yRot, float radius1, float radius2, float length);
	~Cylinder(void);
	void draw(void);
	void drawHighlighted(void);
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
	void drawCyl(void);
};
