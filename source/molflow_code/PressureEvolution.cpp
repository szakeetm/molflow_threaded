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
#include "PressureEvolution.h"
#include "GLApp/GLToolkit.h"
#include "GLApp/GLMessageBox.h"
#include "GLApp/GLToggle.h"
#include "GLApp/MathTools.h"
#include "GLApp/GLList.h"
#include "GLApp/GLChart/GLChart.h"
#include "GLApp/GLLabel.h"
#include "GLApp/GLCombo.h"
#include "GLApp/GLButton.h"
#include "GLApp/GLParser.h"
#include "GLApp/GLTextField.h"
#include "Geometry_shared.h"
#include "Facet_shared.h"
#include <math.h>

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

extern const char*profType[];

/**
* \brief Constructor with initialisation for Pressure evolution window (Time/Pressure evolution)
* \param w worker handle
*/
PressureEvolution::PressureEvolution(Worker *w) :GLWindow() {

	int wD = 750;
	int hD = 375;

	SetTitle("Pressure evolution plotter");
	SetIconfiable(true);
	lastUpdate = 0.0f;

	colors = {
		GLColor(255,000,055), //red
		GLColor(000,000,255), //blue
		GLColor(000,204,051), //green
		GLColor(000,000,000), //black
		GLColor(255,153,051), //orange
		GLColor(153,204,255), //light blue
		GLColor(153,000,102), //violet
		GLColor(255,230,005)  //yellow
	};

	chart = new GLChart(0);
	chart->SetBorder(BORDER_BEVEL_IN);
	chart->GetY1Axis()->SetGridVisible(true);
	chart->GetXAxis()->SetGridVisible(true);
	chart->GetXAxis()->SetName("Time (s)");
	chart->GetY1Axis()->SetAutoScale(true);
	chart->GetY2Axis()->SetAutoScale(true);
	chart->GetY1Axis()->SetAnnotation(VALUE_ANNO);
	chart->GetXAxis()->SetAnnotation(VALUE_ANNO);
	Add(chart);

	selButton = new GLButton(0, "<-Show Facet");
	Add(selButton);

	removeButton = new GLButton(0, "Remove");
	Add(removeButton);

	removeAllButton = new GLButton(0, "Remove all");
	Add(removeAllButton);

	addButton = new GLButton(0, "Add selected facet");
	Add(addButton);

	profCombo = new GLCombo(0);
	Add(profCombo);

	logXToggle = new GLToggle(0, "Log X");
	Add(logXToggle);

	logYToggle = new GLToggle(0, "Log Y");
	Add(logYToggle);

	normLabel = new GLLabel("Y scale:");
	Add(normLabel);

	yScaleCombo = new GLCombo(0);
	yScaleCombo->SetEditable(true);
	yScaleCombo->SetSize(5);
	yScaleCombo->SetValueAt(0, "MC Hits");
	yScaleCombo->SetValueAt(1, "Equiv. hits");
	yScaleCombo->SetValueAt(2, "Pressure (mbar)");
	yScaleCombo->SetValueAt(3, "Density (1/m3)");
	yScaleCombo->SetValueAt(4, "Imp.rate (1/s/m2)");
	yScaleCombo->SetSelectedIndex(2); //Pressure by default
	Add(yScaleCombo);

	// Center dialog
	int wS, hS;
	GLToolkit::GetScreenSize(&wS, &hS);
	int xD = (wS - wD) / 2;
	int yD = (hS - hD) / 2;
	SetBounds(xD, yD, wD, hD);
	SetResizable(true);
	SetMinimumSize(wD, 220);

	RestoreDeviceObjects();

	worker = w;
	Refresh();

}

/**
* \brief Sets positions and sizes of all UI elements
* \param x x-coordinate of the element
* \param y y-coordinate of the element
* \param w width of the element
* \param h height of the element
*/
void PressureEvolution::SetBounds(int x, int y, int w, int h) {

	chart->SetBounds(7, 5, w - 15, h - 60);
	profCombo->SetBounds(7, h - 45, 117, 19);
	selButton->SetBounds(130, h - 45, 80, 19);
	removeButton->SetBounds(215, h - 45, 60, 19);
	removeAllButton->SetBounds(280, h - 45, 70, 19);
	addButton->SetBounds(370, h - 45, 110, 19);

	logXToggle->SetBounds(w - 105, h - 45, 40, 19);
	logYToggle->SetBounds(w - 55, h - 45, 40, 19);
	normLabel->SetBounds(490, h - 42, 50, 19);
	yScaleCombo->SetBounds(535, h - 45, 105, 19);

	GLWindow::SetBounds(x, y, w, h);

}

/**
* \brief Rebuilds combo and calls refreshviews
*/
void PressureEvolution::Refresh() {
	//Rebuilds combo and calls refreshviews

	Geometry *geom = worker->GetGeometry();

	//Remove views that aren't present anymore
	for (auto i = views.begin();i!=views.end();) {
		if ((*i)->userData1 >= geom->GetNbFacet()) { //If pointing to non-existent facet
			chart->GetY1Axis()->RemoveDataView(*i);
			SAFE_DELETE(*i);
			i=views.erase(i);
		}
		else {
			i++;
		}
	}
	

	//Construct combo
	profCombo->SetSize(views.size());
	size_t nbProf = 0;
	for (auto& v : views) {
		profCombo->SetValueAt(nbProf++, v->GetName(), v->userData1);
	}
	profCombo->SetSelectedIndex(nbProf ? (int)nbProf-1 : -1);

	//Refresh chart
	refreshChart();
}

/**
* \brief Calls refreshChart if needed
* \param appTime current time of the applicaiton
* \param force if chart should be refreshed no matter what
*/
void PressureEvolution::Update(float appTime, bool force) {
	//Calls refreshChart if needed
	if (!IsVisible() || IsIconic()) return;

	if (force) {
		refreshChart();
		lastUpdate = appTime;
		return;
	}

	if ((appTime - lastUpdate > 1.0f || force) && views.size() > 0) {
		if (worker->isRunning) refreshChart();
		lastUpdate = appTime;
	}

}

/**
* \brief refreshes chart values
*/
void PressureEvolution::refreshChart() {
	//refreshes chart values

	// Lock during update
	GlobalSimuState* results = worker->GetHits();
	if (!results) return;
	int yScaleMode = yScaleCombo->GetSelectedIndex();

	Geometry *geom = worker->GetGeometry();
	//GlobalHitBuffer *gHits = (GlobalHitBuffer *)buffer;
	double nbDes = (double)results->globalHits.globalHits.nbDesorbed;
	double scaleY;
	size_t facetHitsSize = (1 + worker->moments.size()) * sizeof(FacetHitBuffer);

	for (auto& v : views) {

		if (v->userData1 >= 0 && v->userData1 < geom->GetNbFacet()) {
			Facet *f = geom->GetFacet(v->userData1);
			v->Reset();
			
			switch (yScaleMode) {
			case 0: { //MC Hits
				for (size_t m = 1; m <= Min(worker->moments.size(), (size_t)10000); m++) { //max 10000 points
					//FacetHitBuffer* facetHits = (FacetHitBuffer*)(buffer + f->sh.hitOffset + m * sizeof(FacetHitBuffer));
					FacetHitBuffer& facetHits = results->facetStates[v->userData1].momentResults[m].hits;
					v->Add(worker->moments[m - 1], (double)facetHits.nbMCHit, false);
				}
				break;
			}
			case 1: { //Equiv Hits
				for (size_t m = 1; m <= Min(worker->moments.size(), (size_t)10000); m++) { //max 10000 points
					//FacetHitBuffer* facetHits = (FacetHitBuffer*)(buffer + f->sh.hitOffset + m * sizeof(FacetHitBuffer));
					FacetHitBuffer& facetHits = results->facetStates[v->userData1].momentResults[m].hits;
					v->Add(worker->moments[m - 1], facetHits.nbHitEquiv, false);
				}
				break;
			}
			case 2: {//Pressure
				scaleY = 1.0 / nbDes / (f->GetArea() * 1E-4)* worker->wp.gasMass / 1000 / 6E23 * 0.0100; //0.01: Pa->mbar
				scaleY *= worker->wp.totalDesorbedMolecules / worker->wp.timeWindowSize;
				for (size_t m = 1; m <= Min(worker->moments.size(), (size_t)10000); m++) { //max 10000 points
					//FacetHitBuffer* facetHits = (FacetHitBuffer*)(buffer + f->sh.hitOffset + m * sizeof(FacetHitBuffer));
					FacetHitBuffer& facetHits = results->facetStates[v->userData1].momentResults[m].hits;
					v->Add(worker->moments[m - 1], facetHits.sum_v_ort*scaleY, false);
				}
				break;
			}
			case 3: {//Particle density
				scaleY = 1.0 / nbDes / (f->GetArea() * 1E-4);
				scaleY *= worker->wp.totalDesorbedMolecules / worker->wp.timeWindowSize;
				scaleY *= f->DensityCorrection();
				for (size_t m = 1; m <= Min(worker->moments.size(), (size_t)10000); m++) { //max 10000 points
					//FacetHitBuffer* facetHits = (FacetHitBuffer*)(buffer + f->sh.hitOffset + m * sizeof(FacetHitBuffer));
					FacetHitBuffer& facetHits = results->facetStates[v->userData1].momentResults[m].hits;
					v->Add(worker->moments[m - 1], facetHits.sum_1_per_ort_velocity*scaleY, false);
				}
				break;
			}
			case 4: {//Imp.rate
				scaleY = 1.0 / nbDes / (f->GetArea() * 1E-4);
				scaleY *= worker->wp.totalDesorbedMolecules / worker->wp.timeWindowSize;
				for (size_t m = 1; m <= Min(worker->moments.size(), (size_t)10000); m++) { //max 10000 points
					//FacetHitBuffer* facetHits = (FacetHitBuffer*)(buffer + f->sh.hitOffset + m * sizeof(FacetHitBuffer));
					FacetHitBuffer& facetHits = results->facetStates[v->userData1].momentResults[m].hits;
					v->Add(worker->moments[m - 1], facetHits.nbHitEquiv*scaleY, false);
				}
				break;
			}
			}
		}
		v->CommitChange();
	}

	worker->ReleaseHits();
}

/**
* \brief Adds a view to the chart for a specific facet
* \param facetId Id of the facet that should be added
*/
void PressureEvolution::addView(size_t facetId) {

	Geometry *geom = worker->GetGeometry();

	// Check that view is not already added
	{
		bool found = false;

		for (auto v = views.begin(); v != views.end() && !found;v++) {
			found = ((*v)->userData1 == facetId);
		}
		if (found) {
			GLMessageBox::Display("Facet already on chart", "Error", GLDLG_OK, GLDLG_ICONERROR);
			return;
		}
		if (worker->moments.size() > 10000) {
			GLMessageBox::Display("Only the first 10000 moments will be plotted", "Error", GLDLG_OK, GLDLG_ICONWARNING);
		}
	}

	auto v = new GLDataView();
	std::ostringstream tmp;
	tmp << "Facet " << facetId + 1;
	v->SetName(tmp.str().c_str());
	v->SetViewType(TYPE_BAR);
	v->SetMarker(MARKER_DOT);
	GLColor col = chart->GetFirstAvailableColor();
	v->SetColor(col);
	v->SetMarkerColor(col);
	v->userData1 = (int)facetId;
	views.push_back(v);
	chart->GetY1Axis()->AddDataView(v);
	Refresh();
}

/**
* \brief Removes a view from the chart with a specific ID
* \param viewId Id of the view that should be removed
*/
void PressureEvolution::remView(size_t viewId) {

	chart->GetY1Axis()->RemoveDataView(views[viewId]);
	SAFE_DELETE(views[viewId]);
	views.erase(views.begin()+viewId);

}

/**
* \brief Resets view
*/
void PressureEvolution::Reset() {

	chart->GetY1Axis()->ClearDataView();
	for (auto v : views)
		delete v;
	views.clear();
	Refresh();
}

/**
* \brief Function for processing various inputs (button, check boxes etc.)
* \param src Exact source of the call
* \param message Type of the source (button)
*/
void PressureEvolution::ProcessMessage(GLComponent *src, int message) {
	Geometry *geom = worker->GetGeometry();
	switch (message) {
	case MSG_BUTTON:
		if (src == selButton) {
			int idx = profCombo->GetSelectedIndex();
			if (idx >= 0) {
				size_t facetId = profCombo->GetUserValueAt(idx);
				if (facetId >= 0 && facetId < geom->GetNbFacet()) {
					geom->UnselectAll();
					geom->GetFacet(facetId)->selected = true;
					geom->UpdateSelection();
					mApp->UpdateFacetParams(true);
					mApp->facetList->SetSelectedRow((int)facetId);
					mApp->facetList->ScrollToVisible(facetId, 1, true);
				}
			}
		}
		else if (src == addButton) {
			auto selFacets = geom->GetSelectedFacets();
			if (selFacets.size() != 1) {
				GLMessageBox::Display("Select exactly one facet", "Add selected facet to chart", { "Sorry!" }, GLDLG_ICONERROR);
				return;
			}
			else {
				addView(selFacets[0]); //Includes chart refresh
			}
		}
		else if (src == removeButton) {
			int idx = profCombo->GetSelectedIndex();
			if (idx >= 0) remView(idx);
			Refresh();
		}
		else if (src == removeAllButton) {
			Reset();
		}
		break;
	case MSG_COMBO:
		if (src == yScaleCombo) {
			refreshChart();
		}
		break;
	case MSG_TOGGLE:
		if (src == logXToggle) {
			chart->GetXAxis()->SetScale(logXToggle->GetState());
		}
		else if (src == logYToggle) {
			chart->GetY1Axis()->SetScale(logYToggle->GetState());
		}
		break;
	}

	GLWindow::ProcessMessage(src, message);

}