#pragma once
class Shape
{
public:
	Shape(void);
	~Shape(void);
	virtual void draw(void) = 0;
	virtual void drawHighlighted(void) = 0;
	virtual void rotate(float xRot,float yRot) = 0;
	virtual void displace(float x, float y, float z) = 0;
	virtual void resize(float width, float height, float depth) = 0;
};

