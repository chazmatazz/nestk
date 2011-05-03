#pragma once
#include "Shape.h"
#include <XnOpenNI.h>
#include <XnCppWrapper.h>

class RectPrism :public Shape
{
public:
	RectPrism(XnPoint3D center, XnFloat xRot, XnFloat yRot, XnFloat width, XnFloat height, XnFloat depth, XnBoundingBox3D& boundingBox);
	~RectPrism(void);
	void draw(DrawState drawState);
	void rotate(XnPoint3D d);
	void displace(XnPoint3D d);
	void resize(XnPoint3D d);
    XnPoint3D getCenter();
    XnPoint3D getBoundingBoxSize();
    XnBoundingBox3D getBoundingBox();
    XnFloat getCenterDistSq(XnPoint3D p);
    XnBool isInside(XnPoint3D p);
private:
	XnPoint3D center;
	XnFloat xRot,yRot;
	XnFloat width,height,depth;
    XnBoundingBox3D boundingBox;
	void drawBox(DrawState drawState);
};
