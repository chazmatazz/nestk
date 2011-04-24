#pragma once
#include "shape.h"
#include <GL/glut.h>

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
private:
	GLUquadricObj *quadric;
	float xPos,yPos,zPos;
	float xRot,yRot;
	float radius1,radius2,length;
	void drawCyl(void);
};

