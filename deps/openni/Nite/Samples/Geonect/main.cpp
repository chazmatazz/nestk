/****************************************************************************
*                                                                           *
*   Nite 1.3 - Point Viewer Sample                                          *
*                                                                           *
*   Author:     Oz Magal                                                    *
*                                                                           *
****************************************************************************/

/****************************************************************************
*                                                                           *
*   Nite 1.3	                                                            *
*   Copyright (C) 2006 PrimeSense Ltd. All Rights Reserved.                 *
*                                                                           *
*   This file has been provided pursuant to a License Agreement containing  *
*   restrictions on its use. This data contains valuable trade secrets      *
*   and proprietary information of PrimeSense Ltd. and is protected by law. *
*                                                                           *
****************************************************************************/

// Headers for OpenNI
#include <XnOpenNI.h>
#include <XnCppWrapper.h>
#include <XnHash.h>
#include <XnLog.h>

#include <XnVHandPointContext.h>
#include <XnVSessionManager.h>
#include <XnVFlowRouter.h>
#include <XnVCircleDetector.h>
#include <XnVPushDetector.h>
#include <XnVSelectableSlider1D.h>
#include <XnVSteadyDetector.h>
#include <XnVBroadcaster.h>
#include <XnVPushDetector.h>
#include <XnVPointArea.h>

// Header for NITE
#include "XnVNite.h"

// local headers
#include "PointDrawer.h"
#include "Shape.h"
#include "RectPrism.h"
#include "ShapeDrawer.h"

#include <iostream>
//#include <stdlib.h>
#include <jpeglib.h>
#include <jerror.h>
using namespace std;


#define CHECK_RC(rc, what)                                      \
  if (rc != XN_STATUS_OK)                                       \
    {                                                           \
      printf("%s failed: %s\n", what, xnGetStatusString(rc));   \
      return rc;                                                \
    }

#define CHECK_ERRORS(rc, errors, what)  \
  if (rc == XN_STATUS_NO_NODE_PRESENT)	\
    {                                   \
      XnChar strError[1024];            \
      errors.ToString(strError, 1024);	\
      printf("%s\n", strError);			\
      return (rc);						\
    }

#ifdef USE_GLUT
#if (XN_PLATFORM == XN_PLATFORM_MACOSX)
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif
#endif


#include "common.h"

const int NUM_SHAPE_BUTTONS = 4;
const int NUM_TOOL_BUTTONS = 5;
GLuint texture[NUM_SHAPE_BUTTONS+NUM_TOOL_BUTTONS];

typedef enum {
    NO_HANDS, // no hands yet
  SHAPE_SELECTION, // select a shape to work with from a menu
  SHAPE_MANIPULATION, // shape object is created and being manipulated
  TOOL_SELECTION, // select a tool from a menu to use in manipulating the shape
    DESELECTED // the shape is deselected, enable menu mode
} UserMode;

char* sUserMode(UserMode u) {
    switch(u) {
        case NO_HANDS:
            return "NO_HANDS";
        case SHAPE_SELECTION:
            return "SHAPE_SELECTION";
        case SHAPE_MANIPULATION:
            return "SHAPE_MANIPULATION";
        case TOOL_SELECTION:
            return "TOOL_SELECTION";
        case DESELECTED:
            return "DESELECTED";
    }
}

char* sSessionState(SessionState s) {
    switch(s) {
        case NOT_IN_SESSION:
            return "NOT_IN_SESSION";
        case IN_SESSION:
            return "IN_SESSION";
        case QUICK_REFOCUS:
            return "QUICK_REFOCUS";
    }
}

char* sBool(XnBool b) {
    if(b) {
        return "true";
    } else {
        return "false";
    }
}

// load a 64x64 RGB .RAW file as a texture
GLuint LoadTextureRAW( const char * filename, int wrap )
{
    GLuint texture;
    int width, height;
    void * data;
    FILE * file;

    // open texture data
    file = fopen( filename, "rb" );
    if ( file == NULL ) return 0;

    // allocate buffer
    width = 64;
    height = 64;
    data = malloc( width * height * 3 );

    // read texture data
    fread( data, width * height * 3, 1, file );
    fclose( file );

    // allocate a texture name
    glGenTextures( 1, &texture );

    // select our current texture
    glBindTexture( GL_TEXTURE_2D, texture );

    // select modulate to mix texture with color for shading
    glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );

    glTexImage2D(GL_TEXTURE_2D, 0, 3, 
                 width, height, 0, GL_RGB, 
                 GL_UNSIGNED_BYTE, data);

    // when texture area is small, bilinear filter the closest mipmap
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                     GL_LINEAR_MIPMAP_NEAREST );
    // when texture area is large, bilinear filter the first mipmap
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

    // if wrap is true, the texture wraps over at the edges (repeat)
    //       ... false, the texture ends at the edges (clamp)
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
                     wrap ? GL_REPEAT : GL_CLAMP );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,
                     wrap ? GL_REPEAT : GL_CLAMP );

    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    

    // build our texture mipmaps
    gluBuild2DMipmaps( GL_TEXTURE_2D, 3, width, height,
                       GL_RGB, GL_UNSIGNED_BYTE, data );

    // free buffer
    free( data );

    return texture;
}

// Drawing functions
void DrawLine(XnFloat fMinX, XnFloat fMinY, XnFloat fMinZ,
			  XnFloat fMaxX, XnFloat fMaxY, XnFloat fMaxZ,
			  int width, double r = 1, double g = 1, double b = 1)
{
#ifdef USE_GLUT
    
	glLineWidth(width);
	glBegin(GL_LINES);
	glColor3f(r, g, b);
	glVertex3f(fMinX, fMinY, fMinZ);
	glVertex3f(fMaxX, fMaxY, fMaxZ);
	glEnd();
    
#else
    
	const GLubyte ind[2] = {0, 1};
	GLfloat verts[6] = { fMinX, fMinY, fMinZ, fMaxX, fMaxY, fMaxZ };
	glColor4f(r,g,b,1.0f);
	glVertexPointer(3, GL_FLOAT, 0, verts);
	glLineWidth(width);
	glDrawArrays(GL_LINES, 0, 2);
	glFlush();
    
#endif
}

void DrawFrame(const XnPoint3D& ptMins, const XnPoint3D& ptMaxs, int width, double r, double g, double b)
{
	XnPoint3D ptTopLeft = ptMins;
	XnPoint3D ptBottomRight = ptMaxs;
    
	// Top line
	DrawLine(ptTopLeft.X, ptTopLeft.Y, 0,
             ptBottomRight.X, ptTopLeft.Y, 0,
             width, r, g, b);
	// Right Line
	DrawLine(ptBottomRight.X, ptTopLeft.Y, 0,
             ptBottomRight.X, ptBottomRight.Y,0,
             width, r, g, b);
	// Bottom Line
	DrawLine(ptBottomRight.X, ptBottomRight.Y,0,
             ptTopLeft.X, ptBottomRight.Y,0,
             width, r, g, b);
	// Left Line
	DrawLine(ptTopLeft.X, ptBottomRight.Y,0,
             ptTopLeft.X, ptTopLeft.Y,0,
             width, r, g, b);
    
}

void DrawTool(XnFloat center_x, XnFloat center_y, float rotation, int i, int size,
              int width, double r = 1, double g = 1, double b = 1) {
    glColor4f(r,g,b,1.0f);
    glLineWidth(width);
    glPushMatrix();
    glTranslatef(center_x, center_y, 0);
    
    // draw texture
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texture[i]);	
    glBegin (GL_QUADS);
    glTexCoord2f (0.0, 0.0);
    glVertex3f (0.0, 0.0, 0.0);
    glTexCoord2f (1.0, 0.0);
    glVertex3f (size, 0.0, 0.0);
    glTexCoord2f (1.0, 1.0);
    glVertex3f (size, size, 0.0);
    glTexCoord2f (0.0, 1.0);
    glVertex3f (0.0, size, 0.0);
    glEnd ();

    glDisable(GL_TEXTURE_2D);
    
    // end draw texture
    glPopMatrix();
}


#include "SteadyButton.h"

#define GL_WIN_SIZE_X 640
#define GL_WIN_SIZE_Y 480
#define TOOL_SIZE 50
#define COOLDOWN_FRAMES 5 
#define DETECTION_DURATION 300
#define MAXIMUM_VELOCITY .005
#define SAMPLE_XML_PATH "config/Sample-Tracking.xml"
#define STEADY_DELAY 10

#define ZMIN -400
#define ZMAX 400

// OpenNI objects
xn::Context g_Context;
xn::DepthGenerator g_DepthGenerator;
xn::HandsGenerator g_HandsGenerator;
xn::GestureGenerator g_GestureGenerator;

// NITE objects
XnVSessionManager* g_pSessionManager;
XnVFlowRouter* g_pFlowRouter;
XnVBroadcaster* g_pBroadcaster;
XnVCircleDetector*  g_pCircle;
XnVPushDetector* g_pPush;
XnVSteadyDetector* g_pSteadyDetector;

SteadyButton* g_pSButton[NUM_SHAPE_BUTTONS];
SteadyButton* g_pTButton[NUM_TOOL_BUTTONS];

// the drawer
XnVPointDrawer* g_pDrawer;

// Draw the depth map?
XnBool g_bDrawDepthMap = true;
XnBool g_bPrintFrameID = false;
// Use smoothing?
XnFloat g_fSmoothing = 0.0f;
XnBool g_bPause = false;
XnBool g_bQuit = false;
XnBool g_bPlayRecording = false;
int g_Counter = 0;
SessionState g_SessionState = NOT_IN_SESSION;
UserMode g_UserMode;
int g_Tool;
XnBool g_bSelect = false;
GktShapeDrawer* g_pShapeDrawer;
int errorcase;
XnBoundingBox3D g_BoundingBox;
void CleanupExit()
{
  /*	
    for(int i = 0; i < NUM_SHAPE_BUTTONS; i++) {
        delete g_pSButton[i];
    }

    g_pBroadcaster->RemoveListener(g_pDrawer);
    g_pSessionManager->RemoveListener(g_pBroadcaster);

    delete g_pBroadcaster;
    delete g_pSessionManager;
    delete g_pDrawer;
  */
    g_Context.Shutdown();

	exit (1);
}

void setUserMode(UserMode u);

/**
 * Print the state
 */
void printState() {
    printf("***** State *****\n");
    printf("g_SessionState = %s\n", sSessionState(g_SessionState));
    printf("g_userMode = %s\n", sUserMode(g_UserMode));
    printf("g_Tool = %s\n", sButton(g_Tool));
    printf("g_pShapeDrawer->isHover() = %s\n", sBool(g_pShapeDrawer->isHover()));
    printf("g_pShapeDrawer->isSelected() = %s\n", sBool(g_pShapeDrawer->isSelected()));
    printf("***** End State *****\n");
}

// Callback for when the focus is in progress
void XN_CALLBACK_TYPE FocusProgress(const XnChar* strFocus, const XnPoint3D& ptPosition, XnFloat fProgress, void* UserCxt)
{
//	printf("Focus progress: %s @(%f,%f,%f): %f\n", strFocus, ptPosition.X, ptPosition.Y, ptPosition.Z, fProgress);
}
// callback for session start
void XN_CALLBACK_TYPE SessionStarting(const XnPoint3D& ptPosition, void* UserCxt)
{
	printf("Session start: (%f,%f,%f)\n", ptPosition.X, ptPosition.Y, ptPosition.Z);
    printState();
	g_SessionState = IN_SESSION;
    setUserMode(SHAPE_SELECTION);
}
// Callback for session end
void XN_CALLBACK_TYPE SessionEnding(void* UserCxt)
{
	printf("Session end\n");
    printState();
	g_SessionState = NOT_IN_SESSION;
    setUserMode(NO_HANDS);
}
void XN_CALLBACK_TYPE NoHands(void* UserCxt)
{
	if (g_SessionState != NOT_IN_SESSION)
	{
		printf("Quick refocus\n");
		g_SessionState = QUICK_REFOCUS;
	}
}

void setTool(int v) {
    g_Tool = v;
    printf("Switch to %s tool!\n", sButton(v));
    g_pShapeDrawer->SetTool(v);
}

void setShapeMenuActive(XnBool active) {
    for (int i = 0; i < NUM_SHAPE_BUTTONS; i++) {
        if(active) {
            g_pSButton[i]->SetButtonMode(ButtonVisible);
        } else {
            g_pSButton[i]->SetButtonMode(ButtonHidden);
        } 
    }
}

void setToolSelectionMenuActive(bool active) {
    for (int i = 0; i < NUM_TOOL_BUTTONS; i++) {
        if(active) {        
            g_pTButton[i]->SetButtonMode(ButtonVisible);
        } else {    
            g_pTButton[i]->SetButtonMode(ButtonHidden);
        }
    }
}

void setUserMode(UserMode u) {
    printf("set user mode %s\n", sUserMode(u));
    g_UserMode = u;
    if(u != SHAPE_MANIPULATION) {
        g_pShapeDrawer->Drop();
    }
    g_pShapeDrawer->SetActive(u == DESELECTED || u == SHAPE_MANIPULATION);
    setShapeMenuActive(u == SHAPE_SELECTION);
    setToolSelectionMenuActive(u == TOOL_SELECTION);
}

XnBool isShape(int v) {
    return v < NUM_SHAPE_BUTTONS;
}

/**
 * Performs operation when button is pressed
 */
void selectButton(int v) {
    if(isShape(v)) {
        setUserMode(SHAPE_MANIPULATION);
        g_pShapeDrawer->AddShape(v);
        setTool(TRANSLATE);
    } 
    else { // tool
      if(v == SHAPE_SUBMENU) {
        setUserMode(SHAPE_SELECTION);
      } 
      else {
        setUserMode(DESELECTED);
        setTool(v);
      }
    }
}

/**
 * deselectObject
 */
void deselectObject() {
    setUserMode(DESELECTED);
}

/**
 * Callback for the steady button. 
 * This is called when a button is selected. Yup.
 */
void XN_CALLBACK_TYPE SteadyButton_Select(void* cxt)
{
    SteadyButton* button = (SteadyButton*)(cxt);
    printf("button %s selected\n", sButton(button->getType()));
    printState();
    selectButton(button->getType());
}

void XN_CALLBACK_TYPE CircleCB(XnFloat fTimes, XnVCircleDetector::XnVNoCircleReason eReason, void* pUserCxt)
{
  printf("circle finish detected\n");
    printState();
  if(g_UserMode == DESELECTED) {
      printf("entering tool selection\n");
      setUserMode(TOOL_SELECTION);
  }
}

// Callback for the Push event
void XN_CALLBACK_TYPE Drop(XnFloat fVelocity, XnFloat fAngle, void* UserCxt)
{
  //printf("Drop detected!\n");
    
  if(g_UserMode == SHAPE_MANIPULATION){
      printf("deselecting object\n");
      deselectObject();
      printState();
  }
}

/**
 * Callback for shapedrawer
 * This is called when a shape is selected
 */
void XN_CALLBACK_TYPE ShapeDrawer_Select(void *cxt) {
    //printState();
    //printf("Select detected for shape\n");
  if(g_Tool == TRASH) {
    // remove from shape list
    g_pShapeDrawer->RemoveCurrentShape();
    deselectObject();
  }
  else {
    setUserMode(SHAPE_MANIPULATION);
  }
}

// this function is called each frame
void glutDisplay (void)
{
    const int size = 50;
	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Setup the OpenGL viewpoint
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();

    glMatrixMode(GL_MODELVIEW); // Select The Modelview Matrix
    glLoadIdentity(); // Reset The Modelview Matrix

    XnMapOutputMode mode;
	g_DepthGenerator.GetMapOutputMode(mode);
	#ifdef USE_GLUT
    glOrtho(0, mode.nXRes, mode.nYRes, 0, ZMIN, ZMAX);
	#else
	glOrthof(0, mode.nXRes, mode.nYRes, 0, ZMIN, ZMAX);
	#endif

//    cout << "ortho dim x=" << mode.nXRes << " y=" << mode.nYRes << endl;

	//glDisable(GL_TEXTURE_2D);

	if (!g_bPause)
	{
		// Read next available data
		g_Context.WaitAndUpdateAll();
		// Update NITE tree
		g_pSessionManager->Update(&g_Context);
		PrintSessionState(g_SessionState);
	}
    
    // shapedrawer, steadybuttons draw on update
    
    // Draw the tool icon if in manipulation mode
    if(g_UserMode == SHAPE_MANIPULATION || g_UserMode == DESELECTED) {
      DrawTool(20, 20, 0, g_Tool, size, 1);
    }

    // draw state
	#ifdef USE_GLUT
	glutSwapBuffers();
	#endif
}

#ifdef USE_GLUT
void glutIdle (void)
{
	if (g_bQuit) {
		CleanupExit();
	}

	// Display the frame
	glutPostRedisplay();
}

void glutKeyboard (unsigned char key, int x, int y)
{
	switch (key)
	{
	case 27:
		// Exit
		CleanupExit();
	case'p':
		// Toggle pause
		g_bPause = !g_bPause;
		break;
	case 'd':
		// Toggle drawing of the depth map
		g_bDrawDepthMap = !g_bDrawDepthMap;
		g_pDrawer->SetDepthMap(g_bDrawDepthMap);
		break;
	case 'f':
		g_bPrintFrameID = !g_bPrintFrameID;
		g_pDrawer->SetFrameID(g_bPrintFrameID);
		break;
	case 's':
		// Toggle smoothing
		if (g_fSmoothing == 0)
			g_fSmoothing = 0.1;
		else 
			g_fSmoothing = 0;
		g_HandsGenerator.SetSmoothing(g_fSmoothing);
		break;
    case 'e':
		// end current session
		g_pSessionManager->EndSession();
		break;
	}
}

void glInit (int * pargc, char ** argv)
{
	glutInit(pargc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
	glutInitWindowSize(GL_WIN_SIZE_X, GL_WIN_SIZE_Y);
	glutCreateWindow ("Geonect");
    //glutFullScreen();
	glutSetCursor(GLUT_CURSOR_NONE);

    glClearColor(0, 0, 0, 0);
    glClearDepth(1.0);
	glutKeyboardFunc(glutKeyboard);
	glutDisplayFunc(glutDisplay);
	glutIdleFunc(glutIdle);

    glDepthFunc(GL_LESS);		// The Type Of Depth Test To Do
	glEnable(GL_DEPTH_TEST);
    glFrontFace(GL_CCW);	    // Counterclockwise polygons face out
	//glDisable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);
    glShadeModel(GL_SMOOTH); // Enables Smooth Color Shading

	glEnableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
    
    glEnable( GL_LIGHTING );
    GLfloat diffuseLight[] = { 1, 1, 1, 1.0 };	/// RGBA, vary the 3 first paramaters to change the light color and intensity.   
    glLightfv( GL_LIGHT0, GL_DIFFUSE, diffuseLight );	/// Create the light source
    glEnable( GL_LIGHT0 );					/// Light it!

    glEnable( GL_COLOR_MATERIAL );
	glColorMaterial( GL_FRONT, GL_AMBIENT_AND_DIFFUSE );
    
    GLfloat ambientLight[] = { 0.5, 0.5, 0.5, 1.0 };		/// Define color and intensity
    glLightfv( GL_LIGHT0, GL_AMBIENT, ambientLight );		/// Add ambient component
    
    GLfloat specularLight[] = { 0.7, 0.7, 0.7, 1.0 };
    GLfloat spectre[] = { 1.0, 1.0, 1.0, 1.0 };
    
    glLightfv( GL_LIGHT0, GL_SPECULAR, specularLight );
    glMaterialfv( GL_FRONT, GL_SPECULAR, spectre );
    glMateriali( GL_FRONT, GL_SHININESS, 128 );

    texture[TRANSLATE] = LoadTextureRAW("../../../textures/translate.raw", 0);
    texture[ROTATE] = LoadTextureRAW("../../../textures/rotate.raw", 0);
    texture[STRETCH] = LoadTextureRAW("../../../textures/stretch.raw", 0);
    texture[TRASH] = LoadTextureRAW("../../../textures/trash.raw", 0);
    texture[SHAPE_SUBMENU] = LoadTextureRAW("../../../textures/shapes.raw", 0);
    texture[CUBE] = LoadTextureRAW("../../../textures/cube.raw", 0);
    texture[CYLINDER] = LoadTextureRAW("../../../textures/cylinder.raw", 0);
    texture[CONE] = LoadTextureRAW("../../../textures/cone.raw", 0);
    texture[ELLIPSOID] = LoadTextureRAW("../../../textures/ellipsoid2.raw", 0);

}
#endif

void initToolPanel() 
{
    // Register callback to the SteadyButton objects for their Select event.
    XnPoint3D ptMax, ptMin;
    int Xshift = (GL_WIN_SIZE_X-(2*TOOL_SIZE*NUM_TOOL_BUTTONS))/2;
    ptMax.Z = 0;
    ptMin.Z = 0;
    ptMin.Y = GL_WIN_SIZE_Y-TOOL_SIZE*2-150;  
    ptMax.Y = GL_WIN_SIZE_Y-150;
    for(int i = 0; i < NUM_TOOL_BUTTONS; i++) {
      
      ptMin.X = Xshift + i*2*TOOL_SIZE;
      ptMax.X = Xshift + (i+1)*2*TOOL_SIZE;
      g_pTButton[i] = new SteadyButton(ptMax, ptMin, i+(NUM_SHAPE_BUTTONS), TOOL_SIZE, g_DepthGenerator);
      g_pTButton[i]->RegisterSelect(NULL, &SteadyButton_Select);
      g_pSessionManager->AddListener(g_pTButton[i]);
    }
}


void initShapePanel() 
{
    // Register callback to the SteadyButton objects for their Select event.
    XnPoint3D ptMax, ptMin;
    int Xshift = (GL_WIN_SIZE_X-(2*TOOL_SIZE*NUM_SHAPE_BUTTONS))/2;

    ptMax.Z = 0;
    ptMin.Z = 0;
    ptMin.Y = GL_WIN_SIZE_Y-TOOL_SIZE*2-150;  
    ptMax.Y = GL_WIN_SIZE_Y-150;
    for(int i = 0; i < NUM_SHAPE_BUTTONS; i++) {
      
      ptMin.X = Xshift + i*2*TOOL_SIZE;
      ptMax.X = Xshift + (i+1)*2*TOOL_SIZE;
      g_pSButton[i] = new SteadyButton(ptMax, ptMin, i, TOOL_SIZE, g_DepthGenerator);
      g_pSButton[i]->RegisterSelect(NULL, &SteadyButton_Select);
      g_pSessionManager->AddListener(g_pSButton[i]);
    }
}


int main(int argc, char ** argv)
{
	XnStatus rc = XN_STATUS_OK;
	xn::EnumerationErrors errors;

	// Initialize OpenNI
    rc = g_Context.InitFromXmlFile(SAMPLE_XML_PATH, &errors);
    CHECK_ERRORS(rc, errors, "InitFromXmlFile");
    CHECK_RC(rc, "InitFromXmlFile");
    
    rc = g_Context.FindExistingNode(XN_NODE_TYPE_DEPTH, g_DepthGenerator);
    CHECK_RC(rc, "Find depth generator");
    rc = g_Context.FindExistingNode(XN_NODE_TYPE_HANDS, g_HandsGenerator);
    CHECK_RC(rc, "Find hands generator");
    rc = g_Context.FindExistingNode(XN_NODE_TYPE_GESTURE, g_GestureGenerator);
    CHECK_RC(rc, "Find gesture generator");
    
    printf("creat nite objects\n");
    // Create NITE objects
    g_pSessionManager = new XnVSessionManager;
    rc = g_pSessionManager->Initialize(&g_Context, "Click,Wave", "RaiseHand");
    CHECK_RC(rc, "SessionManager::Initialize");
    g_pSessionManager->RegisterSession(NULL, SessionStarting, SessionEnding, FocusProgress);
    g_pDrawer = new XnVPointDrawer(20, g_DepthGenerator); 
	g_pCircle = new XnVCircleDetector;
    g_pPush = new XnVPushDetector;
    g_pBroadcaster = new XnVBroadcaster();
    g_pBroadcaster->AddListener(g_pDrawer);    
    g_pBroadcaster->AddListener(g_pCircle);    
    g_pBroadcaster->AddListener(g_pPush);    
    g_pSessionManager->AddListener(g_pBroadcaster);
    g_pCircle->RegisterNoCircle(NULL, CircleCB);    
    g_pPush->RegisterPush(NULL, Drop);    
    g_pDrawer->RegisterNoPoints(NULL, NoHands);
    g_pDrawer->SetDepthMap(g_bDrawDepthMap);
    
    XnPoint3D ptMins;
    ptMins.X = 0;
    ptMins.Y = GL_WIN_SIZE_Y;
    ptMins.Z = -ZMAX;
    XnPoint3D ptMaxs;
    ptMaxs.X = GL_WIN_SIZE_X;
    ptMaxs.Y = 0;
    ptMaxs.Z = ZMAX;
    g_BoundingBox.LeftBottomNear = ptMins;
    g_BoundingBox.RightTopFar = ptMaxs;
    
    printf("shapedrawer init\n");
    g_pShapeDrawer = new GktShapeDrawer(g_DepthGenerator, g_BoundingBox);
    g_pShapeDrawer->RegisterSelect(NULL, &ShapeDrawer_Select);
    g_pSessionManager->AddListener(g_pShapeDrawer);
    
    
    printf("init panels\n");
    initShapePanel();
    initToolPanel();
    setUserMode(NO_HANDS);
    
    // Initialization done. Start generating
    rc = g_Context.StartGeneratingAll();
    CHECK_RC(rc, "StartGenerating");
    
    // Mainloop
    #ifdef USE_GLUT

    glInit(&argc, argv);
    glutMainLoop();
    
    #else
    #endif
    
}
