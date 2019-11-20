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
#include "GrahamScan.h"

GRpoint* GRfirstPoint; //GLOBAL POINTER TO Min GRpoint IN DOUBLELY LINKED LIST
size_t pubNbPoints;
int *pubReturnList;
Vector2d *pubProjectedPoints;

class GRpoint
{
public:
	int id;
	double x; //X POSITION
    double y; //Y POSITION
    GRpoint *next; //POINTER TO NEXT NODE IN THE LIST
	GRpoint *prev; //POINTER TO PREVIOUS NODE IN THE LIST
    double angle; //INTERMEDIATE ANGLE VALUE STORAGE
};

void grahamMain(Vector2d *projectedPoints,size_t nbPoints,int *returnList)
{
	pubNbPoints = nbPoints;
	pubProjectedPoints = projectedPoints;
	pubReturnList=returnList;
	grahamInit(); //INITIALIZE DATA FOR GRAHAM'S SCAN
    //printPoints(); //PRINT OUT SORTED POINTS
    //drawPoints(); //RENDER POINTS ON SCREEN
    //drawPermeter(1); //DRAW PERIMETER BEFORE GRAHAM'S SCAN
    grahamScan(GRfirstPoint->next); //RUN GRAHAM'S SCAN STARTING AT SECOND NODE CLOCKWISE
    //drawPermeter(2); //DRAW PERIMETER AFTER GRAHAM'S SCAN
    //printPoints(); //PRINT OUT CONVEX HULL
	constructReturnList();
}

void grahamScan(GRpoint *P)
{
    GRpoint *tempPrev, *tempNext;
    
    if (P==GRfirstPoint) //IF RETURNED TO FIRST GRpoint, DONE
        return;
    
    if (!isConvexPoint(P)) //IF GRpoint IS CONCAVE, ELIMINATE FROM PERIMETER
    {
        tempPrev=P->prev; 
        tempNext=P->next;
        tempPrev->next=tempNext;
        tempNext->prev=tempPrev;
        //drawLine(tempPrev, tempNext,3); //DRAW LINE SHOWING NEW EDGE
        delete P; //FREE MEMORY
        grahamScan(tempPrev); //RUN GRAHAM'S SCAN ON PREVIOUS GRpoint TO CHECK IF CONVEXITY HAS CHANGED IT

    }
    else //GRpoint IS CONVEX
        grahamScan(P->next); //PROCEED TO NEXT GRpoint
}

void grahamInit()
{
    int minPoint=0;
    double tempAngle=0;
	GRpoint *tempPoints = (GRpoint *)malloc(pubNbPoints*sizeof(GRpoint));
    GRpoint *tempPtr;
    
    GRfirstPoint=NULL; //INIT FIRSTPOINT POINTER
    
    for (int i=0;i<pubNbPoints;i++) //GENERATE RANDOM POINTS
    {
		tempPoints[i].x=pubProjectedPoints[i].u+1.0; //add 1000 so all values are positive
        tempPoints[i].y=pubProjectedPoints[i].v+1.0; //add 1000 so all values are positive
		tempPoints[i].id=i;
    }
    
    for (int k=1;k<pubNbPoints;k++)  //FIND Min GRpoint, lowest y coordinate
        if (tempPoints[k].y<tempPoints[minPoint].y)
            minPoint=k;
    
    //cout<<"MinPoint: "<<tempPoints[minPoint].x<<" "<<tempPoints[minPoint].y<<endl;  //PRINT OUT MINIMAL GRpoint
    
    for (int i=0;i<pubNbPoints;i++) //SORT RANDOM POINTS
    {
        tempPoints[i].angle=findAngle(tempPoints[minPoint].x,tempPoints[minPoint].y,tempPoints[i].x,tempPoints[i].y);
        addPoint(tempPoints[i]);
        //cout<<"angle: "<<tempPoints[i].angle<<" x: "<<tempPoints[i].x<<" y: "<<tempPoints[i].y<<endl;
    }
    
    tempPtr=GRfirstPoint;
    do  //FIND LAST NODE IN LINKED LIST
    {
        tempPtr=tempPtr->next;
    } while (tempPtr->next!=NULL);
        
    tempPtr->next=GRfirstPoint; //COMPLETE CIRCULAR LINKED LIST
    GRfirstPoint->prev=tempPtr; //COMPLETE CIRCULAR LINKED LIST
    
    tempPtr=GRfirstPoint->next;
    for (int i=1;i<pubNbPoints;i++) //DRAW LINES FROM MIN GRpoint TO ALL OTHERS
    {
        //drawLine(GRfirstPoint, tempPtr, 3);
        tempPtr=tempPtr->next;
    } 
}

bool isConvexPoint(GRpoint* P)
{
    double CWAngle=findAngle(P->x,P->y,P->prev->x,P->prev->y); //COMPUTE CLOCKWISE ANGLE
    double CCWAngle=findAngle(P->x,P->y,P->next->x,P->next->y); //COMPUTER COUNTERCLOCKWISE ANGLE
    double difAngle;
    
    //drawLine(P->prev, P->next, 3);
    
    if (CWAngle>CCWAngle)
    {
        difAngle=CWAngle-CCWAngle;  //COMPUTE DIFFERENCE BETWEEN THE TWO ANGLES
        
        if (difAngle>180)
            return false; //GRpoint IS CONCAVE
        else
            return true; //GRpoint IS CONVEX
    }
    else if (CWAngle<CCWAngle)
    {
        difAngle=CCWAngle-CWAngle;  //COMPUTE DIFFERENCE BETWEEN THE TWO ANGLES
        
        if (difAngle>180)
            return true; //GRpoint IS CONVEX
        else
            return false; //GRpoint IS CONCAVE
    }
    else //if (CWAngle == CCWAngle)
	return false; //GRpoint IS collinear
}

void addPoint(GRpoint Point)
{
    GRpoint *tempPoint,*tempPointA,*tempPointB, *curPoint;
    
    //ALLOCATE A NEW GRpoint STRUCTURE AND INITIALIZE INTERNAL VARIABLES
    tempPoint = new GRpoint;
    tempPoint->x=Point.x;
    tempPoint->y=Point.y;
	tempPoint->id=Point.id;
    tempPoint->angle=Point.angle;  
    tempPoint->next=NULL;
    tempPoint->prev=NULL;
    
    
    if (GRfirstPoint==NULL) //TEST IF LIST IS EMPTY
    {
        GRfirstPoint=tempPoint;
        return;
    }

    if (GRfirstPoint->next==NULL && tempPoint->angle >= GRfirstPoint->angle) 
    //TEST IF ONLY ONE NODE IN LIST AND CURRENT NODE HAS GREATER ANGLE
    {
        GRfirstPoint->next=tempPoint;
        tempPoint->prev=GRfirstPoint;
        return;
    }
    
    curPoint=GRfirstPoint;
    
    while (tempPoint->angle >= curPoint->angle && curPoint->next!=NULL)
    //CONTINUE THROUGH LIST UNTIL A NODE IS FOUND WITH A GREATER ANGLE THAN CURRENT NODE
        curPoint=curPoint->next;
  
    if (curPoint==GRfirstPoint) //TEST IF NODE IS FIRSTPOINT.  IF SO, ADD AT FRONT OF LIST.
    {
        GRfirstPoint->prev=tempPoint;
        tempPoint->next=GRfirstPoint;
        GRfirstPoint=tempPoint;
        return;
    }
    else if (curPoint->next==NULL && tempPoint->angle >= curPoint->angle) 
    //TEST IF WHILE LOOP REACHED FINAL NODE IN LIST.  IF SO, ADD AT END OF THE LIST.
    {
        curPoint->next=tempPoint;
        tempPoint->prev=curPoint;
        return;
    }
    else //OTHERWISE, INTERMEDIATE NODE HAS BEEN FOUND.  INSERT INTO LIST.  
    {
        tempPointA=curPoint->prev;
        tempPointB=curPoint->prev->next;
        tempPoint->next=tempPointB;
        tempPoint->prev=tempPointA;
        tempPoint->prev->next=tempPoint;
        tempPoint->next->prev=tempPoint;
    }
    
    return;   
}

double findAngle(double x1, double y1, double x2, double y2)
{
    double deltaX=(double)(x2-x1);
    double deltaY=(double)(y2-y1);
    double angle;
 
    if (deltaX==0 && deltaY==0)
        return 0;
   
    angle=atan2(deltaY,deltaX)*57.295779513082;
  
   if (angle < 0)
 	angle += 360.;

    return angle;
}

void constructReturnList()
{
    GRpoint *tempPtr=GRfirstPoint;
    
    for (int i=0;i<pubNbPoints;i++) //DRAW LINES FROM ONE GRpoint TO THE NEXT, FOR ALL NODES, BACK TO THE FIRSTPOINT
    {
		pubReturnList[i]=tempPtr->id;
        tempPtr=tempPtr->next; 
    }   
}

//--------------------------PERIPHERAL FUNCTIONS-------------------------------
/*
void drawPermeter(int color)
{
    GRpoint *tempPtr=GRfirstPoint;
    
    for (int i=0;i<NumPoints;i++) //DRAW LINES FROM ONE GRpoint TO THE NEXT, FOR ALL NODES, BACK TO THE FIRSTPOINT
    {
        drawLine(tempPtr, tempPtr->next, color);  //DRAW LINE
        tempPtr=tempPtr->next; 
    }   
}

void printPoints()
{
    GRpoint *curPoint=GRfirstPoint;
    
    do
    {
        cout<<"angle: "<<curPoint->angle<<" x: "<<curPoint->x<<" y: "<<curPoint->y<<endl;
        curPoint=curPoint->next;
    } while (curPoint!=GRfirstPoint); //CONTINUE UNTIL HAVING LOOPED BACK AROUND TO FIRSTPOINT
}

void drawPoints()
{
    GRpoint *curPoint = GRfirstPoint;
    
    do 
    {
        filledCircleRGBA (screen, curPoint->x, curPoint->y, 3, 255, 0, 0, 255); //DRAW RED CIRCLE OF RADIUS 3
        curPoint=curPoint->next;
    } while (curPoint!=GRfirstPoint); //CONTINUE UNTIL HAVING LOOPED BACK AROUND TO FIRSTPOINT
    
    return;
}

void graphicsLoop()
{
    SDL_Event event;
    Uint8 *keys;
    
    SDL_Flip(screen); //FLIP GRAPHICS BUFFER
    
    while (true)
    {
        SDL_PollEvent(&event);
        keys=SDL_GetKeyState(NULL);
        
        if (keys[SDLK_ESCAPE]) //TEST FOR USER INPUT (ESCAPE KEY)
        {
            exit(1); //EXIT PROGRAM
        }
        
        if(keys[SDLK_SPACE]) //TEST FOR USER INPUT (SPACE BAR)
        {
            boxRGBA(screen, 0, 0, 500, 500, 0, 0, 0, 255); //CLEAR SCREEN WITH BLACK BOX
            grahamMain(); //GENERATE BRAND NEW CONVEX HULL
            SDL_Flip(screen); //FLIP GRAPHICS BUFFER
        }
        
        //SDL_Delay(100);
    }        
}

void initScreen()
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0) //TEST SDL STARTED PROPERLY
        exit(1); //ON ERROR, EXIT

    atexit(SDL_Quit); //CLEAN UP SDL BEFORE EXITING
    screen=SDL_SetVideoMode(500, 500, 16, SDL_SWSURFACE); //SET SCREEN MODE
}

void drawLine(GRpoint *A, GRpoint *B, int color)
{
    if (color==1) //IF COLOR==1, DRAW RED LINE 
        lineRGBA (screen, A->x, A->y, B->x, B->y, 225, 0, 0, 255);
    if (color==2) //IF COLOR==1, DRAW GREEN LINE
        lineRGBA (screen, A->x, A->y, B->x, B->y, 0, 255, 0, 255);
    if (color==3) //IF COLOR==1, DRAW LIGHT BLUE LINE
        lineRGBA (screen, A->x, A->y, B->x, B->y, 0, 150, 220, 255);
}

*/