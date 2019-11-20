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
#define XMODE 1
#define YMODE 2
#define ZMODE 3
#define FACETUMODE 4
#define FACETVMODE 5
#define FACETNMODE 6
#define TWOVERTEXMODE 7
#define EQMODE 8

#include "RotateFacet.h"
#include "Facet_shared.h"
#include "GLApp/GLToolkit.h"
#include "GLApp/MathTools.h"
#include "GLApp/GLMessageBox.h"

#include "GLApp/GLButton.h"
#include "GLApp/GLTextField.h"
#include "GLApp/GLLabel.h"
#include "GLApp/GLToggle.h"
#include "GLApp/GLTitledPanel.h"

#include "Geometry_shared.h"

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

RotateFacet::RotateFacet(Geometry *g,Worker *w):GLWindow() {

	int wD = 350;
	int hD = 375;

	SetTitle("Rotate selected facets");

	iPanel = new GLTitledPanel("Axis definiton mode");
	iPanel->SetBounds(5,5,wD-10,290);
	Add(iPanel);

	l1 = new GLToggle(0,"Axis X");
	l1->SetBounds(10,20,100,18);
	iPanel->Add(l1);

	l2 = new GLToggle(0,"Axis Y");
	l2->SetBounds(10,45,100,18);
	iPanel->Add(l2);

	l3 = new GLToggle(0,"Axis Z");
	l3->SetBounds(10,70,100,18);
	iPanel->Add(l3);

	GLTitledPanel *facetPanel = new GLTitledPanel("From facet");
	facetPanel->SetBounds(10,90,330,90);
	iPanel->Add(facetPanel);

	l4 = new GLToggle(0,"U vector");
	l4->SetBounds(15,105,100,18);
	facetPanel->Add(l4);

	l5 = new GLToggle(0,"V vector");
	l5->SetBounds(15,130,100,18);
	facetPanel->Add(l5);

	l6 = new GLToggle(0,"Normal vector");
	l6->SetBounds(15,155,100,18);
	facetPanel->Add(l6);

	lNum = new GLLabel("of facet #");
	lNum->SetBounds(100,130,80,18);
	iPanel->Add(lNum);

	facetNumber = new GLTextField(0,"0");
	facetNumber->SetBounds(160,130,60,18);
	facetNumber->SetEditable(false);
	iPanel->Add(facetNumber);
	
	getSelFacetButton = new GLButton(0,"<-Get selected");
	getSelFacetButton->SetBounds(225, 130, 80, 18);
	getSelFacetButton->SetEnabled(false);
	iPanel->Add(getSelFacetButton);

	l7 = new GLToggle(0,"Define by 2 selected vertex");
	l7->SetBounds(10,190,100,18);
	iPanel->Add(l7);

	l8 = new GLToggle(0,"Define by equation:");
	l8->SetBounds(10,215,100,18);
	iPanel->Add(l8);

	GLLabel *pLabel = new GLLabel("Point:");
	pLabel->SetBounds(10,240,50,18);
	iPanel->Add(pLabel);

	aLabel = new GLLabel("a:");
	aLabel->SetBounds(70, 240, 10, 18);
	iPanel->Add(aLabel);
	
	aText = new GLTextField(0,"0");
	aText->SetBounds(85,240,40,18);
	aText->SetEditable(false);
	iPanel->Add(aText);
		
	bLabel = new GLLabel("b:");
	bLabel->SetBounds(135,240,10,18);
	iPanel->Add(bLabel);
	
	bText = new GLTextField(0,"0");
	bText->SetBounds(150,240,40,18);
	bText->SetEditable(false);
	iPanel->Add(bText);

	cLabel = new GLLabel("c:");
	cLabel->SetBounds(200,240,10,18);
	iPanel->Add(cLabel);

	cText = new GLTextField(0,"0");
	cText->SetBounds(215,240,40,18);
	cText->SetEditable(false);
	iPanel->Add(cText);

	getBaseVertexButton = new GLButton(0, "<-Get base");
	getBaseVertexButton->SetBounds(270, 240, 60, 18);
	iPanel->Add(getBaseVertexButton);

	GLLabel *dLabel = new GLLabel("Direction:");
	dLabel->SetBounds(10,265,50,18);
	iPanel->Add(dLabel);

	uLabel = new GLLabel("u:");
	uLabel->SetBounds(70, 265, 10, 18);
	iPanel->Add(uLabel);

	uText = new GLTextField(0,"0");
	uText->SetBounds(85,265,40,18);
	uText->SetEditable(false);
	iPanel->Add(uText);

	vLabel = new GLLabel("v");
	vLabel->SetBounds(135,265,10,18);
	iPanel->Add(vLabel);

	vText = new GLTextField(0,"0");
	vText->SetBounds(150,265,40,18);
	vText->SetEditable(false);
	iPanel->Add(vText);

	wLabel = new GLLabel("w");
	wLabel->SetBounds(200,265,10,18);
	iPanel->Add(wLabel);

	wText = new GLTextField(0,"0");
	wText->SetBounds(215,265,40,18);
	wText->SetEditable(false);
	iPanel->Add(wText);

	getDirVertexButton = new GLButton(0, "<-Calc diff");
	getDirVertexButton->SetBounds(270, 265, 60, 18);
	iPanel->Add(getDirVertexButton);

	degLabel = new GLLabel("Degrees:");
	degLabel->SetBounds(10,300,60,18);
	Add(degLabel);

	degText = new GLTextField(0,"0");
	degText->SetBounds(65,300,80,18);
	degText->SetEditable(true);
	Add(degText);

	radLabel = new GLLabel("Radians:");
	radLabel->SetBounds(170, 300, 60, 18);
	Add(radLabel);

	radText = new GLTextField(0, "0");
	radText->SetBounds(225, 300, 80, 18);
	radText->SetEditable(true);
	Add(radText);

	moveButton = new GLButton(0,"Rotate facet");
	moveButton->SetBounds(25,hD-44,85,21);
	Add(moveButton);

	copyButton = new GLButton(0,"Copy facet");
	copyButton->SetBounds(115,hD-44,85,21);
	Add(copyButton);

	cancelButton = new GLButton(0,"Dismiss");
	cancelButton->SetBounds(205,hD-44,85,21);
	Add(cancelButton);

	// Center dialog
	int wS,hS;
	GLToolkit::GetScreenSize(&wS,&hS);
	int xD = (wS-wD)/2;
	int yD = (hS-hD)/2;
	SetBounds(xD,yD,wD,hD);

	RestoreDeviceObjects();

	geom = g;
	work = w;
	axisMode = -1;
}

void RotateFacet::ProcessMessage(GLComponent *src,int message) {
	double a,b,c,u,v,w,deg,rad;
	int facetNum;

	switch(message) {
		
	case MSG_TOGGLE:
		UpdateToggle(src);
		break;

	case MSG_BUTTON:

		if(src==cancelButton) {

			GLWindow::ProcessMessage(NULL,MSG_CLOSE);

		} else if (src==moveButton || src==copyButton) {
			if (geom->GetNbSelectedFacets()==0) {
				GLMessageBox::Display("No facets selected","Nothing to mirror",GLDLG_OK,GLDLG_ICONERROR);
				return;
			}
			//Calculate the plane
			Vector3d AXIS_P0,AXIS_DIR;
			int selVert1id, selVert2id;

			if (!(radText->GetNumber(&rad))) {
				GLMessageBox::Display("Invalid angle (radians field)","Error",GLDLG_OK,GLDLG_ICONERROR);
				return;
			}

			switch (axisMode) {
			case XMODE:
				AXIS_P0 = Vector3d(0.0, 0.0, 0.0);
				AXIS_DIR = Vector3d(1.0, 0.0, 0.0);
				break;
			case YMODE:
				AXIS_P0 = Vector3d(0.0, 0.0, 0.0);
				AXIS_DIR = Vector3d(0.0, 1.0, 0.0);
				break;
			case ZMODE:
				AXIS_P0 = Vector3d(0.0, 0.0, 0.0);
				AXIS_DIR = Vector3d(0.0, 0.0, 1.0);
				break;
			case FACETUMODE:
				if( !(facetNumber->GetNumberInt(&facetNum))||facetNum<1||facetNum>geom->GetNbFacet() ) {
					GLMessageBox::Display("Invalid facet number","Error",GLDLG_OK,GLDLG_ICONERROR);
					return;
				}
				AXIS_P0=geom->GetFacet(facetNum-1)->sh.O;
				AXIS_DIR=geom->GetFacet(facetNum-1)->sh.U;
				break;
			case FACETVMODE:
				if( !(facetNumber->GetNumberInt(&facetNum))||facetNum<1||facetNum>geom->GetNbFacet() ) {
					GLMessageBox::Display("Invalid facet number","Error",GLDLG_OK,GLDLG_ICONERROR);
					return;
				}
				AXIS_P0=geom->GetFacet(facetNum-1)->sh.O;
				AXIS_DIR=geom->GetFacet(facetNum-1)->sh.V;
				break;
			case FACETNMODE:
				if( !(facetNumber->GetNumberInt(&facetNum))||facetNum<1||facetNum>geom->GetNbFacet() ) {
					GLMessageBox::Display("Invalid facet number","Error",GLDLG_OK,GLDLG_ICONERROR);
					return;
				}
				AXIS_P0=geom->GetFacet(facetNum-1)->sh.center;
				AXIS_DIR=geom->GetFacet(facetNum-1)->sh.N;
				break;
			case TWOVERTEXMODE:
				if (geom->GetNbSelectedVertex()!=2) {
					GLMessageBox::Display("Select exactly 2 vertices","Can't define axis",GLDLG_OK,GLDLG_ICONERROR);
					return;
				}
				selVert1id = selVert2id = -1;

				for(int i=0;selVert2id == -1 && i<geom->GetNbVertex();i++ ) {
					if( geom->GetVertex(i)->selected ) {
						if (selVert1id == -1) {
							selVert1id = i;
						}
						else {
							selVert2id = i;
						}
					}
				}

				AXIS_DIR = *(geom->GetVertex(selVert2id)) - *(geom->GetVertex(selVert1id));
				AXIS_P0 = *(geom->GetVertex(selVert1id));

				break;
			case EQMODE:
				if( !(aText->GetNumber(&a)) ) {
					GLMessageBox::Display("Invalid a coordinate","Error",GLDLG_OK,GLDLG_ICONERROR);
					return;
				}
				if( !(bText->GetNumber(&b)) ) {
					GLMessageBox::Display("Invalid b coordinate","Error",GLDLG_OK,GLDLG_ICONERROR);
					return;
				}
				if( !(cText->GetNumber(&c)) ) {
					GLMessageBox::Display("Invalid c coordinate","Error",GLDLG_OK,GLDLG_ICONERROR);
					return;
				}
				if( !(uText->GetNumber(&u)) ) {
					GLMessageBox::Display("Invalid u coordinate","Error",GLDLG_OK,GLDLG_ICONERROR);
					return;
				}
				if( !(vText->GetNumber(&v)) ) {
					GLMessageBox::Display("Invalid v coordinate","Error",GLDLG_OK,GLDLG_ICONERROR);
					return;
				}
				if( !(wText->GetNumber(&w)) ) {
					GLMessageBox::Display("Invalid w coordinate","Error",GLDLG_OK,GLDLG_ICONERROR);
					return;
				}

				if ((u==0.0)&&(v==0.0)&&(w==0.0)) {
					GLMessageBox::Display("u, v, w are all zero. That's not a vector.","Error",GLDLG_OK,GLDLG_ICONERROR);
					return;
				}

				AXIS_P0  = Vector3d(a,b,c);
				AXIS_DIR = Vector3d(u,v,w);
				break;
			default:
				GLMessageBox::Display("Select an axis definition mode.","Error",GLDLG_OK,GLDLG_ICONERROR);
				return;
			}
			if (mApp->AskToReset()) {
				geom->RotateSelectedFacets(AXIS_P0,AXIS_DIR,rad,src==copyButton,work);
				//mApp->UpdateModelParams();
				work->Reload();
				mApp->UpdateFacetlistSelected();
				mApp->UpdateViewers();
				//GLWindowManager::FullRepaint();
				mApp->changedSinceSave = true;
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
		 }
		 else if (src == getBaseVertexButton) {
			 if (geom->GetNbSelectedVertex()!=1) {
				 GLMessageBox::Display("Select exactly one vertex.", "Error", GLDLG_OK, GLDLG_ICONERROR);
				 return;
			 }
			 UpdateToggle(l8);
			 int selVertexId = -1;
			 for (int i = 0; selVertexId == -1 && i < geom->GetNbVertex(); i++) {
				 if (geom->GetVertex(i)->selected) {
					 selVertexId = i;
				 }
			 }
			 Vector3d *selVertex = geom->GetVertex(selVertexId);
			 aText->SetText(selVertex->x);
			 bText->SetText(selVertex->y);
			 cText->SetText(selVertex->z);
		}
		 else if (src == getDirVertexButton) {
			 if (geom->GetNbSelectedVertex() != 1) {
				 GLMessageBox::Display("Select exactly one vertex.", "Error", GLDLG_OK, GLDLG_ICONERROR);
				 return;
			 }
			 if (!(aText->GetNumber(&a))) {
				 GLMessageBox::Display("Invalid a coordinate", "Error", GLDLG_OK, GLDLG_ICONERROR);
				 return;
			 }
			 if (!(bText->GetNumber(&b))) {
				 GLMessageBox::Display("Invalid b coordinate", "Error", GLDLG_OK, GLDLG_ICONERROR);
				 return;
			 }
			 if (!(cText->GetNumber(&c))) {
				 GLMessageBox::Display("Invalid c coordinate", "Error", GLDLG_OK, GLDLG_ICONERROR);
				 return;
			 }
			 UpdateToggle(l8);
			 int selVertexId = -1;
			 for (int i = 0; selVertexId == -1 && i < geom->GetNbVertex(); i++) {
				 if (geom->GetVertex(i)->selected) {
					 selVertexId = i;
				 }
			 }
			 Vector3d *selVertex = geom->GetVertex(selVertexId);
			 uText->SetText(selVertex->x-a);
			 vText->SetText(selVertex->y-b);
			 wText->SetText(selVertex->z-c);
		 }
		break;
		case MSG_TEXT_UPD:
			if (src == degText) {
				if (degText->GetNumber(&deg)) {
					radText->SetText(deg / 180.0*PI);
				}
			}
			else if (src == radText){
				if (radText->GetNumber(&rad)) {
					degText->SetText(rad / PI * 180.0);
				}
			}
		break;
	}

	GLWindow::ProcessMessage(src,message);
}

void RotateFacet::UpdateToggle(GLComponent *src) {
	l1->SetState(false);
	l2->SetState(false);
	l3->SetState(false);
	l4->SetState(false);
	l5->SetState(false);
	l6->SetState(false);
	l7->SetState(false);
	l8->SetState(false);

	GLToggle *toggle=(GLToggle*)src;
	toggle->SetState(true);

	facetNumber->SetEditable(src == l4 || src == l5 || src == l6);
	getSelFacetButton->SetEnabled(src == l4 || src == l5 || src == l6);

	aText->SetEditable(src==l8);
	bText->SetEditable(src==l8);
	cText->SetEditable(src==l8);
	uText->SetEditable(src==l8);
	vText->SetEditable(src==l8);
	wText->SetEditable(src==l8);

	if (src==l1) axisMode=XMODE;
	if (src==l2) axisMode=YMODE;
	if (src==l3) axisMode=ZMODE;
	if (src==l4) axisMode=FACETUMODE;
	if (src==l5) axisMode=FACETVMODE;
	if (src==l6) axisMode=FACETNMODE;
	if (src==l7) axisMode=TWOVERTEXMODE;
	if (src==l8) axisMode=EQMODE;

}