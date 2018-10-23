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

/*
  File:        MirrorVertex.h
  Description: Mirror vertex to plane dialog
*/
#ifndef _MirrorVertexH_
#define _MirrorVertexH_

#include "GLApp/GLWindow.h"
#include "Geometry_shared.h" //UndoPoint
#include <vector>

class Geometry;
class Worker;
class GLButton;
class GLTextField;
class GLLabel;
class GLToggle;
class GLTitledPanel;

class MirrorVertex : public GLWindow {

public:
  // Construction
  MirrorVertex(Geometry *geom,Worker *work);
  void ClearUndoVertices();
  void ProcessMessage(GLComponent *src,int message);

  // Implementation
private:

  void UpdateToggle(GLComponent *src);
  
  GLTitledPanel *iPanel;
  GLButton     *mirrorButton, *mirrorCopyButton;
  GLButton    *projectButton, *projectCopyButton, *undoProjectButton;
  GLButton    *cancelButton;
  GLButton	  *getPlaneButton;
  GLToggle     *l1;
  GLToggle     *l2;
  GLToggle     *l3;
  GLToggle     *l4;
  GLToggle     *l6;
  GLTextField *aText;
  GLTextField *bText;
  GLTextField *cText;
  GLTextField *dText;
  GLLabel		*aLabel;
  GLLabel		*bLabel;
  GLLabel		*cLabel;
  GLLabel		*dLabel;

  int nbFacetS;
  int    planeMode;
  std::vector<UndoPoint> undoPoints;

  Geometry     *geom;
  Worker	   *work;

};

#endif /* _MirrorVertexH_ */
