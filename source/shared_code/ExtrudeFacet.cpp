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
#include "ExtrudeFacet.h"
#include "Facet_shared.h"
#include "GLApp/GLTitledPanel.h"
#include "GLApp/GLToolkit.h"
#include "GLApp/GLWindowManager.h"
#include "GLApp/GLMessageBox.h"
#include "GLApp/MathTools.h"
#include "GLApp/GLToggle.h"
#include "GLApp/GLTextField.h"
#include "GLApp/GLLabel.h"
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

ExtrudeFacet::ExtrudeFacet(Geometry *g, Worker *w) :GLWindow() {

	int wD = 315;
	int hD = 595;
	groupBox1 = new GLTitledPanel("Towards / against normal");
	groupBox1->SetBounds(5, 3, 305, 68);
	Add(groupBox1);
	groupBox2 = new GLTitledPanel("Along straight path");
	groupBox2->SetBounds(5, 73, 305, 110);
	Add(groupBox2);
	groupBox3 = new GLTitledPanel("Along curve");
	groupBox3->SetBounds(5, 189, 305, 356);
	Add(groupBox3);
	offsetCheckbox = new GLToggle(0, "Direction vector:");
	groupBox2->SetCompBounds(offsetCheckbox, 10, 18, 104, 17);
	groupBox2->Add(offsetCheckbox);

	label3 = new GLLabel("cm");
	groupBox2->SetCompBounds(label3, 75, 45, 21, 13);
	groupBox2->Add(label3);

	dxText = new GLTextField(0, "");
	groupBox2->SetCompBounds(dxText, 35, 39, 40, 20);
	groupBox2->Add(dxText);

	label4 = new GLLabel("dX:");
	groupBox2->SetCompBounds(label4, 10, 45, 23, 13);
	groupBox2->Add(label4);

	label5 = new GLLabel("dY:");
	groupBox2->SetCompBounds(label5, 109, 45, 23, 13);
	groupBox2->Add(label5);

	label6 = new GLLabel("cm");
	groupBox2->SetCompBounds(label6, 174, 45, 21, 13);
	groupBox2->Add(label6);

	dyText = new GLTextField(0, "");
	groupBox2->SetCompBounds(dyText, 134, 39, 40, 20);
	groupBox2->Add(dyText);

	label7 = new GLLabel("dZ:");
	groupBox2->SetCompBounds(label7, 206, 44, 23, 13);
	groupBox2->Add(label7);

	label8 = new GLLabel("cm");
	groupBox2->SetCompBounds(label8, 271, 44, 21, 13);
	groupBox2->Add(label8);

	dzText = new GLTextField(0, "");
	groupBox2->SetCompBounds(dzText, 231, 38, 40, 20);
	groupBox2->Add(dzText);

	extrudeButton = new GLButton(0, "Extrude");
	extrudeButton->SetBounds(111, 551, 90, 21);
	Add(extrudeButton);

	getBaseButton = new GLButton(0, "Get Base Vertex");
	groupBox2->SetCompBounds(getBaseButton, 35, 69, 105, 21);
	groupBox2->Add(getBaseButton);

	getDirButton = new GLButton(0, "Get Dir. Vertex");
	groupBox2->SetCompBounds(getDirButton, 166, 69, 105, 21);
	groupBox2->Add(getDirButton);

	towardsNormalCheckbox = new GLToggle(0, "Towards normal");
	groupBox1->SetCompBounds(towardsNormalCheckbox, 10, 19, 101, 17);
	groupBox1->Add(towardsNormalCheckbox);

	againstNormalCheckbox = new GLToggle(0, "Against normal");
	groupBox1->SetCompBounds(againstNormalCheckbox, 166, 19, 95, 17);
	groupBox1->Add(againstNormalCheckbox);

	label1 = new GLLabel("extrusion length:");
	groupBox1->SetCompBounds(label1, 45, 43, 84, 13);
	groupBox1->Add(label1);

	distanceTextbox = new GLTextField(0, "");
	groupBox1->SetCompBounds(distanceTextbox, 134, 40, 40, 20);
	groupBox1->Add(distanceTextbox);

	label2 = new GLLabel("cm");
	groupBox1->SetCompBounds(label2, 173, 43, 21, 13);
	groupBox1->Add(label2);

	dirLabel = new GLLabel("dirLabel");
	groupBox2->SetCompBounds(dirLabel, 196, 93, 44, 13);
	groupBox2->Add(dirLabel);

	baseLabel = new GLLabel("baseLabel");
	groupBox2->SetCompBounds(baseLabel, 55, 93, 56, 13);
	groupBox2->Add(baseLabel);

	curveGetDirButton = new GLButton(0, "Towards selected");
	groupBox3->SetCompBounds(curveGetDirButton, 207, 152, 90, 21);
	groupBox3->Add(curveGetDirButton);

	curveGetBaseButton = new GLButton(0, "From selected");
	groupBox3->SetCompBounds(curveGetBaseButton, 207, 70, 90, 21);
	groupBox3->Add(curveGetBaseButton);

	label11 = new GLLabel("dZ:");
	groupBox3->SetCompBounds(label11, 206, 211, 23, 13);
	groupBox3->Add(label11);

	label12 = new GLLabel("cm");
	groupBox3->SetCompBounds(label12, 272, 211, 21, 13);
	groupBox3->Add(label12);

	curvedZText = new GLTextField(0, "");
	groupBox3->SetCompBounds(curvedZText, 231, 205, 40, 20);
	groupBox3->Add(curvedZText);

	label13 = new GLLabel("dY:");
	groupBox3->SetCompBounds(label13, 109, 211, 23, 13);
	groupBox3->Add(label13);

	label14 = new GLLabel("cm");
	groupBox3->SetCompBounds(label14, 174, 211, 21, 13);
	groupBox3->Add(label14);

	curvedYText = new GLTextField(0, "");
	groupBox3->SetCompBounds(curvedYText, 134, 205, 40, 20);
	groupBox3->Add(curvedYText);

	label15 = new GLLabel("dX:");
	groupBox3->SetCompBounds(label15, 13, 211, 23, 13);
	groupBox3->Add(label15);

	label16 = new GLLabel("cm");
	groupBox3->SetCompBounds(label16, 78, 211, 21, 13);
	groupBox3->Add(label16);

	curvedXText = new GLTextField(0, "");
	groupBox3->SetCompBounds(curvedXText, 38, 205, 40, 20);
	groupBox3->Add(curvedXText);

	label17 = new GLLabel("Z0:");
	groupBox3->SetCompBounds(label17, 206, 103, 23, 13);
	groupBox3->Add(label17);

	label23 = new GLLabel("Radius:");
	groupBox3->SetCompBounds(label23, 16, 246, 43, 13);
	groupBox3->Add(label23);

	curveRadiusLengthText = new GLTextField(0, "");
	groupBox3->SetCompBounds(curveRadiusLengthText, 134, 243, 40, 20);
	groupBox3->Add(curveRadiusLengthText);

	curveAgainstNormalCheckbox = new GLToggle(0, "Against normal");
	groupBox3->SetCompBounds(curveAgainstNormalCheckbox, 173, 19, 95, 17);
	groupBox3->Add(curveAgainstNormalCheckbox);

	curveTowardsNormalCheckbox = new GLToggle(0, "Towards normal");
	groupBox3->SetCompBounds(curveTowardsNormalCheckbox, 10, 19, 101, 17);
	groupBox3->Add(curveTowardsNormalCheckbox);

	label18 = new GLLabel("cm");
	groupBox3->SetCompBounds(label18, 272, 103, 21, 13);
	groupBox3->Add(label18);

	curveZ0Text = new GLTextField(0, "");
	groupBox3->SetCompBounds(curveZ0Text, 231, 97, 40, 20);
	groupBox3->Add(curveZ0Text);

	label19 = new GLLabel("Y0:");
	groupBox3->SetCompBounds(label19, 109, 103, 23, 13);
	groupBox3->Add(label19);

	label20 = new GLLabel("cm");
	groupBox3->SetCompBounds(label20, 174, 103, 21, 13);
	groupBox3->Add(label20);

	curveY0Text = new GLTextField(0, "");
	groupBox3->SetCompBounds(curveY0Text, 134, 97, 40, 20);
	groupBox3->Add(curveY0Text);

	label21 = new GLLabel("X0:");
	groupBox3->SetCompBounds(label21, 13, 103, 23, 13);
	groupBox3->Add(label21);

	label22 = new GLLabel("cm");
	groupBox3->SetCompBounds(label22, 78, 103, 21, 13);
	groupBox3->Add(label22);

	curveX0Text = new GLTextField(0, "");
	groupBox3->SetCompBounds(curveX0Text, 38, 97, 40, 20);
	groupBox3->Add(curveX0Text);

	label24 = new GLLabel("Total angle:");
	groupBox3->SetCompBounds(label24, 16, 273, 63, 13);
	groupBox3->Add(label24);

	curveTotalAngleDegText = new GLTextField(0, "");
	groupBox3->SetCompBounds(curveTotalAngleDegText, 134, 270, 40, 20);
	groupBox3->Add(curveTotalAngleDegText);

	label25 = new GLLabel("Steps:");
	groupBox3->SetCompBounds(label25, 16, 325, 37, 13);
	groupBox3->Add(label25);

	curveStepsText = new GLTextField(0, "");
	groupBox3->SetCompBounds(curveStepsText, 134, 322, 40, 20);
	groupBox3->Add(curveStepsText);

	label26 = new GLLabel("deg");
	groupBox3->SetCompBounds(label26, 173, 273, 25, 13);
	groupBox3->Add(label26);

	label27 = new GLLabel("rad");
	groupBox3->SetCompBounds(label27, 270, 273, 22, 13);
	groupBox3->Add(label27);

	curveTotalAngleRadText = new GLTextField(0, "");
	groupBox3->SetCompBounds(curveTotalAngleRadText, 231, 270, 40, 20);
	groupBox3->Add(curveTotalAngleRadText);

	label10 = new GLLabel("cm");
	groupBox3->SetCompBounds(label10, 174, 246, 21, 13);
	groupBox3->Add(label10);

	label9 = new GLLabel("Radius direction:");
	groupBox3->SetCompBounds(label9, 13, 130, 86, 13);
	groupBox3->Add(label9);

	curveFacetVButton = new GLButton(0, "Facet V");
	groupBox3->SetCompBounds(curveFacetVButton, 111, 152, 90, 21);
	groupBox3->Add(curveFacetVButton);

	curveFacetUButton = new GLButton(0, "Facet U");
	groupBox3->SetCompBounds(curveFacetUButton, 13, 152, 90, 21);
	groupBox3->Add(curveFacetUButton);

	curveFacetIndex1Button = new GLButton(0, "Facet index1");
	groupBox3->SetCompBounds(curveFacetIndex1Button, 111, 70, 90, 21);
	groupBox3->Add(curveFacetIndex1Button);

	curveFacetCenterButton = new GLButton(0, "Facet center");
	groupBox3->SetCompBounds(curveFacetCenterButton, 13, 70, 90, 21);
	groupBox3->Add(curveFacetCenterButton);

	label28 = new GLLabel("Radius base:");
	groupBox3->SetCompBounds(label28, 10, 50, 69, 13);
	groupBox3->Add(label28);

	curveDirLabel = new GLLabel("curveDirLabel");
	groupBox3->SetCompBounds(curveDirLabel, 123, 130, 73, 13);
	groupBox3->Add(curveDirLabel);

	curveBaseLabel = new GLLabel("curveBaseLabel");
	groupBox3->SetCompBounds(curveBaseLabel, 117, 50, 84, 13);
	groupBox3->Add(curveBaseLabel);

	facetNYbutton = new GLButton(0, "Facet N x Y");
	groupBox3->SetCompBounds(facetNYbutton, 111, 178, 90, 21);
	groupBox3->Add(facetNYbutton);

	facetNXbutton = new GLButton(0, "Facet N x X");
	groupBox3->SetCompBounds(facetNXbutton, 13, 178, 90, 21);
	groupBox3->Add(facetNXbutton);

	facetNZbutton = new GLButton(0, "Facet N x Z");
	groupBox3->SetCompBounds(facetNZbutton, 207, 178, 90, 21);
	groupBox3->Add(facetNZbutton);

	label30 = new GLLabel("cm");
	groupBox3->SetCompBounds(label30, 175, 299, 21, 13);
	groupBox3->Add(label30);

	label29 = new GLLabel("Total length:");
	groupBox3->SetCompBounds(label29, 16, 299, 66, 13);
	groupBox3->Add(label29);

	curveTotalLengthText = new GLTextField(0, "");
	groupBox3->SetCompBounds(curveTotalLengthText, 134, 296, 40, 20);
	groupBox3->Add(curveTotalLengthText);

	SetTitle("Extrude Facet");
	// Center dialog
	int wS, hS;
	GLToolkit::GetScreenSize(&wS, &hS);
	int xD = (wS - wD) / 2;
	int yD = (hS - hD) / 2;
	SetBounds(xD, yD, wD, hD);

	baseLabel->SetText("");
	dirLabel->SetText("");
	curveBaseLabel->SetText("");
	curveDirLabel->SetText("");
	towardsNormalCheckbox->SetState(1);
	distanceTextbox->SetText("1");
	baseId = dirId = -1;
	EnableDisableControls();

	RestoreDeviceObjects();

	geom = g;
	work = w;

}

void ExtrudeFacet::ProcessMessage(GLComponent *src, int message) {
	double x0, y0, z0, dX, dY, dZ, dist, radiusLength, totalAngle, totalLength;
	int noSteps;

	switch(message) {
	case MSG_BUTTON:

		if (src==extrudeButton) {
			if (geom->GetNbSelectedFacets()==0) {
				GLMessageBox::Display("No facets selected","Nothing to move",GLDLG_OK,GLDLG_ICONINFO);
				return;
			}
			else if (geom->GetNbSelectedFacets() > 1) {
				char warningMsg[512];
				sprintf(warningMsg, "Extrude %zd facets at once?", geom->GetNbSelectedFacets());
				int rep = GLMessageBox::Display(warningMsg, "Extrusion of more than one facet", GLDLG_OK | GLDLG_CANCEL, GLDLG_ICONINFO);
				if (rep != GLDLG_OK) {
					return;
				}
			}

			if( offsetCheckbox->GetState()==1 && !dxText->GetNumber(&dX) ) {
				GLMessageBox::Display("Invalid direction vector dX value","Error",GLDLG_OK,GLDLG_ICONERROR);
				return;
			}
			if (offsetCheckbox->GetState() == 1 && !dyText->GetNumber(&dY)) {
				GLMessageBox::Display("Invalid direction vector dY value","Error",GLDLG_OK,GLDLG_ICONERROR);
				return;
			}
			if (offsetCheckbox->GetState() == 1 && !dzText->GetNumber(&dZ)) {
				GLMessageBox::Display("Invalid direction vector dZ value","Error",GLDLG_OK,GLDLG_ICONERROR);
				return;
			}
			if ( (towardsNormalCheckbox->GetState() == 1 || againstNormalCheckbox->GetState()==1) 
				&& !distanceTextbox->GetNumber(&dist)) {
				GLMessageBox::Display("Invalid extrusion length value", "Error", GLDLG_OK, GLDLG_ICONERROR);
				return;
			}
			if ((curveTowardsNormalCheckbox->GetState() == 1 || curveAgainstNormalCheckbox->GetState() == 1)
				&& !curveX0Text->GetNumber(&x0)) {
				GLMessageBox::Display("Invalid radius base X0 value", "Error", GLDLG_OK, GLDLG_ICONERROR);
				return;
			}
			if ((curveTowardsNormalCheckbox->GetState() == 1 || curveAgainstNormalCheckbox->GetState() == 1)
				&& !curveY0Text->GetNumber(&y0)) {
				GLMessageBox::Display("Invalid radius base Y0 value", "Error", GLDLG_OK, GLDLG_ICONERROR);
				return;
			}
			if ((curveTowardsNormalCheckbox->GetState() == 1 || curveAgainstNormalCheckbox->GetState() == 1)
				&& !curveZ0Text->GetNumber(&z0)) {
				GLMessageBox::Display("Invalid radius base Z0 value", "Error", GLDLG_OK, GLDLG_ICONERROR);
				return;
			}
			if ((curveTowardsNormalCheckbox->GetState() == 1 || curveAgainstNormalCheckbox->GetState() == 1)
				&& !curvedXText->GetNumber(&dX)) {
				GLMessageBox::Display("Invalid radius direction dX value", "Error", GLDLG_OK, GLDLG_ICONERROR);
				return;
			}
			if ((curveTowardsNormalCheckbox->GetState() == 1 || curveAgainstNormalCheckbox->GetState() == 1)
				&& !curvedYText->GetNumber(&dY)) {
				GLMessageBox::Display("Invalid radius direction dY value", "Error", GLDLG_OK, GLDLG_ICONERROR);
				return;
			}
			if ((curveTowardsNormalCheckbox->GetState() == 1 || curveAgainstNormalCheckbox->GetState() == 1)
				&& !curvedZText->GetNumber(&dZ)) {
				GLMessageBox::Display("Invalid radius direction dZ value", "Error", GLDLG_OK, GLDLG_ICONERROR);
				return;
			}
			if ((curveTowardsNormalCheckbox->GetState() == 1 || curveAgainstNormalCheckbox->GetState() == 1)
				&& !curveRadiusLengthText->GetNumber(&radiusLength)) {
				GLMessageBox::Display("Invalid radius length value", "Error", GLDLG_OK, GLDLG_ICONERROR);
				return;
			}
			if ((curveTowardsNormalCheckbox->GetState() == 1 || curveAgainstNormalCheckbox->GetState() == 1)
				&& !curveTotalAngleDegText->GetNumber(&totalAngle)) {
				GLMessageBox::Display("Invalid total angle (deg) value", "Error", GLDLG_OK, GLDLG_ICONERROR);
				return;
			}
			if ((curveTowardsNormalCheckbox->GetState() == 1 || curveAgainstNormalCheckbox->GetState() == 1)
				&& !curveTotalLengthText->GetNumber(&totalLength)) {
				GLMessageBox::Display("Invalid total length value", "Error", GLDLG_OK, GLDLG_ICONERROR);
				return;
			}
			if ((curveTowardsNormalCheckbox->GetState() == 1 || curveAgainstNormalCheckbox->GetState() == 1)
				&& !curveStepsText->GetNumberInt(&noSteps)) {
				GLMessageBox::Display("Invalid 'number of steps' value", "Error", GLDLG_OK, GLDLG_ICONERROR);
				return;
			}
			int mode;
				if (towardsNormalCheckbox->GetState() || againstNormalCheckbox->GetState()) mode = 1; //By distance
				else if (offsetCheckbox->GetState()) mode = 2; // By offset vector
				else mode = 3; //Along curve
			
				if ((mode!=1) && dX*dX + dY*dY + dZ*dZ < 1E-8) {
				GLMessageBox::Display("Direction is a null-vector", "Error", GLDLG_OK, GLDLG_ICONERROR);
				return;
			} if (mode==3 && std::abs(radiusLength)<1E-8) {
				GLMessageBox::Display("Radius length can't be 0", "Error", GLDLG_OK, GLDLG_ICONERROR);
				return;
			} if (mode == 1 && std::abs(dist)<1E-8) {
				GLMessageBox::Display("Extrusion length can't be 0", "Error", GLDLG_OK, GLDLG_ICONERROR);
				return;
			} if (mode == 3 && (totalAngle<-360 || totalAngle>360)) {
				int rep = GLMessageBox::Display("Total angle outside -360..+360 degree. Are you sure?", "Warning", GLDLG_OK | GLDLG_CANCEL, GLDLG_ICONWARNING);
				if (rep!=GLDLG_OK) return;
			} if (mode == 3 && (std::abs(totalAngle)<1E-8 || totalAngle>360)) {
				GLMessageBox::Display("Total angle can't be 0", "Error", GLDLG_OK, GLDLG_ICONERROR);
				return;
			} if (mode == 3 && !(noSteps>0)) {
				GLMessageBox::Display("Invalid number of steps", "Error", GLDLG_OK, GLDLG_ICONERROR);
				return;
			} if (mode == 3 && (noSteps>50)) {
				char tmp[128];
				sprintf(tmp, "Are you sure you want to sweep in %d steps?", noSteps);
				int rep = GLMessageBox::Display(tmp, "Warning", GLDLG_OK | GLDLG_CANCEL, GLDLG_ICONWARNING);
				if (rep != GLDLG_OK) return;
			}

			if (mApp->AskToReset()){
				
				Vector3d radiusBase, offsetORradiusdir;
				radiusBase.x = x0;
				radiusBase.y = y0;
				radiusBase.z = z0;
				offsetORradiusdir.x = dX;
				offsetORradiusdir.y = dY;
				offsetORradiusdir.z = dZ;
				double distanceORradius;
				if (mode == 1) distanceORradius = dist;
				else if (mode == 3) distanceORradius = radiusLength;

				geom->Extrude(mode, radiusBase, offsetORradiusdir, 
					againstNormalCheckbox->GetState() || curveAgainstNormalCheckbox->GetState(), 
					(mode==1)?dist:radiusLength,
					totalAngle/180*PI, noSteps);

				work->Reload(); 
				mApp->changedSinceSave = true;
				mApp->UpdateFacetlistSelected();	
				mApp->UpdateViewers();
			}
		}
		else if (src == getBaseButton) {
			if (auto foundId = AssertOneVertexSelected()) {
				ClearToggles(offsetCheckbox);
				EnableDisableControls();

				baseId = *foundId;
				char tmp[32];
				sprintf(tmp, "Vertex %zd", baseId + 1);
				baseLabel->SetText(tmp);
				if (dirId > 0 && dirId < geom->GetNbVertex()) {
					dxText->SetText(geom->GetVertex(dirId)->x - geom->GetVertex(baseId)->x);
					dyText->SetText(geom->GetVertex(dirId)->y - geom->GetVertex(baseId)->y);
					dzText->SetText(geom->GetVertex(dirId)->z - geom->GetVertex(baseId)->z);
				}
			}
		}
		else if (src == getDirButton) {
			if (auto foundId = AssertOneVertexSelected()) {			

				ClearToggles(offsetCheckbox);
				EnableDisableControls();

				dirId = *foundId;
				char tmp[32];
				sprintf(tmp, "Vertex %zd", dirId + 1);
				dirLabel->SetText(tmp);
				if (baseId>0 && baseId < geom->GetNbVertex()) {
					dxText->SetText(geom->GetVertex(dirId)->x - geom->GetVertex(baseId)->x);
					dyText->SetText(geom->GetVertex(dirId)->y - geom->GetVertex(baseId)->y);
					dzText->SetText(geom->GetVertex(dirId)->z - geom->GetVertex(baseId)->z);
				}
			}
		} else if (src==curveFacetCenterButton) {
			if (auto foundId = AssertOneFacetSelected()) {
				char tmp[32];
				sprintf(tmp, "Facet %zd center", *foundId + 1);
				curveBaseLabel->SetText(tmp);
				Vector3d center3d = geom->GetFacet(*foundId)->sh.center;
				curveX0Text->SetText(center3d.x);
				curveY0Text->SetText(center3d.y);
				curveZ0Text->SetText(center3d.z);
			}
		}
		else if (src == curveFacetIndex1Button) {
			if (auto foundId = AssertOneFacetSelected()) {
				size_t vertexId = geom->GetFacet(*foundId)->indices[0];
				char tmp[32];
				sprintf(tmp, "Facet %zd index1: Vertex %zd", *foundId + 1, vertexId+1);
				curveBaseLabel->SetText(tmp);
				curveX0Text->SetText(geom->GetVertex(vertexId)->x);
				curveY0Text->SetText(geom->GetVertex(vertexId)->y);
				curveZ0Text->SetText(geom->GetVertex(vertexId)->z);
			}
		}
		else if (src == curveGetBaseButton) {
			if (auto foundId = AssertOneVertexSelected()) {
				char tmp[32];
				sprintf(tmp, "Vertex %zd", *foundId + 1);
				curveBaseLabel->SetText(tmp);
				curveX0Text->SetText(geom->GetVertex(*foundId)->x);
				curveY0Text->SetText(geom->GetVertex(*foundId)->y);
				curveZ0Text->SetText(geom->GetVertex(*foundId)->z);
			}
		}
		else if (src == curveFacetUButton) {
			if (auto foundId = AssertOneFacetSelected()) {
				char tmp[32];
				sprintf(tmp, "Facet %zd \201", *foundId + 1);
				curveDirLabel->SetText(tmp);
				curvedXText->SetText(geom->GetFacet(*foundId)->sh.U.x);
				curvedYText->SetText(geom->GetFacet(*foundId)->sh.U.y);
				curvedZText->SetText(geom->GetFacet(*foundId)->sh.U.z);
			}
		}
		else if (src == curveFacetVButton) {
			if (auto foundId = AssertOneFacetSelected()) {
				char tmp[32];
				sprintf(tmp, "Facet %zd \202", *foundId + 1);
				curveDirLabel->SetText(tmp);
				curvedXText->SetText(geom->GetFacet(*foundId)->sh.V.x);
				curvedYText->SetText(geom->GetFacet(*foundId)->sh.V.y);
				curvedZText->SetText(geom->GetFacet(*foundId)->sh.V.z);
			}
		}
		else if (src == curveGetDirButton) {
			if (auto foundId = AssertOneVertexSelected()) {
				double x0, y0, z0;
				if (curveX0Text->GetNumber(&x0) && curveY0Text->GetNumber(&y0) && curveZ0Text->GetNumber(&z0)) {
					char tmp[32];
					sprintf(tmp, "Vertex %zd", *foundId + 1);
					curveDirLabel->SetText(tmp);
					curvedXText->SetText(geom->GetVertex(*foundId)->x-x0);
					curvedYText->SetText(geom->GetVertex(*foundId)->y-y0);
					curvedZText->SetText(geom->GetVertex(*foundId)->z-z0);
				}				
			}
		}
		else if (src == facetNXbutton) {
			if (auto foundId = AssertOneFacetSelected()) {
				char tmp[32];
				sprintf(tmp, "Facet %zd N x X", *foundId + 1);
				curveDirLabel->SetText(tmp);
				curvedXText->SetText(0);
				curvedYText->SetText(geom->GetFacet(*foundId)->sh.N.z);
				curvedZText->SetText(-geom->GetFacet(*foundId)->sh.N.y);
			}
		}
		else if (src == facetNYbutton) {
			if (auto foundId = AssertOneFacetSelected()) {
				char tmp[32];
				sprintf(tmp, "Facet %zd N x Y", *foundId + 1);
				curveDirLabel->SetText(tmp);
				curvedXText->SetText(-geom->GetFacet(*foundId)->sh.N.z);
				curvedYText->SetText(0);
				curvedZText->SetText(geom->GetFacet(*foundId)->sh.N.x);
			}
		}
		else if (src == facetNZbutton) {
			if (auto foundId = AssertOneFacetSelected()) {
				char tmp[32];
				sprintf(tmp, "Facet %zd N x Z", *foundId + 1);
				curveDirLabel->SetText(tmp);
				curvedXText->SetText(geom->GetFacet(*foundId)->sh.N.y);
				curvedYText->SetText(-geom->GetFacet(*foundId)->sh.N.x);
				curvedZText->SetText(0);
			}
		}
		break;
	case MSG_TOGGLE:
		ClearToggles((GLToggle*)src);
		EnableDisableControls();
		break;
	case MSG_TEXT_UPD:
		if (src == curveTotalAngleDegText) {
			double deg;
			if (curveTotalAngleDegText->GetNumber(&deg)) {
				curveTotalAngleRadText->SetText(deg/180.0*PI);
				double radius;
				if (curveRadiusLengthText->GetNumber(&radius)) {
					curveTotalLengthText->SetText(deg / 180.0*PI*radius);
				}
			}
		} else if (src == curveTotalAngleRadText) {
			double rad;
			if (curveTotalAngleRadText->GetNumber(&rad)) {
				curveTotalAngleDegText->SetText(rad / PI*180.0);
				double radius;
				if (curveRadiusLengthText->GetNumber(&radius)) {
					curveTotalLengthText->SetText(rad * radius);
				}
			}
		} else if (src == curveTotalLengthText) {
			double len,radius;
			if (curveTotalLengthText->GetNumber(&len) && curveRadiusLengthText->GetNumber(&radius)) {
				curveTotalAngleDegText->SetText(len / radius * 180.0 / PI);
				curveTotalAngleRadText->SetText(len / radius);
			}
		} else if (src == curveRadiusLengthText) {
			double rad, radius;
			if (curveRadiusLengthText->GetNumber(&radius) && curveTotalAngleRadText->GetNumber(&rad)) {
				curveTotalAngleDegText->SetText(rad * 180.0 / PI);
				curveTotalLengthText->SetText(rad * radius);
			}
		}
	}

	GLWindow::ProcessMessage(src,message);
}

void ExtrudeFacet::ClearToggles(GLToggle* leaveChecked) {
	std::vector<GLToggle*> toggles = { towardsNormalCheckbox,againstNormalCheckbox,offsetCheckbox,curveTowardsNormalCheckbox,curveAgainstNormalCheckbox };
	for (auto& toggle : toggles) 
		toggle->SetState(toggle==leaveChecked);
}

void ExtrudeFacet::EnableDisableControls() {
	distanceTextbox->SetEditable(towardsNormalCheckbox->GetState() || againstNormalCheckbox->GetState());
	dxText->SetEditable(offsetCheckbox->GetState() != 0);
	dyText->SetEditable(offsetCheckbox->GetState() != 0);
	dzText->SetEditable(offsetCheckbox->GetState() != 0);
	curveX0Text->SetEditable(curveTowardsNormalCheckbox->GetState() || curveAgainstNormalCheckbox->GetState());
	curveY0Text->SetEditable(curveTowardsNormalCheckbox->GetState() || curveAgainstNormalCheckbox->GetState());
	curveZ0Text->SetEditable(curveTowardsNormalCheckbox->GetState() || curveAgainstNormalCheckbox->GetState());
	curvedXText->SetEditable(curveTowardsNormalCheckbox->GetState() || curveAgainstNormalCheckbox->GetState());
	curvedYText->SetEditable(curveTowardsNormalCheckbox->GetState() || curveAgainstNormalCheckbox->GetState());
	curvedZText->SetEditable(curveTowardsNormalCheckbox->GetState() || curveAgainstNormalCheckbox->GetState());
	curveRadiusLengthText->SetEditable(curveTowardsNormalCheckbox->GetState() || curveAgainstNormalCheckbox->GetState());
	curveTotalAngleDegText->SetEditable(curveTowardsNormalCheckbox->GetState() || curveAgainstNormalCheckbox->GetState());
	curveTotalAngleRadText->SetEditable(curveTowardsNormalCheckbox->GetState() || curveAgainstNormalCheckbox->GetState());
	curveTotalLengthText->SetEditable(curveTowardsNormalCheckbox->GetState() || curveAgainstNormalCheckbox->GetState());
	curveStepsText->SetEditable(curveTowardsNormalCheckbox->GetState() || curveAgainstNormalCheckbox->GetState());
	curveFacetCenterButton->SetEnabled(curveTowardsNormalCheckbox->GetState() || curveAgainstNormalCheckbox->GetState());
	curveFacetIndex1Button->SetEnabled(curveTowardsNormalCheckbox->GetState() || curveAgainstNormalCheckbox->GetState());
	curveGetBaseButton->SetEnabled(curveTowardsNormalCheckbox->GetState() || curveAgainstNormalCheckbox->GetState());
	curveFacetUButton->SetEnabled(curveTowardsNormalCheckbox->GetState() || curveAgainstNormalCheckbox->GetState());
	curveFacetVButton->SetEnabled(curveTowardsNormalCheckbox->GetState() || curveAgainstNormalCheckbox->GetState());
	curveGetDirButton->SetEnabled(curveTowardsNormalCheckbox->GetState() || curveAgainstNormalCheckbox->GetState());
	facetNXbutton->SetEnabled(curveTowardsNormalCheckbox->GetState() || curveAgainstNormalCheckbox->GetState());
	facetNYbutton->SetEnabled(curveTowardsNormalCheckbox->GetState() || curveAgainstNormalCheckbox->GetState());
	facetNZbutton->SetEnabled(curveTowardsNormalCheckbox->GetState() || curveAgainstNormalCheckbox->GetState());
}

std::optional<size_t> ExtrudeFacet::AssertOneVertexSelected() {
	auto selectedVertices = geom->GetSelectedVertices();
	if (selectedVertices.size()==0) {
		GLMessageBox::Display("No vertex selected", "Can't define direction", GLDLG_OK, GLDLG_ICONINFO);
		return std::nullopt;
	}
	else if (selectedVertices.size() > 1) {
		GLMessageBox::Display("More than one vertex is selected", "Can't define direction", GLDLG_OK, GLDLG_ICONINFO);
		return std::nullopt;
	}
	else return selectedVertices[0];
}

std::optional<size_t> ExtrudeFacet::AssertOneFacetSelected() {
	auto selectedFacets = geom->GetSelectedFacets();
	if (selectedFacets.size() == 0) {
		GLMessageBox::Display("No facet selected", "Can't define source", GLDLG_OK, GLDLG_ICONINFO);
		return std::nullopt;
	}
	else if (selectedFacets.size() > 1) {
		GLMessageBox::Display("More than one facet is selected", "Can't define source", GLDLG_OK, GLDLG_ICONINFO);
		return std::nullopt;
	}
	else return selectedFacets[0];
}