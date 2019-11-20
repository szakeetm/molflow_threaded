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
#include "GLApp/GLToggle.h"

#include "CollapseSettings.h"
#include "GLApp/GLTextField.h"
#include "GLApp/GLTitledPanel.h"
#include "GLApp/GLLabel.h"
#include "GLApp/GLTextField.h"
#include "GLApp/GLButton.h"
#include "GLApp/GLToolkit.h"
#include "GLApp/GLWindowManager.h"
#include "GLApp/GLMessageBox.h"
#include "VertexCoordinates.h"
#include "FacetCoordinates.h"
#include "ProfilePlotter.h"
#include "HistogramPlotter.h"
#include "Geometry_shared.h"

#ifdef MOLFLOW
#include "MolFlow.h"
#include "TimewisePlotter.h"
#include "PressureEvolution.h"
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
* \brief Constructor with initialisation for the CollapseSettings window (Facet/Collapse)
*/
CollapseSettings::CollapseSettings():GLWindow() {

	int wD = 270;
	int hD = 225;

	SetTitle("Collapse Settings");

	l1 = new GLToggle(0,"Vertices closer than (cm):");
	l1->SetBounds(5,5,170,18);
	l1->SetState(true);
	Add(l1);

	vThreshold = new GLTextField(0,"1E-5");
	vThreshold->SetBounds(185,5,80,18);
	Add(vThreshold);

	l2 = new GLToggle(0,"Facets more coplanar than:");
	l2->SetBounds(5,30,170,18);
	l2->SetState(true);
	Add(l2);

	pThreshold = new GLTextField(0,"1E-5");
	pThreshold->SetBounds(185,30,80,18);
	Add(pThreshold);

	l3 = new GLToggle(0,"Sides more collinear than (degrees):");
	l3->SetBounds(5,55,170,18);
	l3->SetState(true);
	Add(l3);

	lThreshold = new GLTextField(0,"1E-3");
	lThreshold->SetBounds(185,55,80,18);
	Add(lThreshold);

	GLTitledPanel *panel = new GLTitledPanel("Optimisation results");
	panel->SetBounds(5,80,wD-10,hD-105);
	Add(panel);

	resultLabel = new GLLabel("");
	resultLabel->SetBounds(10,95,wD-20,hD-125);
	Add(resultLabel);

	goButton = new GLButton(0,"Collapse");
	goButton->SetBounds(wD-265,hD-44,85,21);
	Add(goButton);

	goSelectedButton = new GLButton(0,"Collapse Selected");
	goSelectedButton->SetBounds(wD-175,hD-44,105,21);
	Add(goSelectedButton);

	cancelButton = new GLButton(0,"Dismiss");
	cancelButton->SetBounds(wD-65,hD-44,60,21);
	Add(cancelButton);

	// Center dialog
	int wS,hS;
	GLToolkit::GetScreenSize(&wS,&hS);
	int xD = (wS-wD)/2;
	int yD = (hS-hD)/2;
	SetBounds(xD,yD,wD,hD);

	RestoreDeviceObjects();

	isRunning = false;
	geom = NULL;

}

/**
* \brief Constructor with initialisation for the CollapseSettings window (Facet/Collapse)
* \param geom geometry used for the settings
* \brief w Worker handle
*/
void CollapseSettings::SetGeometry(Geometry *geom,Worker *w) {

	char tmp[512];

	this->geom = geom;
	work = w;

	nbVertexS = geom->GetNbVertex();
	nbFacetS = geom->GetNbFacet();
	nbFacetSS = geom->GetNbSelectedFacets();

	sprintf(tmp,"Selected: %zd\nVertex:    %zd\nFacet:     %zd",
		geom->GetNbSelectedFacets(),geom->GetNbVertex(),geom->GetNbFacet());
	resultLabel->SetText(tmp);

}

/**
* \brief Function for processing various inputs (button, check boxes etc.)
* \param src Exact source of the call
* \param message Type of the source (button)
*/
void CollapseSettings::ProcessMessage(GLComponent *src,int message) {
	double vT,fT,lT;

	switch(message) {
	case MSG_BUTTON:

		if(src==cancelButton) {

			GLWindow::ProcessMessage(NULL,MSG_CLOSE);

		} else if (src==goButton || src==goSelectedButton) {
			if (!isRunning) {
				if (!vThreshold->GetNumber(&vT) || !(vT > 0.0)) {
					GLMessageBox::Display("Invalid vertex distance value.\nMust be a positive number.", "Error", GLDLG_OK, GLDLG_ICONERROR);
					return;
				}
				if (!pThreshold->GetNumber(&fT) || !(fT > 0.0)) {
					GLMessageBox::Display("Invalid planarity threshold value.\nMust be a positive number.", "Error", GLDLG_OK, GLDLG_ICONERROR);
					return;
				}
				if (!lThreshold->GetNumber(&lT) || !(lT > 0.0)) {
					GLMessageBox::Display("Invalid linearity threshold value.\nMust be a positive number.", "Error", GLDLG_OK, GLDLG_ICONERROR);
					return;
				}
				if (!mApp->AskToReset(work)) return;
				GLProgress *progressDlg = new GLProgress("Collapse", "Please wait");
				progressDlg->SetProgress(0.0);
				progressDlg->SetVisible(true);
				if (!l1->GetState()) vT = 0.0;
				if (!l2->GetState()) fT = 0.0;
				if (!l3->GetState()) lT = 0.0;

				((GLButton*)src)->SetText("Stop collapse");
				isRunning = true;

				geom->Collapse(vT, fT, lT, (src == goSelectedButton), work,progressDlg);

				if (src == goButton) goButton->SetText("Collapse");
				else if (src == goSelectedButton) goSelectedButton->SetText("Collapse selected");
				isRunning = false;

				geom->CheckCollinear();
				//geom->CheckNonSimple();
				geom->CheckIsolatedVertex();

				mApp->UpdateModelParams();
				if (mApp->vertexCoordinates) mApp->vertexCoordinates->Update();
				if (mApp->facetCoordinates) mApp->facetCoordinates->UpdateFromSelection();
				if (mApp->profilePlotter) mApp->profilePlotter->Refresh();
				if (mApp->histogramPlotter) mApp->histogramPlotter->Refresh();
#ifdef MOLFLOW
				if (mApp->pressureEvolution) mApp->pressureEvolution->Refresh();
				if (mApp->timewisePlotter) mApp->timewisePlotter->Refresh();
#endif
#ifdef SYNRAD
				if (mApp->spectrumPlotter) mApp->spectrumPlotter->Refresh();
#endif
				// Send to sub process
				try { work->Reload(); }
				catch (Error &e) {
					GLMessageBox::Display(e.GetMsg(), "Error reloading worker", GLDLG_OK, GLDLG_ICONERROR);
				}

				progressDlg->SetVisible(false);
				SAFE_DELETE(progressDlg);

				// Update result
				char tmp[512];
				sprintf(tmp, "Selected: %zd\nVertex:    %zd/%zd\nFacet:    %zd/%zd\n\nLast action: Collapse all",
					geom->GetNbSelectedFacets(), geom->GetNbVertex(), nbVertexS, geom->GetNbFacet(), nbFacetS);
				resultLabel->SetText(tmp);

				GLWindowManager::FullRepaint();
			}
			else {
				if (src == goButton) goButton->SetText("Collapse");
				else if (src == goSelectedButton) goSelectedButton->SetText("Collapse selected");
				isRunning = false;
				work->abortRequested = true;
			}
		}
		break;
	}

	GLWindow::ProcessMessage(src,message);
}

