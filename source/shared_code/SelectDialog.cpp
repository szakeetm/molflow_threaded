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
#include "SelectDialog.h"
#include "Facet_shared.h"
#include "GLApp/GLToolkit.h"
#include "GLApp/GLList.h"
#include "GLApp/GLMessageBox.h"

#include "GLApp/GLTextField.h"
#include "GLApp/GLButton.h"
#include "GLApp/GLLabel.h"

#include "Geometry_shared.h"
#include "GLApp/MathTools.h" //Splitstring
#include <numeric> //iota

#ifdef MOLFLOW
#include "MolFlow.h"
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

// Construct a message dialog box
SelectDialog::SelectDialog(Geometry *g):GLWindow() {

  int /*xD,yD,*/wD,hD;
  wD=400;hD=105;
  SetTitle("Select facet(s) by number");

  // Label
  GLLabel  *label = new GLLabel("Facet number:");
  label->SetBounds(10,4,100,18);
  Add(label);

  numText = new GLTextField(0,NULL);
  numText->SetBounds(90,3,280,18);
  Add(numText);

  GLLabel* label2 = new GLLabel("You can enter a list and/or range(s), examples: 1,2,3 or 1-10 or 1-10,20-30");
  label2->SetBounds(10, 28, 100, 18);
  Add(label2);

  // Buttons
  int startX = 5;
  int startY = 55;
     
    selButton = new GLButton(0,"Select");
	selButton->SetBounds(startX,startY,125,20);
    Add(selButton);
    startX+=130;
  
    addButton = new GLButton(0,"Add to selection");
    addButton->SetBounds(startX,startY,125,20);
    Add(addButton);
    startX+=130;
  
    remButton = new GLButton(0,"Remove from selection");
    remButton->SetBounds(startX,startY,125,20);
    Add(remButton);
    startX+=130;  

  // Center dialog
  /*int wS,hS;
  GLToolkit::GetScreenSize(&wS,&hS);
  if( wD > wS ) wD = wS;
  xD = (wS-wD)/2;
  yD = (hS-hD)/2;*/
  SetBounds(10,30,wD,hD);

  // Create objects
  RestoreDeviceObjects();

  rCode = GLDLG_CANCEL;

  geom = g;
}

void SelectDialog::ProcessMessage(GLComponent *src,int message) {
  if(message==MSG_BUTTON) {
	  std::vector<size_t> facetIds;
	  auto ranges = SplitString(numText->GetText(), ',');
	  if (ranges.size() == 0) {
		  GLMessageBox::Display("Can't parse input", "Error", GLDLG_OK, GLDLG_ICONERROR);
		  return;
	  }
	  for (const auto& range : ranges) {
		  auto tokens = SplitString(range, '-');
		  if (!Contains({ 1,2 },tokens.size())) {
			  std::ostringstream tmp;
			  tmp << "Can't parse \"" << range << "\". Should be a facet number or a range.";
			  GLMessageBox::Display(tmp.str().c_str(), "Error", GLDLG_OK, GLDLG_ICONERROR);
			  return;
		  }
		  else if (tokens.size() == 1) {
			  //One facet
			  try {
				  int facetId = std::stoi(tokens[0]);
				  std::ostringstream tmp;
				  tmp << "Wrong facet id " << tokens[0];
				  if (facetId <= 0 || facetId > geom->GetNbFacet()) throw std::invalid_argument(tmp.str());
				  facetIds.push_back(facetId - 1);
			  }
			  catch (std::invalid_argument arg) {
				  std::ostringstream tmp;
				  tmp << "Invalid facet number " << tokens[0] <<"\n" << arg.what();
				  GLMessageBox::Display(tmp.str().c_str(), "Error", GLDLG_OK, GLDLG_ICONERROR);
				  return;
			  }
		  }
		  else if (tokens.size() == 2) {
			  //Range
			  try {
				  std::ostringstream tmp;
				  int facetId1 = std::stoi(tokens[0]);
				  if (facetId1 <= 0 || facetId1 > geom->GetNbFacet()) {
					  tmp << "Wrong facet id " << tokens[0];
					  throw std::invalid_argument(tmp.str());
				  }
				  int facetId2 = std::stoi(tokens[1]);
				  tmp << "Wrong facet id " << tokens[1];
				  if (facetId2 <= 0 || facetId2 > geom->GetNbFacet()) {
					  tmp << "Wrong facet id " << tokens[0];
					  throw std::invalid_argument(tmp.str());
				  }
				  if (facetId2 <= facetId1) {
					  std::ostringstream tmp;
					  tmp << "Invalid range " << facetId1 << "-" << facetId2;
					  throw Error(tmp.str().c_str());					
				  }
				  size_t oldSize = facetIds.size();
				  facetIds.resize(oldSize + facetId2 - facetId1 + 1);
				  std::iota(facetIds.begin() + oldSize, facetIds.end(), facetId1 - 1);
			  }
			  catch (std::invalid_argument arg) {
				  std::ostringstream tmp;
				  tmp << "Invalid facet number " << tokens[0] << "\n" << arg.what();
				  GLMessageBox::Display(tmp.str().c_str(), "Error", GLDLG_OK, GLDLG_ICONERROR);
				  return;
			  }
			  catch (Error err) {
				  GLMessageBox::Display(err.GetMsg(), "Error", GLDLG_OK, GLDLG_ICONERROR);
				  return;
			  }
		  }
	  }

		  if (src == selButton) geom->UnselectAll();
		  for (const auto& facetId : facetIds) {
			  geom->GetFacet(facetId)->selected = Contains({ selButton,addButton },src);
		  }
		  geom->UpdateSelection();
		  mApp->UpdateFacetParams(true);
		  mApp->UpdateFacetlistSelected();
		  mApp->facetList->ScrollToVisible(facetIds.back(),1,true);
  }
  GLWindow::ProcessMessage(src,message);
}