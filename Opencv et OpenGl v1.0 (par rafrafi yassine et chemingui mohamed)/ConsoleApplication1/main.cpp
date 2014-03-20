#ifdef WIN32
#include <windows.h>
#endif
#include <stdio.h>
#include <gl/glew.h>
#include <gl/glut.h>
#include "opencv/highgui.h"


#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

using namespace std;
using namespace cv;
//-----------------------------------------------------------------------------
// prototipes des functions

void initGlut(int argc, char **argv);
void displayFunc(void);
void idleFunc(void);
void reshapeFunc(int width, int height);
void mouseFunc(int button, int state, int x, int y);
void mouseMotionFunc(int x, int y);
void keyboardFunc(unsigned char key, int x, int y);
void specialFunc(int key, int x, int y);
void countFrames(void);
void renderBitmapString(float x, float y, float z, void *font, char *string);

//-----------------------------------------------------------------------------

bool bFullsreen = false;
int nWindowID;

//-----------------------------------------------------------------------------

// parameters du frame conuter
char pixelstring[30];
int cframe = 0;
int time = 0;
int timebase = 0;

//-----------------------------------------------------------------------------

//  variables OpenCV

CvCapture *cvCapture = 0;

GLuint cameraImageTextureID;

//-----------------------------------------------------------------------------

bool bInit = false;

//-----------------------------------------------------------------------------



/** Global variables */
String face_cascade_name = "haarcascade_frontalface_alt.xml";
//String eyes_cascade_name = "haarcascade_eye_tree_eyeglasses.xml";
CascadeClassifier face_cascade;
//CascadeClassifier eyes_cascade;

/** @function detectAndDisplay */
void detectAndDisplay(Mat frame)
{

	std::vector<Rect> faces;
	Mat frame_gray;

	cvtColor(frame, frame_gray, CV_BGR2GRAY);
	equalizeHist(frame_gray, frame_gray);

	//-- Detect faces
	face_cascade.detectMultiScale(frame_gray, faces, 1.1, 2, 0 | CV_HAAR_SCALE_IMAGE, Size(30, 30));

	//for (size_t i = 0; i < faces.size(); i++)
	//{
	if (faces.size() > 0)
	{
		Point center(
			faces[0].x + faces[0].width*0.5, 
			faces[0].y + faces[0].height*0.5
			);
		ellipse(
			frame, center, 
			Size(faces[0].width*0.5, faces[0].height*0.5), 
			0, 0, 360, 
			Scalar(255, 0, 255), 
			4, 8, 0
			);
	}
		//Mat faceROI = frame_gray(faces[i]);
		//std::vector<Rect> eyes;

		//-- In each face, detect eyes
		//eyes_cascade.detectMultiScale(faceROI, eyes, 1.1, 2, 0 | CV_HAAR_SCALE_IMAGE, Size(30, 30));

		/*for (size_t j = 0; j < eyes.size(); j++)
		{
			Point center(faces[i].x + eyes[j].x + eyes[j].width*0.5, faces[i].y + eyes[j].y + eyes[j].height*0.5);
			int radius = cvRound((eyes[j].width + eyes[j].height)*0.25);
			circle(frame, center, radius, Scalar(255, 0, 0), 4, 8, 0);
		}*/
	//}
	//-- Show what you got
	//imshow(window_name, frame);
}


void displayFunc(void) {

	if (!bInit) {

		// initialisation de la camera sur le bus
		cvCapture = cvCreateCameraCapture(0);

		// initialisation de la texture	OpenGL	
		glEnable(GL_TEXTURE_RECTANGLE_ARB);

		glGenTextures(1, &cameraImageTextureID);
		glBindTexture(GL_TEXTURE_RECTANGLE_ARB, cameraImageTextureID);

		glTexParameterf(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameterf(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameterf(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

		bInit = true;
	}

	IplImage* newImage = cvQueryFrame(cvCapture);
	
	detectAndDisplay(newImage);
	if ((newImage->width > 0) && (newImage->height > 0)) {

		// preparation des buffers
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//glDisable(GL_DEPTH_TEST);
		//glDisable(GL_LIGHTING);
		glEnable(GL_TEXTURE_RECTANGLE_ARB);

		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		//gluOrtho2D(0.0,(GLdouble)newImage->width,0.0,(GLdouble)newImage->height);	
		// projection perspective et positionement de la camera
		gluPerspective(90.0f, (float)newImage->width / newImage->height, 1, 1024);
		gluLookAt(
			0, 0, 240.0,
			0, 0, 0.0,
			0.0, 1.0, 0.0);



		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		glBindTexture(GL_TEXTURE_RECTANGLE_ARB, cameraImageTextureID);

		// creation d'un plan et application des images capturé sur le plan comme etant une texture
		glPushMatrix();
		if (newImage->nChannels == 3)
			glTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, GL_RGB, newImage->width, newImage->height, 0, GL_BGR, GL_UNSIGNED_BYTE, newImage->imageData);
		else if (newImage->nChannels == 4)
			glTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, GL_RGBA, newImage->width, newImage->height, 0, GL_BGRA, GL_UNSIGNED_BYTE, newImage->imageData);
		//glTranslatef(10.0, 10.0, 0);
		glBegin(GL_QUADS);
		glTexCoord2i(0, newImage->height);
		glVertex2i(-newImage->width/2, -newImage->height/2);

		glTexCoord2i(newImage->width, newImage->height);
		glVertex2i(newImage->width / 2, -newImage->height / 2);

		glTexCoord2i(newImage->width, 0);
		glVertex2i(newImage->width/2, newImage->height/2);

		glTexCoord2i(0, 0);
		glVertex2i(-newImage->width / 2, newImage->height / 2);
		glEnd();

		glPopMatrix();



	}

	glDisable(GL_TEXTURE_RECTANGLE_ARB);
	glLoadIdentity();

	glPushMatrix();
	GLfloat materialColor[] = { 1.0f, 0.0f, 1.0f, 1.0f };
	glMaterialfv(GL_FRONT, GL_DIFFUSE, materialColor);
	glTranslatef(0, 0, 100);
	glRotatef(50, 1, 1, 0);  

	glutSolidCube(50);
	glPopMatrix();
	countFrames();

	glutSwapBuffers();
}

//-----------------------------------------------------------------------------
void myinit(void) {
	// preparation de la scene pour etre prete a acceuillr des objet en 3d avec tout les proprietes lumiere ...
	glShadeModel(GL_SMOOTH);
	glDepthFunc(GL_LESS);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_AUTO_NORMAL);
	glEnable(GL_NORMALIZE);
}
void initGlut(int argc, char **argv) {

	// initialisation de la fenetre grace a glut
	glutInit(&argc, argv);
	glutInitWindowSize(640, 480);
	glutInitWindowPosition(100, 100);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
	nWindowID = glutCreateWindow("Glut et Opencv");
	myinit();
	// lancer les fonctions callbacks:
	glutDisplayFunc(displayFunc);
	glutReshapeFunc(reshapeFunc);
	glutKeyboardFunc(keyboardFunc);
	glutSpecialFunc(specialFunc);
	glutMouseFunc(mouseFunc);
	glutMotionFunc(mouseMotionFunc);
	glutIdleFunc(idleFunc);
}



//-----------------------------------------------------------------------------

void idleFunc(void) {
	glutPostRedisplay();
}

//-----------------------------------------------------------------------------

void reshapeFunc(int width, int height) {
	glViewport(0, 0, width, height);
}

//-----------------------------------------------------------------------------


void mouseFunc(int button, int state, int x, int y) {

}

//-----------------------------------------------------------------------------

void mouseMotionFunc(int x, int y) {

}

//-----------------------------------------------------------------------------

void keyboardFunc(unsigned char key, int x, int y) {

	switch (key) {

		// -----------------------------------------

#ifdef WIN32
		// exit on escape
	case '\033':

		if (bInit) {
			glDeleteTextures(1, &cameraImageTextureID);
			cvReleaseCapture(&cvCapture);
		}
		exit(0);
		break;
#endif

		// -----------------------------------------

		// switch to fullscreen
	case 'f':

		bFullsreen = !bFullsreen;
		if (bFullsreen)
			glutFullScreen();
		else {
			glutSetWindow(nWindowID);
			glutPositionWindow(100, 100);
			glutReshapeWindow(640, 480);
		}
		break;

		// -----------------------------------------
	}
}

//-----------------------------------------------------------------------------

void specialFunc(int key, int x, int y) {
	//printf("key pressed: %d\n", key);
}

//-----------------------------------------------------------------------------

void countFrames(void)  {

	time = glutGet(GLUT_ELAPSED_TIME);
	cframe++;
	if (time - timebase > 50) {
		sprintf_s(pixelstring, "fps: %4.2f", cframe*1000.0 / (time - timebase));
		timebase = time;
		cframe = 0;
		// Draw status text and uni-logo:
	}
	//glDisable(GL_LIGHTING);
	glColor4f(1.0, 1.0, 1.0, 1.0);
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(0, 200, 0, 200);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	// render the string
	renderBitmapString(5, 5, 0.0, GLUT_BITMAP_HELVETICA_10, pixelstring);

	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
}

//-----------------------------------------------------------------------------

void renderBitmapString(float x, float y, float z, void *font, char *string) {
	char *c;
	glRasterPos3f(x, y, z);
	for (c = string; *c != '\0'; c++) {
		glutBitmapCharacter(font, *c);
	}
}

//-----------------------------------------------------------------------------


void main(int argc, char **argv) {

	if (!face_cascade.load(face_cascade_name)){ printf("--(!)Error loading\n"); };

	initGlut(argc, argv);

	glutMainLoop();
	
	
}



