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
	Cone(XnPoint3D center, XnFloat xRot, XnFloat yRot, XnFloat radius, XnFloat length, XnBoundingBox3D& boundingBox);
	~Cone(void);
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
	GLUquadricObj *quadric;
	XnPoint3D center;
	XnFloat xRot,yRot;
	XnFloat radius,length;
    XnBoundingBox3D boundingBox;
};
