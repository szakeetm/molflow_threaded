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
#include "SelectTextureType.h"
#include "GLApp/GLMessageBox.h"
#include "Facet_shared.h"
#include "GLApp/GLToolkit.h"
#include "GLApp/GLWindowManager.h"
#include "GLApp/GLTitledPanel.h"
#include "GLApp/GLToggle.h"
#include "GLApp/GLTextField.h"
#include "GLApp/MathTools.h" //IsEqual
#include "GLApp/GLButton.h"

#include "Geometry_shared.h"

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
SelectTextureType::SelectTextureType(Worker *w) :GLWindow() {

	int xD, yD, wD, hD;
	wD = 280; hD = 210;
	SetTitle("Select facets by texture properties");

	resolutionpanel = new GLTitledPanel("Texture resolution");
	resolutionpanel->SetBounds(5, 5, wD - 10, 70);
	resolutionpanel->SetClosable(false);
	Add(resolutionpanel);

	ratioToggle = new GLToggle(0, "Exactly                   /cm");
	resolutionpanel->Add(ratioToggle);
	resolutionpanel->SetCompBounds(ratioToggle, 5, 16, 150, 20);

	ratioText = new GLTextField(0, "");
	resolutionpanel->Add(ratioText);
	resolutionpanel->SetCompBounds(ratioText, 70, 15, 60, 19);

	ratioMinMaxToggle = new GLToggle(1, "Between                 /cm and                  /cm");
	resolutionpanel->Add(ratioMinMaxToggle);
	resolutionpanel->SetCompBounds(ratioMinMaxToggle, 5, 42, 250, 20);

	ratioMinText = new GLTextField(0, "");
	resolutionpanel->Add(ratioMinText);
	resolutionpanel->SetCompBounds(ratioMinText, 70, 42, 60, 19);

	ratioMaxText = new GLTextField(0, "");
	resolutionpanel->Add(ratioMaxText);
	resolutionpanel->SetCompBounds(ratioMaxText, 180, 42, 60, 19);

	textureTypePanel = new GLTitledPanel("Texture type");
	textureTypePanel->SetBounds(5, 80, wD - 10, 80);
	textureTypePanel->SetClosable(false);
	Add(textureTypePanel);

	desorbToggle = new GLToggle(0, "Count desorbtion");
	desorbToggle->AllowMixedState(true);
	desorbToggle->SetState(2);
	textureTypePanel->Add(desorbToggle);
	textureTypePanel->SetCompBounds(desorbToggle, 5, 15, 90, 18);

	absorbToggle = new GLToggle(0, "Count absorbtion");
	absorbToggle->AllowMixedState(true);
	absorbToggle->SetState(2);
	textureTypePanel->Add(absorbToggle);
	textureTypePanel->SetCompBounds(absorbToggle, 5, 35, 90, 18);

	reflectToggle = new GLToggle(0, "Count reflection");
	reflectToggle->AllowMixedState(true);
	reflectToggle->SetState(2);
	textureTypePanel->Add(reflectToggle);
	textureTypePanel->SetCompBounds(reflectToggle, 130, 15, 90, 18);

	transparentToggle = new GLToggle(0, "Count transp. pass");
	transparentToggle->AllowMixedState(true);
	transparentToggle->SetState(2);
	textureTypePanel->Add(transparentToggle);
	textureTypePanel->SetCompBounds(transparentToggle, 130, 35, 80, 18);

	directionToggle = new GLToggle(0, "Count direction");
	directionToggle->AllowMixedState(true);
	directionToggle->SetState(2);
	textureTypePanel->Add(directionToggle);
	textureTypePanel->SetCompBounds(directionToggle, 130, 55, 80, 18);

	// Buttons
	int startX = 5;
	int startY = 165;

	selectButton = new GLButton(0, "Select");
	selectButton->SetBounds(startX, startY, 75, 20);
	Add(selectButton);
	startX += 80;

	addSelectButton = new GLButton(0, "Add to sel.");
	addSelectButton->SetBounds(startX, startY, 95, 20);
	Add(addSelectButton);
	startX += 100;

	remSelectButton = new GLButton(GLDLG_SELECT_REM, "Remove from sel.");
	remSelectButton->SetBounds(startX, startY, 90, 20);
	Add(remSelectButton);

	// Center dialog
	int wS, hS;
	GLToolkit::GetScreenSize(&wS, &hS);
	if (wD > wS) wD = wS;
	xD = (wS - wD) / 2;
	yD = (hS - hD) / 2;
	SetBounds(xD, yD, wD, hD);

	// Create objects
	RestoreDeviceObjects();

	work = w;
	geom = w->GetGeometry();
}

void SelectTextureType::ProcessMessage(GLComponent *src, int message) {
	if (message == MSG_BUTTON) {
		double ratio, minRatio, maxRatio;
		bool exactRatio = false;
		bool minmaxRatio = false;
		if (ratioToggle->GetState()) {
			if (!ratioText->GetNumber(&ratio) || ratio <= 0.0) {
				GLMessageBox::Display("Invalid ratio", "Error", GLDLG_OK, GLDLG_ICONINFO);
				return;
			}
			exactRatio = true;
		}
		else if (ratioMinMaxToggle->GetState()) {
			if (!ratioMinText->GetNumber(&minRatio) || minRatio <= 0.0) {
				GLMessageBox::Display("Invalid min. ratio", "Error", GLDLG_OK, GLDLG_ICONINFO);
				return;
			}
			if (!ratioMaxText->GetNumber(&maxRatio) || maxRatio <= 0.0) {
				GLMessageBox::Display("Invalid max. ratio", "Error", GLDLG_OK, GLDLG_ICONINFO);
				return;
			}
			if (maxRatio <= minRatio) {
				GLMessageBox::Display("Max. ratio must be larger than min. ratio", "Error", GLDLG_OK, GLDLG_ICONINFO);
				return;
			}
			minmaxRatio = true;
		}
		if (src == selectButton) geom->UnselectAll();
		for (size_t i = 0; i < geom->GetNbFacet(); i++) {
			Facet* f = geom->GetFacet(i);
			bool match = f->sh.isTextured;
			if (exactRatio) match = match && IsEqual(ratio, f->tRatio);
			if (minmaxRatio) match = match && (minRatio <= f->tRatio) && (f->tRatio <= maxRatio);
#ifdef MOLFLOW
			if (desorbToggle->GetState() != 2) match = match && f->sh.countDes;
#endif
			if (absorbToggle->GetState() != 2) match = match && (absorbToggle->GetState()==1) == f->sh.countAbs;
			if (reflectToggle->GetState() != 2) match = match && (reflectToggle->GetState()==1) == f->sh.countRefl;
			if (transparentToggle->GetState() != 2) match = match && (transparentToggle->GetState()==1) == f->sh.countTrans;
			if (directionToggle->GetState() != 2) match = match && (directionToggle->GetState()==1) && f->sh.countDirection;

			if (match) f->selected = (src != remSelectButton);
		}
		geom->UpdateSelection();
		mApp->UpdateFacetParams(true);
		mApp->UpdateFacetlistSelected();
	}
	else if (message == MSG_TOGGLE) {
		if (src == ratioToggle) ratioMinMaxToggle->SetState(false);
		if (src == ratioMinMaxToggle) ratioToggle->SetState(false);
	}
	else if (message == MSG_TEXT_UPD) {
		if (src == ratioText) {
			ratioToggle->SetState(1);
			ratioMinMaxToggle->SetState(0);
		}
		else {
			ratioToggle->SetState(0);
			ratioMinMaxToggle->SetState(1);
		}
	}
	GLWindow::ProcessMessage(src, message);
}