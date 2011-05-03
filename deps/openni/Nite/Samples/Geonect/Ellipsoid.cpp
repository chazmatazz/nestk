#ifdef USE_GLUT
#if (XN_PLATFORM == XN_PLATFORM_MACOSX)
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#include "StdAfx.h"
#endif
#else
#include "opengles.h"
#endif

#include "Ellipsoid.h"


Ellipsoid::Ellipsoid(XnPoint3D center, XnFloat xRot, XnFloat yRot, XnFloat radius1, XnFloat radius2, XnFloat radius3, XnBoundingBox3D& boundingBox)
{
	this->center = center;
	this->xRot = xRot;
	this->yRot = yRot;
    this->radius1 = radius1;
	this->radius2 = radius2;
    this->radius3 = radius3;
    this->boundingBox = boundingBox;
}

void Ellipsoid::draw(DrawState drawState)
{
    if(drawState == DRAWSTATE_HOVER) {
        drawBoundingBox(getBoundingBox());
    }
	glPushMatrix();
	glTranslatef(0.0f,0.0f,-10.0f);
	glTranslatef(this->center.X,this->center.Y,this->center.Z);
	glRotatef(this->xRot, 1.0f, 0.0f, 0.0f);
	glRotatef(this->yRot, 0.0f, 1.0f, 0.0f);
    glScalef(this->radius1, this->radius2, this->radius3);
    setglColor(drawState, SIDE1); // side
    glutWireSphere(1, 32, 8);
    glPopMatrix();
}
void Ellipsoid::rotate(XnPoint3D d)
{
    // intentionally swapped
	this->xRot += d.Y;
	this->yRot += d.X;
}
void Ellipsoid::displace(XnPoint3D d)
{
	this->center.X += d.X;
	this->center.Y += d.Y;
	this->center.Z += d.Z;
    clamp_point(center, boundingBox);

}
void Ellipsoid::resize(XnPoint3D d)
{
	this->radius1 += d.X;
	this->radius2 += d.Y;
    this->radius3 += d.Z;
    clamp_val(radius1, SIZE_MIN_DIM/2, SIZE_MAX_DIM/2);
    clamp_val(radius2, SIZE_MIN_DIM/2, SIZE_MAX_DIM/2);
    clamp_val(radius3, SIZE_MIN_DIM/2, SIZE_MAX_DIM/2);
}
XnPoint3D Ellipsoid::getCenter() {
    return center;
}
XnPoint3D Ellipsoid::getBoundingBoxSize() {
    XnFloat max;
    if(radius1 > radius2) {
        max = radius1;
    } else {
        max = radius2;
    }
    if(radius3 > max) {
        max = radius3;
    }
    max *= 2 * SHRINK;
    XnPoint3D size;
    size.X = max;
    size.Y = max;
    size.Z = max;
    return size;
}
XnBoundingBox3D Ellipsoid::getBoundingBox() {
    return calcBoundingBox(getCenter(), getBoundingBoxSize());
}
XnFloat Ellipsoid::getCenterDistSq(XnPoint3D p)
{
    return dist_sq(p, getCenter());
}
XnBool Ellipsoid::isInside(XnPoint3D p) {
    return inBoundingBox(p, getBoundingBox());
}

Ellipsoid::~Ellipsoid(void){}
