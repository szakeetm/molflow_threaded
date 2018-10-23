/*
  File:        DrawingArea.h
  Description: Simple drawing area
  Program:     MolFlow
  Author:      R. KERSEVAN / J-L PONS / M ADY
  Copyright:   E.S.R.F / CERN

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
*/

#include "GLApp/GLComponent.h"
#include "MolflowTypes.h"
#include "Polygon.h"

class DrawingArea : public GLComponent {

public:

  // Construction
  DrawingArea();

  // Implementation
  void Paint();

private:

  void DrawPoly(POLYGON *p,double tx,double ty);
  void DrawTest(POLYGON *p,int nb,POLYGON *r,int tx,int ty);

  POLYGON p1;
  POLYGON p21;
  POLYGON *result1;
  int nbPoly1;

  POLYGON p22;
  POLYGON *result2;
  int nbPoly2;

  POLYGON p23;
  POLYGON *result3;
  int nbPoly3;

  POLYGON p24;
  POLYGON *result4;
  int nbPoly4;

  POLYGON p25;
  POLYGON *result5;
  int nbPoly5;

  POLYGON p26;
  POLYGON *result6;
  int nbPoly6;

  POLYGON p27;
  POLYGON *result7;
  int nbPoly7;

  POLYGON p28;
  POLYGON *result8;
  int nbPoly8;

  POLYGON p29;
  POLYGON *result9;
  int nbPoly9;

  POLYGON p2A;
  POLYGON *resultA;
  int nbPolyA;

  POLYGON p2B;
  POLYGON *resultB;
  int nbPolyB;

  POLYGON p2C;
  POLYGON *resultC;
  int nbPolyC;

  POLYGON p2D;
  POLYGON *resultD;
  int nbPolyD;

  POLYGON p2E;
  POLYGON *resultE;
  int nbPolyE;

  POLYGON p2F;
  POLYGON *resultF;
  int nbPolyF;

  POLYGON p2G;
  POLYGON *resultG;
  int nbPolyG;

  POLYGON p2H;
  POLYGON *resultH;
  int nbPolyH;

  POLYGON p2I;
  POLYGON *resultI;
  int nbPolyI;

  POLYGON p2J;
  POLYGON *resultJ;
  int nbPolyJ;

};
