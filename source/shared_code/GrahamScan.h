/*
Program:     MolFlow+ / Synrad+
Description: Monte Carlo simulator for ultra-high vacuum and synchrotron radiation
Authors:     Jean-Luc PONS / Roberto KERSEVAN / Marton ADY / Pascal BAEHR
Copyright:   E.S.R.F / CERN
Website:     https://cern.ch/molflow

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

Full license text: https://www.gnu.org/licenses/old-licenses/gpl-2.0.en.html
*/
#pragma once

//#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <iostream>
#include <time.h>
using namespace std;

#include "Vector.h"

//--------------------GRpoint DATA STRUCTURE---------------------------
class GRpoint;

//--------------------GLOBAL VARIABLES---------------------------
const int NumPoints = 15; // n<1000
//SDL_Surface *screen; //GLOBAL SDL GRAPHICS SURFACE POINTER

//--------------------GRAHAM'S SCAN FUNCTIONS---------------------------
void grahamInit(); //INITIALIZE VARIABLES, RANDOMLY GENERATE POINTS,
                   //LOCATE MIN GRpoint, AND SORT POINTS BY RELATIVE ANGLES
void grahamMain(Vector2d *projectedPoints,size_t nbPoints,int *returnList); //SETUP, RUN GRAHAM'S SCAN, AND DISPLAY RESULTS
void grahamScan(GRpoint *P); //ACTUAL GRAHAM'S SCAN PROCEDURE
void constructReturnList();
bool isConvexPoint(GRpoint *P); //TEST GRpoint FOR CONVEXITY
void addPoint(GRpoint GRpoint); //ADDS GRpoint TO DOUBLELY LINKED LIST (USED DURING SORTING)
double findAngle(double x1, double y1, double x2, double y2); //FIND ANGLE GIVEN TWO POINTS

/*
//--------------------AUXILARY GRAPHICS FUNCTIONS---------------------------
void initScreen(); //SETUP THE GRAPHICS SURFACE AND WINDOW
void drawPoints(); //DRAW POINTS FROM GLOBAL DOUBLELY LINKED LIST
void drawLine(GRpoint *A, GRpoint *B, int color); //DRAWS A LINE WITH 3 COLOR POSSIBILITIES
void drawPermeter(int color); //DRAWS PERIMETER WITH 3 COLOR POSSIBILITIES
void graphicsLoop(); //MAIN GRAPHICS LOOP
void printPoints(); //PRINTS ALL POINTS IN DOUBLELY LINKED LIST
*/
