/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   rollercoaster.c
 * Author: kristofer
 * 
 * Created on July 18, 2017, 9:40 PM
 */

#include "rollercoaster.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <GL/glut.h>
#include <FTGL/ftgl.h>

#ifndef M_PI
    #define M_PI 3.14159265358979323846
#endif

#define RAD2DEG 180.0/M_PI
#define DEG2RAD M_PI/180.0

#define MAX_RAIL_SEGMENTS 10
#define sixth 0.16666667163372039794921875

typedef float (*bSpline)(float,float,float,float,float);

int xMax, yMax;
static float rotate, cubeRotate=0;
static double distance = 10.0;
int list;

bSpline uniformBSplineFunctions[] = {&uniformBSpline, &uniformBSplineDerivative, &uniformBSplineSecondDerivative};

int main(int argc, char *argv[]){

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGB|GLUT_DEPTH);
    glutInitWindowSize(500, 500);
    glutCreateWindow("RollerCoaster");

    glutDisplayFunc(myDisplay);
    glutIgnoreKeyRepeat(1);
    glutKeyboardFunc(myKey);
    glutSpecialFunc(keyPress);
    glutSpecialUpFunc(keyRelease);
    glutReshapeFunc(myReshape);
    glutTimerFunc(33, myTimer, 0);

    glClearColor(0.0, 0.0, 0.0, 1.0);

    init();

    glutMainLoop();

}

void init(){

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glEnable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();


    glEnable(GL_POINT_SMOOTH);
    glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);

    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

    rotate = 0;

}

void myDisplay(){
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);


    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(distance*sin(rotate*10), 3, distance*cos(rotate*10),
            0, 0, 0,
            0, 1, 0);


//    glPushMatrix();
    glTranslatef(0, 0, -5);
    glRotatef(RAD2DEG*(cubeRotate+=0.1), 1, -1, 1);
    drawBox();

    glPushMatrix();
        glRotatef(RAD2DEG*cubeRotate, 1, 1, 0);
        glScalef(0.5, 0.5, 0.5);
        glTranslatef(-5.0, -5.0, -5.0);

        glRotatef(RAD2DEG*(cubeRotate/2.0), 1, -1, 1);

        drawBox();

    glPopMatrix();
//    glPopMatrix();

    glCallList(list);
    glutSwapBuffers();
    if(cubeRotate > 360.0) cubeRotate = 0;
}

void myTimer(int value){
    glutPostRedisplay();
    glutTimerFunc(33, myTimer, value);
    rotate+=0.001;
    if(rotate>2*M_PI) rotate -= 2*M_PI;
}

void myKey(unsigned char key, int x, int y){

}

void keyPress(int key, int x, int y){

}

void keyRelease(int key, int x, int y){

}

void myReshape(int w, int h){
   /*
     *	reshape callback function; the upper and lower boundaries of the
     *	window are at 100.0 and 0.0, respectively; the aspect ratio is
     *  determined by the aspect ratio of the viewport
     */

    xMax = 100.0*w/h;
    yMax = 100.0;

    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(90, xMax/yMax, 0.1, 20);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void drawBox(){
    glBegin(GL_QUADS);

    glColor3f(1.0,1.0,1.0);
        glVertex3f(1,1,1);
        glVertex3f(-1,1,1);
        glVertex3f(-1,1,-1);
        glVertex3f(1,1,-1);
    glColor3f(1.0,0,1.0);
        glVertex3f(1,-1,1);
        glVertex3f(-1,-1,1);
        glVertex3f(-1,-1,-1);
        glVertex3f(1,-1,-1);
    glColor3f(0,0,1.0);
        glVertex3f(1,1,1);
        glVertex3f(1,-1,1);
        glVertex3f(1,-1,-1);
        glVertex3f(1,1,-1);
    glColor3f(1.0,0,0);
        glVertex3f(-1,1,1);
        glVertex3f(-1,-1,1);
        glVertex3f(-1,-1,-1);
        glVertex3f(-1,1,-1);
    glColor3f(1.0,1.0,0);
        glVertex3f(1,1,-1);
        glVertex3f(1,-1,-1);
        glVertex3f(-1,-1,-1);
        glVertex3f(-1,1,-1);
    glColor3f(0,1.0,1.0);
        glVertex3f(1,1,1);
        glVertex3f(1,-1,1);
        glVertex3f(-1,-1,1);
        glVertex3f(-1,1,1);
  glEnd();
}

void drawText(double x, double y, char *text){

		//glColor3i(1,1,1);
		glRasterPos2i( x, y);
		for(int i = 0; i < strlen(text); i++){
			glutBitmapCharacter(GLUT_BITMAP_9_BY_15, text[i]);

		}
/*		printf("\nText is %s\n",text);*/
/*		fflush(stdout);		*/

}

int q(const vector3* list, float t, int derivation, vector3* result){
    float u = t - (int)t;
    for(int i = 0; i < 3; i++){
        result->coord[i] =
                uniformBSplineFunctions[derivation](
                                                    list[(int)t-3].coord[i],
                                                    list[(int)t-2].coord[i],
                                                    list[(int)t-1].coord[i],
                                                    list[(int)t].coord[i],
                                                    u
                                                );
    }
    return 0;
}

float uniformBSpline(float p0, float p1, float p2, float p3, float t){
    //p0 = p(i-3), with the other p-vals equivalent up to p3 = p(i)
    float r0,r1,r2,r3;

    r0 = sixth*(t*t*t);
    r1 = sixth*(-3.0*(t*t*t) + 3.0*(t*t) + 3.0*t + 1);
    r2 = sixth*(3.0*(t*t*t) - 6.0*(t*t) + 4);
    r3 = sixth*((1-t)*(1-t)*(1-t));

    return ( r3*p0 + r2*p1 + r1*p2+ r0*p3 );
}

float uniformBSplineDerivative(float p0, float p1, float p2, float p3, float t){
    //p0 = p(i-3), with the other p-vals equivalent up to p3 = p(i)
    float r0,r1,r2,r3;

    r0 = (t*t)/2;
    r1 = 0.5 + t - ((3.0*t*t)/2);
    r2 = t*( ((3*t)/2) - 2);
    r3 = (-0.5)*((1-t)*(1-t));

    return ( r3*p0 + r2*p1 + r1*p2+ r0*p3 );
}

float uniformBSplineSecondDerivative(float p0, float p1, float p2, float p3, float t){
    //p0 = p(i-3), with the other p-vals equivalent up to p3 = p(i)
    float r0,r1,r2,r3;

    r0 = t;
    r1 = 1-3*t;
    r2 = 3*t-2;
    r3 = 1-t;

    return ( r3*p0 + r2*p1 + r1*p2+ r0*p3 );
}