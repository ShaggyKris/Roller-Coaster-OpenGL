/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <GL/glut.h>
#include <FTGL/ftgl.h>

#ifndef M_PI
#    define M_PI 3.14159265358979323846
#endif

#define RAD2DEG 180.0/M_PI
#define DEG2RAD M_PI/180.0
 

static void myDisplay(void);
static void myTimer(int value);
static void myKey(unsigned char key, int x, int y);
static void keyPress(int key, int x, int y);
static void keyRelease(int key, int x, int y);
static void myReshape(int w, int h);

static void init(void);

static void drawText(double x, double y, char *text);
static void drawBox();

int xMax, yMax;
static float rotate, cubeRotate=0;

int list;

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
    
/*
    glEnable(GL_POINT_SMOOTH);
    glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
    
    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
*/
    rotate = 0;
}

void myDisplay(){
   
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    
/*
    glMatrixMode(GL_MODELVIEW);
       
*/  
    glLoadIdentity();
    gluLookAt(10*sin(rotate*10), 3, 10*cos(rotate*10), 
            0, 0, 0, 
            0, 1, 0);
    
    glTranslatef(0, 0, -5);
    glPushMatrix();  
        
        glRotatef(cubeRotate++, 1, -1, 1);    
        drawBox();
        
        glPushMatrix();
            glTranslatef(-1,-2, -3);
            glScalef(0.5,0.5,0.5);
            glRotatef((cubeRotate*2), -1, 1, 1);
            drawBox();
        
        glPopMatrix();
    glPopMatrix();
    
    
        
        
    
    
    
    
    glCallList(list);
    glutSwapBuffers();  
    
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