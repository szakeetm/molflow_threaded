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
#include "MoveVertex.h"
#include "GLApp/GLTitledPanel.h"
#include "GLApp/GLToolkit.h"
#include "GLApp/GLWindowManager.h"
#include "GLApp/GLMessageBox.h"

#include "GLApp/GLButton.h"
#include "GLApp/GLTextField.h"
#include "GLApp/GLLabel.h"
#include "GLApp/GLToggle.h"

#include "Geometry_shared.h"
#include "Facet_shared.h"

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

MoveVertex::MoveVertex(Geometry *g, Worker *w) :GLWindow() {

	int wD = 228;
	int hD = 365;
	directionPanel = new GLTitledPanel("In direction");
	directionPanel->SetBounds(4, 111, 216, 187);
	Add(directionPanel);
	dirPanel = new GLTitledPanel("Direction");
	directionPanel->SetCompBounds(dirPanel, 110, 92, 100, 89);
	directionPanel->Add(dirPanel);
	basePanel = new GLTitledPanel("Base");
	directionPanel->SetCompBounds(basePanel, 8, 92, 100, 89);
	directionPanel->Add(basePanel);
	offsetCheckbox = new GLToggle(0, "Absolute offset");
	offsetCheckbox->SetBounds(5, 6, 96, 17);
	Add(offsetCheckbox);

	directionCheckBox = new GLToggle(0, "Direction and distance");
	directionCheckBox->SetBounds(100, 6, 125, 17);
	Add(directionCheckBox);

	distanceText = new GLTextField(0, "");
	directionPanel->SetCompBounds(distanceText, 63, 23, 98, 20);
	directionPanel->Add(distanceText);

	dxLabel = new GLLabel("dX");
	dxLabel->SetBounds(8, 32, 20, 13);
	Add(dxLabel);

	xText = new GLTextField(0, "0");
	xText->SetBounds(49, 29, 116, 20);
	Add(xText);

	cmLabelX = new GLLabel("cm");
	cmLabelX->SetBounds(171, 32, 21, 13);
	Add(cmLabelX);

	cmLabelY = new GLLabel("cm");
	cmLabelY->SetBounds(171, 58, 21, 13);
	Add(cmLabelY);

	yText = new GLTextField(0, "0");
	yText->SetBounds(49, 55, 116, 20);
	Add(yText);

	dyLabel = new GLLabel("dY");
	dyLabel->SetBounds(8, 58, 20, 13);
	Add(dyLabel);

	cmLabelZ = new GLLabel("cm");
	cmLabelZ->SetBounds(171, 84, 21, 13);
	Add(cmLabelZ);

	zText = new GLTextField(0, "0");
	zText->SetBounds(49, 81, 116, 20);
	Add(zText);

	dzLabel = new GLLabel("dZ");
	dzLabel->SetBounds(8, 84, 20, 13);
	Add(dzLabel);

	dirFacetCenterButton = new GLButton(0, "Facet center");
	dirPanel->SetCompBounds(dirFacetCenterButton, 6, 60, 84, 20);
	dirPanel->Add(dirFacetCenterButton);

	dirVertexButton = new GLButton(0, "Selected Vertex");
	dirPanel->SetCompBounds(dirVertexButton, 6, 31, 84, 20);
	dirPanel->Add(dirVertexButton);

	baseFacetCenterButton = new GLButton(0, "Facet center");
	basePanel->SetCompBounds(baseFacetCenterButton, 6, 60, 84, 20);
	basePanel->Add(baseFacetCenterButton);

	baseVertexButton = new GLButton(0, "Selected Vertex");
	basePanel->SetCompBounds(baseVertexButton, 6, 31, 84, 20);
	basePanel->Add(baseVertexButton);

	facetNormalButton = new GLButton(0, "Facet normal");
	directionPanel->SetCompBounds(facetNormalButton, 62, 56, 99, 20);
	directionPanel->Add(facetNormalButton);

	label4 = new GLLabel("cm");
	directionPanel->SetCompBounds(label4, 167, 26, 21, 13);
	directionPanel->Add(label4);

	label1 = new GLLabel("Distance:");
	directionPanel->SetCompBounds(label1, 8, 26, 52, 13);
	directionPanel->Add(label1);

	directionStatusLabel = new GLLabel("Choose base first");
	dirPanel->SetCompBounds(directionStatusLabel, 9, 14, 88, 13);
	dirPanel->Add(directionStatusLabel);

	baseStatusLabel = new GLLabel("base status");
	basePanel->SetCompBounds(baseStatusLabel, 7, 14, 61, 13);
	basePanel->Add(baseStatusLabel);

	copyButton = new GLButton(0, "Copy vertices");
	copyButton->SetBounds(120, 314, 84, 20);
	Add(copyButton);

	moveButton = new GLButton(0, "Move vertices");
	moveButton->SetBounds(18, 314, 84, 20);
	Add(moveButton);

	SetTitle("Move vertex");
	// Center dialog
	int wS, hS;
	GLToolkit::GetScreenSize(&wS, &hS);
	int xD = (wS - wD) / 2;
	int yD = (hS - hD) / 2;
	SetBounds(xD, yD, wD, hD);

	RestoreDeviceObjects();

	baseStatusLabel->SetText("");

	//Set absolute offset mode
	offsetCheckbox->SetState(1);
	directionCheckBox->SetState(0);
	distanceText->SetEditable(false);

	geom = g;
	work = w;
	dirVertexButton->SetEnabled(false);
	dirFacetCenterButton->SetEnabled(false);

}

void MoveVertex::ProcessMessage(GLComponent *src, int message) {
	double dX, dY, dZ, distance;

	switch (message) {
	case MSG_BUTTON:

		if (src == moveButton || src == copyButton) {
			if (geom->GetNbSelectedVertex() == 0) {
				GLMessageBox::Display("No vertices selected", "Nothing to move", GLDLG_OK, GLDLG_ICONERROR);
				return;
			}
			if (!xText->GetNumber(&dX)) {
				GLMessageBox::Display("Invalid X offset/direction", "Error", GLDLG_OK, GLDLG_ICONERROR);
				return;
			}
			if (!yText->GetNumber(&dY)) {
				GLMessageBox::Display("Invalid Y offset/direction", "Error", GLDLG_OK, GLDLG_ICONERROR);
				return;
			}
			if (!zText->GetNumber(&dZ)) {
				GLMessageBox::Display("Invalid Z offset/direction", "Error", GLDLG_OK, GLDLG_ICONERROR);
				return;
			}
			bool towardsDirectionMode = directionCheckBox->GetState() == 1;
			if (towardsDirectionMode && !distanceText->GetNumber(&distance)) {
				GLMessageBox::Display("Invalid offset distance", "Error", GLDLG_OK, GLDLG_ICONERROR);
				return;
			}
			if (towardsDirectionMode && dX == 0.0 && dY == 0.0 && dZ == 0.0) {
				GLMessageBox::Display("Direction can't be null-vector", "Error", GLDLG_OK, GLDLG_ICONERROR);
				return;
			}

			if (mApp->AskToReset()) {

				geom->MoveSelectedVertex(dX, dY, dZ, towardsDirectionMode, distance, src == copyButton);
				work->Reload();
				mApp->changedSinceSave = true;
				mApp->UpdateFacetlistSelected();
				mApp->UpdateViewers();
			}
		}
		else if (src == facetNormalButton) {
			auto selFacets = geom->GetSelectedFacets();
			if (selFacets.size() != 1) {
				GLMessageBox::Display("Select exactly one facet", "Error", GLDLG_OK, GLDLG_ICONERROR);
				return;
			}
			Vector3d facetNormal = geom->GetFacet(selFacets[0])->sh.N;
			xText->SetText(facetNormal.x);
			yText->SetText(facetNormal.y);
			zText->SetText(facetNormal.z);

			//Switch to direction mode
			offsetCheckbox->SetState(0);
			directionCheckBox->SetState(1);
			distanceText->SetEditable(true);
		}
		else if (src == baseVertexButton) {
			auto selVertices = geom->GetSelectedVertices();
			if (selVertices.size() != 1) {
				GLMessageBox::Display("Select exactly one vertex", "Error", GLDLG_OK, GLDLG_ICONERROR);
				return;
			}
			baseLocation = (Vector3d)*(geom->GetVertex(selVertices[0]));
			std::stringstream tmp; tmp << "Vertex " << selVertices[0] + 1;
			baseStatusLabel->SetText(tmp.str());
			dirFacetCenterButton->SetEnabled(true);
			dirVertexButton->SetEnabled(true);
		}
		else if (src == baseFacetCenterButton) {
			auto selFacets = geom->GetSelectedFacets();
			if (selFacets.size() != 1) {
				GLMessageBox::Display("Select exactly one facet", "Error", GLDLG_OK, GLDLG_ICONERROR);
				return;
			}
			baseLocation = geom->GetFacet(selFacets[0])->sh.center;
			std::stringstream tmp; tmp << "Center of facet " << selFacets[0] + 1;
			baseStatusLabel->SetText(tmp.str());
			dirFacetCenterButton->SetEnabled(true);
			dirVertexButton->SetEnabled(true);
		}
		else if (src == dirVertexButton) { //only enabled once base is defined
			auto selVertices = geom->GetSelectedVertices();
			if (selVertices.size() != 1) {
				GLMessageBox::Display("Select exactly one vertex", "Error", GLDLG_OK, GLDLG_ICONERROR);
				return;
			}
			Vector3d translation = *(geom->GetVertex(selVertices[0])) - baseLocation;

			xText->SetText(translation.x);
			yText->SetText(translation.y);
			zText->SetText(translation.z);
			distanceText->SetText(translation.Norme());

			std::stringstream tmp; tmp << "Vertex " << selVertices[0] + 1;
			directionStatusLabel->SetText(tmp.str());
		}
		else if (src == dirFacetCenterButton) { //only enabled once base is defined
			auto selFacets = geom->GetSelectedFacets();
			if (selFacets.size() != 1) {
				GLMessageBox::Display("Select exactly one facet", "Error", GLDLG_OK, GLDLG_ICONERROR);
				return;
			}
			Vector3d translation = geom->GetFacet(selFacets[0])->sh.center - baseLocation;

			xText->SetText(translation.x);
			yText->SetText(translation.y);
			zText->SetText(translation.z);
			distanceText->SetText(translation.Norme());

			std::stringstream tmp; tmp << "Center of facet " << selFacets[0] + 1;
			directionStatusLabel->SetText(tmp.str());
		}
		break;
	case MSG_TOGGLE:
		bool towardsDirectionMode = src == directionCheckBox;
		offsetCheckbox->SetState((int)(!towardsDirectionMode));
		directionCheckBox->SetState((int)towardsDirectionMode);
		distanceText->SetEditable(towardsDirectionMode);
		dxLabel->SetText(towardsDirectionMode ? "dir.X" : "dX");
		dyLabel->SetText(towardsDirectionMode ? "dir.Y" : "dY");
		dzLabel->SetText(towardsDirectionMode ? "dir.Z" : "dZ");
	}

	GLWindow::ProcessMessage(src, message);
}

