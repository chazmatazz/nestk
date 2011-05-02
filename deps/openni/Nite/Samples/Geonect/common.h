#ifndef GKT_COMMON_H
#define GKT_COMMON_H

#define CUBE 0
#define CYLINDER 1
#define SHAPE3 2
#define SHAPE4 3
#define TRANSLATE 4
#define ROTATE 5
#define STRETCH 6
#define BEND 7
#define SHAPE_SUBMENU 8

#define DISPLAY_DELAY 2

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

#endif


