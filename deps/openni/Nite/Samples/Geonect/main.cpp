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
#include <XnVSwipeDetector.h>
#include <XnVSelectableSlider1D.h>
#include <XnVSteadyDetector.h>
#include <XnVBroadcaster.h>
#include <XnVPushDetector.h>
#include <XnVPointArea.h>

// Header for NITE
#include "XnVNite.h"

// local headers
#include "PointDrawer.h"


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
    glRotatef(rotation, 0, 1, 0);
    switch (i) {
        case 0:
            glutWireCube(size);
            break;
        case 1:
            glutWireSphere(size, 8, 8);
            break;
        case 2:
            glutWireCone(size, size, 8, 8);
            break;
        case 3:
            glutWireTorus(size/2, size, 8, 8);
            break;
    }
    glPopMatrix();
}


#include "SteadyButton.h"

#define GL_WIN_SIZE_X 720
#define GL_WIN_SIZE_Y 480
#define TOOL_SIZE 40
#define COOLDOWN_FRAMES 5 
#define DETECTION_DURATION 300
#define MAXIMUM_VELOCITY .005
#define SAMPLE_XML_PATH "config/Sample-Tracking.xml"
#define STEADY_DELAY 10
#define DISPLAY_DELAY 2

// OpenNI objects
xn::Context g_Context;
xn::DepthGenerator g_DepthGenerator;
xn::HandsGenerator g_HandsGenerator;
xn::GestureGenerator g_GestureGenerator;

// NITE objects
XnVSessionManager* g_pSessionManager;
XnVFlowRouter* g_pFlowRouter;
XnVBroadcaster* g_pBroadcaster;

const int NUM_STEADY_BUTTONS = 4;
SteadyButton* g_pSButton[NUM_STEADY_BUTTONS];

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
int counter = 0;
SessionState g_SessionState = NOT_IN_SESSION;

typedef enum {
  SHAPE_SELECTION,
  SHAPE_MANIPULATION,
  TOOL_SELECTION,
} UserMode;

UserMode g_UserMode = SHAPE_SELECTION;

typedef enum {
  TRANSLATION,
  ROTATION,
  SCALE,
} ToolSelected;

ToolSelected g_ToolSelected = TRANSLATION;

void CleanupExit()
{
  /*	
    for(int i = 0; i < NUM_STEADY_BUTTONS; i++) {
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

// Callback for when the focus is in progress
void XN_CALLBACK_TYPE FocusProgress(const XnChar* strFocus, const XnPoint3D& ptPosition, XnFloat fProgress, void* UserCxt)
{
//	printf("Focus progress: %s @(%f,%f,%f): %f\n", strFocus, ptPosition.X, ptPosition.Y, ptPosition.Z, fProgress);
}
// callback for session start
void XN_CALLBACK_TYPE SessionStarting(const XnPoint3D& ptPosition, void* UserCxt)
{
	printf("Session start: (%f,%f,%f)\n", ptPosition.X, ptPosition.Y, ptPosition.Z);
	g_SessionState = IN_SESSION;
}
// Callback for session end
void XN_CALLBACK_TYPE SessionEnding(void* UserCxt)
{
	printf("Session end\n");
	g_SessionState = NOT_IN_SESSION;
}
void XN_CALLBACK_TYPE NoHands(void* UserCxt)
{
	if (g_SessionState != NOT_IN_SESSION)
	{
		printf("Quick refocus\n");
		g_SessionState = QUICK_REFOCUS;
	}
}

// Callback for the SteadyButton
void XN_CALLBACK_TYPE SteadyButton_Select(void* cxt)
{
  if(g_UserMode == SHAPE_SELECTION) {
    counter++;
    
    SteadyButton* box = (SteadyButton*)(cxt);
    box->SetSelected();
    
    // Create object for this box
    // Automatically deselect the object
    // Add object to a list for tracking all objects being manupulated
    
    // remove the shape selection panel
    //delete [] g_pSButton;
    
    if(counter == DISPLAY_DELAY) {
      // Switch to manupulation mode
      g_UserMode = SHAPE_MANIPULATION;
      g_ToolSelected = TRANSLATION;
      for(int i = 0; i < NUM_STEADY_BUTTONS; i++) {
        g_pSButton[i]->SetUnselected();
      } 
      counter = 0;
    }
  }

  if(g_UserMode == SHAPE_MANIPULATION) {
    // detect object selection on steady
  }

  if(g_UserMode == TOOL_SELECTION) {
    // select tool
  }

}

// this function is called each frame
void glutDisplay (void)
{

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
    glOrtho(0, mode.nXRes, mode.nYRes, 0, -100.0, 100.0);
	#else
	glOrthof(0, mode.nXRes, mode.nYRes, 0, -100.0, 100.0);
	#endif

	glDisable(GL_TEXTURE_2D);

	if (!g_bPause)
	{
		// Read next available data
		g_Context.WaitAndUpdateAll();
		// Update NITE tree
		g_pSessionManager->Update(&g_Context);
		PrintSessionState(g_SessionState);
	}
    
    // Draw the shape icons if in shape selection mode
    if(g_UserMode == SHAPE_SELECTION) {
      for(int i = 0; i < NUM_STEADY_BUTTONS; i++) {
        g_pSButton[i]->Draw();
      }
    }

    // Draw the tool icon if in manupulation mode
    if(g_UserMode == SHAPE_MANIPULATION) {
      // draw current tool
      // initialize tool manipulation class/code
    }

    // Draw tool selection panel
    if(g_UserMode == TOOL_SELECTION) {
      // draw tool panel
    }

    
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
	glutCreateWindow ("PrimeSense Nite Point Viewer");
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
}
#endif


void initShapePanel() 
{
    // Register callback to the SteadyButton objects for their Select event.
    XnPoint3D ptMax, ptMin;
    ptMax.Z = 0;
    ptMin.Z = 0;
    ptMin.Y = GL_WIN_SIZE_Y-TOOL_SIZE*2-100;  
    ptMax.Y = GL_WIN_SIZE_Y-100;
    for(int i = 0; i < NUM_STEADY_BUTTONS; i++) {
      
      ptMin.X = 200 + i*2*TOOL_SIZE;
      ptMax.X = 200 + (i+1)*2*TOOL_SIZE;
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
    
    // Create NITE objects
    g_pSessionManager = new XnVSessionManager;
    rc = g_pSessionManager->Initialize(&g_Context, "Click,Wave", "RaiseHand");
    CHECK_RC(rc, "SessionManager::Initialize");
    g_pSessionManager->RegisterSession(NULL, SessionStarting, SessionEnding, FocusProgress);
    g_pDrawer = new XnVPointDrawer(20, g_DepthGenerator); 
    g_pBroadcaster = new XnVBroadcaster();
    g_pBroadcaster->AddListener(g_pDrawer);    
    g_pSessionManager->AddListener(g_pBroadcaster);
    g_pDrawer->RegisterNoPoints(NULL, NoHands);
    g_pDrawer->SetDepthMap(g_bDrawDepthMap);
    
    initShapePanel();

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
