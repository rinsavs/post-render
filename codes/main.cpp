#include <iostream>
#include "GL\glew.h"
#include "GL/glut.h"
#include "../include/opencv2/opencv.hpp"

#include "Robot.h"

using namespace std;
using namespace glm;

Robot robot;
mat4 cam, projection;
int totalVertSize = 0, totalColorSize = 0;

mat4 mvp, mv;

GLuint matrixID, matrixMV, shadingState;
GLint fragcolor, eyepos;
vec3 campos, camup;

//bg tex
GLuint backgroundTex;

int shadingType = 0;
int actionType = 0;

void idle()
{
	glutPostRedisplay();
}

void display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0, 1, 0, 1);

	/*glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, backgroundTex);

	glBegin(GL_QUADS);
	glTexCoord2f(0, 0); glVertex3f(1, 1, 0);
	glTexCoord2f(0, 1); glVertex3f(1, -1, 0);
	glTexCoord2f(1, 1); glVertex3f(-1, -1, 0);
	glTexCoord2f(1, 0); glVertex3f(-1, 1, 0);
	glEnd();

	glDisable(GL_TEXTURE_2D);*/

	
	robot.render(projection, cam, shadingType, actionType);


	glutSwapBuffers();
}

void menuHandle(int value) {

}

void actionHandle(int value) {
	actionType = value;
}

void shaderHandle(int value) {
	shadingType = value;
	robot.resetTime();
}

void createMenu() {
	int actionMenu, shaderMenu, mainMenu;

	actionMenu = glutCreateMenu(actionHandle);
	glutSetMenu(actionMenu);
	glutAddMenuEntry("Idle", 0);
	glutAddMenuEntry("Walk", 1);

	shaderMenu = glutCreateMenu(shaderHandle);
	glutSetMenu(shaderMenu);
	glutAddMenuEntry("Phong shading", 0);
	glutAddMenuEntry("Toon shading", 1);
	glutAddMenuEntry("Blur", 2);
	glutAddMenuEntry("Fish eye", 3);
	glutAddMenuEntry("Pixelate", 4);
	glutAddMenuEntry("Circle", 5);
	glutAddMenuEntry("Comb Marbling", 6);
	glutAddMenuEntry("Circular Swirl", 7);
	glutAddMenuEntry("Vortex Swirl", 8);
	glutAddMenuEntry("Sobel - Edge Darkening", 9);
	glutAddMenuEntry("Ripple", 10);
	glutAddMenuEntry("aa", 11);

	mainMenu = glutCreateMenu(menuHandle);
	glutSetMenu(mainMenu);
	glutAddSubMenu("Action", actionMenu);
	glutAddSubMenu("Shader", shaderMenu);

	//bind menu to right mouse button
	glutAttachMenu(GLUT_RIGHT_BUTTON);
}

void init()
{
	robot.load();

	projection = perspective(60.0f, 1.0f / 1.0f, 0.1f, 1000.0f);
	campos = vec3(0, 0, 50);
	camup = vec3(0, 1, 0);
	cam = lookAt(campos, vec3(0, 0, -10), camup);

	//bg stuff
	/*IplImage* bgTexture = cvLoadImage("./bg.jpg", 1);
	glGenTextures(1, &backgroundTex);
	glBindTexture(GL_TEXTURE_2D, backgroundTex);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, bgTexture->width, bgTexture->height, 0, GL_BGR, GL_UNSIGNED_BYTE, (GLvoid*)bgTexture->imageData);*/
}

void keyboard_event(unsigned char key, int xmouse, int ymouse)
{

}

void mouse_event(int button, int state, int x, int y)
{
	//int mouseLeft = GLUT_LEFT_BUTTON;
	//int mouseRight = GLUT_RIGHT_BUTTON;
	//int mouseUp = GLUT_UP;
	//int mouseDown = GLUT_DOWN;
	//if (button == mouseLeft && state == mouseDown) {
	//	mouse = true;
	//}
	//if (button == mouseLeft && state == mouseUp) {
	//	mouse = false;
	//}
	//glutPostRedisplay();
}

void mouse_motion(int x, int y) {
	//if (mouse) {
	//	int local_x, local_y, delta_x, delta_y;
	//	float cosine = cos(0.1), sine = sin(0.1);
	//
	//	local_x = x - 250;			local_y = 250 - y;
	//	delta_x = local_x - cur_x;	delta_y = local_y - cur_y;
	//	cur_x = local_x;			cur_y = local_y;
	//
	//	if (delta_x < 0) {
	//		//rotate y - left
	//		campos = vec3(campos.x * cosine + campos.z * sine,
	//			campos.y,
	//			campos.x * -sine + campos.z * cosine);
	//		camup = vec3(camup.x * cosine + camup.z * sine,
	//			camup.y,
	//			camup.x * -sine + camup.z * cosine);
	//	}
	//	else if (delta_x > 0) {
	//		//rotate y - right
	//		campos = vec3(campos.x * cosine - campos.z * sine,
	//			campos.y,
	//			campos.x * sine + campos.z * cosine);
	//		camup = vec3(camup.x * cosine - camup.z * sine,
	//			camup.y,
	//			camup.x * sine + camup.z * cosine);
	//	}
	//
	//	if (delta_y > 0) {
	//		//rotate x 
	//		campos = vec3(campos.x,
	//			campos.y * cosine + campos.z * -sine,
	//			campos.y * sine + campos.z * cosine);
	//		camup = vec3(camup.x,
	//			camup.y * cosine + camup.z * -sine,
	//			camup.y * sine + camup.z * cosine);
	//	}
	//	else if (delta_y < 0) {
	//		//rotate x
	//		campos = vec3(campos.x,
	//			campos.y * cosine + campos.z * sine,
	//			campos.y * -sine + campos.z * cosine);
	//		camup = vec3(camup.x,
	//			camup.y * cosine + camup.z * sine,
	//			camup.y * -sine + camup.z * cosine);
	//	}
	//}
}

int main(int argc, char **argv)
{
	//init Glut
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(500, 500);
	glutInitWindowPosition(50, 50);
	glutCreateWindow("Project 3");
	glutDisplayFunc(display);
	glutIdleFunc(idle);
	glutKeyboardFunc(keyboard_event);
	glutMouseFunc(mouse_event);
	glutMotionFunc(mouse_motion);

	GLenum err = glewInit();
	if (err != GLEW_OK) {
		cout << "Error: " << glewGetErrorString(err) << endl;
		return 1;
	}

	//init
	init();
	//menu
	createMenu();
	glutMainLoop();
	return 0;
}