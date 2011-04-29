#pragma once
typedef enum {
    UNSELECTED,
    HOVER,
    SELECTED
} DrawState;

class Shape
{
public:
	Shape(void);
	~Shape(void);
	virtual float getDist(float a, float b, float c)  = 0;
	virtual void draw(DrawState drawState) = 0;
	virtual void rotate(float xRot,float yRot) = 0;
	virtual void displace(float x, float y, float z) = 0;
	virtual void resize(float width, float height, float depth) = 0;
    virtual float getXPos(void) = 0;
    virtual float getYPos(void) = 0;
    virtual float getZPos(void) = 0;
};