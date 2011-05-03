#include "common.h"

char* sButton(int button) {
    switch(button) {
        case CUBE:
            return "CUBE";
            break;
        case CYLINDER:
            return "CYLINDER";
            break;
        case CONE:
            return "CONE";
            break;
        case ELLIPSOID:
            return "ELLIPSOID";
            break;
        case TRANSLATE:
            return "TRANSLATE";
            break;
        case ROTATE:
            return "ROTATE";
            break;
        case STRETCH:
            return "STRETCH";
            break;
        case TRASH:
            return "TRASH";
            break;
        case SHAPE_SUBMENU:
            return "SHAPE_SUBMENU";
            break;
    }
}

XnFloat dist_sq(XnPoint3D a, XnPoint3D b) {
    XnPoint3D d;
    d.X = a.X - b.X;
    d.Y = a.Y - b.Y;
    d.Z = a.Z - b.Z;
    return d.X*d.X + d.Y*d.Y + d.Z*d.Z;
    
}
