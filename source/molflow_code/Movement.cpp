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

#include "Movement.h"
#include "GLApp/GLTitledPanel.h"
#include "GLApp/GLToolkit.h"
#include "GLApp/GLMessageBox.h"
#include "GLApp/GLButton.h"
#include "GLApp/GLTextField.h"
#include "GLApp/GLLabel.h"
#include "GLApp/GLToggle.h"
#include "GLApp/GLTitledPanel.h"
#include "Geometry_shared.h"
#include "MolFlow.h"

extern MolFlow *mApp;
int    axisMode2;

/**
* \brief Constructor with basic initialisation for the movement window (Tools/moving parts)
* \param g pointer to Geometry
* \param w pointer to Worker handler
*/
Movement::Movement(Geometry *g,Worker *w):GLWindow() {

	int wD = 537;
	int hD = 312;
	GLTitledPanel* groupBox1 = new GLTitledPanel("Motion type");
	groupBox1->SetBounds(15, 49, 495, 206);
	Add(groupBox1);

	label1 = new GLLabel("Movement parameters set here will only apply\nto facets which are marked \"moving\" in their parameters");
	label1->SetBounds(12, 9, 261, 26);
	Add(label1);

	label16 = new GLLabel("Hz");
	groupBox1->SetCompBounds(label16, 284, 165, 20, 13);
	groupBox1->Add(label16);

	label14 = new GLLabel("deg/s");
	groupBox1->SetCompBounds(label14, 201, 165, 35, 13);
	groupBox1->Add(label14);

	label15 = new GLLabel("RPM");
	groupBox1->SetCompBounds(label15, 122, 165, 31, 13);
	groupBox1->Add(label15);

	hzText = new GLTextField(0, "0");
	groupBox1->SetCompBounds(hzText, 308, 161, 40, 20);
	groupBox1->Add(hzText);

	degText = new GLTextField(0, "0");
	groupBox1->SetCompBounds(degText, 238, 161, 40, 20);
	groupBox1->Add(degText);

	rpmText = new GLTextField(0, "0");
	groupBox1->SetCompBounds(rpmText, 156, 161, 40, 20);
	groupBox1->Add(rpmText);

	label17 = new GLLabel("Rotation speed:");
	groupBox1->SetCompBounds(label17, 22, 165, 82, 13);
	groupBox1->Add(label17);

	button2 = new GLButton(0, "Base to sel. vertex");
	groupBox1->SetCompBounds(button2, 354, 131, 123, 23);
	groupBox1->Add(button2);

	label10 = new GLLabel("rz");
	groupBox1->SetCompBounds(label10, 284, 135, 15, 13);
	groupBox1->Add(label10);

	label11 = new GLLabel("ry");
	groupBox1->SetCompBounds(label11, 201, 136, 15, 13);
	groupBox1->Add(label11);

	ryText = new GLTextField(0, "0");
	groupBox1->SetCompBounds(ryText, 238, 132, 40, 20);
	groupBox1->Add(ryText);

	rzText = new GLTextField(0, "0");
	groupBox1->SetCompBounds(rzText, 308, 133, 40, 20);
	groupBox1->Add(rzText);

	label12 = new GLLabel("rx");
	groupBox1->SetCompBounds(label12, 135, 136, 15, 13);
	groupBox1->Add(label12);

	rxText = new GLTextField(0, "0");
	groupBox1->SetCompBounds(rxText, 156, 132, 40, 20);
	groupBox1->Add(rxText);

	label13 = new GLLabel("Axis direction:");
	groupBox1->SetCompBounds(label13, 22, 136, 72, 13);
	groupBox1->Add(label13);

	button1 = new GLButton(0, "Use selected vertex");
	groupBox1->SetCompBounds(button1, 354, 101, 123, 23);
	groupBox1->Add(button1);

	label6 = new GLLabel("az");
	groupBox1->SetCompBounds(label6, 284, 106, 18, 13);
	groupBox1->Add(label6);

	label7 = new GLLabel("ay");
	groupBox1->SetCompBounds(label7, 201, 106, 18, 13);
	groupBox1->Add(label7);

	ayText = new GLTextField(0, "0");
	groupBox1->SetCompBounds(ayText, 238, 102, 40, 20);
	groupBox1->Add(ayText);

	azText = new GLTextField(0, "0");
	groupBox1->SetCompBounds(azText, 308, 103, 40, 20);
	groupBox1->Add(azText);

	label8 = new GLLabel("ax");
	groupBox1->SetCompBounds(label8, 135, 106, 18, 13);
	groupBox1->Add(label8);

	axText = new GLTextField(0, "0");
	groupBox1->SetCompBounds(axText, 156, 102, 40, 20);
	groupBox1->Add(axText);

	label9 = new GLLabel("Axis base point:");
	groupBox1->SetCompBounds(label9, 22, 106, 81, 13);
	groupBox1->Add(label9);

	checkBox3 = new GLToggle(0, "Rotation around axis");
	groupBox1->SetCompBounds(checkBox3, 6, 86, 123, 17);
	groupBox1->Add(checkBox3);

	label5 = new GLLabel("vz");
	groupBox1->SetCompBounds(label5, 284, 62, 18, 13);
	groupBox1->Add(label5);

	label4 = new GLLabel("vy");
	groupBox1->SetCompBounds(label4, 201, 63, 18, 13);
	groupBox1->Add(label4);

	vyText = new GLTextField(0, "0");
	groupBox1->SetCompBounds(vyText, 238, 60, 40, 20);
	groupBox1->Add(vyText);

	vzText = new GLTextField(0, "0");
	groupBox1->SetCompBounds(vzText, 308, 60, 40, 20);
	groupBox1->Add(vzText);

	label3 = new GLLabel("vx");
	groupBox1->SetCompBounds(label3, 135, 63, 18, 13);
	groupBox1->Add(label3);

	vxText = new GLTextField(0, "0");
	groupBox1->SetCompBounds(vxText, 156, 60, 40, 20);
	groupBox1->Add(vxText);

	label2 = new GLLabel("Velocity vector [m/s]:");
	groupBox1->SetCompBounds(label2, 22, 63, 107, 13);
	groupBox1->Add(label2);

	checkBox2 = new GLToggle(0, "Fixed (same velocity vector everywhere)");
	groupBox1->SetCompBounds(checkBox2, 6, 43, 215, 17);
	groupBox1->Add(checkBox2);

	checkBox1 = new GLToggle(0, "No moving parts");
	groupBox1->SetCompBounds(checkBox1, 6, 19, 103, 17);
	checkBox1->SetState(true);
	groupBox1->Add(checkBox1);

	button3 = new GLButton(0, "Apply");
	button3->SetBounds(176, 261, 75, 23);
	Add(button3);

	button4 = new GLButton(0, "Dismiss");
	button4->SetBounds(266, 261, 75, 23);
	Add(button4);

	group1 = { vxText, vyText, vzText }; //C++11 syntax
	group2 = { axText, ayText, azText,
		rxText, ryText, rzText,
		rpmText, degText, hzText };

	for (auto& textField : group1){
		textField->SetEditable(false);
	}
	for (auto& textField : group2){
		textField->SetEditable(false);
	}

	SetTitle("Define moving parts");
	// Center dialog
	int wS, hS;
	GLToolkit::GetScreenSize(&wS, &hS);
	int xD = (wS - wD) / 2;
	int yD = (hS - hD) / 2;
	SetBounds(xD, yD, wD, hD);

	geom = g;
	work = w;
	mode = MODE_NOMOVE;

}

/**
* \brief Function for processing various inputs (button, check boxes etc.)
* \param src Exact source of the call
* \param message Type of the source (button)
*/
void Movement::ProcessMessage(GLComponent *src,int message) {

	switch(message) {
		
	case MSG_TOGGLE:
		UpdateToggle(src);
		break;

	case MSG_BUTTON:

		if(src==button4) { //cancel

			GLWindow::ProcessMessage(NULL,MSG_CLOSE);

		} else if (src==button3) { //Apply
			double a, b, c, u, v, w;
			Vector3d AXIS_P0, AXIS_DIR;
			double degPerSec;
			
			
			
			switch (mode) {

			case MODE_FIXED:
				if (!(vxText->GetNumber(&a))) {
					GLMessageBox::Display("Invalid vx coordinate", "Error", GLDLG_OK, GLDLG_ICONERROR);
					return;
				}
				if (!(vyText->GetNumber(&b))) {
					GLMessageBox::Display("Invalid vy coordinate", "Error", GLDLG_OK, GLDLG_ICONERROR);
					return;
				}
				if (!(vzText->GetNumber(&c))) {
					GLMessageBox::Display("Invalid vz coordinate", "Error", GLDLG_OK, GLDLG_ICONERROR);
					return;
				}
				AXIS_DIR.x = a; AXIS_DIR.y = b; AXIS_DIR.z = c;
				break;
			
			case MODE_ROTATING:
				
				if (!(axText->GetNumber(&a))) {
					GLMessageBox::Display("Invalid ax coordinate", "Error", GLDLG_OK, GLDLG_ICONERROR);
					return;
				}
				if (!(ayText->GetNumber(&b))) {
					GLMessageBox::Display("Invalid ay coordinate", "Error", GLDLG_OK, GLDLG_ICONERROR);
					return;
				}
				if (!(azText->GetNumber(&c))) {
					GLMessageBox::Display("Invalid az coordinate", "Error", GLDLG_OK, GLDLG_ICONERROR);
					return;
				}
				if (!(rxText->GetNumber(&u))) {
					GLMessageBox::Display("Invalid rx coordinate", "Error", GLDLG_OK, GLDLG_ICONERROR);
					return;
				}
				if (!(ryText->GetNumber(&v))) {
					GLMessageBox::Display("Invalid ry coordinate", "Error", GLDLG_OK, GLDLG_ICONERROR);
					return;
				}
				if (!(rzText->GetNumber(&w))) {
					GLMessageBox::Display("Invalid rz coordinate", "Error", GLDLG_OK, GLDLG_ICONERROR);
					return;
				}
				if (!(degText->GetNumber(&degPerSec))) {
					GLMessageBox::Display("Invalid rotation speed (deg/s field)", "Error", GLDLG_OK, GLDLG_ICONERROR);
					return;
				}

				AXIS_P0.x = a; AXIS_P0.y = b; AXIS_P0.z = c;
				AXIS_DIR.x = u; AXIS_DIR.y = v; AXIS_DIR.z = w;

				if (AXIS_DIR.Norme() < 1E-5) {
					GLMessageBox::Display("The rotation vector is shorter than 1E-5 cm.\n"
						"Very likely this is a null vector\n"
						"If not, increase its coefficients while keeping its direction", "Error", GLDLG_OK, GLDLG_ICONERROR);
					return;
				}

				break;
			}
			
			if (mApp->AskToReset()) {
				work->wp.motionType = mode;
				switch (mode) {
				case MODE_FIXED:
					work->wp.motionVector2 = AXIS_DIR;
					break;
				case MODE_ROTATING: 
					work->wp.motionVector1 = AXIS_P0;
					work->wp.motionVector2 = AXIS_DIR.Normalized() * (degPerSec / 180.0 * 3.14159);
					break;
				}

				work->Reload(); 
				mApp->UpdateFacetlistSelected();
				mApp->UpdateViewers();
				//GLWindowManager::FullRepaint();
				mApp->changedSinceSave = true;
				//GLWindow::ProcessMessage(NULL, MSG_CLOSE);
				return;
			}
		}
		else if (src == button1){ //Use selected vertex as base
			size_t nbs = geom->GetNbSelectedVertex();
			if (nbs != 1) {
				std::ostringstream strstr;
				strstr << "Exactly one vertex needs to be selected.\n(You have selected " << nbs << ".)";
				GLMessageBox::Display(strstr.str().c_str(), "Can't use vertex as base", GLDLG_OK, GLDLG_ICONWARNING);
				return;
			}
			else {
				UpdateToggle(checkBox3);
				for (int i = 0; i < geom->GetNbVertex(); i++) {
					if (geom->GetVertex(i)->selected) {
						axText->SetText(geom->GetVertex(i)->x);
						ayText->SetText(geom->GetVertex(i)->y);
						azText->SetText(geom->GetVertex(i)->z);
						break;
					}
				}
			}
		}
		else if (src == button2) {
			size_t nbs = geom->GetNbSelectedVertex();
			if (nbs != 1) {
				std::ostringstream strstr;
				strstr << "Exactly one vertex needs to be selected.\n(You have selected " << nbs << ".)";
				GLMessageBox::Display(strstr.str().c_str(), "Can't use vertex as direction", GLDLG_OK, GLDLG_ICONWARNING);
				return;
			}
			else {
				UpdateToggle(checkBox3);
				double ax, ay, az;
				if (!axText->GetNumber(&ax)) {
					GLMessageBox::Display("Wrong ax value", "Can't use vertex as direction", GLDLG_OK, GLDLG_ICONWARNING);
					return;
				}
				if (!ayText->GetNumber(&ay)) {
					GLMessageBox::Display("Wrong ay value", "Can't use vertex as direction", GLDLG_OK, GLDLG_ICONWARNING);
					return;
				}
				if (!azText->GetNumber(&az)) {
					GLMessageBox::Display("Wrong az value", "Can't use vertex as direction", GLDLG_OK, GLDLG_ICONWARNING);
					return;
				}
				for (int i = 0; i < geom->GetNbVertex(); i++) {
					if (geom->GetVertex(i)->selected) {
						rxText->SetText(geom->GetVertex(i)->x-ax);
						ryText->SetText(geom->GetVertex(i)->y-ay);
						rzText->SetText(geom->GetVertex(i)->z-az);
						break;
					}
				}
			}
		}
		break;
	case MSG_TEXT_UPD:
		if (src == rpmText || src == degText || src == hzText) {
			double num;
			if (((GLTextField*)src)->GetNumber(&num)) { //User entered an interpretable number
				if (src == rpmText) {
					degText->SetText(num * 6);
					hzText->SetText(num / 60);
				}
				else if (src == degText) {
					rpmText->SetText(num / 6);
					hzText->SetText(num / 360);
				}
				else if (src == hzText) {
					rpmText->SetText(num * 60);
					degText->SetText(num * 360);
				}
			}
		}
		break;
	}
	GLWindow::ProcessMessage(src,message);
}

/**
* \brief Updating GUI when checkboxes (for motion types) are toggled
* \param src Exact source of the call (checkbox)
*/
void Movement::UpdateToggle(GLComponent *src) {

	if (src == checkBox1) {
		mode = MODE_NOMOVE;
	}
	else if (src == checkBox2){
		mode = MODE_FIXED;
	}
	else if (src == checkBox3) {
		mode = MODE_ROTATING;
	}
	else {
		throw Error("Unknown movement mode");
	}
	
	checkBox1->SetState(src == checkBox1);
	checkBox2->SetState(src == checkBox2);
	checkBox3->SetState(src == checkBox3);

	for (auto& textBox : group1) {
		textBox->SetEditable(src == checkBox2);
	}

	for (auto& textBox : group2) {
		textBox->SetEditable(src == checkBox3);
	}
}

/**
* \brief Updates values in the GUI for the Movements window
*/
void Movement::Update() {
	
	mode = work->wp.motionType;
	
	checkBox1->SetState(work->wp.motionType == 0);
	checkBox2->SetState(work->wp.motionType == 1);
	checkBox3->SetState(work->wp.motionType == 2);

	if (work->wp.motionType == 0) {
		for (auto& textBox : group1) {
			textBox->SetText("0");
		}
		for (auto& textBox : group2) {
			textBox->SetText("0");
		}
	}
	else if (work->wp.motionType == 1) {
		for (auto& textBox : group1) {
			textBox->SetEditable(true);
		}
		for (auto& textBox : group2) {
			textBox->SetText("0");
		}
		vxText->SetText(work->wp.motionVector2.x);
		vyText->SetText(work->wp.motionVector2.y);
		vzText->SetText(work->wp.motionVector2.z);
	}
	else if (work->wp.motionType == 2) {
		for (auto& textBox : group1) {
			textBox->SetText("0");
		}
		for (auto& textBox : group2) {
			textBox->SetEditable(true);
		}
		axText->SetText(work->wp.motionVector1.x);
		ayText->SetText(work->wp.motionVector1.y);
		azText->SetText(work->wp.motionVector1.z);
		Vector3d rot = work->wp.motionVector2.Normalized();
		rxText->SetText(rot.x);
		ryText->SetText(rot.y);
		rzText->SetText(rot.z);
		double num=work->wp.motionVector2.Norme()/3.14159*180.0;
		degText->SetText(num);
		rpmText->SetText(num / 6);
		hzText->SetText(num / 360);
	}
}