#include <stdio.h>
#include <windows.h>
#include <GL/freeglut.h>
#include <math.h>
#include <string.h>
#include "mechBotAnimator.h"
#include "subdivcurve.h"
#include <GL/glut.h>


int numCirclePoints = 30;
double circleRadius = 0.2;
int hoveredCircle = -1;
int hoveredCircle2 = -1;
int curveIndex = 0;
int currentCurvePoint = 0;
int angle = 0;
int animate = 0;
int delay = 15; // milliseconds

float moveX = 0;
float moveZ = 0;

float moveX2 = 0;
float moveZ2 = 0;

float control = 0;

float robotBodyWidth = 1.0;
float robotBodyLength = 0.165;
float robotBodyDepth = 0.5;
float robotPositionx = 0.0;
float robotPositionz = 0.0;
float headRadius = robotBodyWidth / 2.5;
float headLength = headRadius * 4;
float baseWidth = 2 * robotBodyWidth;
float baseLength = 0.25 * robotBodyLength;

float gunAngle = 0;
float gunAngle2 = 0;
float oldangle = 0;

int count = -3;
int count2 = -3;
float proj = 0;
float proj2 = 0;
boolean firing = false;
boolean hit = false;
boolean hit2 = false;
boolean cannonHit = false;
boolean fp = false;
float bulletX = 0;

GLdouble worldLeft = -12;
GLdouble worldRight = 12;
GLdouble worldBottom = -9;
GLdouble worldTop = 9;
GLdouble worldCenterX = 0.0;
GLdouble worldCenterY = 0.0;
GLdouble wvLeft = -12;
GLdouble wvRight = 12;
GLdouble wvBottom = -9;
GLdouble wvTop = 9;

GLint glutWindowWidth = 800;
GLint glutWindowHeight = 600;
GLint viewportWidth = glutWindowWidth;
GLint viewportHeight = glutWindowHeight;

// Ground Mesh material
GLfloat groundMat_ambient[] = { 0.4, 0.4, 0.4, 1.0 };
GLfloat groundMat_specular[] = { 0.01, 0.01, 0.01, 1.0 };
GLfloat groundMat_diffuse[] = { 0.4, 0.4, 0.7, 1.0 };
GLfloat groundMat_shininess[] = { 1.0 };


GLfloat light_position0[] = { 4.0, 8.0, 8.0, 1.0 };
GLfloat light_diffuse0[] = { 1.0, 1.0, 1.0, 1.0 };

GLfloat light_position1[] = { -4.0, 8.0, 8.0, 1.0 };
GLfloat light_diffuse1[] = { 1.0, 1.0, 1.0, 1.0 };

GLfloat light_specular[] = { 1.0, 1.0, 1.0, 1.0 };
GLfloat model_ambient[] = { 0.5, 0.5, 0.5, 1.0 };

// 
GLdouble spin = 0.0;

// The 2D animation path curve is a subdivision curve
SubdivisionCurve subcurve;
SubdivisionCurve subcurve2;

// Use circles to **draw** (i.e. visualize) subdivision curve control points
Circle circles[MAXCONTROLPOINTS];
Circle circles2[MAXCONTROLPOINTS];

int lastMouseX;
int lastMouseY;
int window2D, window3D;
int window3DSizeX = 800, window3DSizeY = 600;
GLdouble aspect = (GLdouble)window3DSizeX / window3DSizeY;
GLdouble eyeX = 0.0, eyeY = 6.0, eyeZ = 22.0;
GLdouble zNear = 0.1, zFar = 40.0;
GLdouble fov = 60.0;

GLfloat textureMap[64][64][3];
GLuint tex[2];

void assignColor(GLfloat col[3], GLfloat r, GLfloat g, GLfloat b) {
	col[0] = r;
	col[1] = g;
	col[2] = b;
}


void makeTextureMap()
{
	for (int i = 0; i < 64; i++)
		for (int j = 0; j < 64; j++) {
			if ((j / 2) % 2 == 0) {
				assignColor(textureMap[i][j], 1.0, 1.0, 1.0);
			}
			else {
				assignColor(textureMap[i][j], 1.0, 0.0, 0.0);
			}
		}
}

void makeTextures()
{
	GLfloat planes[] = { 0.0, 0.0, 0.25, 0.0 };
	GLfloat planet[] = { 0.0, 0.25, 0.0, 0.0 };

	glGenTextures(2, tex);

	glBindTexture(GL_TEXTURE_2D, tex[0]);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 64, 64, 0, GL_RGB, GL_FLOAT, textureMap);

	glBindTexture(GL_TEXTURE_2D, tex[1]);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
	glTexGenfv(GL_S, GL_OBJECT_PLANE, planes);
	glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
	glTexGenfv(GL_T, GL_OBJECT_PLANE, planet);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 64, 64, 0, GL_RGB, GL_FLOAT, textureMap);
}



int main(int argc, char* argv[])
{
	glutInit(&argc, (char**)argv);

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize(glutWindowWidth, glutWindowHeight);
	glutInitWindowPosition(50, 100);

	// The 2D Window
	window2D = glutCreateWindow("Animation Path Designer");
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	// Initialize the 2D profile curve system
	init2DCurveWindow();
	// A few input handlers
	glutMouseFunc(mouseButtonHandler);
	glutMotionFunc(mouseMotionHandler);
	glutPassiveMotionFunc(mouseHoverHandler);
	glutMouseWheelFunc(mouseScrollWheelHandler);
	glutKeyboardFunc(keyboardHandler);
	glutSpecialFunc(specialKeyHandler);


	// The 3D Window
	window3D = glutCreateWindow("Mech Bot");
	glutPositionWindow(900, 100);
	glutDisplayFunc(display3D);
	glutReshapeFunc(reshape3D);
	glutMouseFunc(mouseButtonHandler3D);
	glutMouseWheelFunc(mouseScrollWheelHandler3D);
	glutMotionFunc(mouseMotionHandler3D);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_TEXTURE_2D);
	makeTextureMap();
	makeTextures();

	// Initialize the 3D system
	init3DSurfaceWindow();

	// Annnd... ACTION!!
	glutMainLoop();

	return 0;
}

void init2DCurveWindow()
{
	glLineWidth(3.0);
	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	glClearColor(0.4F, 0.4F, 0.4F, 0.0F);
	initSubdivisionCurve();
	initSubdivisionCurve2();
	initControlPoints();
	initControlPoints2();
}

void display()
{
	glClear(GL_COLOR_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(wvLeft, wvRight, wvBottom, wvTop);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	draw2DScene();
	glutSwapBuffers();
}


void draw2DScene()
{
	drawAxes();
	drawSubdivisionCurve();
	drawSubdivisionCurve2();
	drawControlPoints();
	drawControlPoints2();
}

void drawAxes()
{
	glPushMatrix();
	glColor3f(1.0, 0.0, 0);
	glBegin(GL_LINE_STRIP);
	glVertex3f(0, 8.0, 0);
	glVertex3f(0, -8.0, 0);
	glEnd();
	glBegin(GL_LINE_STRIP);
	glVertex3f(-8, 0.0, 0);
	glVertex3f(8, 0.0, 0);
	glEnd();
	glPopMatrix();
}

void drawSubdivisionCurve() {
	// Subdivide the given curve
	computeSubdivisionCurve(&subcurve);

	int i = 0;

	glColor3f(0.0, 1.0, 0.0);
	glPushMatrix();
	glBegin(GL_LINE_STRIP);
	for (i = 0; i < subcurve.numCurvePoints; i++) {
		glVertex3f(subcurve.curvePoints[i].x, subcurve.curvePoints[i].y, 0.0);
	}
	glEnd();
	glPopMatrix();
}

void drawSubdivisionCurve2() {
	// Subdivide the given curve
	computeSubdivisionCurve(&subcurve2);

	int i = 0;

	glColor3f(0.0, 1.0, 0.0);
	glPushMatrix();
	glBegin(GL_LINE_STRIP);
	for (i = 0; i < subcurve2.numCurvePoints; i++) {
		subcurve2.curvePoints[i].x += 3;
		glVertex3f(subcurve2.curvePoints[i].x, subcurve2.curvePoints[i].y, 0.0);
	}
	glEnd();
	glPopMatrix();
}

void drawControlPoints() {
	int i, j;
	for (i = 0; i < subcurve.numControlPoints; i++) {
		glPushMatrix();
		glColor3f(1.0f, 0.0f, 0.0f);
		glTranslatef(circles[i].circleCenter.x, circles[i].circleCenter.y, 0);
		// for the hoveredCircle, draw an outline and change its colour
		if (i == hoveredCircle) {
			// outline
			glColor3f(0.0, 1.0, 0.0);
			glBegin(GL_LINE_LOOP);
			for (j = 0; j < numCirclePoints; j++) {
				glVertex3f(circles[i].circlePoints[j].x, circles[i].circlePoints[j].y, 0);
			}
			glEnd();
			// colour change
			glColor3f(0.5, 0.0, 1.0);
		}
		glBegin(GL_LINE_LOOP);
		for (j = 0; j < numCirclePoints; j++) {
			glVertex3f(circles[i].circlePoints[j].x, circles[i].circlePoints[j].y, 0);
		}
		glEnd();
		glPopMatrix();
	}
}

void drawControlPoints2() {
	int i, j;
	for (i = 0; i < subcurve2.numControlPoints; i++) {
		glPushMatrix();
		glColor3f(1.0f, 0.0f, 0.0f);
		glTranslatef(circles2[i].circleCenter.x + 3, circles2[i].circleCenter.y, 0);
		// for the hoveredCircle, draw an outline and change its colour
		if (i == hoveredCircle2) {
			// outline
			glColor3f(0.0, 1.0, 0.0);
			glBegin(GL_LINE_LOOP);
			for (j = 0; j < numCirclePoints; j++) {
				glVertex3f(circles2[i].circlePoints[j].x, circles2[i].circlePoints[j].y, 0);
			}
			glEnd();
			// colour change
			glColor3f(0.5, 0.0, 1.0);
		}
		glBegin(GL_LINE_LOOP);
		for (j = 0; j < numCirclePoints; j++) {
			glVertex3f(circles2[i].circlePoints[j].x, circles2[i].circlePoints[j].y, 0);
		}
		glEnd();
		glPopMatrix();
	}
}

void initSubdivisionCurve() {
	// Initialize 3 control points of the subdivision curve

	GLdouble x, y;

	x = 4 * cos(M_PI * 0.5);
	y = 4 * sin(M_PI * 0.5);
	subcurve.controlPoints[0].x = x;
	subcurve.controlPoints[0].y = y;

	x = 4 * cos(M_PI * 0.25);
	y = 4 * sin(M_PI * 0.25);
	subcurve.controlPoints[1].x = x;
	subcurve.controlPoints[1].y = y;

	x = 4 * cos(M_PI * 0.0);
	y = 4 * sin(M_PI * 0.0);
	subcurve.controlPoints[2].x = x;
	subcurve.controlPoints[2].y = y;


	x = 4 * cos(-M_PI * 0.25);
	y = 4 * sin(-M_PI * 0.25);
	subcurve.controlPoints[3].x = x;
	subcurve.controlPoints[3].y = y;


	x = 4 * cos(-M_PI * 0.5);
	y = 4 * sin(-M_PI * 0.5);
	subcurve.controlPoints[4].x = x;
	subcurve.controlPoints[4].y = y;

	subcurve.numControlPoints = 5;
	subcurve.subdivisionSteps = 4;
}

void initSubdivisionCurve2() {
	// Initialize 3 control points of the subdivision curve

	GLdouble x, y;

	x = 4 * cos(M_PI * 0.5);
	y = 4 * sin(M_PI * 0.5);
	subcurve2.controlPoints[0].x = x;
	subcurve2.controlPoints[0].y = y;

	x = 4 * cos(M_PI * 0.25);
	y = 4 * sin(M_PI * 0.25);
	subcurve2.controlPoints[1].x = x;
	subcurve2.controlPoints[1].y = y;

	x = 4 * cos(M_PI * 0.0);
	y = 4 * sin(M_PI * 0.0);
	subcurve2.controlPoints[2].x = x;
	subcurve2.controlPoints[2].y = y;

	x = 4 * cos(-M_PI * 0.25);
	y = 4 * sin(-M_PI * 0.25);
	subcurve2.controlPoints[3].x = x;
	subcurve2.controlPoints[3].y = y;

	x = 4 * cos(-M_PI * 0.5);
	y = 4 * sin(-M_PI * 0.5);
	subcurve2.controlPoints[4].x = x;
	subcurve2.controlPoints[4].y = y;

	subcurve2.numControlPoints = 5;
	subcurve2.subdivisionSteps = 4;
}

void initControlPoints() {
	int i;
	int num = subcurve.numControlPoints;
	for (i = 0; i < num; i++) {
		constructCircle(circleRadius, numCirclePoints, circles[i].circlePoints);
		circles[i].circleCenter = subcurve.controlPoints[i];
	}
}

void initControlPoints2() {
	int i;
	int num = subcurve2.numControlPoints;
	for (i = 0; i < num; i++) {

		constructCircle(circleRadius, numCirclePoints, circles2[i].circlePoints);
		circles2[i].circleCenter = subcurve2.controlPoints[i];
	}
}


void screenToWorldCoordinates(int xScreen, int yScreen, GLdouble* xw, GLdouble* yw)
{
	GLdouble xView, yView;
	screenToCameraCoordinates(xScreen, yScreen, &xView, &yView);
	cameraToWorldCoordinates(xView, yView, xw, yw);
}

void screenToCameraCoordinates(int xScreen, int yScreen, GLdouble* xCamera, GLdouble* yCamera)
{
	*xCamera = ((wvRight - wvLeft) / glutWindowWidth) * xScreen;
	*yCamera = ((wvTop - wvBottom) / glutWindowHeight) * (glutWindowHeight - yScreen);
}

void cameraToWorldCoordinates(GLdouble xcam, GLdouble ycam, GLdouble* xw, GLdouble* yw)
{
	*xw = xcam + wvLeft;
	*yw = ycam + wvBottom;
}

void worldToCameraCoordiantes(GLdouble xWorld, GLdouble yWorld, GLdouble* xcam, GLdouble* ycam)
{
	double wvCenterX = wvLeft + (wvRight - wvLeft) / 2.0;
	double wvCenterY = wvBottom + (wvTop - wvBottom) / 2.0;
	*xcam = worldCenterX - wvCenterX + xWorld;
	*ycam = worldCenterY - wvCenterY + yWorld;
}

int currentButton;

void mouseButtonHandler(int button, int state, int xMouse, int yMouse)
{
	int i;

	currentButton = button;
	if (button == GLUT_LEFT_BUTTON)
	{
		switch (state) {
		case GLUT_DOWN:
			if (hoveredCircle > -1) {
				screenToWorldCoordinates(xMouse, yMouse, &circles[hoveredCircle].circleCenter.x, &circles[hoveredCircle].circleCenter.y);
				screenToWorldCoordinates(xMouse, yMouse, &subcurve.controlPoints[hoveredCircle].x, &subcurve.controlPoints[hoveredCircle].y);
			}
			else if (hoveredCircle2 > -1) {
				screenToWorldCoordinates(xMouse, yMouse, &circles2[hoveredCircle2].circleCenter.x, &circles2[hoveredCircle2].circleCenter.y);
				screenToWorldCoordinates(xMouse, yMouse, &subcurve2.controlPoints[hoveredCircle2].x, &subcurve2.controlPoints[hoveredCircle2].y);
			}
			break;
		case GLUT_UP:
			glutSetWindow(window3D);
			glutPostRedisplay();
			break;
		}
	}
	else if (button == GLUT_MIDDLE_BUTTON)
	{
		switch (state) {
		case GLUT_DOWN:
			break;
		case GLUT_UP:
			if (hoveredCircle == -1 && subcurve.numControlPoints < MAXCONTROLPOINTS) {
				GLdouble newPointX;
				GLdouble newPointY;
				screenToWorldCoordinates(xMouse, yMouse, &newPointX, &newPointY);
				subcurve.controlPoints[subcurve.numControlPoints].x = newPointX;
				subcurve.controlPoints[subcurve.numControlPoints].y = newPointY;
				constructCircle(circleRadius, numCirclePoints, circles[subcurve.numControlPoints].circlePoints);
				circles[subcurve.numControlPoints].circleCenter = subcurve.controlPoints[subcurve.numControlPoints];
				subcurve.numControlPoints++;
			}
			else if (hoveredCircle > -1 && subcurve.numControlPoints > MINCONTROLPOINTS) {
				subcurve.numControlPoints--;
				for (i = hoveredCircle; i < subcurve.numControlPoints; i++) {
					subcurve.controlPoints[i].x = subcurve.controlPoints[i + 1].x;
					subcurve.controlPoints[i].y = subcurve.controlPoints[i + 1].y;
					circles[i].circleCenter = circles[i + 1].circleCenter;
				}
			}

			if (hoveredCircle2 == -1 && subcurve2.numControlPoints < MAXCONTROLPOINTS) {
				GLdouble newPointX;
				GLdouble newPointY;
				screenToWorldCoordinates(xMouse, yMouse, &newPointX, &newPointY);
				subcurve2.controlPoints[subcurve2.numControlPoints].x = newPointX;
				subcurve2.controlPoints[subcurve2.numControlPoints].y = newPointY;
				constructCircle(circleRadius, numCirclePoints, circles2[subcurve2.numControlPoints].circlePoints);
				circles2[subcurve2.numControlPoints].circleCenter = subcurve2.controlPoints[subcurve2.numControlPoints];
				subcurve2.numControlPoints++;
			}
			else if (hoveredCircle2 > -1 && subcurve2.numControlPoints > MINCONTROLPOINTS) {
				subcurve2.numControlPoints--;
				for (i = hoveredCircle2; i < subcurve2.numControlPoints; i++) {
					subcurve2.controlPoints[i].x = subcurve2.controlPoints[i + 1].x;
					subcurve2.controlPoints[i].y = subcurve2.controlPoints[i + 1].y;
					circles2[i].circleCenter = circles2[i + 1].circleCenter;
				}
			}

			glutSetWindow(window3D);
			glutPostRedisplay();
			break;
		}


	}

	glutSetWindow(window2D);
	glutPostRedisplay();
}

void mouseMotionHandler(int xMouse, int yMouse)
{
	if (currentButton == GLUT_LEFT_BUTTON) {
		if (hoveredCircle > -1) {
			screenToWorldCoordinates(xMouse, yMouse, &circles[hoveredCircle].circleCenter.x, &circles[hoveredCircle].circleCenter.y);
			screenToWorldCoordinates(xMouse, yMouse, &subcurve.controlPoints[hoveredCircle].x, &subcurve.controlPoints[hoveredCircle].y);
		}
		if (hoveredCircle2 > -1) {
			screenToWorldCoordinates(xMouse, yMouse, &circles2[hoveredCircle2].circleCenter.x, &circles2[hoveredCircle2].circleCenter.y);
			screenToWorldCoordinates(xMouse, yMouse, &subcurve2.controlPoints[hoveredCircle2].x, &subcurve2.controlPoints[hoveredCircle2].y);
		}
	}
	else if (currentButton == GLUT_MIDDLE_BUTTON) {
	}
	glutPostRedisplay();
}


void mouseHoverHandler(int xMouse, int yMouse)
{
	hoveredCircle = -1;
	hoveredCircle2 = -1;
	GLdouble worldMouseX, worldMouseY;
	screenToWorldCoordinates(xMouse, yMouse, &worldMouseX, &worldMouseY);
	int i;
	// see if we're hovering over a circle
	for (i = 0; i < subcurve.numControlPoints; i++) {
		GLdouble distToX = worldMouseX - circles[i].circleCenter.x;
		GLdouble distToY = worldMouseY - circles[i].circleCenter.y;
		GLdouble euclideanDist = sqrt(distToX * distToX + distToY * distToY);
		//printf("Dist from point %d is %.2f\n", i, euclideanDist);
		if (euclideanDist < 2.0) {
			hoveredCircle = i;
		}
	}

	for (i = 0; i < subcurve2.numControlPoints; i++) {
		GLdouble distToX = worldMouseX - circles2[i].circleCenter.x;
		GLdouble distToY = worldMouseY - circles2[i].circleCenter.y;
		GLdouble euclideanDist2 = sqrt(distToX * distToX + distToY * distToY);
		//printf("Dist from point %d is %.2f\n", i, euclideanDist);
		if (euclideanDist2 < 2.0) {
			hoveredCircle2 = i;
		}
	}
	glutPostRedisplay();
}

void mouseScrollWheelHandler(int button, int dir, int xMouse, int yMouse)
{

	glutPostRedisplay();

}

bool stop = false;

void keyboardHandler(unsigned char key, int x, int y)
{
	switch (key) {
	case 'q':
	case 'Q':
	case 27:
		// Esc, q, or Q key = Quit 
		exit(0);
		break;
	case 'a':
		// Add code to create timer and call animation handler
		//......
		// Use this to set to 3D window and redraw it
		stop = false;
		glutTimerFunc(50, animationHandler, 0);
		glutSetWindow(window3D);
		glutPostRedisplay();
		break;
	case 'r':
		// reset object position at beginning of curve
		//.......
		stop = true;
		moveX = 1 / 2 * (subcurve.curvePoints[0].x - subcurve.curvePoints[curveIndex].x);
		moveZ = 1 / 2 * (subcurve.curvePoints[0].y - subcurve.curvePoints[curveIndex].y);
		curveIndex = 0;
		angle = 0;
		hit = false;
		hit2 = false;
		count = -3;
		count2 = -3;
		cannonHit = false;
		break;
	case ' ':
		//Space to fire the cannon
		firing = true;
		glutSetWindow(window3D);
		glutPostRedisplay();
		break;
	default:
		break;
	}
	glutPostRedisplay();
}

void specialKeyHandler(int key, int x, int y)
{
	switch (key) {
	case GLUT_KEY_F1:
		printf("LEFT - move cannon left\nRIGHT - move cannon right\nUP - first person mode\nDOWN - normal mode\nSPACE - shoot\na - start animation\nr - reset level");
		break;
	case GLUT_KEY_LEFT:
		// Moves cannon left
		if (cannonHit == false) {
			control -= 0.2;
		}
		glutSetWindow(window3D);
		glutPostRedisplay();
		break;
	case GLUT_KEY_RIGHT:
		// Moves cannon right
		if (cannonHit == false) {
			control += 0.2;
		}
		glutSetWindow(window3D);
		glutPostRedisplay();
		break;
	case GLUT_KEY_UP:
		//Switch to first person
		fp = true;
		eyeX = control;
		eyeY = 1.25;
		eyeZ = 12.5;
		glutSetWindow(window3D);
		glutPostRedisplay();
		break;
	case GLUT_KEY_DOWN:
		//Switch to third person
		fp = false;
		eyeX = 0;
		eyeY = 6.0;
		eyeZ = 22.0;
		glutSetWindow(window3D);
		glutPostRedisplay();
		break;
	}
	glutPostRedisplay();
}


void reshape(int w, int h)
{
	glutWindowWidth = (GLsizei)w;
	glutWindowHeight = (GLsizei)h;
	glViewport(0, 0, glutWindowWidth, glutWindowHeight);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(wvLeft, wvRight, wvBottom, wvTop);
	glMatrixMode(GL_MODELVIEW);
	glutSetWindow(window3D);
	glutPostWindowRedisplay(window3D);
	glLoadIdentity();
}



/************************************************************************************
 *
 *
 * 3D Window Code
 *
 * Fill in the code in the empty functions
 ************************************************************************************/



void init3DSurfaceWindow()
{
	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse0);
	glLightfv(GL_LIGHT0, GL_POSITION, light_position0);
	glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
	glLightfv(GL_LIGHT0, GL_AMBIENT, model_ambient);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, light_diffuse1);
	glLightfv(GL_LIGHT1, GL_SPECULAR, light_specular);
	glLightfv(GL_LIGHT1, GL_AMBIENT, model_ambient);
	glLightfv(GL_LIGHT1, GL_POSITION, light_position1);

	glShadeModel(GL_SMOOTH);
	glEnable(GL_NORMALIZE);    // Renormalize normal vectors 
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHT1);

	glEnable(GL_COLOR_MATERIAL);

	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LINE_SMOOTH);
	glClearColor(0.4F, 0.4F, 0.4F, 0.0F);  // Color and depth for glClear

	glViewport(0, 0, (GLsizei)window3DSizeX, (GLsizei)window3DSizeY);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(fov, aspect, zNear, zFar);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(0.0, 6.0, 22.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
}


void reshape3D(int w, int h)
{
	glutWindowWidth = (GLsizei)w;
	glutWindowHeight = (GLsizei)h;
	glViewport(0, 0, glutWindowWidth, glutWindowHeight);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(fov, aspect, zNear, zFar);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(0.0, 6.0, 22.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
}

void animationHandler(int param)
{
	if ((!stop) and (curveIndex < subcurve.numCurvePoints - 1))
	{
		//Calculates difference in x and z and adds to total displacement to move object
		curveIndex += 1;
		moveX += subcurve.curvePoints[curveIndex].x - subcurve.curvePoints[curveIndex - 1].x;
		moveZ += -2 * subcurve.curvePoints[curveIndex].y - -2 * subcurve.curvePoints[curveIndex - 1].y;

		//Uses difference in x and z to calculate angle
		moveX2 = subcurve.curvePoints[curveIndex].x - subcurve.curvePoints[curveIndex - 1].x;
		moveZ2 = -2 * subcurve.curvePoints[curveIndex].y - -2 * subcurve.curvePoints[curveIndex - 1].y;

		//Calculate robot gun angle to follow cannon
		gunAngle = atan((subcurve.curvePoints[curveIndex].x - control) / (-2 * subcurve.curvePoints[curveIndex].y - 12.0)) * (180 / 3.14);
		gunAngle2 = atan((subcurve2.curvePoints[curveIndex].x + 3 - control) / (-2 * subcurve2.curvePoints[curveIndex].y - 12.0)) * (180 / 3.14);
		angle = 90 - atan(moveZ2 / moveX2) * (180 / 3.14);

		//Collision detection for shooting robot 1
		if (fabs(subcurve.curvePoints[curveIndex].x - bulletX) < 0.8 && fabs(subcurve.curvePoints[curveIndex].y - (12.0 - proj)) < 0.5) {
			hit = true;
			//Uncomment the following line to view cannon destroy animation
			//cannonHit = true;
		}
		//Collision detection for shooting robot 2
		if (fabs(subcurve2.curvePoints[curveIndex].x + 3 - bulletX) < 0.8 && fabs(subcurve2.curvePoints[curveIndex].y - (12.0 - proj)) < 0.5) {
			hit2 = true;
		}

		//Non-working code for cannon collision
		//if (fabs(proj2*(tan(oldangle * (3.14 / 180))) + circles[0].circleCenter.x + moveX - control) < 0.5 && fabs(proj2 - 12.0) < 0.5) {
			//cannonHit = true;
		//}

		//Switches to first person
		if (fp == true) {
			eyeX = control;
			eyeY = 1.25;
			eyeZ = 12.5;
		}

		//Move bullets in z-axis
		proj2 += 1;
		if (proj2 == 20) {
			proj2 = 0;
		}

		if (firing == true) {
			proj += 1;
			if (proj == 20) {
				proj = 0;
				firing = false;
			}
		}

		//Flips object back
		if (angle > 90) {
			angle = angle - 180;
		}

		glutPostRedisplay();
		glutTimerFunc(50, animationHandler, 0);
	}
}

void display3D()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	gluLookAt(eyeX, eyeY, eyeZ, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
	drawGround();
	draw3DSubdivisionCurve();
	draw3DControlPoints();
	draw3DSubdivisionCurve2();
	draw3DControlPoints2();
	drawBot();
	drawCannon();
	glutSwapBuffers();
}

void draw3DSubdivisionCurve()
{
	computeSubdivisionCurve(&subcurve);

	int i = 0;

	glColor3f(0.0, 1.0, 0.0);
	glPushMatrix();
	glBegin(GL_LINE_STRIP);
	for (i = 0; i < subcurve.numCurvePoints; i++) {
		glVertex3f(subcurve.curvePoints[i].x, 0, -2 * subcurve.curvePoints[i].y);
	}
	glEnd();
	glPopMatrix();
}

void draw3DSubdivisionCurve2()
{
	computeSubdivisionCurve(&subcurve2);

	int i = 0;

	glColor3f(0.0, 1.0, 0.0);
	glPushMatrix();
	glBegin(GL_LINE_STRIP);
	for (i = 0; i < subcurve2.numCurvePoints; i++) {
		glVertex3f(subcurve2.curvePoints[i].x + 3, 0, -2 * subcurve2.curvePoints[i].y);
	}
	glEnd();
	glPopMatrix();
}

void draw3DControlPoints()
{
	int i, j;
	for (i = 0; i < subcurve.numControlPoints; i++) {
		glPushMatrix();
		glColor3f(1.0f, 0.0f, 0.0f);
		glTranslatef(circles[i].circleCenter.x + 3, 0, -2 * circles[i].circleCenter.y);
		if (i == hoveredCircle) {
			glColor3f(0.0, 1.0, 0.0);
			glBegin(GL_LINE_LOOP);
			for (j = 0; j < numCirclePoints; j++) {
				glVertex3f(circles[i].circlePoints[j].x, 0, -2 * circles[i].circlePoints[j].y);
			}
			glEnd();
			glColor3f(0.5, 0.0, 1.0);
		}
		glBegin(GL_LINE_LOOP);
		for (j = 0; j < numCirclePoints; j++) {
			glVertex3f(circles[i].circlePoints[j].x, 0, -2 * circles[i].circlePoints[j].y);
		}
		glEnd();
		glPopMatrix();
	}
}

void draw3DControlPoints2()
{
	int i, j;
	for (i = 0; i < subcurve2.numControlPoints; i++) {
		glPushMatrix();
		glColor3f(1.0f, 0.0f, 0.0f);
		glTranslatef(circles2[i].circleCenter.x, 0, -2 * circles2[i].circleCenter.y);
		if (i == hoveredCircle2) {
			glColor3f(0.0, 1.0, 0.0);
			glBegin(GL_LINE_LOOP);
			for (j = 0; j < numCirclePoints; j++) {
				glVertex3f(circles2[i].circlePoints[j].x, 0, -2 * circles2[i].circlePoints[j].y);
			}
			glEnd();
			glColor3f(0.5, 0.0, 1.0);
		}
		glBegin(GL_LINE_LOOP);
		for (j = 0; j < numCirclePoints; j++) {
			glVertex3f(circles2[i].circlePoints[j].x, 0, -2 * circles2[i].circlePoints[j].y);
		}
		glEnd();
		glPopMatrix();
	}
}


GLfloat robotBody_mat_ambient[] = { 0.0f,0.0f,0.0f,1.0f };
GLfloat robotBody_mat_specular[] = { 0.45f,0.55f,0.45f,1.0f };
GLfloat robotBody_mat_diffuse[] = { 0.1f,0.35f,0.1f,1.0f };
GLfloat robotBody_mat_shininess[] = { 20.0F };

void drawBot()
{
	glPushMatrix();
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, robotBody_mat_ambient);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, robotBody_mat_specular);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, robotBody_mat_diffuse);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, robotBody_mat_shininess);

	//Delete bot after it is shot or reaches end
	if (count < 3 and curveIndex < 64) {
		glPushMatrix();
			glColor3f(0.2, 0.2, 0.2);
			//Follows the path
			glTranslatef(moveX, 0, moveZ);
			//Moves to first point
			glTranslatef(circles[0].circleCenter.x, 0, -2 * circles[0].circleCenter.y);
			//Rotates to face forward
			glRotatef(angle, 0, 1, 0);
			//Spin animation after bot is hit
			if (hit == true) {
				glRotatef(25 * curveIndex, 0, 0, 1);
				count += 1;
			}
			//Draw robot minus head
			drawRobot();

			//Rotate head to follow cannon
			glRotatef(gunAngle - angle, 0, 1, 0);

			glEnable(GL_TEXTURE_GEN_S);
			glEnable(GL_TEXTURE_GEN_T);
			glBindTexture(GL_TEXTURE_2D, tex[1]);

			gluQuadricDrawStyle(gluNewQuadric(), GLU_FILL);
			glPolygonMode(GL_FRONT, GL_FILL);
			gluQuadricNormals(gluNewQuadric(), GLU_SMOOTH);
			drawHead();
			glDisable(GL_TEXTURE_GEN_S);
			glDisable(GL_TEXTURE_GEN_T);

		glPopMatrix();
	}

	//Robot 2
	if (count2 < 3 and curveIndex < 64) {
		glPushMatrix();
		glColor3f(0.2, 0.2, 0.2);
		//Follows the path
		glTranslatef(moveX, 0, moveZ);
		//Moves to first point
		glTranslatef(circles[0].circleCenter.x + 3, 0, -2 * circles[0].circleCenter.y);
		//Rotates to face forward
		glRotatef(angle, 0, 1, 0);
		//Spin animation
		if (hit2 == true) {
			glRotatef(25 * curveIndex, 0, 0, 1);
			count2 += 1;
		}
		drawRobot();

		//Rotate head to follow cannon
		glRotatef(gunAngle2 - angle, 0, 1, 0);

		glEnable(GL_TEXTURE_GEN_S);
		glEnable(GL_TEXTURE_GEN_T);
		glBindTexture(GL_TEXTURE_2D, tex[1]);

		gluQuadricDrawStyle(gluNewQuadric(), GLU_FILL);
		glPolygonMode(GL_FRONT, GL_FILL);
		gluQuadricNormals(gluNewQuadric(), GLU_SMOOTH);
		drawHead();
		glDisable(GL_TEXTURE_GEN_S);
		glDisable(GL_TEXTURE_GEN_T);

		glPopMatrix();
	}
	glPopMatrix();
}


void drawRobot()
{
	glEnable(GL_TEXTURE_GEN_S);
	glEnable(GL_TEXTURE_GEN_T);
	glBindTexture(GL_TEXTURE_2D, tex[1]);
	gluQuadricDrawStyle(gluNewQuadric(), GLU_FILL);
	glPolygonMode(GL_FRONT, GL_FILL);
	gluQuadricNormals(gluNewQuadric(), GLU_SMOOTH);
	drawBody();
	drawLowerBody();
	glDisable(GL_TEXTURE_GEN_S);
	glDisable(GL_TEXTURE_GEN_T);
}

// Main body of robot
void drawBody()
{
	glPushMatrix();
		glTranslatef(0, 3.5 * robotBodyLength, 0);
		glPushMatrix();
			glScalef(robotBodyWidth, robotBodyLength, robotBodyDepth);
			glRotatef(90, 1, 0, 0);
			gluDisk(gluNewQuadric(), 0, 0.5, 20, 10);
			gluCylinder(gluNewQuadric(), 0.5, 0.5, 1, 20, 10);
		glPopMatrix();
		// Robot neck
		glPushMatrix();
			glTranslatef(0, robotBodyLength, 0);
			glScalef(robotBodyWidth / 5, robotBodyLength * 3, robotBodyDepth / 2);
			glRotatef(90, 1, 0, 0);
			gluCylinder(gluNewQuadric(), 0.5, 0.5, 1, 20, 10);
		glPopMatrix();
		glPushMatrix();
			glTranslatef(robotBodyWidth / 2, -robotBodyWidth / 2.8, 0);
			// Wheel axle
			glPushMatrix();
				glTranslatef(-0.2, -robotBodyWidth * 0.25, 0);
				glRotatef(90, 0, 1, 0);
				gluDisk(gluNewQuadric(), 0, 1.3 / 12, 20, 10);
				gluCylinder(gluNewQuadric(), 1.3 / 12, 1.3 / 12, 1.3 / 6, 20, 10);
			glPopMatrix();
			//Connecting piece between body and axle
			glScalef(robotBodyDepth / 4, robotBodyWidth * 0.4, robotBodyDepth / 4);
			glTranslatef(0, 0.8, 0);
			glRotatef(90, 1, 0, 0);
			gluDisk(gluNewQuadric(), 0, 0.5, 20, 10);
			gluCylinder(gluNewQuadric(), 0.5, 0.5, 1.5, 20, 10);
		glPopMatrix();
	glPopMatrix();
}

// Build Head
void drawHead()
{
	glPushMatrix();
		glTranslatef(0, robotBodyLength * 6, -headLength / 3);
		//Bullets from robot
		glPushMatrix();
			//Non working code to prevent tracking bullets from robots
			//if (proj2 == 0) {
			//	oldangle = gunAngle;							
			//}
			//glRotatef(-oldangle + gunAngle, 0, 1, 0);
			glTranslatef(0, 0, proj2);
			gluSphere(gluNewQuadric(), 0.10, 20, 10);
		glPopMatrix();
		//Building robot head
		gluDisk(gluNewQuadric(), 0, headRadius, 20, 10);
		glPushMatrix();
			glTranslatef(0, 0, headLength);
			gluDisk(gluNewQuadric(), 0, 0.1, 20, 10);
		glPopMatrix();
		gluCylinder(gluNewQuadric(), headRadius, 0.1, headLength, 20, 10);
	glPopMatrix();
}

void drawLowerBody()
{
	glPushMatrix();
		glScalef(robotBodyWidth / 3, robotBodyLength * 1.5, robotBodyLength * 1.5);
		glRotatef(90, 0, 1, 0);
		glTranslatef(0, -robotBodyLength * 3.5, 0);
		glPushMatrix();
			glTranslatef(0, 0, -1);
			gluCylinder(gluNewQuadric(), 1, 2.3, 1, 20, 10);
		glPopMatrix();
		gluCylinder(gluNewQuadric(), 2.3, 1, 1, 20, 10);
		glPushMatrix();
			glTranslatef(0, 0, 1);
			gluDisk(gluNewQuadric(), 0, 1, 32, 1);
		glPopMatrix();
		glTranslatef(0, 0, -robotBodyLength * 0.7);
		glTranslatef(0, 0, -0.95);
		glRotatef(180, 0, 1, 0);
		gluDisk(gluNewQuadric(), 0, 1, 32, 1);
	glPopMatrix();
}

void drawCannon()
{
	glMaterialfv(GL_FRONT, GL_AMBIENT, robotBody_mat_ambient);
	glMaterialfv(GL_FRONT, GL_SPECULAR, robotBody_mat_specular);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, robotBody_mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SHININESS, robotBody_mat_shininess);

	glEnable(GL_TEXTURE_GEN_S);
	glEnable(GL_TEXTURE_GEN_T);
	glBindTexture(GL_TEXTURE_2D, tex[1]);
	gluQuadricDrawStyle(gluNewQuadric(), GLU_FILL);
	glPolygonMode(GL_FRONT, GL_FILL);
	gluQuadricNormals(gluNewQuadric(), GLU_SMOOTH);

	glColor3f(0.2, 0.2, 0.2);

	//control moves the cannon left or right
	glTranslatef(control, 0, 12);
	glRotatef(-90, 1, 0, 0);
	glPushMatrix();
		glTranslatef(0, 0, 0.7);
		//Head
		gluSphere(gluNewQuadric(), 0.25, 20, 10);
		//Animated the gun upwards when cannon is hit
		if (cannonHit) {
			glRotatef(70, 1, 0, 0);
		}
		glRotatef(-90, 1, 0, 0);
		//Gun
		gluCylinder(gluNewQuadric(), 0.05, 0.05, 0.8, 20, 10);
		if (cannonHit == false) {
			//Animates bullets forward
			if (proj == 0) {
				bulletX = control;
			}
			else {
				glTranslatef(-control + bulletX, 0, 0);
			}
			glTranslatef(0, 0, proj);
			//Bullet
			glPushMatrix();
				glColor3f(1,0,0);
				gluDisk(gluNewQuadric(), 0, 0.05, 20, 10);
				gluCylinder(gluNewQuadric(), 0.05, 0.05, 0.8, 20, 10);
			glPopMatrix();
		}
	glPopMatrix();
	//Base of the cannon
	glColor3f(0.2,0.2,0.2);
	gluCylinder(gluNewQuadric(), 0.4, 0.2, 0.8, 20, 10);

	glDisable(GL_TEXTURE_GEN_S);
	glDisable(GL_TEXTURE_GEN_T);
}


void drawGround() {
	glPushMatrix();
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, groundMat_ambient);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, groundMat_specular);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, groundMat_diffuse);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, groundMat_shininess);
	glBegin(GL_QUADS);

	glColor3f(1, 1, 1);

	glNormal3f(0, 1, 0);
	glVertex3f(-12.0f, -1.0f, -12.0f);
	glVertex3f(-12.0f, -1.0f, 12.0f);
	glVertex3f(12.0f, -1.0f, 12.0f);
	glVertex3f(12.0f, -1.0f, -12.0f);
	glEnd();
	glPopMatrix();
}

void mouseButtonHandler3D(int button, int state, int x, int y)
{

	currentButton = button;
	lastMouseX = x;
	lastMouseY = y;
	switch (button)
	{
	case GLUT_LEFT_BUTTON:

		break;
	case GLUT_RIGHT_BUTTON:

		break;
	case GLUT_MIDDLE_BUTTON:

		break;
	default:
		break;
	}
}

void mouseScrollWheelHandler3D(int button, int dir, int xMouse, int yMouse)
{

}

void mouseMotionHandler3D(int x, int y)
{
	int dx = x - lastMouseX;
	int dy = y - lastMouseY;
	if (currentButton == GLUT_LEFT_BUTTON) {
		;
	}
	if (currentButton == GLUT_RIGHT_BUTTON)
	{
		//Zoom in and out
		eyeY -= 0.05 * dy;
		eyeZ += 0.25 * dy;
	}
	else if (currentButton == GLUT_MIDDLE_BUTTON)
	{
	}
	lastMouseX = x;
	lastMouseY = y;
	glutPostRedisplay();
}



// Some Utility Functions

Vector3D crossProduct(Vector3D a, Vector3D b) {
	Vector3D cross;

	cross.x = a.y * b.z - b.y * a.z;
	cross.y = a.x * b.z - b.x * a.z;
	cross.z = a.x * b.y - b.x * a.y;

	return cross;
}

Vector3D normalize(Vector3D a) {
	GLdouble norm = sqrt(a.x * a.x + a.y * a.y + a.z * a.z);
	Vector3D normalized;
	normalized.x = a.x / norm;
	normalized.y = a.y / norm;
	normalized.z = a.z / norm;
	return normalized;
}