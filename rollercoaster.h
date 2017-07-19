/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   rollercoaster.h
 * Author: kristofer
 *
 * Created on July 18, 2017, 9:40 PM
 */

#ifndef ROLLERCOASTER_H
#define ROLLERCOASTER_H

#endif /* ROLLERCOASTER_H */
typedef union Vector3{
    struct{
        float x,y,z;
    };
    float coord[3];
}vector3;

static void myDisplay(void);
static void myTimer(int value);
static void myKey(unsigned char key, int x, int y);
static void keyPress(int key, int x, int y);
static void keyRelease(int key, int x, int y);
static void myReshape(int w, int h);

static void init(void);

static void drawText(double x, double y, char *text);
static void drawBox();

int q(const vector3* list, float t, int derivation, vector3* result);
float uniformBSpline(float p0, float p1, float p2, float p3, float t);
float uniformBSplineDerivative(float p0, float p1, float p2, float p3, float t);
float uniformBSplineSecondDerivative(float p0, float p1, float p2, float p3, float t);