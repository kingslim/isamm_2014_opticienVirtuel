
#ifdef WIN32
#include <windows.h>
#endif
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <string.h>
#include <gl/glew.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <vector>
#include <cmath>

#include "opencv/highgui.h"


#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

using namespace std;
using namespace cv;

/***************************************************************************
OBJ Loading
***************************************************************************/

class Model_OBJ
{
public:
	Model_OBJ();
	float* Model_OBJ::calculateNormal(float* coord1, float* coord2, float* coord3);
	int Model_OBJ::Load(char *filename);	// Loads the model
	void Model_OBJ::Draw();					// Draws the model on the screen
	void Model_OBJ::Release();				// Release the model

	float* normals;							// Stores the normals
	float* Faces_Triangles;					// Stores the triangles
	float* vertexBuffer;					// Stores the points which make the object
	long TotalConnectedPoints;				// Stores the total number of connected verteces
	long TotalConnectedTriangles;			// Stores the total number of connected triangles

};


#define POINTS_PER_VERTEX 3
#define TOTAL_FLOATS_IN_TRIANGLE 9
using namespace std;

Model_OBJ::Model_OBJ()
{
	this->TotalConnectedTriangles = 0;
	this->TotalConnectedPoints = 0;
}

float* Model_OBJ::calculateNormal(float *coord1, float *coord2, float *coord3)
{
	/* calculate Vector1 and Vector2 */
	float va[3], vb[3], vr[3], val;
	va[0] = coord1[0] - coord2[0];
	va[1] = coord1[1] - coord2[1];
	va[2] = coord1[2] - coord2[2];

	vb[0] = coord1[0] - coord3[0];
	vb[1] = coord1[1] - coord3[1];
	vb[2] = coord1[2] - coord3[2];

	/* cross product */
	vr[0] = va[1] * vb[2] - vb[1] * va[2];
	vr[1] = vb[0] * va[2] - va[0] * vb[2];
	vr[2] = va[0] * vb[1] - vb[0] * va[1];

	/* normalization factor */
	val = sqrt(vr[0] * vr[0] + vr[1] * vr[1] + vr[2] * vr[2]);

	float norm[3];
	norm[0] = vr[0] / val;
	norm[1] = vr[1] / val;
	norm[2] = vr[2] / val;


	return norm;
}


int Model_OBJ::Load(char* filename)
{
	string line;
	ifstream objFile(filename);
	if (objFile.is_open())													// If obj file is open, continue
	{
		objFile.seekg(0, ios::end);										// Go to end of the file, 
		long fileSize = objFile.tellg();									// get file size
		objFile.seekg(0, ios::beg);										// we'll use this to register memory for our 3d model

		vertexBuffer = (float*)malloc(fileSize);							// Allocate memory for the verteces
		Faces_Triangles = (float*)malloc(fileSize*sizeof(float));			// Allocate memory for the triangles
		normals = (float*)malloc(fileSize*sizeof(float));					// Allocate memory for the normals

		int triangle_index = 0;												// Set triangle index to zero
		int normal_index = 0;												// Set normal index to zero

		while (!objFile.eof())											// Start reading file data
		{
			getline(objFile, line);											// Get line from file

			if (line.c_str()[0] == 'v')										// The first character is a v: on this line is a vertex stored.
			{
				line[0] = ' ';												// Set first character to 0. This will allow us to use sscanf

				sscanf_s(line.c_str(), "%f %f %f ",							// Read floats from the line: v X Y Z
					&vertexBuffer[TotalConnectedPoints],
					&vertexBuffer[TotalConnectedPoints + 1],
					&vertexBuffer[TotalConnectedPoints + 2]);

				TotalConnectedPoints += POINTS_PER_VERTEX;					// Add 3 to the total connected points
			}
			if (line.c_str()[0] == 'f')										// The first character is an 'f': on this line is a point stored
			{
				line[0] = ' ';												// Set first character to 0. This will allow us to use sscanf

				int vertexNumber[4] = { 0, 0, 0 };
				sscanf_s(line.c_str(), "%i%i%i",								// Read integers from the line:  f 1 2 3
					&vertexNumber[0],										// First point of our triangle. This is an 
					&vertexNumber[1],										// pointer to our vertexBuffer list
					&vertexNumber[2]);										// each point represents an X,Y,Z.

				vertexNumber[0] -= 1;										// OBJ file starts counting from 1
				vertexNumber[1] -= 1;										// OBJ file starts counting from 1
				vertexNumber[2] -= 1;										// OBJ file starts counting from 1


				/********************************************************************
				* Create triangles (f 1 2 3) from points: (v X Y Z) (v X Y Z) (v X Y Z).
				* The vertexBuffer contains all verteces
				* The triangles will be created using the verteces we read previously
				*/

				int tCounter = 0;
				for (int i = 0; i < POINTS_PER_VERTEX; i++)
				{
					Faces_Triangles[triangle_index + tCounter] = vertexBuffer[3 * vertexNumber[i]];
					Faces_Triangles[triangle_index + tCounter + 1] = vertexBuffer[3 * vertexNumber[i] + 1];
					Faces_Triangles[triangle_index + tCounter + 2] = vertexBuffer[3 * vertexNumber[i] + 2];
					tCounter += POINTS_PER_VERTEX;
				}

				/*********************************************************************
				* Calculate all normals, used for lighting
				*/
				float coord1[3] = { Faces_Triangles[triangle_index], Faces_Triangles[triangle_index + 1], Faces_Triangles[triangle_index + 2] };
				float coord2[3] = { Faces_Triangles[triangle_index + 3], Faces_Triangles[triangle_index + 4], Faces_Triangles[triangle_index + 5] };
				float coord3[3] = { Faces_Triangles[triangle_index + 6], Faces_Triangles[triangle_index + 7], Faces_Triangles[triangle_index + 8] };
				float *norm = this->calculateNormal(coord1, coord2, coord3);

				tCounter = 0;
				for (int i = 0; i < POINTS_PER_VERTEX; i++)
				{
					normals[normal_index + tCounter] = norm[0];
					normals[normal_index + tCounter + 1] = norm[1];
					normals[normal_index + tCounter + 2] = norm[2];
					tCounter += POINTS_PER_VERTEX;
				}

				triangle_index += TOTAL_FLOATS_IN_TRIANGLE;
				normal_index += TOTAL_FLOATS_IN_TRIANGLE;
				TotalConnectedTriangles += TOTAL_FLOATS_IN_TRIANGLE;
			}
		}
		objFile.close();														// Close OBJ file
	}
	else
	{
		cout << "Unable to open file";
	}
	return 0;
}

void Model_OBJ::Release()
{
	free(this->Faces_Triangles);
	free(this->normals);
	free(this->vertexBuffer);
}

void Model_OBJ::Draw()
{
	glEnableClientState(GL_VERTEX_ARRAY);						// Enable vertex arrays
	glEnableClientState(GL_NORMAL_ARRAY);						// Enable normal arrays
	glVertexPointer(3, GL_FLOAT, 0, Faces_Triangles);				// Vertex Pointer to triangle array
	glNormalPointer(GL_FLOAT, 0, normals);						// Normal pointer to normal array
	glDrawArrays(GL_TRIANGLES, 0, TotalConnectedTriangles);		// Draw the triangles
	glDisableClientState(GL_VERTEX_ARRAY);						// Disable vertex arrays
	glDisableClientState(GL_NORMAL_ARRAY);						// Disable normal arrays
}


Model_OBJ obj;


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

double ox = 0;
double oy = 0;
double oz = 10;
double ratio = 50;
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
	face_cascade.detectMultiScale(frame_gray, faces, 1.1, 2, 0 | CV_HAAR_SCALE_IMAGE, Size(100, 100));

	//for (size_t i = 0; i < faces.size(); i++)
	//{
	if (faces.size() > 0)
	{
		/*Point center(
		faces[0].x + faces[0].width*0.5,
		faces[0].y + faces[0].height*0.5
		);
		ellipse(
		frame, center,
		Size(faces[0].width*0.5, faces[0].height*0.5),
		0, 0, 360,
		Scalar(255, 0, 255),
		4, 8, 0
		);*/
		ratio = (faces[0].width*faces[0].height / 1000) - 30;
		//printf("%f \n", ratio);
		ox = faces[0].x + faces[0].width*0.5 - 315; //300
		oy = -(faces[0].y + faces[0].height*0.5) + 235; //260
		//printf("%f %f \n",ox,oy);
	}
	else{
		ox = 1000;
		oy = 1000;
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
	newImage = cvQueryFrame(cvCapture);

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
		glVertex2i(-newImage->width / 2, -newImage->height / 2);

		glTexCoord2i(newImage->width, newImage->height);
		glVertex2i(newImage->width / 2, -newImage->height / 2);

		glTexCoord2i(newImage->width, 0);
		glVertex2i(newImage->width / 2, newImage->height / 2);

		glTexCoord2i(0, 0);
		glVertex2i(-newImage->width / 2, newImage->height / 2);
		glEnd();

		glPopMatrix();





	}

	glDisable(GL_TEXTURE_RECTANGLE_ARB);
	glLoadIdentity();

	glPushMatrix();
	glTranslatef(ox, oy, oz);
	//glRotatef(50, 1, 1, 0); 
	GLfloat materialColor[] = { 0.0f, 0.0f, 1.0f, 1.0f };
	glMaterialfv(GL_FRONT, GL_DIFFUSE, materialColor);

	//glutSolidCube(ratio);
	glScalef(ratio, ratio, ratio);
	glRotatef(-90.0, 0.0, 1.0, 0.0);
	obj.Draw();

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
	obj.Load("Glasses.obj");
	glutMainLoop();


}