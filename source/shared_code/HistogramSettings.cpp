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
#include "HistogramSettings.h"
#include "HistogramPlotter.h" //To call refresh

#include "GLApp/GLToggle.h"
#include "GLApp/GLTextField.h"
#include "GLApp/GLLabel.h"
#include "GLApp/GLTextField.h"
#include "GLApp/GLButton.h"
#include "GLApp/GLTitledPanel.h"
#include "GLApp/GLToolkit.h"
#include "GLApp/GLWindowManager.h"
#include "GLApp/GLMessageBox.h"
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

HistogramSettings::HistogramSettings(Geometry *g, Worker *w):GLWindow() {

	int wD = 270;
	int panelHeight = 215;
	int hD = 2*panelHeight+10;

#ifdef MOLFLOW
	hD += 6 * 25; //Time parameters
	panelHeight += 50;
#endif

	SetTitle("Histogram settings");

	//Global histogram settings
	globalSettingsPanel = new GLTitledPanel("Global histogram");
	globalSettingsPanel->SetBounds(5, 5, wD - 11, panelHeight);
	Add(globalSettingsPanel);

	globalRecordBounceToggle = new GLToggle(0, "Record bounces until absorbtion");
	globalSettingsPanel->SetCompBounds(globalRecordBounceToggle, 5, 15, 175, 25);
	globalSettingsPanel->Add(globalRecordBounceToggle);

	GLLabel* globalLabel1 = new GLLabel("Max recorded no. of bounces:");
	globalSettingsPanel->SetCompBoundsRelativeTo(globalRecordBounceToggle, globalLabel1, 0, 25, globalRecordBounceToggle->GetWidth(), 20);
	globalSettingsPanel->Add(globalLabel1);

	globalHitLimitText = new GLTextField(0, "");
	globalSettingsPanel->SetCompBoundsRelativeTo(globalLabel1, globalHitLimitText, globalLabel1->GetWidth()+5, 0, 70, globalLabel1->GetHeight());
	globalSettingsPanel->Add(globalHitLimitText);

	GLLabel* globalLabel2 = new GLLabel("Bounces bin size:");
	globalSettingsPanel->SetCompBoundsRelativeTo(globalLabel1, globalLabel2, 0, 25, globalLabel1->GetWidth(), globalLabel1->GetHeight());
	globalSettingsPanel->Add(globalLabel2);

	globalHitBinsizeText = new GLTextField(0, "");
	globalSettingsPanel->SetCompBoundsRelativeTo(globalHitLimitText, globalHitBinsizeText, 0, 25, globalHitLimitText->GetWidth(), globalLabel1->GetHeight());
	globalSettingsPanel->Add(globalHitBinsizeText);

	globalRecordDistanceToggle = new GLToggle(0, "Record flight distance until absorption");
	globalSettingsPanel->SetCompBoundsRelativeTo(globalLabel2, globalRecordDistanceToggle, 0, 25, globalLabel1->GetWidth(), globalLabel1->GetHeight());
	globalSettingsPanel->Add(globalRecordDistanceToggle);
	
	GLLabel* globalLabel3 = new GLLabel("Max recorded flight distance (cm):");
	globalSettingsPanel->SetCompBoundsRelativeTo(globalRecordDistanceToggle, globalLabel3, 0, 25, globalLabel1->GetWidth(), globalLabel1->GetHeight());
	globalSettingsPanel->Add(globalLabel3);

	globalDistanceLimitText = new GLTextField(0, "");
	globalSettingsPanel->SetCompBoundsRelativeTo(globalHitBinsizeText, globalDistanceLimitText, 0, 50, globalHitLimitText->GetWidth(), globalLabel1->GetHeight());
	globalSettingsPanel->Add(globalDistanceLimitText);

	GLLabel* globalLabel4 = new GLLabel("Distance bin size (cm):");
	globalSettingsPanel->SetCompBoundsRelativeTo(globalLabel3, globalLabel4, 0, 25, globalLabel1->GetWidth(), globalLabel1->GetHeight());
	globalSettingsPanel->Add(globalLabel4);

	globalDistanceBinsizeText = new GLTextField(0, "");
	globalSettingsPanel->SetCompBoundsRelativeTo(globalDistanceLimitText, globalDistanceBinsizeText, 0, 25, globalHitLimitText->GetWidth(), globalLabel1->GetHeight());
	globalSettingsPanel->Add(globalDistanceBinsizeText);

#ifdef MOLFLOW
	globalRecordTimeToggle = new GLToggle(0, "Record flight time until absorption");
	globalSettingsPanel->SetCompBoundsRelativeTo(globalLabel4, globalRecordTimeToggle, 0, 25, globalLabel1->GetWidth(), globalLabel1->GetHeight());
	globalSettingsPanel->Add(globalRecordTimeToggle);

	GLLabel* globalLabel5 = new GLLabel("Max recorded flight time (s):");
	globalSettingsPanel->SetCompBoundsRelativeTo(globalRecordTimeToggle, globalLabel5, 0, 25, globalLabel1->GetWidth(), globalLabel1->GetHeight());
	globalSettingsPanel->Add(globalLabel5);

	globalTimeLimitText = new GLTextField(0, "");
	globalSettingsPanel->SetCompBoundsRelativeTo(globalDistanceBinsizeText, globalTimeLimitText, 0, 50, globalHitLimitText->GetWidth(), globalLabel1->GetHeight());
	globalSettingsPanel->Add(globalTimeLimitText);

	GLLabel* globalLabel6 = new GLLabel("Time bin size (s):");
	globalSettingsPanel->SetCompBoundsRelativeTo(globalLabel5, globalLabel6, 0, 25, globalLabel1->GetWidth(), globalLabel1->GetHeight());
	globalSettingsPanel->Add(globalLabel6);

	globalTimeBinsizeText = new GLTextField(0, "");
	globalSettingsPanel->SetCompBoundsRelativeTo(globalTimeLimitText, globalTimeBinsizeText, 0, 25, globalHitLimitText->GetWidth(), globalLabel1->GetHeight());
	globalSettingsPanel->Add(globalTimeBinsizeText);
#endif

	globalMemoryEstimateLabel = new GLLabel("Memory estimate of global histogram:");
	globalSettingsPanel->SetCompBoundsRelativeTo(globalLabel6, globalMemoryEstimateLabel, 0, 25, globalLabel3->GetWidth(), globalLabel1->GetHeight());
	globalSettingsPanel->Add(globalMemoryEstimateLabel);


	//Facet histogram settings
	facetSettingsPanel = new GLTitledPanel("Facet histogram");
	SetCompBoundsRelativeTo(globalSettingsPanel, facetSettingsPanel, 0, globalSettingsPanel->GetHeight() + 5, globalSettingsPanel->GetWidth(), globalSettingsPanel->GetHeight());
	Add(facetSettingsPanel);

	facetRecordBounceToggle = new GLToggle(0, "Record bounces until absorbtion");
	facetSettingsPanel->SetCompBounds(facetRecordBounceToggle, 5, 15, 175, 25);
	facetSettingsPanel->Add(facetRecordBounceToggle);

	GLLabel* facetLabel1 = new GLLabel("Max recorded no. of bounces:");
	facetSettingsPanel->SetCompBoundsRelativeTo(facetRecordBounceToggle, facetLabel1, 0, 25, facetRecordBounceToggle->GetWidth(), 20);
	facetSettingsPanel->Add(facetLabel1);

	facetHitLimitText = new GLTextField(0, "");
	facetSettingsPanel->SetCompBoundsRelativeTo(facetLabel1, facetHitLimitText, facetLabel1->GetWidth() + 5, 0, 70, facetLabel1->GetHeight());
	facetSettingsPanel->Add(facetHitLimitText);

	GLLabel* facetLabel2 = new GLLabel("Bounces bin size:");
	facetSettingsPanel->SetCompBoundsRelativeTo(facetLabel1, facetLabel2, 0, 25, facetLabel1->GetWidth(), facetLabel1->GetHeight());
	facetSettingsPanel->Add(facetLabel2);

	facetHitBinsizeText = new GLTextField(0, "");
	facetSettingsPanel->SetCompBoundsRelativeTo(facetHitLimitText, facetHitBinsizeText, 0, 25, facetHitLimitText->GetWidth(), facetLabel1->GetHeight());
	facetSettingsPanel->Add(facetHitBinsizeText);

	facetRecordDistanceToggle = new GLToggle(0, "Record flight distance until absorption");
	facetSettingsPanel->SetCompBoundsRelativeTo(facetLabel2, facetRecordDistanceToggle, 0, 25, facetLabel1->GetWidth(), facetLabel1->GetHeight());
	facetSettingsPanel->Add(facetRecordDistanceToggle);

	GLLabel* facetLabel3 = new GLLabel("Max recorded flight distance (cm):");
	facetSettingsPanel->SetCompBoundsRelativeTo(facetRecordDistanceToggle, facetLabel3, 0, 25, facetLabel1->GetWidth(), facetLabel1->GetHeight());
	facetSettingsPanel->Add(facetLabel3);

	facetDistanceLimitText = new GLTextField(0, "");
	facetSettingsPanel->SetCompBoundsRelativeTo(facetHitBinsizeText, facetDistanceLimitText, 0, 50, facetHitLimitText->GetWidth(), facetLabel1->GetHeight());
	facetSettingsPanel->Add(facetDistanceLimitText);

	GLLabel* facetLabel4 = new GLLabel("Distance bin size (cm):");
	facetSettingsPanel->SetCompBoundsRelativeTo(facetLabel3, facetLabel4, 0, 25, facetLabel1->GetWidth(), facetLabel1->GetHeight());
	facetSettingsPanel->Add(facetLabel4);

	facetDistanceBinsizeText = new GLTextField(0, "");
	facetSettingsPanel->SetCompBoundsRelativeTo(facetDistanceLimitText, facetDistanceBinsizeText, 0, 25, facetHitLimitText->GetWidth(), facetLabel1->GetHeight());
	facetSettingsPanel->Add(facetDistanceBinsizeText);

#ifdef MOLFLOW
	facetRecordTimeToggle = new GLToggle(0, "Record flight time until absorption");
	facetSettingsPanel->SetCompBoundsRelativeTo(facetLabel4, facetRecordTimeToggle, 0, 25, facetLabel1->GetWidth(), facetLabel1->GetHeight());
	facetSettingsPanel->Add(facetRecordTimeToggle);

	GLLabel* facetLabel5 = new GLLabel("Max recorded flight time (s):");
	facetSettingsPanel->SetCompBoundsRelativeTo(facetRecordTimeToggle, facetLabel5, 0, 25, facetLabel1->GetWidth(), facetLabel1->GetHeight());
	facetSettingsPanel->Add(facetLabel5);

	facetTimeLimitText = new GLTextField(0, "");
	facetSettingsPanel->SetCompBoundsRelativeTo(facetDistanceBinsizeText, facetTimeLimitText, 0, 50, facetHitLimitText->GetWidth(), facetLabel1->GetHeight());
	facetSettingsPanel->Add(facetTimeLimitText);

	GLLabel* facetLabel6 = new GLLabel("Time bin size (s):");
	facetSettingsPanel->SetCompBoundsRelativeTo(facetLabel5, facetLabel6, 0, 25, facetLabel1->GetWidth(), facetLabel1->GetHeight());
	facetSettingsPanel->Add(facetLabel6);

	facetTimeBinsizeText = new GLTextField(0, "");
	facetSettingsPanel->SetCompBoundsRelativeTo(facetTimeLimitText, facetTimeBinsizeText, 0, 25, facetHitLimitText->GetWidth(), facetLabel1->GetHeight());
	facetSettingsPanel->Add(facetTimeBinsizeText);
#endif

	facetMemoryEstimateLabel = new GLLabel("Memory estimate of facet histogram:");
	facetSettingsPanel->SetCompBoundsRelativeTo(facetLabel6, facetMemoryEstimateLabel, 0, 25, facetLabel3->GetWidth(), facetLabel1->GetHeight());
	facetSettingsPanel->Add(facetMemoryEstimateLabel);
	



	applyButton = new GLButton(0,"Apply");
	applyButton->SetBounds(wD/2-50,hD-44,100,21);
	Add(applyButton);
	

	// Right center
	SetBounds(5,35,wD,hD); //Default position

	RestoreDeviceObjects();
	
	this->geom = g;
	this->work = w;
}

bool HistogramSettings::Apply() {
	//Check input, return false if error, otherwise apply and return true
	
	bool globalRecBounce;
	size_t globalHitLimit; bool doGlobalHitLimit=false;
	size_t globalHitBinsize; bool doGlobalHitBinsize=false;
	bool globalRecDistance;
	double globalDistanceLimit; bool doGlobalDistanceLimit=false;
	double globalDistanceBinsize; bool doGlobalDistanceBinsize=false;
#ifdef MOLFLOW
	bool globalRecTime;
	double globalTimeLimit; bool doGlobalTimeLimit=false;
	double globalTimeBinsize; bool doGlobalTimeBinsize=false;
#endif

	bool facetRecBounce; bool doFacetRecBounce = false;
	size_t facetHitLimit; bool doFacetHitLimit = false;
	size_t facetHitBinsize; bool doFacetHitBinsize = false;
	bool facetRecDistance; bool doFacetRecDistance = false;
	double facetDistanceLimit; bool doFacetDistanceLimit = false;
	double facetDistanceBinsize; bool doFacetDistanceBinsize = false;
#ifdef MOLFLOW
	bool facetRecTime; bool doFacetRecTime = false;
	double facetTimeLimit; bool doFacetTimeLimit = false;
	double facetTimeBinsize; bool doFacetTimeBinsize = false;
#endif

	globalRecBounce = globalRecordBounceToggle->GetState();

	if (globalRecBounce) {

		if (globalHitLimitText->GetText() != "...")
		{
			if (!globalHitLimitText->GetNumberSizeT(&globalHitLimit) || globalHitLimit < 0) {
				GLMessageBox::Display("Global bounce limit must be a non-negative integer", "Histogram parameter error", GLDLG_OK, GLDLG_ICONERROR);
				return false;
			}
			doGlobalHitLimit = true;
		}

		if (globalHitBinsizeText->GetText() != "...")
		{
			if (!globalHitBinsizeText->GetNumberSizeT(&globalHitBinsize) || globalHitBinsize < 1) {
				GLMessageBox::Display("Global bounce bin size must be a positive integer", "Histogram parameter error", GLDLG_OK, GLDLG_ICONERROR);
				return false;
			}
			doGlobalHitBinsize = true;
		}
	}

		globalRecDistance = globalRecordDistanceToggle->GetState();

		if (globalRecDistance) {
			if (globalDistanceLimitText->GetText() != "...") {
				if (!globalDistanceLimitText->GetNumber(&globalDistanceLimit) || globalDistanceLimit < 0) {
					GLMessageBox::Display("Global distance limit must be a non-negative scalar", "Histogram parameter error", GLDLG_OK, GLDLG_ICONERROR);
					return false;
				}
				doGlobalDistanceLimit = true;
			}

			if (globalDistanceBinsizeText->GetText() != "...") {
				
				if (!globalDistanceBinsizeText->GetNumber(&globalDistanceBinsize) || globalDistanceBinsize <= 0) {
					GLMessageBox::Display("Global distance bin size must be a positive scalar", "Histogram parameter error", GLDLG_OK, GLDLG_ICONERROR);
					return false;
				}
				doGlobalDistanceBinsize = true;
			}
		}

#ifdef MOLFLOW
		globalRecTime = globalRecordTimeToggle->GetState();

		if (globalRecTime) {
			if (globalTimeLimitText->GetText() != "...") {
				
				if (!globalTimeLimitText->GetNumber(&globalTimeLimit) || globalTimeLimit < 0) {
					GLMessageBox::Display("Global time limit must be a non-negative scalar", "Histogram parameter error", GLDLG_OK, GLDLG_ICONERROR);
					return false;
				}
				doGlobalTimeLimit = true;
			}

			if (globalTimeBinsizeText->GetText() != "...") {
				
				if (!globalTimeBinsizeText->GetNumber(&globalTimeBinsize) || globalTimeBinsize <= 0) {
					GLMessageBox::Display("Global time bin size must be a positive scalar", "Histogram parameter error", GLDLG_OK, GLDLG_ICONERROR);
					return false;
				}
				doGlobalTimeBinsize = true;
			}
		}
#endif
	


	//FACETS
	int facetRecBounceStatus = facetRecordBounceToggle->GetState();
	if (facetRecBounceStatus != 2) {
		facetRecBounce = (bool)facetRecBounceStatus;
		doFacetRecBounce = true;
	}

	if (doFacetRecBounce && facetRecBounce) {

		if (facetHitLimitText->GetText() != "...")
		{
			if (!facetHitLimitText->GetNumberSizeT(&facetHitLimit) || facetHitLimit < 0) {
				GLMessageBox::Display("Facet bounce limit must be a non-negative integer", "Histogram parameter error", GLDLG_OK, GLDLG_ICONERROR);
				return false;
			}
			doFacetHitLimit = true;
		}

		if (facetHitBinsizeText->GetText() != "...")
		{
			if (!facetHitBinsizeText->GetNumberSizeT(&facetHitBinsize) || facetHitBinsize < 1) {
				GLMessageBox::Display("Facet bounce bin size must be a positive integer", "Histogram parameter error", GLDLG_OK, GLDLG_ICONERROR);
				return false;
			}
			doFacetHitBinsize = true;
		}
	}

		int facetRecDistanceStatus = facetRecordDistanceToggle->GetState();
		if (facetRecDistanceStatus != 2) {
			facetRecDistance = (bool)facetRecDistanceStatus;
			doFacetRecDistance = true;
		}

		if (doFacetRecDistance && facetRecDistance) {
			if (facetDistanceLimitText->GetText() != "...") {
				if (!facetDistanceLimitText->GetNumber(&facetDistanceLimit) || facetDistanceLimit < 0) {
					GLMessageBox::Display("Facet distance limit must be a non-negative scalar", "Histogram parameter error", GLDLG_OK, GLDLG_ICONERROR);
					return false;
				}
				doFacetDistanceLimit = true;
			}

			if (facetDistanceBinsizeText->GetText() != "...") {

				if (!facetDistanceBinsizeText->GetNumber(&facetDistanceBinsize) || facetDistanceBinsize <= 0) {
					GLMessageBox::Display("Facet distance bin size must be a positive scalar", "Histogram parameter error", GLDLG_OK, GLDLG_ICONERROR);
					return false;
				}
				doFacetDistanceBinsize = true;
			}
		}

#ifdef MOLFLOW
		int facetRecTimeStatus = facetRecordTimeToggle->GetState();
		if (facetRecTimeStatus != 2) {
			facetRecTime = (bool)facetRecTimeStatus;
			doFacetRecTime = true;
		}

		if (doFacetRecTime && facetRecTime) {
			if (facetTimeLimitText->GetText() != "...") {

				if (!facetTimeLimitText->GetNumber(&facetTimeLimit) || facetTimeLimit < 0) {
					GLMessageBox::Display("Facet time limit must be a non-negative scalar", "Histogram parameter error", GLDLG_OK, GLDLG_ICONERROR);
					return false;
				}
				doFacetTimeLimit = true;
			}

			if (facetTimeBinsizeText->GetText() != "...") {

				if (!facetTimeBinsizeText->GetNumber(&facetTimeBinsize) || facetTimeBinsize <= 0) {
					GLMessageBox::Display("Facet time bin size must be a positive scalar", "Histogram parameter error", GLDLG_OK, GLDLG_ICONERROR);
					return false;
				}
				doFacetTimeBinsize = true;
			}
		}
#endif
	

	if (mApp->AskToReset()) {
		//Apply
		work->wp.globalHistogramParams.recordBounce = globalRecBounce;
		if (doGlobalHitLimit) work->wp.globalHistogramParams.nbBounceMax = globalHitLimit;
		if (doGlobalHitBinsize) work->wp.globalHistogramParams.nbBounceBinsize = globalHitBinsize;
		work->wp.globalHistogramParams.recordDistance = globalRecDistance;
		if (doGlobalDistanceLimit) work->wp.globalHistogramParams.distanceMax = globalDistanceLimit;
		if (doGlobalDistanceBinsize) work->wp.globalHistogramParams.distanceBinsize = globalDistanceBinsize;
#ifdef MOLFLOW
		work->wp.globalHistogramParams.recordTime = globalRecTime;
		if (doGlobalTimeLimit) work->wp.globalHistogramParams.timeMax = globalTimeLimit;
		if (doGlobalTimeBinsize) work->wp.globalHistogramParams.timeBinsize = globalTimeBinsize;
#endif

		auto selectedFacets = geom->GetSelectedFacets();
		for (const auto& facetId : selectedFacets) {
			Facet* f = geom->GetFacet(facetId);
			if (doFacetRecBounce) f->sh.facetHistogramParams.recordBounce = facetRecBounce;
			if (doFacetHitLimit) f->sh.facetHistogramParams.nbBounceMax = facetHitLimit;
			if (doFacetHitBinsize) f->sh.facetHistogramParams.nbBounceBinsize = facetHitBinsize;
			if (doFacetRecDistance) f->sh.facetHistogramParams.recordDistance = facetRecDistance;
			if (doFacetDistanceLimit) f->sh.facetHistogramParams.distanceMax = facetDistanceLimit;
			if (doFacetDistanceBinsize) f->sh.facetHistogramParams.distanceBinsize = facetDistanceBinsize;
#ifdef MOLFLOW
			if (doFacetRecTime) f->sh.facetHistogramParams.recordTime = facetRecTime;
			if (doFacetTimeLimit) f->sh.facetHistogramParams.timeMax = facetTimeLimit;
			if (doFacetTimeBinsize) f->sh.facetHistogramParams.timeBinsize = facetTimeBinsize;
#endif
		}

		mApp->changedSinceSave = true;
		work->RealReload();
		work->Update(mApp->m_fTime); //To refresh histogram cache
		if (mApp->histogramPlotter) mApp->histogramPlotter->Refresh();
	}
	

	return true;
}

void HistogramSettings::Refresh(const std::vector<size_t>& selectedFacetIds) {
	//Update displayed info based on selected facets
	globalRecordBounceToggle->SetState(work->wp.globalHistogramParams.recordBounce);
	globalHitLimitText->SetText(work->wp.globalHistogramParams.nbBounceMax);
	globalHitBinsizeText->SetText(work->wp.globalHistogramParams.nbBounceBinsize);
	globalRecordDistanceToggle->SetState(work->wp.globalHistogramParams.recordDistance);
	globalDistanceLimitText->SetText(work->wp.globalHistogramParams.distanceMax);
	globalDistanceBinsizeText->SetText(work->wp.globalHistogramParams.distanceBinsize);
#ifdef MOLFLOW
	globalRecordTimeToggle->SetState(work->wp.globalHistogramParams.recordTime);
	globalTimeLimitText->SetText(work->wp.globalHistogramParams.timeMax);
	globalTimeBinsizeText->SetText(work->wp.globalHistogramParams.timeBinsize);
#endif

	bool hasFacetSelected = selectedFacetIds.size() > 0;
	if (!hasFacetSelected) {
		facetRecordBounceToggle->SetState(false);
		facetHitLimitText->SetText("");
		facetHitBinsizeText->SetText("");
		facetRecordDistanceToggle->SetState(false);
		facetDistanceLimitText->SetText("");
		facetDistanceBinsizeText->SetText("");
#ifdef MOLFLOW
		facetRecordTimeToggle->SetState(false);
		facetTimeLimitText->SetText("");
		facetTimeBinsizeText->SetText("");
#endif
	}
	facetRecordBounceToggle->SetEnabled(hasFacetSelected);
	facetRecordDistanceToggle->SetEnabled(hasFacetSelected);
	facetRecordTimeToggle->SetEnabled(hasFacetSelected);
	
	if (hasFacetSelected) {
		//Fill in facet-specific text
		bool recordBounceEqual = true, bounceMaxEqual = true, bounceBinsizeEqual = true,
			recordDistanceEqual = true, distanceMaxEqual = true, distanceBinsizeEqual = true;
#ifdef MOLFLOW	
		bool recordTimeEqual = true, timeMaxEqual = true, timeBinsizeEqual = true;
#endif
		Facet* f0 = geom->GetFacet(selectedFacetIds[0]);
		bool recBounce = f0->sh.facetHistogramParams.recordBounce;
		size_t bounceMax = f0->sh.facetHistogramParams.nbBounceMax;
		size_t bounceBinsize = f0->sh.facetHistogramParams.nbBounceBinsize;
		bool recDist = f0->sh.facetHistogramParams.recordDistance;
		double distMax = f0->sh.facetHistogramParams.distanceMax;
		double distBinsize = f0->sh.facetHistogramParams.distanceBinsize;
#ifdef MOLFLOW
		bool recTime = f0->sh.facetHistogramParams.recordTime;
		double timeMax = f0->sh.facetHistogramParams.timeMax;
		double timeBinsize = f0->sh.facetHistogramParams.timeBinsize;
#endif

		for (size_t i = 1; i < selectedFacetIds.size();i++) {
			Facet* f = geom->GetFacet(selectedFacetIds[i]);
			recordBounceEqual = recordBounceEqual && (f->sh.facetHistogramParams.recordBounce == recBounce);
			bounceMaxEqual = bounceMaxEqual && (f->sh.facetHistogramParams.nbBounceMax == bounceMax);
			bounceBinsizeEqual = bounceBinsizeEqual && (f->sh.facetHistogramParams.nbBounceBinsize== bounceBinsize);
			recordDistanceEqual = recordDistanceEqual && (f->sh.facetHistogramParams.recordDistance == recDist);
			distanceMaxEqual = distanceMaxEqual && (f->sh.facetHistogramParams.distanceMax == distMax);
			distanceBinsizeEqual = distanceBinsizeEqual && (f->sh.facetHistogramParams.distanceBinsize == distBinsize);
#ifdef MOLFLOW
			recordTimeEqual = recordDistanceEqual && (f->sh.facetHistogramParams.recordDistance == recDist);
			timeMaxEqual = timeMaxEqual && (f->sh.facetHistogramParams.timeMax == timeMax);
			timeBinsizeEqual = timeBinsizeEqual && (f->sh.facetHistogramParams.timeBinsize == timeBinsize);
#endif
		}

		facetRecordBounceToggle->AllowMixedState(!recordBounceEqual);
		facetRecordBounceToggle->SetState(recordBounceEqual ? recBounce : 2);
		if (bounceMaxEqual) {
			facetHitLimitText->SetText(bounceMax);
		}
		else {
			facetHitLimitText->SetText("...");
		}
		if (bounceBinsizeEqual) {
			facetHitBinsizeText->SetText(bounceBinsize);
		}
		else {
			facetHitBinsizeText->SetText("...");
		}

		facetRecordDistanceToggle->AllowMixedState(!recordDistanceEqual);
		facetRecordDistanceToggle->SetState(recordDistanceEqual ? recDist : 2);
		if (distanceMaxEqual) {
			facetDistanceLimitText->SetText(distMax);
		}
		else {
			facetDistanceLimitText->SetText("...");
		}
		if (distanceBinsizeEqual) {
			facetDistanceBinsizeText->SetText(distBinsize);
		}
		else {
			facetDistanceBinsizeText->SetText("...");
		}
#ifdef MOLFLOW
		facetRecordTimeToggle->AllowMixedState(!recordTimeEqual);
		facetRecordTimeToggle->SetState(recordTimeEqual ? recTime : 2);
		if (timeMaxEqual) {
			facetTimeLimitText->SetText(timeMax);
		}
		else {
			facetTimeLimitText->SetText("...");
		}
		if (timeBinsizeEqual) {
			facetTimeBinsizeText->SetText(timeBinsize);
		}
		else {
			facetTimeBinsizeText->SetText("...");
		}
#endif

	}

	EnableDisableControls();
}

void HistogramSettings::EnableDisableControls() {
	globalHitLimitText->SetEditable(globalRecordBounceToggle->GetState());
	globalHitBinsizeText->SetEditable(globalRecordBounceToggle->GetState());
	globalDistanceLimitText->SetEditable(globalRecordDistanceToggle->GetState());
	globalDistanceBinsizeText->SetEditable(globalRecordDistanceToggle->GetState());
#ifdef MOLFLOW
	globalTimeLimitText->SetEditable(globalRecordTimeToggle->GetState());
	globalTimeBinsizeText->SetEditable(globalRecordTimeToggle->GetState());
#endif

	facetHitLimitText->SetEditable(facetRecordBounceToggle->GetState()==1);
	facetHitBinsizeText->SetEditable(facetRecordBounceToggle->GetState()==1);
	facetDistanceLimitText->SetEditable(facetRecordDistanceToggle->GetState()==1);
	facetDistanceBinsizeText->SetEditable(facetRecordDistanceToggle->GetState()==1);
#ifdef MOLFLOW
	facetTimeLimitText->SetEditable(facetRecordTimeToggle->GetState()==1);
	facetTimeBinsizeText->SetEditable(facetRecordTimeToggle->GetState()==1);
#endif
}

void HistogramSettings::ProcessMessage(GLComponent *src,int message) {
	

	switch (message) {
		
		case MSG_BUTTON:

			if (src==applyButton) {
				//Set histogram parameters on selected facets
				Apply();
			}
			break;
		
		case MSG_TOGGLE:
			EnableDisableControls();
			break;
		
	}
	GLWindow::ProcessMessage(src,message);
}

