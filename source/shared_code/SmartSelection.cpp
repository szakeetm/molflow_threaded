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
#include "SmartSelection.h"
#include "GLApp/GLTitledPanel.h"
#include "GLApp/GLToolkit.h"
#include "GLApp/GLWindowManager.h"
#include "GLApp/GLMessageBox.h"
#include "GLApp/GLButton.h"
#include "GLApp/GLTextField.h"
#include "GLApp/GLLabel.h"
#include "GLApp/GLToggle.h"

#include "Geometry_shared.h"
//#include "Worker.h"

#ifdef MOLFLOW
#include "MolFlow.h"
#endif

#ifdef SYNRAD
#include "SynRad.h"
#endif

extern GLApplication *theApp;

#ifdef MOLFLOW
extern MolFlow *mApp;
#endif

#ifdef SYNRAD
extern SynRad*mApp;
#endif

SmartSelection::SmartSelection(Geometry *g,Worker *w):GLWindow() {

	int wD = 270;
	int hD = 125;

	SetTitle("Smart selection");

	analyzeButton = new GLButton(0,"Analyze");
	analyzeButton->SetBounds(90,5,100,20);
	Add(analyzeButton);
	
	GLLabel *l1 = new GLLabel("Max. plane diff. between neighbors (deg):");
	l1->SetBounds(5, 30, 200, 20);
	Add(l1);

	angleThreshold = new GLTextField(0,"30");
	angleThreshold->SetBounds(210,30,40,18);
	Add(angleThreshold);

	resultLabel = new GLLabel("No neighborhood analysis yet.");
	resultLabel->SetBounds(50,55,150,20);
	Add(resultLabel);

	enableToggle = new GLToggle(0,"Enable smart selection");
	enableToggle->SetBounds(55,80,170,18);
	enableToggle->SetState(false);
	enableToggle->SetEnabled(false);
	Add(enableToggle);

	SetBounds(10,60,wD,hD);

	RestoreDeviceObjects();

	geom = g;
	work = w;
	isRunning = false;
}

bool SmartSelection::IsSmartSelection()
{
	if (!IsVisible()) return false;
	else return (enableToggle->GetState()==1);
}

double SmartSelection::GetMaxAngle()
{
	if (!IsVisible()) return -1.0;
	double maxAngleDiff;
	if (!angleThreshold->GetNumber(&maxAngleDiff) || !(maxAngleDiff>=0.0)) {
		GLMessageBox::Display("Invalid angle threshold in Smart Selection dialog\nMust be a non-negative number.", "Error", GLDLG_OK, GLDLG_ICONERROR);
		return -1.0;
	}
	return maxAngleDiff/180.0*3.14159;
}

void SmartSelection::ProcessMessage(GLComponent *src,int message) {
	

	switch(message) {
	case MSG_BUTTON:

		if (src==analyzeButton) {
			if (!isRunning) {
				if (!geom->IsLoaded()) {
					GLMessageBox::Display("Geometry not loaded yet", "Error", GLDLG_OK, GLDLG_ICONERROR);
					return;
				}
				analyzeButton->SetText("Stop analyzing");
				isRunning = true;
				GLProgress *progressDlg = new GLProgress("Analyzing facets", "Please wait");
				progressDlg->SetProgress(0.0);
				progressDlg->SetVisible(true);

				size_t nbAnalyzed = geom->AnalyzeNeighbors(work,progressDlg);

				progressDlg->SetVisible(false);
				SAFE_DELETE(progressDlg);
				analyzeButton->SetText("Analyze");
				isRunning = false;
				std::stringstream tmp;
				tmp << "Analyzed " << nbAnalyzed << " facets.";
				resultLabel->SetText(tmp.str().c_str());
				enableToggle->SetEnabled(true);
				enableToggle->SetState(1);
			}
			else {
				analyzeButton->SetText("Analyze");
				isRunning = false;
				work->abortRequested = true;
			}
		}
		break;
	}

	GLWindow::ProcessMessage(src,message);
}

