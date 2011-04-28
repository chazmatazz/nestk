#pragma once
#include "Shape.h"

class RectPrism :public Shape
{
public:
	RectPrism(float x, float y, float z, float xRot, float yRot, float width, float height, float depth);
	~RectPrism(void);
	void draw(void);
	void drawHighlighted(void);
	void rotate(float xRot,float yRot);
	void displace(float x, float y, float z);
	void resize(float width, float height, float depth);
    float getXPos(void) { return xPos;}
    float getYPos(void) { return yPos;}
    float getZPos(void) { return zPos;}

private:
	float xPos,yPos,zPos;
	float xRot,yRot;
	float width,height,depth;
	void drawBox(void);
	void drawBoxH(void);
};

