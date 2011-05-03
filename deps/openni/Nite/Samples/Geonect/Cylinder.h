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
	Cylinder(XnPoint3D center, XnFloat xRot, XnFloat yRot, XnFloat radius, XnFloat length, XnBoundingBox3D& boundingBox);
	~Cylinder(void);
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
    XnFloat length, radius;
    XnBoundingBox3D boundingBox;
};
