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
  File:        SplitFacet.h
  Description: Split facet by plane dialog
*/

#ifndef _SPLITFACETH_
#define _SPLITFACETH_

#include "GLApp/GLWindow.h"

class GLButton;
class GLTextField;
class GLLabel;
class GLToggle;
class GLTitledPanel;

class Geometry;
class Worker;

class SplitFacet : public GLWindow {

public:
  // Construction
  SplitFacet(Geometry *geom,Worker *work);
  ~SplitFacet();
  void ProcessMessage(GLComponent *src,int message);
  void ClearUndoFacets();

  // Implementation
private:

  void EnableDisableControls(int mode);
  
  GLTitledPanel	*planeDefPanel;
  GLLabel	*label1;
  GLToggle	*eqmodeCheckbox;
  GLButton	*XZplaneButton;
  GLButton	*YZplaneButton;
  GLButton	*XYplaneButton;
  GLTextField	*dTextbox;
  GLLabel	*label4;
  GLTextField	*cTextbox;
  GLLabel	*label3;
  GLTextField	*bTextbox;
  GLLabel	*label2;
  GLTextField	*aTextbox;
  GLToggle	*vertexModeCheckbox;
  GLTextField	*facetIdTextbox;
  GLToggle	*facetmodeCheckbox;
  GLLabel	*resultLabel;
  GLButton	*splitButton;
  GLButton	*undoButton;
  GLButton	*getSelectedFacetButton;

  int planeMode;
  std::vector<DeletedFacet> deletedFacetList;
  size_t nbFacet, nbCreated;

  Geometry     *geom;
  Worker	   *work;

};

#endif /* _SplitFacetH_ */
