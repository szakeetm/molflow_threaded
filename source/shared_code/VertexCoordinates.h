/*
Program:     MolFlow+ / Synrad+
Description: Monte Carlo simulator for ultra-high vacuum and synchrotron radiation
Authors:     Jean-Luc PONS / Roberto KERSEVAN / Marton ADY
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
#ifndef _VERTEXCOORDINATESH_
#define _VERTEXCOORDINATESH_

#include "GLApp/GLWindow.h"

class GLTextField;
class GLButton;
class GLList;

class Worker;

class VertexCoordinates : public GLWindow {

public:

  // Construction
  VertexCoordinates();

  // Component method
  void Display(Worker *w);
  void Update();

  // Implementation
  void ProcessMessage(GLComponent *src,int message);

private:

  //void GetSelected();

  Worker       *worker;
  //Facet        *selFacet;
  GLList       *vertexListC;
  GLButton     *dismissButton;
  GLButton     *updateButton;
  //GLButton     *insert1Button;
  //GLButton     *insert2Button;
  //GLButton     *removeButton;
  //GLTextField  *insertPosText;
  GLButton      *setXbutton, *setYbutton, *setZbutton;
};

#endif /* _VertexCoordinatesH_ */
