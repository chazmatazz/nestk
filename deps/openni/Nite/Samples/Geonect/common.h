#ifndef GKT_COMMON_H
#define GKT_COMMON_H

#include <XnOpenNI.h>
#include <XnCppWrapper.h>

#define CUBE 0
#define CYLINDER 1
#define CONE 2
#define ELLIPSOID 3
#define TRANSLATE 4
#define ROTATE 5
#define STRETCH 6
#define SHAPE_SUBMENU 7
#define TRASH 8

#define BUTTON_DELAY 4
#define SHAPE_SELECTION_DELAY 3

// Affects the existence and color of the Button in drawing
typedef enum {
    ButtonHidden, // the button is hidden
    ButtonVisible, // the button is visible but not hovered
    ButtonHover // button is hovered
} ButtonMode;

/**
 * Applies to the button types #defined above
 */
char* sButton(int button);

XnFloat dist_sq(XnPoint3D a, XnPoint3D b);

#endif


