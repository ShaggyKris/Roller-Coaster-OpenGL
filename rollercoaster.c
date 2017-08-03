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

#define SKY_STEPS 40
#define SKY_HEIGHT 80


#define RAD2DEG 180.0/M_PI
#define DEG2RAD M_PI/180.0

#define NUM_CONTROL_POINTS 30
#define RAIL_WIDTH 0.1


#define sixth 0.16666667163372039794921875

typedef float (*bSpline)(float,float,float,float,float);
int fp, addUpVector;
int xMax, yMax;
static float rotate, cubeRotate=0;
static double distance = 8.0;
int list, hill;
vector3* controlPoints;
float uCam, vCam;


bSpline uniformBSplineFunctions[] = {&uniformBSpline, &uniformBSplineDerivative, &uniformBSplineSecondDerivative};

vector3 largest, smallest, cameraPos, cameraUp, focalPoint, max;
//GLUquadric* qobj;

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
    uCam = hill = 0;
    vCam = 1;
    max.x = max.y = max.z = 0;
    fp = addUpVector = 0;
    //qobj = gluNewQuadric();
    
    //gluQuadricNormals(qobj, GLU_SMOOTH);
    
    
    drawCoasterPath();
//    printf("We've started\n");
    
}

void myDisplay(){
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    char speed[64];

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    //Ability to change perspective
    if(fp){
        gluLookAt(cameraPos.x, cameraPos.y, cameraPos.z,
            focalPoint.x, focalPoint.y, focalPoint.z,            
            cameraUp.x, cameraUp.y, cameraUp.z);
        
    }
    else{
    gluLookAt(distance*sin(rotate*10), 2, distance*cos(rotate*10),
            0, 1, 0,
            0, 1, 0);
    }
//    vectorAdd_Sub(&cameraPos, &cameraUp, -1);
//    glPushMatrix();
//    glColor3f(0,0,0);
//        glTranslatef(cameraPos.x, cameraPos.y, cameraPos.z);
//        glScalef(0.1,0.1,0.1);
//        drawBox();
//    glPopMatrix();
    glCallList(list);
    
    //This was a workaround to get text on screen
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
        glLoadIdentity();
        glOrtho(0.0, xMax, yMax, 0.0, -1.0, 10);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();


        glDisable(GL_CULL_FACE);

        glClear(GL_DEPTH_BUFFER_BIT);

        glColor3f(1,1,1);
        drawText(0, 10, "Press SPACE to change perspective.");
        drawText(0, 15, "Press F to add Up vector to Camera Position.");
        sprintf(speed, "Speed: %f", vCam);
        drawText(0, 20, speed);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    glutSwapBuffers();
    
    
    if(cubeRotate > 360.0) cubeRotate = 0;
}
//This is all pretty explanatory due to the names of the variables
void myTimer(int value){
    vector3 velocity, n, s;
    float mag, w, m = 5, g = 9.81, lastY, yVal, qt;
    q(controlPoints, uCam, 0, &cameraPos);
    q(controlPoints, uCam, 1, &velocity);    
    q(controlPoints, uCam, 2, &s);
    
    w = 0.5*(vCam*vCam) + (g*cameraPos.y);
    vCam = sqrt(vCam*vCam);
    
    mag = vectorMagnitude(&velocity);
    
    //printf("This is uCam then: %f\n", uCam);
    uCam += (vCam*0.033)/mag;    
    //Resets uCam so that we don't go out of boundary
    if(uCam > NUM_CONTROL_POINTS) uCam -= NUM_CONTROL_POINTS;
          
    
    //Calculate the movement along the track using acceleration
    w = 0.5*(vCam*vCam*m) + (m*g*(yMax-cameraPos.y));
    vCam = sqrt(2*g*(yMax-cameraPos.y))/50;
//    printf("vCam: %f\n",vCam);
    
    
    calculateUpVector(&velocity, &s, &cameraUp);   
    normalizeVector(&cameraUp);
    
//    if(cameraUp.x > M_PI/2) cameraUp.x = M_PI/2;
//    if(cameraUp.x < -M_PI/2) cameraUp.x = -M_PI/2;
//    if(cameraUp.y > M_PI/2) cameraUp.y = M_PI/2;
//    if(cameraUp.y < -M_PI/2) cameraUp.y = -M_PI/2;
//    if(cameraUp.z > M_PI/2) cameraUp.z = M_PI/2;
//    if(cameraUp.z < -M_PI/2) cameraUp.z = -M_PI/2;
    
    vectorAdd_Sub(&cameraPos, &cameraUp, addUpVector);
    
    focalPoint = cameraPos;
    vectorAdd_Sub(&focalPoint, &velocity, 1); 
    
//    printf("Cam Up.x: %f\tCam up.y: %f\tCam up.z: %f\n",cameraUp.x,cameraUp.y,cameraUp.z);
      
    //cameraPos.y -= 1;
    rotate+=0.001;
    if(rotate>2*M_PI) rotate -= 2*M_PI;
    
    glutPostRedisplay();
    glutTimerFunc(33, myTimer, value);
    
       
}

void myKey(unsigned char key, int x, int y){
    switch(key){
        case ' ':
            fp = !fp;
            break;
        case 'f':
            addUpVector = !addUpVector;
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
    gluPerspective(90, xMax/yMax, 0.1, 5*NUM_CONTROL_POINTS);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

//void findCenterOf3DObject(vector3 smallest, vector3 largest, vector3* middle){    
//    //vector3 middle;
//    middle->x = -(largest.x+smallest.x)/2.0f;
//    middle->y = -(largest.y+smallest.y)/2.0f;
//    middle->z = -(largest.z+smallest.z)/2.0f;
//}


/*
 * This function takes the x, y, and z values to ensure that the scene that is drawn
 * is larger than the 3d structure inside. However, there's a weird graphical glitch.
 */
void drawSkyAndGround(float x, float y, float z){
    int i;
    float step = (2*M_PI)/SKY_STEPS;
    vector3 point;
    //Draw enclosing sky part
    glBegin(GL_QUAD_STRIP);
    for(i = 0; i < SKY_STEPS + 1; i++){
        point.x = sin(step*i)*sqrt(x*x+z*z);
        point.z = cos(step*i)*sqrt(x*x+z*z);
        glColor3f(0.3, 0.3, 0.6);
        glVertex3f(point.x, 0, point.z);
        glColor3f(0.5, 0.8, 1);      
        glVertex3f(point.x, SKY_HEIGHT, point.z);
    }
    glEnd();
    //Draw ground
    glBegin(GL_TRIANGLE_FAN);
        glColor3f(0.1,1,0.1);
        glVertex3d(0,0,0);
        glColor3f(0.3,0.612,0.5);
        for(i = 0; i < SKY_STEPS + 1;  i++){
        glVertex3f(sin(step*i)*sqrt(x*x+z*z), 0, cos(step*i)*sqrt(x*x+z*z));
        }
    glEnd();
    //Draw top of cylinder
    glPushMatrix();
        glBegin(GL_TRIANGLE_FAN);
            glColor3f(0.5, 0.8, 1);
            glTranslatef(0,SKY_HEIGHT,0);
            glVertex3f(0,SKY_HEIGHT,0);
            
            for(i = 0; i < SKY_STEPS + 1; i++){
              glVertex3f(sin(step*i)*sqrt(x*x+z*z), SKY_HEIGHT, cos(step*i)*sqrt(x*x+z*z));
            }
        glEnd();
    glPopMatrix();
  
}

void drawCoasterPath(){
    vector3 point, lastPoint, up, u, n, s;
    int count=0;
    float step = 0, lastStep, mag;
    up.x = 0;
    up.y = 1;
    up.z = 0;
    
    controlPoints = (vector3*)malloc((NUM_CONTROL_POINTS + 3)*sizeof(vector3));    
    
    glNewList(list, GL_COMPILE);
    
    controlPoints[0].x = controlPoints[0].y = controlPoints[0].z = 0.0;
    controlPoints[1].x = controlPoints[1].y = 1.0;
    controlPoints[1].z = 0.0;
    
    drawSkyAndGround(NUM_CONTROL_POINTS+1, SKY_HEIGHT, NUM_CONTROL_POINTS+1);
    
    //Procedurally generate control points for a random track
    glBegin(GL_POINTS);
    for(int i = 2; i < NUM_CONTROL_POINTS; i++){
        controlPoints[i].x = ((float)(rand()%NUM_CONTROL_POINTS+1)-NUM_CONTROL_POINTS)+1;
        
        if(abs(controlPoints[i].y - controlPoints[i-2].y) < 10 || abs(controlPoints[i].y - controlPoints[i-1].y) < 10)
            controlPoints[i].y = (float)(rand()%(SKY_HEIGHT-3)-5);
        else
            controlPoints[i].y = (float)(rand()%(SKY_HEIGHT-3));
            
            
        
        
        controlPoints[i].z = ((float)(rand()%NUM_CONTROL_POINTS+1)-NUM_CONTROL_POINTS)+1;
        if(abs(controlPoints[i].x - controlPoints[i].z) > 10){
            if(rand()%2 == 0)
                controlPoints[i].x+=20;
            else
                controlPoints[i].z+=20;
        }
        //glVertex3f(controlPoints[i].x,controlPoints[i].y,controlPoints[i].z);
        if(controlPoints[i].x > max.x) max.x = controlPoints[i].x;
        if(controlPoints[i].y > max.y) max.y = controlPoints[i].y;
        if(controlPoints[i].z > max.z) max.z = controlPoints[i].z;
    }
    //Set here to easily loop back to the beginning
    controlPoints[NUM_CONTROL_POINTS] = controlPoints[0];
    controlPoints[NUM_CONTROL_POINTS + 1] = controlPoints[1];
    controlPoints[NUM_CONTROL_POINTS + 2] = controlPoints[2];   
    glEnd();
    
    
   
     
    //Draw the main line track. This would have been omitted had I got things working.
//    glBegin(GL_LINE_LOOP);
//    for(float i = 0; i < NUM_CONTROL_POINTS; i+=0.01){
//        q(controlPoints, i, 0, &point);
//        q(controlPoints, i, 1, &n);
//        
//        step = RAIL_WIDTH/(sqrt(n.x*n.x + n.y*n.y + n.z*n.z));
//        if(step < 0.01) step = 0.01;
//        
//             
//        glColor3f(1,1,1);
//        glVertex3f(point.x,point.y,point.z);
//    }
//    glEnd();
    float x,y,z,f;
    x = y = z = 0.1;
    
    for(float i = 0; i < NUM_CONTROL_POINTS; i+=step){
        if(i>0) lastPoint = point;
        float lastPointl, pointl;
        
        lastPointl = vectorMagnitude(&lastPoint);
        
        q(controlPoints, i, 0, &point);       
        q(controlPoints, i, 1, &n);
        q(controlPoints, i, 2, &s);
        pointl = vectorMagnitude(&point);
        f = sqrt(
            (point.x - lastPoint.x)*(point.x - lastPoint.x) +
            (point.y - lastPoint.y)*(point.y - lastPoint.y) +
            (point.z - lastPoint.z)*(point.z - lastPoint.z)
            )*5;
        printf("Pointl: %f\tLastPointl: %f\tLength: %f\n",pointl,lastPointl,f);
        if(distance < point.x)
            distance = point.x+1;
        else if(distance < point.z)
            distance = point.z+1;       
        
        step = 0.1/(sqrt(n.x*n.x + n.y*n.y + n.z*n.z));
        //if(step < 0.01) step = 0.01;
        
        
        mag = vectorMagnitude(&n);
        step = 0.1/mag;
        //if(step < 0.01) step = 0.01;               
        
        calculateUpVector(&n, &s, &up);
        normalizeVector(&up);
        
        crossProduct(&n, &up, &u);
        normalizeVector(&u);
        //printf("Length: %f\n", vectorMagnitude(&u));
        scale(&u,0.7);       
        
        
        
        float matrix[] = {
            u.x, u.y, u.z, 0.0f,
            up.x, up.y, up.z, 0.0f,
            n.x, n.y, n.z, 0.0f,
            point.x, point.y, point.z, 1.0f
        };
        
        glColor3f(0,0,0);
        glPushMatrix();
            
            glMultMatrixf(matrix);
            glTranslatef(0, 0, 0);
            
            //printf("Point x: %f\tPoint y: %f\tPoint z: %f\n",point.x, point.y, point.z);
            glPushMatrix();
                glTranslatef(0.5,0,0);
//                glTranslatef(u.x,u.y,u.z);
                //printf("u x: %f\tu y: %f\tu z: %f\n",u.x,u.y,u.z);
                glScalef(x, y, z);
                drawBox(0.5,0.5,f);
            glPopMatrix();

            glColor3f(1,0,0);
            glPushMatrix();
                glTranslatef(-0.5,0,0);
//                glTranslatef(-u.x,-u.y,-u.z);
                glScalef(x, y, z);
                drawBox(0.5,0.5,f);
            glPopMatrix();
            glPushMatrix();
                glTranslatef(0,0,0);
                glScalef(x*0.7,y*0.7,z*0.7);
                glColor3f(1,1,1);
                drawBox(6,0,0.5);
            glPopMatrix();
            
         
        glPopMatrix();
        //glFlush();    
       
    }
    
    glEndList();
}



void drawSquare(float x, float y, float z){
    glBegin(GL_LINE_LOOP);
    glVertex3f(x-1, y+1, z);
    glVertex3f(x-1, y-1, z);
    glVertex3f(x+1, y-1, z);
    glVertex3f(x+1,y+1, z);
    glEnd();
}

void drawBox(float x, float y, float z){
    glBegin(GL_QUADS);
    //float x = 1, y = 1, z = 4; 


/*Back face*/
//    glColor3f(1.0,1.0,0);
        glVertex3f(x,y,-z);
        glVertex3f(x,-y,-z);
        glVertex3f(-x,-y,-z);
        glVertex3f(-x,y,-z);
/*Front face*/
//    glColor3f(0,1.0,1.0);
        glVertex3f(x,y,z);
        glVertex3f(x,-y,z);
        glVertex3f(-x,-y,z);
        glVertex3f(-x,y,z);    
/*Top face*/    
//        glColor3f(1.0,1.0,1.0);
        glVertex3f(x,y,z);
        glVertex3f(-x,y,z);
        glVertex3f(-x,y,-z);
        glVertex3f(x,y,-z);
/*Bottom face*/
        glColor3f(1.0,0,1.0);
        glVertex3f(x,-y,z);
        glVertex3f(-x,-y,z);
        glVertex3f(-x,-y,-z);
        glVertex3f(x,-y,-z);
/*Right face*/
        glColor3f(0,0,1.0);
        glVertex3f(x,y,z);
        glVertex3f(x,-y,z);
        glVertex3f(x,-y,-z);
        glVertex3f(x,y,-z);
/*Left face*/
        glColor3f(0,1,0);
        glVertex3f(-x,y,z);
        glVertex3f(-x,-y,z);
        glVertex3f(-x,-y,-z);
        glVertex3f(-x,y,-z);

  glEnd();
}

void drawText(double x, double y, char *text){

		//glColor3i(1,1,1);
		glRasterPos3f( x, y, 0);
		for(int i = 0; i < strlen(text); i++){
			glutBitmapCharacter(GLUT_BITMAP_9_BY_15, text[i]);

		}
//		printf("\nText is %s\n",text);
//		fflush(stdout);		

}

void q(const vector3* list, float u, int derivation, vector3* result){
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
    if(derivation != 0)
        normalizeVector(result);
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
    scale(v, (1/mag));    
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
    float k = numerator.y/(mag*mag*mag);
    //k /= 8;
    if(k > M_PI/4)
        k = M_PI/4;
    else if(k < -M_PI/4)
        k = -M_PI/4;
    
    up->x = (1-cos(-k))*r->x*r->y-sin(-k)*r->z;
    up->y = (1-cos(-k))*r->y*r->y+cos(-k);
    up->z = (1-cos(-k))*r->y*r->z+sin(-k)*r->x;
    
    
    
}

vector3 negativeVector(vector3 v){
    v.x *= (-1);
    v.y *= (-1);
    v.z *= (-1);
    return v;
}

void vectorAdd_Sub(vector3* affected, const vector3* effector, int flag){
    affected->x += flag*effector->x;
    affected->y += flag*effector->y;
    affected->z += flag*effector->z;
}

void dotProduct(const vector3* a, const vector3* b, vector3* product){
    product->x = a->x*b->x;
    product->y = a->y*b->y;
    product->z = a->z*b->z;
}
//void vectorAdd_Sub(vector3* affected, float effector, int flag){
//    affected->x += flag*effector;
//    affected->y += flag*effector;
//    affected->z += flag*effector;
//}