#pragma once
#include "Shape.h"
#include <XnOpenNI.h>
#include <XnCppWrapper.h>

class RectPrism :public Shape
{
public:
	RectPrism(float x, float y, float z, float xRot, float yRot, float width, float height, float depth, XnBoundingBox3D& boundingBox);
	~RectPrism(void);
	void draw(DrawState drawState);
	void rotate(float xRot,float yRot);
	void displace(float x, float y, float z);
	void resize(float width, float height, float depth);
	float getDistSq(float a, float b, float c);
    float getXPos(void) { return xPos;}
    float getYPos(void) { return yPos;}
    float getZPos(void) { return zPos;}
private:
	float xPos,yPos,zPos;
	float xRot,yRot;
	float width,height,depth;
    XnBoundingBox3D boundingBox;
	void drawBox(DrawState drawState);
};
