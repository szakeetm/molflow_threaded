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
#include "AddVertex.h"
#include "GLApp/GLTitledPanel.h"
#include "GLApp/GLToolkit.h"
#include "GLApp/GLWindowManager.h"
#include "GLApp/GLMessageBox.h"
#include "GLApp/GLTextField.h"
#include "GLApp/GLLabel.h"
#include "GLApp/GLButton.h"
#include "Geometry_shared.h"
#include "Facet_shared.h"
#include "GLApp/MathTools.h" //Contains

#ifdef MOLFLOW
#include "MolFlow.h"
#include "SuperFacet.h"

#endif

#ifdef SYNRAD
#include "SynRad.h"
#endif

#ifdef MOLFLOW
extern MolFlow *mApp;
#endif

#ifdef SYNRAD
extern SynRad*mApp;
#endif

/**
* \brief Constructor with initialisation for the AddVertex window (Vertex/Add new)
* \param g pointer to the Geometry
* \param w Worker handle
*/
AddVertex::AddVertex(Geometry *g,Worker *w):GLWindow() {

  int wD = 335;
  int hD = 120;

  SetTitle("Add new vertex");

  GLLabel *l1 = new GLLabel("X:");
  l1->SetBounds(10,5,10,18);
  Add(l1);

  x = new GLTextField(0,"0");
  x->SetBounds(25,4,80,18);
  Add(x);

  GLLabel *l2 = new GLLabel("Y:");
  l2->SetBounds(120,5,10,18);
  Add(l2);

  y = new GLTextField(0,"0");
  y->SetBounds(135,4,80,18);
  Add(y);
  
  GLLabel *l3 = new GLLabel("Z:");
  l3->SetBounds(230,5,10,18);
  Add(l3);

  z = new GLTextField(0,"0");
  z->SetBounds(245,4,80,18);
  Add(z);

  facetCenterButton = new GLButton(0, "Facet center");
  facetCenterButton->SetBounds(10, 30, 75, 20);
  Add(facetCenterButton);

  facetUButton = new GLButton(0, "Facet \201");
  facetUButton->SetBounds(90, 30, 75, 20);
  Add(facetUButton);

  facetVButton = new GLButton(0, "Facet \202");
  facetVButton->SetBounds(170, 30, 75, 20);
  Add(facetVButton);

  facetNormalButton = new GLButton(0, "Facet N");
  facetNormalButton->SetBounds(250, 30, 75, 20);
  Add(facetNormalButton);

  addButton = new GLButton(0,"Add vertex");
  addButton->SetBounds(120,hD-44,100,20);
  Add(addButton);

  // Center dialog
  int wS,hS;
  GLToolkit::GetScreenSize(&wS,&hS);
  int xD = (wS-wD)/2;
  int yD = (hS-hD)/2;
  SetBounds(xD,yD,wD,hD);

  RestoreDeviceObjects();

  geom = g;
  work = w;

}

/**
* \brief Function for processing various inputs (button, check boxes etc.)
* \param src Exact source of the call
* \param message Type of the source (button)
*/
void AddVertex::ProcessMessage(GLComponent *src,int message) {

  double X,Y,Z;

  switch (message) {
  case MSG_BUTTON:

	  if (src == addButton) {

		  if (!x->GetNumber(&X)) {
			  GLMessageBox::Display("Invalid X coordinate", "Error", GLDLG_OK, GLDLG_ICONERROR);
			  return;
		  }
		  if (!y->GetNumber(&Y)) {
			  GLMessageBox::Display("Invalid Y coordinate", "Error", GLDLG_OK, GLDLG_ICONERROR);
			  return;
		  }
		  if (!z->GetNumber(&Z)) {
			  GLMessageBox::Display("Invalid Z coordinate", "Error", GLDLG_OK, GLDLG_ICONERROR);
			  return;
		  }
		  geom->AddVertex(X, Y, Z);
	  }
	  else if (Contains({ facetCenterButton,facetUButton,facetVButton,facetNormalButton }, src)) {
		  auto selFacetIds = geom->GetSelectedFacets();
		  if (selFacetIds.size() != 1) {
			  GLMessageBox::Display("Select exactly one facet", "Error", GLDLG_OK, GLDLG_ICONERROR);
			  return;
		  }
		  Vector3d location;
		  auto sh = geom->GetFacet(selFacetIds[0])->sh;
		  if (src == facetCenterButton) {
			  location = sh.center;
		  }
		  else if (src == facetUButton) {
			  location = sh.O + sh.U;
		  }
		  else if (src == facetVButton) {
			  location = sh.O + sh.V;
		  }
		  else if (src == facetNormalButton) {
			  location = sh.center + sh.N;
		  }
		  geom->AddVertex(location);
	  }
	  break;
  }

  GLWindow::ProcessMessage(src, message);
}

