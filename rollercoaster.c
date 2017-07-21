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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <GL/glut.h>
#include <FTGL/ftgl.h>
#include "rollercoaster.h"


#ifndef M_PI
    #define M_PI 3.14159265358979323846
#endif



#define SKY_RAD 10
#define SKY_STEPS 40
#define SKY_HEIGHT 10


#define RAD2DEG 180.0/M_PI
#define DEG2RAD M_PI/180.0

#define NUM_CONTROL_POINTS 5
#define RAIL_WIDTH 0.1


#define sixth 0.16666667163372039794921875

typedef float (*bSpline)(float,float,float,float,float);
int fp;
int xMax, yMax;
static float rotate, cubeRotate=0;
static double distance = 8.0;
int list;
vector3* controlPoints;
float uCam, vCam, maxY;

bSpline uniformBSplineFunctions[] = {&uniformBSpline, &uniformBSplineDerivative, &uniformBSplineSecondDerivative};

vector3 largest, smallest, cameraPos, cameraUp, focalPoint;

int main(int argc, char *argv[]){
    srand((unsigned int) time(NULL));
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
    list = glGenLists(1);
    uCam = 0;
    vCam = 1;
    fp = 0;
    drawCoasterPath();
    
    
}

void myDisplay(){
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);


    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    if(fp){
        gluLookAt(cameraPos.x, cameraPos.y, cameraPos.z,
            focalPoint.x, focalPoint.y, focalPoint.z,
            cameraUp.x, cameraUp.y, cameraUp.z);
    }
    else{
    gluLookAt(distance*sin(rotate*10), 2, distance*cos(rotate*10),
            0, 0, 0,
            0, 1, 0);
    }
    
    //DrawLine();
//    glPushMatrix();
/*
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
*/
//    glPopMatrix();
    
    glCallList(list);
    
    glutSwapBuffers();
    
    
    if(cubeRotate > 360.0) cubeRotate = 0;
}

void myTimer(int value){
    vector3 velocity, s;
    
    q(controlPoints, uCam, 1, &velocity);
    //float mag = vectorMagnitude(&velocity);
    printf("This is uCam then: %f\n", uCam);
    uCam += (vCam * 0.033)/ (sqrt(velocity.x*velocity.x+velocity.y*velocity.y+velocity.z*velocity.z));
    
    if(uCam > NUM_CONTROL_POINTS) uCam -= NUM_CONTROL_POINTS;
    printf("This is uCam now: %f\n", uCam);
    
    q(controlPoints, uCam, 0, &cameraPos);
    q(controlPoints, uCam, 2, &s);
    normalizeVector(&velocity);
    calculateUpVector(&velocity, &s, &cameraUp);
    normalizeVector(&cameraUp);
    
    vCam = sqrt((maxY+maxY*0.25) - cameraPos.y);
    
//    focalPoint = cameraPos;
//    focalPoint.x += velocity.x;
//    focalPoint.y += velocity.y;
//    focalPoint.z += velocity.z;
    
    vectorAdd_Sub(&cameraPos, &cameraUp, 1);
    
     
    
    glutPostRedisplay();
    glutTimerFunc(33, myTimer, value);
    
    rotate+=0.001;
    if(rotate>2*M_PI) rotate -= 2*M_PI;   
}

void myKey(unsigned char key, int x, int y){
    switch(key){
        case ' ':
            fp = !fp;
            break;
    }
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

int findCenterOf3DObject(vector3 smallest, vector3 largest, vector3* middle){    
    //vector3 middle;
    middle->x = -(largest.x+smallest.x)/2.0f;
    middle->y = -(largest.y+smallest.y)/2.0f;
    middle->z = -(largest.z+smallest.z)/2.0f;
    return 0;
}

void drawSkyAndGround(){
    int i;
    float step = (2*M_PI)/SKY_STEPS, x,z;
    glBegin(GL_QUAD_STRIP);
    for(i = 0; i < SKY_STEPS+1; i++){
        x = sin(step*i)*SKY_RAD;
        z = cos(step*i)*SKY_RAD;
        glColor3f(0.1, 0.1, 0.45);
        glVertex3f(x, -SKY_HEIGHT, z);
        glColor3f(0.5, 0.8, 1);      
        glVertex3f(x, SKY_HEIGHT, z);
    }
    glEnd();
    glBegin(GL_TRIANGLE_FAN);
        glColor3f(0.1,1,0.1);
        glVertex3d(0,0,0);
        glColor3f(0.3,0.612,0.5);
        for(i=0; i < SKY_STEPS + 1; i++){
        glVertex3f(sin(step*i)*SKY_RAD, 0, cos(step*i)*SKY_RAD);
        }
    glEnd();
    glPushMatrix();
        glBegin(GL_TRIANGLE_FAN);
            glColor3f(0.5, 0.8, 1);
            glTranslatef(0,SKY_HEIGHT,0);
            glVertex3f(0,SKY_HEIGHT,0);
            
            for(i=0; i < SKY_STEPS + 1; i++){
              glVertex3f(sin(step*i)*SKY_RAD, SKY_HEIGHT, cos(step*i)*SKY_RAD);
            }
        glEnd();
    glPopMatrix();
  
}

void drawCoasterPath(){
    vector3 point, up, u, n, v, s;
    int count=0;
    float step = 0;
    
    up.x = 0;
    up.y = 1;
    up.z = 0;
    
    controlPoints = (vector3*)malloc((NUM_CONTROL_POINTS + 3)*sizeof(vector3));    
    
    glNewList(list, GL_COMPILE);
    drawSkyAndGround();
    controlPoints[0].x = controlPoints[0].y = controlPoints[0].z = 0.0;
    controlPoints[1].x = controlPoints[1].y = 1.0;
    controlPoints[1].z = 0.0;
    
    for(int i = 2; i < NUM_CONTROL_POINTS; i++){
        controlPoints[i].x = (float)(rand()%NUM_CONTROL_POINTS)+1;
        controlPoints[i].y = (float)(rand()%NUM_CONTROL_POINTS)/2;
        controlPoints[i].z = (float)(rand()%NUM_CONTROL_POINTS)+1;
        if(controlPoints[i].x == controlPoints[i].z){
            if(rand()%2 == 0)
                controlPoints[i].x++;
            else
                controlPoints[i].z++;
        }
        if(controlPoints[i].y > maxY) maxY = controlPoints[i].y;
    }
    controlPoints[NUM_CONTROL_POINTS] = controlPoints[0];
    controlPoints[NUM_CONTROL_POINTS + 1] = controlPoints[1];
    controlPoints[NUM_CONTROL_POINTS + 2] = controlPoints[2];   
/*
    for(float i = 0; i < NUM_CONTROL_POINTS; i+=0.1){
        q(controlPoints, i, 0, &point);

        if(smallest.x > point.x) smallest.x = point.x;
        if(smallest.y > point.y) smallest.y = point.y;
        if(smallest.z > point.z) smallest.z = point.z;
        if(largest.x < point.x) largest.x = point.x;
        if(largest.y < point.y) largest.y = point.y;
        if(largest.z < point.z) largest.z = point.z;       
    }
    
    findCenterOf3DObject(smallest, largest, &middle);
    glTranslatef(middle.x, 0, middle.z);    
*/     

    glBegin(GL_LINE_LOOP);
    
  
    for(float i = 0; i < NUM_CONTROL_POINTS; i+=step){
        q(controlPoints, i, 0, &point);
        
        
        if(distance < point.x)
            distance += (point.x+1);
        else if(distance < (point.z+1))
            distance += point.z;
        
        q(controlPoints, i, 1, &n);
        
        
        step = RAIL_WIDTH/(sqrt(n.x*n.x + 
            n.y*n.y + 
            n.z*n.z)
            );
        if(step < 0.01) step = 0.01;
        
        glColor3f(0,0,0);
        glVertex3f(point.x,point.y,point.z); 
        
        q(controlPoints, i, 2, &s);           
                
        negativeVector(&n);        
        normalizeVector(&n);        
        crossProduct(&up, &n, &u);        
        normalizeVector(&u);        
        crossProduct(&n,&u,&v);
        calculateUpVector(&n,&s,&up);
        
//        glPushMatrix();
//            glColor3f(0.5,1,1);
//            //glVertex3f(up.x,up.y,up.z);
//            printf("Up.x: %0.10f\tUp.y: %0.10f\tUp.z: %0.10f\t\n", up.x,up.y,up.z);
//        glPopMatrix();
        
    }
    
    glEnd();
    glEndList();
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

int q(const vector3* list, float u, int derivation, vector3* result){
    float t = u - (int)u;
    //vector3 result;
    for(int i = 0; i < 3; i++){
        result->coord[i] =
                uniformBSplineFunctions[derivation]( list[(int)u].coord[i],
                                                     list[(int)u+1].coord[i],
                                                     list[(int)u+2].coord[i],
                                                     list[(int)u+3].coord[i],
                                                     t );
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

void crossProduct(const vector3* a, const vector3* b, vector3* result){
    result->x = (a->y * b->z) - (a->z * b->y);
    result->y = (a->z * b->x) - (a->x * b->z);
    result->z = (a->x * b->y) - (a->y * b->x);
}

float vectorMagnitude(const vector3* v){
  return sqrt((v->x * v->x) + (v->y * v->y) + (v->z * v->z));
}

void normalizeVector(vector3* v){
    float mag = vectorMagnitude(v);
    scale(&v, 1/mag);
}

void scale(vector3* v, float amount){
    v->x *= amount;
    v->y *= amount;
    v->z *= amount;
}

void calculateUpVector(const vector3* r, const vector3* s, vector3* up){
    vector3 numerator;
    crossProduct(&r,&s,&numerator);
    float mag = vectorMagnitude(r);
    float k = numerator.y/pow(mag, 3);
    k /= 8;
    if(k > M_PI/2)
        k = M_PI/2;
    else if(k < -M_PI/2)
        k = -M_PI/2;
    
    up->x = (1-cos(-k))*r->x*r->y-sin(-k)*r->z;
    up->y = (1-cos(-k))*r->y*r->y+cos(-k);
    up->z = (1-cos(-k))*r->y*r->z+sin(-k)*r->x;
    normalizeVector(up);
}

void negativeVector(vector3* v){
    v->x *= (-1);
    v->y *= (-1);
    v->z *= (-1);
}

void vectorAdd_Sub(vector3* affected, const vector3* effector, int flag){
    affected->x += flag*effector->x;
    affected->y += flag*effector->y;
    affected->z += flag*effector->z;
}