#pragma once

#include "common.h"
#include <XnCppWrapper.h>

typedef enum {
    DRAWSTATE_UNSELECTED,
    DRAWSTATE_HOVER,
    DRAWSTATE_SELECTED
} DrawState;

typedef enum {
    SIDE1,
    SIDE2,
    SIDE3
} ObjectFace;


#define SIZE_MIN_DIM 10
#define SIZE_MAX_DIM 1000
#define SQRT_2 1.4
// random constant to make the bounding boxes smaller
#define SHRINK 0.5

/**
 * performs a single glColor3f call.
 */
void setglColor(DrawState d, ObjectFace f);

/**
 * determine if point in bounding box
 */
XnBool inBoundingBox(XnPoint3D p, XnBoundingBox3D b);

/**
 * Calculates a bounding box based on center and size
 */
XnBoundingBox3D calcBoundingBox(XnPoint3D center, XnPoint3D size);

/**
 * clamp point inside bounding box
 * useful for translate?
 */
void clamp_point(XnPoint3D& p, XnBoundingBox3D b);

/**
 * clamp value
 */
void clamp_val(XnFloat& v, XnFloat min, XnFloat max);

/**
 * Draw bounding box
 */
void drawBoundingBox(XnBoundingBox3D b);

class Shape
{
public:
	Shape(void);
	~Shape(void);
    virtual XnPoint3D getCenter() = 0;
    virtual XnPoint3D getBoundingBoxSize() = 0;
    virtual XnBoundingBox3D getBoundingBox() = 0;
	virtual XnFloat getCenterDistSq(XnPoint3D p)  = 0;
    virtual XnBool isInside(XnPoint3D p) = 0;
	virtual void draw(DrawState drawState) = 0;
	virtual void rotate(XnPoint3D d) = 0;
	virtual void displace(XnPoint3D d) = 0;
	virtual void resize(XnPoint3D d) = 0;
};