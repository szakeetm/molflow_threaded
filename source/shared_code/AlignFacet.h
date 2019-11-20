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

#include "GLApp/GLWindow.h"

class GLButton;
class GLTextField;
class GLLabel;
class GLToggle;
class GLTitledPanel;
class Vector3d;

#include <vector> //Std vectors

class Geometry;
class Worker;

class AlignFacet : public GLWindow {

public:

  // Construction
  AlignFacet(Geometry *geom,Worker *work);

  // Implementation
  void ProcessMessage(GLComponent *src,int message);
  void MemorizeSelection();

private:

  Geometry     *geom;
  Worker	   *work;

  std::vector<size_t> memorizedSelection;
  std::vector<std::vector<Vector3d>> oriPositions;

  GLButton    *memoSel;
  GLLabel     *numFacetSel;
  GLButton    *alignButton;
  GLButton    *copyButton;
  GLButton    *undoButton;
  
  GLButton    *cancelButton;

  GLLabel     *l1;
  GLToggle    *invertNormal;
  GLToggle    *invertDir1;
  GLToggle    *invertDir2;

  GLTitledPanel *step1;
  GLTitledPanel *step2;
  GLTitledPanel *step3;

};
