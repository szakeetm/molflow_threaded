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
#define XYMODE 0
#define XZMODE 1
#define YZMODE 2
#define FACETNMODE 3
#define THREEVERTEXMODE 4
#define ABCDMODE 5

#include "MirrorFacet.h"
#include "Facet_shared.h"
#include "GLApp/GLTitledPanel.h"
#include "GLApp/GLToolkit.h"
#include "GLApp/GLWindowManager.h"
#include "GLApp/GLMessageBox.h"

#include "GLApp/GLButton.h"
#include "GLApp/GLTextField.h"
#include "GLApp/GLLabel.h"
#include "GLApp/GLToggle.h"
#include "GLApp/GLTitledPanel.h"

#ifdef MOLFLOW
#include "MolFlow.h"
#include "SuperFacet.h"

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

MirrorFacet::MirrorFacet(Geometry *g, Worker *w) :GLWindow() {

	int wD = 300;
	int hD = 275;

	SetTitle("Mirror/project selected facets");

	iPanel = new GLTitledPanel("Plane definiton mode");
	iPanel->SetBounds(5, 5, wD - 10, 190);
	Add(iPanel);

	l1 = new GLToggle(0, "XY plane");
	l1->SetBounds(10, 20, 100, 18);
	iPanel->Add(l1);

	l2 = new GLToggle(0, "YZ plane");
	l2->SetBounds(10, 45, 100, 18);
	iPanel->Add(l2);

	l3 = new GLToggle(0, "XZ plane");
	l3->SetBounds(10, 70, 100, 18);
	iPanel->Add(l3);

	l4 = new GLToggle(0, "Plane of facet #");
	l4->SetBounds(10, 95, 100, 18);
	iPanel->Add(l4);

	facetNumber = new GLTextField(0, "0");
	facetNumber->SetBounds(110, 95, 55, 18);
	facetNumber->SetEditable(false);
	iPanel->Add(facetNumber);

	getSelFacetButton = new GLButton(0, "<-Get selected");
	getSelFacetButton->SetBounds(170, 95, 80, 18);
	iPanel->Add(getSelFacetButton);

	l5 = new GLToggle(0, "Define by 3 selected vertex");
	l5->SetBounds(10, 120, 100, 18);
	iPanel->Add(l5);

	l6 = new GLToggle(0, "Define by plane equation:");
	l6->SetBounds(10, 145, 100, 18);
	iPanel->Add(l6);

	aText = new GLTextField(0, "0");
	aText->SetBounds(15, 170, 30, 18);
	aText->SetEditable(false);
	iPanel->Add(aText);

	aLabel = new GLLabel("*X +");
	aLabel->SetBounds(50, 170, 20, 18);
	iPanel->Add(aLabel);

	bText = new GLTextField(0, "0");
	bText->SetBounds(75, 170, 30, 18);
	bText->SetEditable(false);
	iPanel->Add(bText);

	bLabel = new GLLabel("*Y +");
	bLabel->SetBounds(110, 170, 20, 18);
	iPanel->Add(bLabel);

	cText = new GLTextField(0, "0");
	cText->SetBounds(135, 170, 30, 18);
	cText->SetEditable(false);
	iPanel->Add(cText);

	cLabel = new GLLabel("*Z +");
	cLabel->SetBounds(170, 170, 20, 18);
	iPanel->Add(cLabel);

	dText = new GLTextField(0, "0");
	dText->SetBounds(195, 170, 30, 18);
	dText->SetEditable(false);
	iPanel->Add(dText);

	dLabel = new GLLabel("= 0");
	dLabel->SetBounds(230, 170, 20, 18);
	iPanel->Add(dLabel);

	mirrorButton = new GLButton(0, "Mirror facet");
	mirrorButton->SetBounds(5, hD - 72, 90, 21);
	Add(mirrorButton);

	mirrorCopyButton = new GLButton(0, "Copy mirror facet");
	mirrorCopyButton->SetBounds(100, hD - 72, 95, 21);
	Add(mirrorCopyButton);

	projectButton = new GLButton(0, "Project facet");
	projectButton->SetBounds(5, hD - 45, 90, 21);
	Add(projectButton);

	projectCopyButton = new GLButton(0, "Copy project facet");
	projectCopyButton->SetBounds(100, hD - 45, 95, 21);
	Add(projectCopyButton);
	
	undoProjectButton = new GLButton(0, "Undo projection");
	undoProjectButton->SetBounds(200, hD - 45, 95, 21);
	undoProjectButton->SetEnabled(false);
	Add(undoProjectButton);

	/*cancelButton = new GLButton(0,"Dismiss");
	cancelButton->SetBounds(185,hD-44,85,21);
	Add(cancelButton);*/

	// Center dialog
	int wS, hS;
	GLToolkit::GetScreenSize(&wS, &hS);
	int xD = (wS - wD) / 2;
	int yD = (hS - hD) / 2;
	SetBounds(xD, yD, wD, hD);

	RestoreDeviceObjects();

	geom = g;
	work = w;
	planeMode = -1;
}

void MirrorFacet::ClearUndoVertices() {
	undoPoints.clear();
	undoProjectButton->SetEnabled(false);
}

void MirrorFacet::ProcessMessage(GLComponent *src, int message) {
	double a, b, c, d;
	int facetNum;

	switch (message) {
		
	case MSG_TOGGLE:
		UpdateToggle(src);
		break;

	case MSG_BUTTON:

		if (src == cancelButton) {

			GLWindow::ProcessMessage(NULL, MSG_CLOSE);

		}
		else if (src == mirrorButton || src == mirrorCopyButton || src == projectButton || src == projectCopyButton) {
			if (geom->GetNbSelectedFacets() == 0) {
				GLMessageBox::Display("No facets selected", "Nothing to mirror", GLDLG_OK, GLDLG_ICONERROR);
				return;
			}
			//Calculate the plane
			Vector3d P0, N;
			double nN2;
			
			switch (planeMode) {
			case XYMODE:
				P0.x = 0.0; P0.y = 0.0; P0.z = 0.0;
				N.x = 0.0; N.y = 0.0; N.z = 1.0;
				break;
			case XZMODE:
				P0.x = 0.0; P0.y = 0.0; P0.z = 0.0;
				N.x = 0.0; N.y = 1.0; N.z = 0.0;
				break;
			case YZMODE:
				P0.x = 0.0; P0.y = 0.0; P0.z = 0.0;
				N.x = 1.0; N.y = 0.0; N.z = 0.0;
				break;
			case FACETNMODE:
				if (!(facetNumber->GetNumberInt(&facetNum)) || facetNum<1 || facetNum>geom->GetNbFacet()) {
					GLMessageBox::Display("Invalid facet number", "Error", GLDLG_OK, GLDLG_ICONERROR);
					return;
				}
				P0 = *geom->GetVertex(geom->GetFacet(facetNum - 1)->indices[0]);
				N = geom->GetFacet(facetNum - 1)->sh.N;
				break;
			case THREEVERTEXMODE:
			{
				if (geom->GetNbSelectedVertex() != 3) {
					GLMessageBox::Display("Select exactly 3 vertices", "Can't define plane", GLDLG_OK, GLDLG_ICONERROR);
					return;
				}
				int v1Id = -1;
				int v2Id = -1;
				int v3Id = -1;

				for (int i = 0; v3Id == -1 && i < geom->GetNbVertex(); i++) {
					if (geom->GetVertex(i)->selected) {
						if (v1Id == -1) v1Id = i;
						else if (v2Id == -1) v2Id = i;
						else v3Id = i;
					}
				}

				Vector3d U2, V2, N2;
				U2 = (*geom->GetVertex(v1Id) - *geom->GetVertex(v2Id)).Normalized();
				V2 = (*geom->GetVertex(v1Id) - *geom->GetVertex(v3Id)).Normalized();
				N2 = CrossProduct(V2, U2);
				nN2 = N2.Norme();
				if (nN2 < 1e-8) {
					GLMessageBox::Display("The 3 selected vertices are on a line.", "Can't define plane", GLDLG_OK, GLDLG_ICONERROR);
					return;
				}
				N2 = 1.0 / nN2 * N2; // Normalize N2
				N = N2;
				P0 = *geom->GetVertex(v1Id);
				break;
			}
			case ABCDMODE:
				if (!(aText->GetNumber(&a))) {
					GLMessageBox::Display("Invalid A coefficient", "Error", GLDLG_OK, GLDLG_ICONERROR);
					return;
				}
				if (!(bText->GetNumber(&b))) {
					GLMessageBox::Display("Invalid B coefficient", "Error", GLDLG_OK, GLDLG_ICONERROR);
					return;
				}
				if (!(cText->GetNumber(&c))) {
					GLMessageBox::Display("Invalid C coefficient", "Error", GLDLG_OK, GLDLG_ICONERROR);
					return;
				}
				if (!(dText->GetNumber(&d))) {
					GLMessageBox::Display("Invalid D coefficient", "Error", GLDLG_OK, GLDLG_ICONERROR);
					return;
				}
				if ((a == 0.0) && (b == 0.0) && (c == 0.0) && (d == 0.0)) {
					GLMessageBox::Display("A, B, C are all zero. That's not a plane.", "Error", GLDLG_OK, GLDLG_ICONERROR);
					return;
				}
				N.x = a; N.y = b; N.z = c;
				P0.x = 0.0; P0.y = 0; P0.z = 0;
				if (!a == 0) P0.x = -d / a;
				else if (!b == 0) P0.y = -d / b;
				else if (!c == 0) P0.z = -d / c;
				break;
			default:
				GLMessageBox::Display("Select a plane definition mode.", "Error", GLDLG_OK, GLDLG_ICONERROR);
				return;
			}
			
			if (mApp->AskToReset()) {
				undoPoints = geom->MirrorProjectSelectedFacets(P0, N,
					(src == projectButton) || (src == projectCopyButton),
					(src == mirrorCopyButton) || (src == projectCopyButton), work);
				undoProjectButton->SetEnabled(src == projectButton);
				//theApp->UpdateModelParams();
				work->Reload();
				mApp->UpdateFacetlistSelected();
				mApp->UpdateViewers();
				//GLWindowManager::FullRepaint();
			}
		}
		else if (src == getSelFacetButton) {
			if (geom->GetNbSelectedFacets() != 1) {
				GLMessageBox::Display("Select exactly one facet.", "Error", GLDLG_OK, GLDLG_ICONERROR);
				return;
			}
			int selFacetId = -1;
			for (int i = 0; selFacetId == -1 && i < geom->GetNbFacet(); i++) {
				if (geom->GetFacet(i)->selected) {
					selFacetId = i;
				}
			}
			facetNumber->SetText(selFacetId + 1);
			UpdateToggle(l4);
		}
		else if (src == undoProjectButton) {
			if (!mApp->AskToReset(work)) return;
			for (UndoPoint oriPoint : undoPoints) {
				if (oriPoint.oriId < geom->GetNbVertex()) geom->GetVertex(oriPoint.oriId)->SetLocation(oriPoint.oriPos);
			}
			undoProjectButton->SetEnabled(false);
			geom->InitializeGeometry();
			//for(int i=0;i<nbSelected;i++)
			//	geom->SetFacetTexture(selection[i],geom->GetFacet(selection[i])->tRatio,geom->GetFacet(selection[i])->hasMesh);	
			work->Reload();
			mApp->UpdateFacetlistSelected();
			mApp->UpdateViewers();
		}
		break;
	}

	GLWindow::ProcessMessage(src, message);
}

void MirrorFacet::UpdateToggle(GLComponent *src) {
	l1->SetState(false);
	l2->SetState(false);
	l3->SetState(false);
	l4->SetState(false);
	l5->SetState(false);
	l6->SetState(false);

	GLToggle *toggle = (GLToggle*)src;
	toggle->SetState(true);

	facetNumber->SetEditable(src == l4);
	aText->SetEditable(src == l6);
	bText->SetEditable(src == l6);
	cText->SetEditable(src == l6);
	dText->SetEditable(src == l6);

	if (src == l1) planeMode = XYMODE;
	if (src == l2) planeMode = YZMODE;
	if (src == l3) planeMode = XZMODE;
	if (src == l4) planeMode = FACETNMODE;
	if (src == l5) planeMode = THREEVERTEXMODE;
	if (src == l6) planeMode = ABCDMODE;

}