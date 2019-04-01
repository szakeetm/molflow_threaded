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
#include "HistogramPlotter.h"
#include "HistogramSettings.h"
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

HistogramPlotter::HistogramPlotter(Worker *w) :GLTabWindow() {

	worker = w;

	int wD = 750;
	int hD = 400;

	SetTitle("Histogram plotter");
	SetIconfiable(true);
	lastUpdate = 0.0f;

	HistogramMode bounceMode;
	bounceMode.name = "Bounces before absorption";
	bounceMode.XaxisLabel = "Number of bounces";
	modes.push_back(bounceMode);

	HistogramMode distanceMode;
	distanceMode.name = "Flight distance before absorption";
	distanceMode.XaxisLabel = "Distance [cm]";
	modes.push_back(distanceMode);

#ifdef MOLFLOW
	HistogramMode timeMode;
	timeMode.name = "Flight time before absorption";
	timeMode.XaxisLabel = "Time [s]";
	modes.push_back(timeMode);
#endif

	SetPanelNumber((int)modes.size());

	for (int i = 0; i < modes.size(); i++) {
		SetPanelName(i, modes[i].name.c_str());
		modes[i].chart = new GLChart(0);
		modes[i].chart->SetBorder(BORDER_BEVEL_IN);
		modes[i].chart->GetY1Axis()->SetGridVisible(true);
		modes[i].chart->GetXAxis()->SetGridVisible(true);
		modes[i].chart->GetY1Axis()->SetAutoScale(true);
		modes[i].chart->GetY2Axis()->SetAutoScale(true);
		modes[i].chart->GetY1Axis()->SetAnnotation(VALUE_ANNO);
		modes[i].chart->GetXAxis()->SetAnnotation(VALUE_ANNO);
		modes[i].chart->GetXAxis()->SetName(modes[i].XaxisLabel.c_str());
		//modes[i].chart->SetVisible(i == 0);
		Add(i,modes[i].chart);
	}	

	selButton = new GLButton(0, "<-Show Facet");
	GLWindow::Add(selButton);
	
	addButton = new GLButton(0, "Add");
	GLWindow::Add(addButton);

	removeButton = new GLButton(0, "Remove");
	GLWindow::Add(removeButton);

	removeAllButton = new GLButton(0, "Remove all");
	GLWindow::Add(removeAllButton);

	histogramSettingsButton = new GLButton(0, "<< Hist.settings");
	GLWindow::Add(histogramSettingsButton);

	profCombo = new GLCombo(0);
	GLWindow::Add(profCombo);

	logXToggle = new GLToggle(0, "Log X");
	GLWindow::Add(logXToggle);

	logYToggle = new GLToggle(0, "Log Y");
	GLWindow::Add(logYToggle);

	normLabel = new GLLabel("Y scale:");
	GLWindow::Add(normLabel);

	yScaleCombo = new GLCombo(0);
	yScaleCombo->SetEditable(true);
	yScaleCombo->SetSize(2);
	yScaleCombo->SetValueAt(0, "Absolute");
	yScaleCombo->SetValueAt(1, "Normalized");
	yScaleCombo->SetSelectedIndex(0); //Absolute by default
	GLWindow::Add(yScaleCombo);

	UpdateBar(); //Build menubar

	
	int wS, hS;
	//GLToolkit::GetScreenSize(&wS, &hS);
	//int xD = (wS - wD) / 2;
	//int yD = (hS - hD) / 2;
	SetBounds(280, 35, wD, hD);
	SetResizable(true);
	SetMinimumSize(wD, 220);

	RestoreDeviceObjects();
	Refresh();
}

void HistogramPlotter::SetBounds(int x, int y, int w, int h) {

	for (const auto& mode : modes) {
		
		mode.chart->SetBounds( 5, 0, w - 15, h - 85);
	}

	histogramSettingsButton->SetBounds(7, h-70, 90, 18);
	profCombo->SetBounds(110, h - 70, 120, 19);
	selButton->SetBounds(235, h - 70, 75, 19);
	addButton->SetBounds(315, h - 70, 40, 19);
	removeButton->SetBounds(360, h - 70, 45, 19);
	removeAllButton->SetBounds(410, h - 70, 75, 19);
	
	normLabel->SetBounds(w-260, h - 69, 50, 19);
	yScaleCombo->SetBounds(w - 215, h - 70, 105, 19);
	logXToggle->SetBounds(w - 105, h - 70, 40, 19);
	logYToggle->SetBounds(w - 55, h - 70, 40, 19);
	
	

	GLTabWindow::SetBounds(x, y, w, h);

}

void HistogramPlotter::Refresh() {
	//Rebuilds combo and calls refreshviews

	Geometry *geom = worker->GetGeometry();

	//Collect histogram facets for currently displayed mode
	size_t modeId = GetSelectedTabIndex();
	std::vector<int> histogramFacetIds;

	bool recordGlobal = ((modeId == HISTOGRAM_MODE_BOUNCES && worker->wp.globalHistogramParams.recordBounce)
		|| (modeId == HISTOGRAM_MODE_DISTANCE && worker->wp.globalHistogramParams.recordDistance)
#ifdef MOLFLOW
		|| (modeId == HISTOGRAM_MODE_TIME && worker->wp.globalHistogramParams.recordTime)
#endif
		);
	if (recordGlobal) histogramFacetIds.push_back(-1); // -1 == Global histogram

	for (size_t i = 0; i < geom->GetNbFacet(); i++) {
		if (
			(modeId == HISTOGRAM_MODE_BOUNCES && geom->GetFacet(i)->sh.facetHistogramParams.recordBounce)
			|| (modeId == HISTOGRAM_MODE_DISTANCE && geom->GetFacet(i)->sh.facetHistogramParams.recordDistance)
#ifdef MOLFLOW
			|| (modeId == HISTOGRAM_MODE_TIME && geom->GetFacet(i)->sh.facetHistogramParams.recordTime)
#endif
			) {
			histogramFacetIds.push_back((int)i);
		}
	}
	


	//Construct combo
	profCombo->SetSize(histogramFacetIds.size());
	size_t nbProf = 0;
	for (const int& id : histogramFacetIds) {
		std::ostringstream name;
		if (id == -1)
			name << "Global";
		else
			name << "Facet #" << (id + 1);
		profCombo->SetValueAt(nbProf++, name.str().c_str(),id);
	}
	profCombo->SetSelectedIndex(histogramFacetIds.size() ? 0 : -1);
	//Refresh chart
	refreshChart();
}



void HistogramPlotter::Update(float appTime, bool force) {
	//Calls refreshChart if needed
	if (!IsVisible() || IsIconic()) return;

	if (force) {
		refreshChart();
		lastUpdate = appTime;
		return;
	}

	size_t modeId = GetSelectedTabIndex();
	if ((appTime - lastUpdate > 1.0f || force) && modes[modeId].views.size() > 0) {
		if (worker->isRunning) refreshChart();
		lastUpdate = appTime;
	}

}

void HistogramPlotter::refreshChart() {
	//refreshes chart values
	size_t modeId = GetSelectedTabIndex();

	int yScaleMode = yScaleCombo->GetSelectedIndex();
	Geometry *geom = worker->GetGeometry();
	for (auto& v : modes[modeId].views) {

		if (v->userData1 >= -1 && v->userData1 < (int)geom->GetNbFacet()) {
			v->Reset();

			auto [histogramValues, xMax, xSpacing,nbBins] = GetHistogramValues(v->userData1, modeId);

			modes[modeId].chart->GetXAxis()->SetMaximum(xMax);
			
			switch (yScaleMode) {
				case 0: { //Absolute
					size_t plotLimit = std::min(histogramValues->size(), (size_t)1000);
					for (size_t i = 0; i < plotLimit;i++) {
						v->Add((double)i*xSpacing, (*histogramValues)[i]);
					}
					break;
				}
				case 1: { //Normalized
					double yMax = 0.0;
					size_t plotLimit = std::min(histogramValues->size(), (size_t)1000);
					for (size_t i = 0; i < plotLimit; i++) {
						yMax = std::max(yMax, (*histogramValues)[i]);
					}
					double scaleY = 1.0 / yMax; //Multiplication is faster than division (unless compiler optimizes this away)
					for (size_t i = 0; i < plotLimit; i++) {
						v->Add((double)i*xSpacing, (*histogramValues)[i]*scaleY);
					}
					break;
				}
			}
		}
		v->CommitChange();
	}
}

std::tuple<std::vector<double>*,double,double,size_t> HistogramPlotter::GetHistogramValues(int facetId,size_t modeId)
{
	//facetId: -1 if global, otherwise facet id
	//modeId: bounce/distance/time (0/1/2)
	//returns pointer to values, max X value, X bin size, number of values (which can't be derived from the vector size when nothing's recorded yet)

	Geometry *geom = worker->GetGeometry();
	double xMax;
	double xSpacing;
	size_t nbBins;

	std::vector<double>* histogramValues;

	if (facetId == -1) { //Global histogram
		if (modeId == HISTOGRAM_MODE_BOUNCES) {
			histogramValues = &(worker->globalHistogramCache.nbHitsHistogram);
			xMax = (double)worker->wp.globalHistogramParams.nbBounceMax;
			xSpacing = (double)worker->wp.globalHistogramParams.nbBounceBinsize;
			nbBins = worker->wp.globalHistogramParams.GetBounceHistogramSize();
		}
		else if (modeId == HISTOGRAM_MODE_DISTANCE) {
			histogramValues = &(worker->globalHistogramCache.distanceHistogram);
			xMax = worker->wp.globalHistogramParams.distanceMax;
			xSpacing = worker->wp.globalHistogramParams.distanceBinsize;
			nbBins = worker->wp.globalHistogramParams.GetDistanceHistogramSize();
		}
#ifdef MOLFLOW
		else if (modeId == HISTOGRAM_MODE_TIME) {
			histogramValues = &(worker->globalHistogramCache.timeHistogram);
			worker->wp.globalHistogramParams.timeMax;
			xSpacing = worker->wp.globalHistogramParams.timeBinsize;
			nbBins = worker->wp.globalHistogramParams.GetTimeHistogramSize();
		}
#endif
	}
	else { //Facet histogram
		if (modeId == HISTOGRAM_MODE_BOUNCES) {
			histogramValues = &(geom->GetFacet(facetId)->facetHistogramCache.nbHitsHistogram);
			(double)geom->GetFacet(facetId)->sh.facetHistogramParams.nbBounceMax;
			xSpacing = (double)(geom->GetFacet(facetId)->sh.facetHistogramParams.nbBounceBinsize);
			nbBins = geom->GetFacet(facetId)->sh.facetHistogramParams.GetBounceHistogramSize();
		}
		else if (modeId == HISTOGRAM_MODE_DISTANCE) {
			histogramValues = &(geom->GetFacet(facetId)->facetHistogramCache.distanceHistogram);
			geom->GetFacet(facetId)->sh.facetHistogramParams.distanceMax;
			xSpacing = geom->GetFacet(facetId)->sh.facetHistogramParams.distanceBinsize;
			nbBins = geom->GetFacet(facetId)->sh.facetHistogramParams.GetDistanceHistogramSize();
		}
#ifdef MOLFLOW
		else if (modeId == HISTOGRAM_MODE_TIME) {
			histogramValues = &(geom->GetFacet(facetId)->facetHistogramCache.timeHistogram);
			geom->GetFacet(facetId)->sh.facetHistogramParams.timeMax;
			xSpacing = geom->GetFacet(facetId)->sh.facetHistogramParams.timeBinsize;
			nbBins = geom->GetFacet(facetId)->sh.facetHistogramParams.GetTimeHistogramSize();
		}
#endif

	}
	return { histogramValues,xMax,xSpacing,nbBins };
}

void HistogramPlotter::addView(int facetId) {
	int modeId = GetSelectedTabIndex();
	Geometry *geom = worker->GetGeometry();

	// Check that view is not already added
	{
		bool found = false;

		for (auto v = modes[modeId].views.begin(); v != modes[modeId].views.end() && !found;v++) {
			found = ((*v)->userData1 == facetId);
		}
		if (found) {
			GLMessageBox::Display("Histogram already on chart", "Error", GLDLG_OK, GLDLG_ICONERROR);
			return;
		}
	}

	auto v = new GLDataView();
	std::ostringstream tmp;
	if (facetId == -1)
		tmp << "Global";
	else
		tmp << "Facet " << facetId + 1;
	v->SetName(tmp.str().c_str());
	v->SetViewType(TYPE_BAR);
	v->SetMarker(MARKER_DOT);
	GLColor col = modes[modeId].chart->GetFirstAvailableColor();
	v->SetColor(col);
	v->SetMarkerColor(col);
	v->userData1 = facetId;
	modes[modeId].views.push_back(v);
	modes[modeId].chart->GetY1Axis()->AddDataView(v);
	auto[histogramValues, xMax, xSpacing, nbBins] = GetHistogramValues(facetId, modeId);
	if (nbBins > 1000) {
		GLMessageBox::Display("For performance reasons only the first 1000 histogram points will be plotted.\n"
			"This, among others, will cut the last histogram point representing out-of-limit values.\n"
			"You can still get the data of the whole histogram by using the To clipboard button", "More than 1000 histogram points", { "OK" }, GLDLG_ICONWARNING);
	}
	//Refresh();
}

void HistogramPlotter::remView(int facetId) {
	size_t modeId = GetSelectedTabIndex();
	Geometry *geom = worker->GetGeometry();

	bool found = false;
	size_t i = 0;
	for (;!found && i<modes[modeId].views.size();i++) {
		found = (modes[modeId].views[i]->userData1 == facetId);
	}
	if (!found) {
		GLMessageBox::Display("Histogram not on chart", "Error", GLDLG_OK, GLDLG_ICONERROR);
		return;
	}
	modes[modeId].chart->GetY1Axis()->RemoveDataView(modes[modeId].views[--i]);
	SAFE_DELETE(modes[modeId].views[i]);
	modes[modeId].views.erase(modes[modeId].views.begin() + i);
}

void HistogramPlotter::Reset() {
	for (auto& mode : modes) {
		mode.chart->GetY1Axis()->ClearDataView();
		for (auto v : mode.views)
			delete v;
		mode.views.clear();
	}
	Refresh();
}

void HistogramPlotter::ProcessMessage(GLComponent *src, int message) {
	size_t modeId = GetSelectedTabIndex();
	Geometry *geom = worker->GetGeometry();
	switch (message) {
	case MSG_BUTTON:
		if (src == selButton) {

			int idx = profCombo->GetSelectedIndex();
			if (idx >= 0) {
				int facetId = profCombo->GetUserValueAt(idx);
				if (facetId >= 0 && facetId <geom->GetNbFacet()) { //Not global histogram
					geom->UnselectAll();
					geom->GetFacet(profCombo->GetUserValueAt(idx))->selected = true;
					geom->UpdateSelection();

					mApp->UpdateFacetParams(true);

					mApp->facetList->SetSelectedRow(profCombo->GetUserValueAt(idx));
					mApp->facetList->ScrollToVisible(profCombo->GetUserValueAt(idx), 1, true);
				}
			}
		}
		else if (src == addButton) {

			int idx = profCombo->GetSelectedIndex();

			if (idx >= 0) {
				addView(profCombo->GetUserValueAt(idx));
				refreshChart();
			}
		}
		else if (src == removeButton) {

			int idx = profCombo->GetSelectedIndex();

			if (idx >= 0) remView(profCombo->GetUserValueAt(idx));
			refreshChart();
		}
		else if (src == removeAllButton) {

			Reset();
		}
		else if (src == histogramSettingsButton) {
			if (!mApp->histogramSettings || !mApp->histogramSettings->IsVisible())
			{
				SAFE_DELETE(mApp->histogramSettings);
				mApp->histogramSettings = new HistogramSettings(geom, worker);
				mApp->histogramSettings->Refresh(geom->GetSelectedFacets());
				mApp->histogramSettings->SetVisible(true);
			} else {
				mApp->histogramSettings->SetVisible(false);				
			}
		}
		break;
	case MSG_COMBO:
		if (src == yScaleCombo) {
			refreshChart();
		}
	case MSG_TAB:
	/*
	//Hide/show managed by GLTabWindow

			modeId = GetSelectedTabIndex();
			for (size_t i = 0; i < modes.size(); i++) {
				modes[i].chart->SetVisible(i == modeId);
			}
	*/
			Refresh(); //Rebuild prof combo
		
	
	break;
	case MSG_TOGGLE:
		for (auto& mode : modes) {
			if (src == logXToggle) {
				mode.chart->GetXAxis()->SetScale(logXToggle->GetState());
			}
			else if (src == logYToggle) {
				mode.chart->GetY1Axis()->SetScale(logYToggle->GetState());
			}
		}
		break;
	}

	GLTabWindow::ProcessMessage(src, message);

}

