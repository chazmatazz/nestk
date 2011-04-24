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


#define CUBE 0
#define SPHERE 1
#define CONE 2
#define TORUS 3
#define TRANSLATE 4
#define ROTATE 5
#define STRETCH 6
#define SELECT 7

typedef enum {
  SHAPE_SELECTION,
  SHAPE_MANIPULATION,
  TOOL_SELECTION,
} UserMode;

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
        case CUBE:
            glutWireCube(size);
            break;
        case SPHERE:
            glutWireSphere(size, 8, 8);
            break;
        case CONE:
            glutWireCone(size, size, 8, 8);
            break;
        case TORUS:
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
XnVCircleDetector*  g_pCircle;

const int NUM_SHAPE_BUTTONS = 4;
const int NUM_TOOL_BUTTONS = 2;
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
int counter = 0;
SessionState g_SessionState = NOT_IN_SESSION;

UserMode g_UserMode = SHAPE_SELECTION;
int g_Shape;
int g_Tool;
XnBool g_bSelect = false;


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
  printf("button selected\n");

  // Select shape
  if(g_UserMode == SHAPE_SELECTION) {
    counter++;

    SteadyButton* button = (SteadyButton*)(cxt);
    button->SetSelected();

    // Delay for highlighing time    
    if(counter == DISPLAY_DELAY) {
      counter = 0;

      // deactivate the shape menu
      for(int i = 0; i < NUM_SHAPE_BUTTONS; i++) {
        g_pSButton[i]->TurnOff();
      } 

      g_Shape = button->getType();    

      // Create object for this button
      switch(g_Shape) {
        case CUBE:
          printf("Created shape CUBE!\n");
          break;
        case SPHERE:
          printf("Created shape SPHERE!\n");
          break;
        case CONE:
          printf("Created shape CONE!\n");
          break;
        case TORUS:
          printf("Created shape TORUS!\n");
          break;
      }

      // Automatically deselect the object
      // Add object to a list for tracking all objects being manupulated
      g_UserMode = SHAPE_MANIPULATION;
      g_Tool = TRANSLATE;
    }
  }


  // detect object selection on steady
  if(g_UserMode == SHAPE_MANIPULATION) {
    // TODO?
  }

  

  // select tool
  if(g_UserMode == TOOL_SELECTION) {
    counter++;
    
    SteadyButton* button = (SteadyButton*)(cxt);
    button->SetSelected();

    // Delay for highlighting time
    if(counter == DISPLAY_DELAY) {
      g_UserMode = SHAPE_MANIPULATION;
      counter = 0;

      // deactivate the tool selection menu
      for(int i = 0; i < NUM_TOOL_BUTTONS; i++) {
        g_pTButton[i]->TurnOff();
      } 

      g_Tool = button->getType();    

      // Update the tool icon in the corner
      switch(g_Tool) {
        case TRANSLATE:
          printf("Switch to Translation tool!\n");
          break;
        case ROTATE:
          printf("Switch to Rotation tool!\n");
          break;
      }

      // Automatically deselect the object
    }
  }
}

void XN_CALLBACK_TYPE CircleCB(XnFloat fTimes, XnVCircleDetector::XnVNoCircleReason eReason, void* pUserCxt)
{
  printf("circle finish detected\n");
  if(g_UserMode == SHAPE_MANIPULATION) {
    printf("open tool menu\n");
    g_UserMode = TOOL_SELECTION;
    for (int i = 0; i < NUM_TOOL_BUTTONS; i++) {
      // Activate the button
      g_pTButton[i]->SetUnselected();
    }
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
      for(int i = 0; i < NUM_SHAPE_BUTTONS; i++) {
        g_pSButton[i]->Draw();
      }
    }

    // Draw the tool icon if in manupulation mode
    if(g_UserMode == SHAPE_MANIPULATION) {
      int i;
      switch(g_Tool) 
        {
        case TRANSLATE:
          i = 0;
          break;
        case ROTATE:
          i = 1;
          break;
        case STRETCH:
          i = 2;
          break;
        }

      // Temporary just to draw something for state
      DrawTool(size, size, 0, g_Tool, size, 1);
    }

    // Draw tool selection panel
    if(g_UserMode == TOOL_SELECTION) {
      // might handle this in shape manipulation mode and just make this
      // another tool mode
        for(int i = 0; i < NUM_TOOL_BUTTONS; i++) {
          g_pTButton[i]->Draw();
        }
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
    case 'n':
        // select mode (New) 
        g_Tool = SELECT;
        break;
    case 't':
        // translate mode 
        g_Tool = TRANSLATE;
        break;
    case 'r':
        // rotate mode 
        g_Tool = ROTATE;
        break;
    case 'k':
        // stretch mode (K) 
        g_Tool = STRETCH;
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
}
#endif

void initToolPanel() 
{
    // Register callback to the SteadyButton objects for their Select event.
    XnPoint3D ptMax, ptMin;
    int Xshift = (GL_WIN_SIZE_X-(2*TOOL_SIZE*NUM_TOOL_BUTTONS))/2;
    printf("init %d tools", NUM_TOOL_BUTTONS);
    ptMax.Z = 0;
    ptMin.Z = 0;
    ptMin.Y = GL_WIN_SIZE_Y-TOOL_SIZE*2-150;  
    ptMax.Y = GL_WIN_SIZE_Y-150;
    for(int i = 0; i < NUM_TOOL_BUTTONS; i++) {
      
      ptMin.X = Xshift + i*2*TOOL_SIZE;
      ptMax.X = Xshift + (i+1)*2*TOOL_SIZE;
      g_pTButton[i] = new SteadyButton(ptMax, ptMin, i+(NUM_SHAPE_BUTTONS), TOOL_SIZE, g_DepthGenerator);
      g_pTButton[i]->RegisterSelect(NULL, &SteadyButton_Select);
      g_pTButton[i]->TurnOff();
      g_pSessionManager->AddListener(g_pTButton[i]);

    }
    printf("finished init tools");
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
    
    // Create NITE objects
    g_pSessionManager = new XnVSessionManager;
    rc = g_pSessionManager->Initialize(&g_Context, "Click,Wave", "RaiseHand");
    CHECK_RC(rc, "SessionManager::Initialize");
    g_pSessionManager->RegisterSession(NULL, SessionStarting, SessionEnding, FocusProgress);
    g_pDrawer = new XnVPointDrawer(20, g_DepthGenerator); 
	g_pCircle = new XnVCircleDetector;
    g_pBroadcaster = new XnVBroadcaster();
    g_pBroadcaster->AddListener(g_pDrawer);    
    g_pBroadcaster->AddListener(g_pCircle);    
    g_pSessionManager->AddListener(g_pBroadcaster);
    g_pCircle->RegisterNoCircle(NULL, CircleCB);    
    g_pDrawer->RegisterNoPoints(NULL, NoHands);
    g_pDrawer->SetDepthMap(g_bDrawDepthMap);

    initShapePanel();
    initToolPanel();

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
