//#include "stdafx.h"
#include "iostream"
#ifdef USE_GLUT
#if (XN_PLATFORM == XN_PLATFORM_MACOSX)
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif
#endif

bool fullscreen = false;
bool mouseDown = false;

float xrot = 0.0f;
float yrot = 0.0f;

float x = 0.0f;
float y = 0.0f;
float z = 0.0f;

float width = 1.0f;
float depth = 1.0f;
float height = 1.0f;

float xdiff = 0.0f;
float ydiff = 0.0f;

int keyStates[256] = {0};

void keyOperations (void) 
{
	if (keyStates['a'] || keyStates['A']) 
	{
		x -= .01f;
	}
	if (keyStates['d'] || keyStates['D']) 
	{
		x += .01f;
	}
	if (keyStates['w'] || keyStates['W']) 
	{
		y += .01f;
	}
	if (keyStates['s'] || keyStates['S']) 
	{
		y -= .01f;
	}
	if (keyStates['q'] || keyStates['Q']) 
	{
		z += .01f;
	}
	if (keyStates['e'] || keyStates['E']) 
	{
		z -= .01f;
	}
	if (keyStates['r'] || keyStates['R']) 
	{
		width += .01f;
	}
	if (keyStates['t'] || keyStates['T']) 
	{
		height += .01f;
	}
	if (keyStates['y'] || keyStates['Y']) 
	{
		depth += .01f;
	}
	if (keyStates['f'] || keyStates['F']) 
	{
		width -= .01f;
	}
	if (keyStates['g'] || keyStates['G']) 
	{
		height -= .01f;
	}
	if (keyStates['h'] || keyStates['H']) 
	{
		depth -= .01f;
	}
}
void keyPressed (unsigned char key, int x, int y) 
{
	keyStates[key] = true; // Set the state of the current key to pressed
}

void keyUp (unsigned char key, int x, int y) 
{
	keyStates[key] = false; // Set the state of the current key to not pressed
}

void drawBox(float width, float height, float depth)
{
	glBegin(GL_QUADS);

		glColor3f(1.0f, 0.0f, 0.0f);
		// FRONT
		glVertex3f(-width/2, -height/2, depth/2);
		glVertex3f( width/2, -height/2, depth/2);
		glVertex3f( width/2, height/2, depth/2);
		glVertex3f(-width/2, height/2, depth/2);
		// BACK
		glVertex3f(-width/2, -height/2, -depth/2);
		glVertex3f(-width/2, height/2, -depth/2);
		glVertex3f( width/2, height/2, -depth/2);
		glVertex3f( width/2, -height/2, -depth/2);

		glColor3f(0.0f, 1.0f, 0.0f);
		// LEFT
		glVertex3f(-width/2, -height/2, depth/2);
		glVertex3f(-width/2, height/2, depth/2);
		glVertex3f(-width/2, height/2, -depth/2);
		glVertex3f(-width/2, -height/2, -depth/2);
		// RIGHT
		glVertex3f( width/2, -height/2, -depth/2);
		glVertex3f( width/2, height/2, -depth/2);
		glVertex3f( width/2, height/2, depth/2);
		glVertex3f( width/2, -height/2, depth/2);

		glColor3f(0.0f, 0.0f, 1.0f);
		// TOP
		glVertex3f(-width/2, height/2, depth/2);
		glVertex3f( width/2, height/2, depth/2);
		glVertex3f( width/2, height/2, -depth/2);
		glVertex3f(-width/2, height/2, -depth/2);
		// BOTTOM
		glVertex3f(-width/2, -height/2, depth/2);
		glVertex3f(-width/2, -height/2, -depth/2);
		glVertex3f( width/2, -height/2, -depth/2);
		glVertex3f( width/2, -height/2, depth/2);

	glEnd();

}

bool init()
{
	glClearColor(0.93f, 0.93f, 0.93f, 0.0f);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glClearDepth(1.0f);

	return true;
}

void display()
{
	keyOperations();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	glTranslatef(0.0f,0.0f,-10.0f);
	glTranslatef(x,y,z);
	glRotatef(xrot, 1.0f, 0.0f, 0.0f);
	glRotatef(yrot, 0.0f, 1.0f, 0.0f);

	drawBox(width,height,depth);

	glFlush();
	glutSwapBuffers();
}

void resize(int w, int h)
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glViewport(0, 0, w, h);

	gluPerspective(45.0f, 1.0f * w / h, 1.0f, 100.0f);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void keyboard(unsigned char key, int x, int y)
{
	switch(key)
	{
		case 27 : 
		exit(1); break;
	}
}

void specialKeyboard(int key, int x, int y)
{
	if (key == GLUT_KEY_F1)
	{
		fullscreen = !fullscreen;

		if (fullscreen)
		glutFullScreen();
		else
		{
			glutReshapeWindow(500, 500);
			glutPositionWindow(50, 50);
		}
	}
}

void mouse(int button, int state, int x, int y)
{
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
	{
		mouseDown = true;

		xdiff = x - yrot;
		ydiff = -y + xrot;
	}
	else
		mouseDown = false;
}

void mouseMotion(int x, int y)
{
	if (mouseDown)
	{
		yrot = x - xdiff;
		xrot = y + ydiff;

		glutPostRedisplay();
	}
}

int init(int argc, char *argv[])
{
	glutInit(&argc, argv);

	glutInitWindowPosition(50, 50);
	glutInitWindowSize(500, 500);

	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);

	glutCreateWindow("13 - Solid Shapes");

	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
	glutSpecialFunc(specialKeyboard);
	glutMouseFunc(mouse);
	glutMotionFunc(mouseMotion);
	glutReshapeFunc(resize);
	glutIdleFunc(display);
	glutKeyboardFunc(keyPressed); // Tell GLUT to use the method "keyPressed" for key presses
	glutKeyboardUpFunc(keyUp); // Tell GLUT to use the method "keyUp" for key up events
	
	if (!init())
		return 1;

	glutMainLoop();

	return 0;
}
