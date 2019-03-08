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
#include "ProfilePlotter.h"
#include "GLApp/GLToolkit.h"
#include "GLApp/GLMessageBox.h"
#include "GLApp/GLButton.h"
#include "GLApp/GLLabel.h"
#include "GLApp/GLCombo.h"
#include "GLApp/GLTextField.h"
#include "GLApp/GLToggle.h"
#include "GLApp/MathTools.h"
#include "GLApp/GLList.h"
#include "GLApp/GLChart/GLChart.h"
#include "Geometry_shared.h"
#include "Facet_shared.h"
#include <math.h>
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

const char* profType[] = {
	"None",
	"Pressure \201 [mbar]",
	"Pressure \202 [mbar]",
	"Incident angle [deg]",
	"Speed [m/s]",
	"Ort. velocity [m/s]",
	"Tan. velocity [m/s]"
};

ProfilePlotter::ProfilePlotter() :GLWindow() {

	int wD = 650;
	int hD = 400;

	SetTitle("Profile plotter");
	SetIconfiable(true);
	nbView = 0;
	worker = NULL;

	lastUpdate = 0.0f;

	nbColors = 8;
	colors[0] = new GLColor(); colors[0]->r = 255; colors[0]->g = 000; colors[0]->b = 055; //red
	colors[1] = new GLColor(); colors[1]->r = 000; colors[1]->g = 000; colors[1]->b = 255; //blue
	colors[2] = new GLColor(); colors[2]->r = 000; colors[2]->g = 204; colors[2]->b = 051; //green
	colors[3] = new GLColor(); colors[3]->r = 000; colors[3]->g = 000; colors[3]->b = 000; //black
	colors[4] = new GLColor(); colors[4]->r = 255; colors[4]->g = 153; colors[4]->b = 051; //orange
	colors[5] = new GLColor(); colors[5]->r = 153; colors[5]->g = 204; colors[5]->b = 255; //light blue
	colors[6] = new GLColor(); colors[6]->r = 153; colors[6]->g = 000; colors[6]->b = 102; //violet
	colors[7] = new GLColor(); colors[7]->r = 255; colors[7]->g = 230; colors[7]->b = 005; //yellow

	chart = new GLChart(0);
	chart->SetBorder(BORDER_BEVEL_IN);
	chart->GetY1Axis()->SetGridVisible(true);
	chart->GetXAxis()->SetGridVisible(true);
	chart->GetY1Axis()->SetAutoScale(true);
	chart->GetY2Axis()->SetAutoScale(true);
	chart->GetY1Axis()->SetAnnotation(VALUE_ANNO);
	chart->GetXAxis()->SetAnnotation(VALUE_ANNO);
	Add(chart);

	dismissButton = new GLButton(0, "Dismiss");
	Add(dismissButton);

	selButton = new GLButton(0, "Show Facet");
	Add(selButton);

	addButton = new GLButton(0, "Add curve");
	Add(addButton);

	removeButton = new GLButton(0, "Remove curve");
	Add(removeButton);

	removeAllButton = new GLButton(0, "Remove all");
	Add(removeAllButton);

	profCombo = new GLCombo(0);
	profCombo->SetEditable(true);
	Add(profCombo);

	normLabel = new GLLabel("Normalize:");
	Add(normLabel);

	normCombo = new GLCombo(0);
	normCombo->SetEditable(true);
	normCombo->SetSize(6);
	normCombo->SetValueAt(0, "None (raw data)");
	normCombo->SetValueAt(1, "Pressure (mbar)");
	normCombo->SetValueAt(2, "Density (1/m3)");
	normCombo->SetValueAt(3, "Speed (m/s)");
	normCombo->SetValueAt(4, "Angle (deg)");
	normCombo->SetValueAt(5, "Normalize to 1");
	normCombo->SetSelectedIndex(1);
	Add(normCombo);

	logYToggle = new GLToggle(0, "Log Y");

	Add(logYToggle);

	correctForGas = new GLToggle(0, "Surface->Volume conversion");
	correctForGas->SetVisible(false);
	Add(correctForGas);

	formulaText = new GLTextField(0, "");
	formulaText->SetEditable(true);
	Add(formulaText);

	formulaBtn = new GLButton(0, "-> Plot");
	Add(formulaBtn);

	// Center dialog
	int wS, hS;
	GLToolkit::GetScreenSize(&wS, &hS);
	int xD = (wS - wD) / 2;
	int yD = (hS - hD) / 2;
	SetBounds(xD, yD, wD, hD);
	SetResizable(true);
	SetMinimumSize(wD, 220);

	RestoreDeviceObjects();

}

void ProfilePlotter::SetBounds(int x, int y, int w, int h) {

	chart->SetBounds(7, 5, w - 15, h - 110);
	profCombo->SetBounds(7, h - 95, 180, 19);
	selButton->SetBounds(190, h - 95, 80, 19);
	addButton->SetBounds(275, h - 95, 80, 19);
	removeButton->SetBounds(360, h - 95, 80, 19);
	removeAllButton->SetBounds(445, h - 95, 80, 19);
	logYToggle->SetBounds(190, h - 70, 40, 19);
	correctForGas->SetBounds(240, h - 70, 80, 19);
	normLabel->SetBounds(7, h - 68, 50, 19);
	normCombo->SetBounds(61, h - 70, 125, 19);
	formulaText->SetBounds(7, h - 45, 350, 19);
	formulaBtn->SetBounds(360, h - 45, 80, 19);;
	dismissButton->SetBounds(w - 100, h - 45, 90, 19);

	GLWindow::SetBounds(x, y, w, h);

}

void ProfilePlotter::Refresh() {

	if (!worker) return;

	//Rebuild selection combo box
	Geometry *geom = worker->GetGeometry();
	size_t nb = geom->GetNbFacet();
	size_t nbProf = 0;
	for (size_t i = 0; i < nb; i++)
		if (geom->GetFacet(i)->sh.isProfile) nbProf++;
	profCombo->Clear();
	if (nbProf) profCombo->SetSize(nbProf);
	nbProf = 0;
	for (size_t i = 0; i < nb; i++) {
		Facet *f = geom->GetFacet(i);
		if (f->sh.isProfile) {
			char tmp[128];
			sprintf(tmp, "F#%zd %s", i + 1, profType[f->sh.profileType]);
			profCombo->SetValueAt(nbProf, tmp, (int)i);
			nbProf++;
		}
	}
	profCombo->SetSelectedIndex(nbProf ? 0 : -1);
	//Remove profiles that aren't present anymore
	for (size_t v = 0; v < nbView; v++)
		if (views[v]->userData1 >= geom->GetNbFacet() || !geom->GetFacet(views[v]->userData1)->sh.isProfile) {
			chart->GetY1Axis()->RemoveDataView(views[v]);
			SAFE_DELETE(views[v]);
			for (size_t j = v; j < nbView - 1; j++) views[j] = views[j + 1];
			nbView--;
		}

	//Update values
	refreshViews();

}

void ProfilePlotter::Display(Worker *w) {

	
    SetWorker(w);
	Refresh();
	SetVisible(true);

}

void ProfilePlotter::Update(float appTime, bool force) {

	if (!IsVisible() || IsIconic()) return;

	if (force) {
		refreshViews();
		lastUpdate = appTime;
		return;
	}

	if ((appTime - lastUpdate > 1.0f || force) && nbView) {
		if (worker->isRunning) refreshViews();
		lastUpdate = appTime;
	}

}

void ProfilePlotter::plot() {

	GLParser *parser = new GLParser();
	parser->SetExpression(formulaText->GetText().c_str());
	if (!parser->Parse()) {
		GLMessageBox::Display(parser->GetErrorMsg(), "Error", GLDLG_OK, GLDLG_ICONERROR);
		SAFE_DELETE(parser);
		return;
	}

	int nbVar = parser->GetNbVariable();
	if (nbVar == 0) {
		GLMessageBox::Display("Variable 'x' not found", "Error", GLDLG_OK, GLDLG_ICONERROR);
		SAFE_DELETE(parser);
		return;
	}
	if (nbVar > 1) {
		GLMessageBox::Display("Too much variables or unknown constant", "Error", GLDLG_OK, GLDLG_ICONERROR);
		SAFE_DELETE(parser);
		return;
	}
	VLIST *var = parser->GetVariableAt(0);
	if (!iequals(var->name, "x")) {
		GLMessageBox::Display("Variable 'x' not found", "Error", GLDLG_OK, GLDLG_ICONERROR);
		SAFE_DELETE(parser);
		return;
	}

	Geometry *geom = worker->GetGeometry();
	GLDataView *v;

	// Check that view is not already added
	bool found = false;
	int i = 0;
	while (i < nbView && !found) {
		found = (views[i]->userData1 == -1);
		if (!found) i++;
	}

	if (found) {
		v = views[i];
		v->SetName(formulaText->GetText().c_str());
		v->Reset();
	}
	else {

		if (nbView < 50) {
			v = new GLDataView();
			v->SetName(formulaText->GetText().c_str());
			v->userData1 = -1;
			chart->GetY1Axis()->AddDataView(v);
			views[nbView] = v;
			nbView++;
		}
		else {
			return;
		}
	}

	// Plot
	for (int i = 0; i < 1000; i++) {
		double x = (double)i;
		double y;
		var->value = x;
		parser->Evaluate(&y);
		v->Add(x, y, false);
	}
	v->CommitChange();

	delete parser;

}

void ProfilePlotter::refreshViews() {

	// Lock during update
	GlobalSimuState* results = worker->GetHits();
	if (!results) return;
	int displayMode = normCombo->GetSelectedIndex();

	Geometry *geom = worker->GetGeometry();

	double scaleY;

	size_t facetHitsSize = (1 + worker->moments.size()) * sizeof(FacetHitBuffer);
	for (int i = 0; i < nbView; i++) {

		GLDataView *v = views[i];
		if (v->userData1 >= 0 && v->userData1 < geom->GetNbFacet()) {
			Facet *f = geom->GetFacet(v->userData1);

			v->Reset();
			std::vector<ProfileSlice>& profilePtr = results->facetStates[v->userData1].momentResults[worker->displayedMoment].profile;
			//ProfileSlice *profilePtr = (ProfileSlice *)(buffer + f->sh.hitOffset + facetHitsSize + worker->displayedMoment*sizeof(ProfileSlice)*PROFILE_SIZE);

			//FacetHitBuffer *fCount = (FacetHitBuffer *)(buffer + f->wp.hitOffset+ worker->displayedMoment*sizeof(FacetHitBuffer));
			//double fnbHit = (double)fCount->nbMCHit;
			//if (fnbHit == 0.0) fnbHit = 1.0;
			if (worker->globalHitCache.globalHits.nbDesorbed > 0){

				switch (displayMode) {
				case 0: //Raw data
					for (int j = 0; j < PROFILE_SIZE; j++)
						v->Add((double)j, profilePtr[j].countEquiv, false);

					break;

				case 1: //Pressure
					scaleY = 1.0 / (f->GetArea() / (double)PROFILE_SIZE*1E-4)* worker->wp.gasMass / 1000 / 6E23 * 0.0100; //0.01: Pa->mbar
					scaleY *= worker->GetMoleculesPerTP(worker->displayedMoment);

					for (int j = 0; j < PROFILE_SIZE; j++)
						v->Add((double)j, profilePtr[j].sum_v_ort*scaleY, false);
					break;
				case 2: //Particle density
					scaleY = 1.0 / ((f->GetArea() * 1E-4) / (double)PROFILE_SIZE);
					scaleY *= worker->GetMoleculesPerTP(worker->displayedMoment) * f->DensityCorrection();
					
					for (int j = 0; j < PROFILE_SIZE; j++)
						v->Add((double)j, profilePtr[j].sum_1_per_ort_velocity*scaleY, false);
					break;
				case 3: {//Velocity
					double sum = 0.0;
					double val;
					double scaleX = f->sh.maxSpeed / (double)PROFILE_SIZE;
					std::vector<double> values;
					values.reserve(PROFILE_SIZE);
					for (int j = 0; j < PROFILE_SIZE; j++) {//count distribution sum
						if (!correctForGas->GetState())
							val = profilePtr[j].countEquiv;
						else
							val = profilePtr[j].countEquiv / (((double)j + 0.5)*scaleX); //fnbhit not needed, sum will take care of normalization
						sum += val;
						values.push_back(val);
					}

					for (int j = 0; j < PROFILE_SIZE; j++)
						v->Add((double)j*scaleX, values[j] / sum, false);
					break; }
				case 4: {//Angle
					double sum = 0.0;
					double val;
					double scaleX = 90.0 / (double)PROFILE_SIZE;
					std::vector<double> values;
					values.reserve(PROFILE_SIZE);
					for (int j = 0; j < PROFILE_SIZE; j++) {//count distribution sum
						if (!correctForGas->GetState())
							val = profilePtr[j].countEquiv;
						else
							val = profilePtr[j].countEquiv / sin(((double)j + 0.5)*PI / 2.0 / (double)PROFILE_SIZE); //fnbhit not needed, sum will take care of normalization
						sum += val;
						values.push_back(val);
					}

					for (int j = 0; j < PROFILE_SIZE; j++)
						v->Add((double)j*scaleX, values[j] / sum, false);
					break; }
				case 5: //To 1 (max value)
					double max = 1.0;

					for (int j = 0; j < PROFILE_SIZE; j++)
					{
						if (profilePtr[j].countEquiv > max) max = profilePtr[j].countEquiv;
					}
					scaleY = 1.0 / (double)max;

					for (int j = 0; j < PROFILE_SIZE; j++)
						v->Add((double)j, profilePtr[j].countEquiv*scaleY, false);
					break;
				}

			}
			v->CommitChange();
		}
	}
	worker->ReleaseHits();
}

void ProfilePlotter::addView(int facet) {

	char tmp[128];
	Geometry *geom = worker->GetGeometry();

	// Check that view is not already added
	bool found = false;
	int i = 0;
	while (i < nbView && !found) {
		found = (views[i]->userData1 == facet);
		if (!found) i++;
	}
	if (found) {
		GLMessageBox::Display("Profile already plotted", "Info", GLDLG_OK, GLDLG_ICONINFO);
		return;
	}
	if (nbView < MAX_VIEWS) {
		Facet *f = geom->GetFacet(facet);
		GLDataView *v = new GLDataView();
		//sprintf(tmp, "F#%d %s", facet + 1, profType[f->wp.profileType]);
		sprintf(tmp, "F#%d", facet + 1);
		v->SetName(tmp);
		v->SetColor(*colors[nbView%nbColors]);
		v->SetMarkerColor(*colors[nbView%nbColors]);
		v->SetLineWidth(2);
		v->userData1 = facet;

		chart->GetY1Axis()->AddDataView(v);
		views[nbView] = v;
		nbView++;
	}

}

void ProfilePlotter::remView(int facet) {

	Geometry *geom = worker->GetGeometry();

	bool found = false;
	int i = 0;
	while (i < nbView && !found) {
		found = (views[i]->userData1 == facet);
		if (!found) i++;
	}
	if (!found) {
		GLMessageBox::Display("Profile not plotted", "Error", GLDLG_OK, GLDLG_ICONERROR);
		return;
	}
	chart->GetY1Axis()->RemoveDataView(views[i]);
	SAFE_DELETE(views[i]);
	for (int j = i; j < nbView - 1; j++) views[j] = views[j + 1];
	nbView--;

}

void ProfilePlotter::Reset() {

	chart->GetY1Axis()->ClearDataView();
	for (int i = 0; i < nbView; i++) SAFE_DELETE(views[i]);
	nbView = 0;

}

void ProfilePlotter::ProcessMessage(GLComponent *src, int message) {
	Geometry *geom = worker->GetGeometry();

	switch (message) {
	case MSG_BUTTON:
		if (src == dismissButton) {
			SetVisible(false);
		}
		else if (src == selButton) {
			int idx = profCombo->GetSelectedIndex();
			if (idx >= 0) { //Something selected (not -1)
				int facetId = profCombo->GetUserValueAt(idx);
				//if (facetId >= 0 && facetId < geom->GetNbFacet()) { //Check commented out: should never be able to select non-existing facet
					geom->UnselectAll();
					geom->GetFacet(facetId)->selected = true;
					geom->UpdateSelection();

					mApp->UpdateFacetParams(true);

					mApp->facetList->SetSelectedRow(profCombo->GetUserValueAt(idx));
					mApp->facetList->ScrollToVisible(profCombo->GetUserValueAt(idx), 1, true);
				//}
			}
		}
		else if (src == addButton) {

			int idx = profCombo->GetSelectedIndex();

			if (idx >= 0) { //Something selected (not -1)
				addView(profCombo->GetUserValueAt(idx));
				refreshViews();
			}
		}
		else if (src == removeButton) {

			int idx = profCombo->GetSelectedIndex();

			if (idx >= 0) remView(profCombo->GetUserValueAt(idx));
			refreshViews();
		}
		else if (src == removeAllButton) {

			Reset();
		}
		else if (src == formulaBtn) {

			plot();
		}
		break;
	case MSG_COMBO:
		if (src == normCombo) {
			int normMode = normCombo->GetSelectedIndex();
			correctForGas->SetVisible(normMode == 3 || normMode == 4);
			refreshViews();
		}
		break;
	case MSG_TOGGLE:
		if (src == logYToggle) {
			chart->GetY1Axis()->SetScale(logYToggle->GetState());
		}
		else if (src == correctForGas) {

			refreshViews();

		}
		break;
	}

	GLWindow::ProcessMessage(src, message);

}

void ProfilePlotter::SetViews(std::vector<int> views) {
	Reset();
	for (int view : views)
		if (view<worker->GetGeometry()->GetNbFacet() && worker->GetGeometry()->GetFacet(view)->sh.isProfile)
			addView(view);
	Refresh();
}

std::vector<int> ProfilePlotter::GetViews() {
	std::vector<int>v;
	v.reserve(nbView);
	for (size_t i = 0; i < nbView; i++)
		v.push_back(views[i]->userData1);
	return v;
}

bool ProfilePlotter::IsLogScaled() {
	return chart->GetY1Axis()->GetScale();
}
void ProfilePlotter::SetLogScaled(bool logScale){
	chart->GetY1Axis()->SetScale(logScale);
	logYToggle->SetState(logScale);
}

void ProfilePlotter::SetWorker(Worker *w) { //for loading views before the full geometry

	worker = w;

}