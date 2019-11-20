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
#include <math.h>

#include "FacetAdvParams.h"
#include "Facet_shared.h"
#include "GLApp/GLLabel.h"
#include "GLApp/GLToolkit.h"
#include "GLApp/GLMessageBox.h"
#include "GLApp/MathTools.h"
#include "GLApp/GLToggle.h"
#include "GLApp/GLButton.h"
#include "GLApp/GLTextField.h"
#include "GLApp/GLTitledPanel.h"
//#include "GLApp/GLFileBox.h"
//#include "GLApp/GLProgress.h"
#include "GLApp/GLCombo.h"
//#include "Worker.h"
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

/**
* \brief Constructor for the Advanced facet parameters panel
* \param w Pointer to a worker
*
* This is the constructor for the class
*/
FacetAdvParams::FacetAdvParams(Worker *w) :GLWindow() {

	worker = w;
	geom = w->GetGeometry();

	SetIconfiable(true);

	int wD = 320;
	int hD = 694;
	aPanel = new GLTitledPanel("Texture properties");
	aPanel->SetBounds(5, 3, 309, 123);
	Add(aPanel);
	mPanel = new GLTitledPanel("Texture cell / memory");
	mPanel->SetBounds(5, 129, 309, 44);
	Add(mPanel);
	vPanel = new GLTitledPanel("View settings");
	vPanel->SetBounds(5, 383, 309, 44);
	Add(vPanel);
	desPanel = new GLTitledPanel("Dynamic desorption");
	desPanel->SetBounds(5, 433, 309, 69);
	Add(desPanel);
	paramPanel = new GLTitledPanel("Additional parameters");
	paramPanel->SetBounds(5, 177, 309, 200);
	Add(paramPanel);
	angleMapPanel = new GLTitledPanel("Incident angle distribution");
	angleMapPanel->SetBounds(5, 508, 309, 149);
	Add(angleMapPanel);
	lengthText = new GLTextField(0, "");
	aPanel->SetCompBounds(lengthText, 180, 36, 72, 18);
	aPanel->Add(lengthText);

	perCm = new GLLabel("cells/cm");
	aPanel->SetCompBounds(perCm, 125, 39, 40, 12);
	aPanel->Add(perCm);

	resolutionText = new GLTextField(0, "");
	aPanel->SetCompBounds(resolutionText, 60, 36, 62, 18);
	aPanel->Add(resolutionText);

	l5 = new GLLabel("Resolution:");
	aPanel->SetCompBounds(l5, 5, 39, 52, 12);
	aPanel->Add(l5);

	enableBtn = new GLToggle(0, "Enable texture");
	aPanel->SetCompBounds(enableBtn, 5, 18, 83, 16);
	aPanel->Add(enableBtn);

	recordDesBtn = new GLToggle(0, "Count desorption");
	aPanel->SetCompBounds(recordDesBtn, 5, 60, 94, 16);
	aPanel->Add(recordDesBtn);

	perCell = new GLLabel("cm/cell");
	aPanel->SetCompBounds(perCell, 260, 39, 35, 12);
	aPanel->Add(perCell);

	recordDirBtn = new GLToggle(0, "Record direction vectors");
	aPanel->SetCompBounds(recordDirBtn, 165, 102, 125, 16);
	aPanel->Add(recordDirBtn);

	recordTransBtn = new GLToggle(0, "Count transparent pass");
	aPanel->SetCompBounds(recordTransBtn, 165, 81, 120, 16);
	aPanel->Add(recordTransBtn);

	recordReflBtn = new GLToggle(0, "Count reflection");
	aPanel->SetCompBounds(recordReflBtn, 165, 60, 89, 16);
	aPanel->Add(recordReflBtn);

	recordACBtn = new GLToggle(0, "Angular coefficient");
	aPanel->SetCompBounds(recordACBtn, 5, 102, 101, 16);
	aPanel->Add(recordACBtn);

	recordAbsBtn = new GLToggle(0, "Count absorption");
	aPanel->SetCompBounds(recordAbsBtn, 5, 81, 94, 16);
	aPanel->Add(recordAbsBtn);

	showTexture = new GLToggle(0, "Draw Texture");
	vPanel->SetCompBounds(showTexture, 10, 18, 80, 16);
	vPanel->Add(showTexture);

	showVolume = new GLToggle(0, "Draw Volume");
	vPanel->SetCompBounds(showVolume, 110, 18, 81, 16);
	vPanel->Add(showVolume);

	cellText = new GLTextField(0, "");
	mPanel->SetCompBounds(cellText, 195, 19, 107, 18);
	mPanel->Add(cellText);

	l8 = new GLLabel("Cells:");
	mPanel->SetCompBounds(l8, 166, 22, 29, 12);
	mPanel->Add(l8);

	ramText = new GLTextField(0, "");
	mPanel->SetCompBounds(ramText, 58, 19, 100, 18);
	mPanel->Add(ramText);

	l7 = new GLLabel("Memory:");
	mPanel->SetCompBounds(l7, 10, 22, 43, 12);
	mPanel->Add(l7);

	quickApply = new GLButton(0, "<- Change draw");
	vPanel->SetCompBounds(quickApply, 200, 15, 99, 20);
	vPanel->Add(quickApply);

	fileYieldText = new GLTextField(0, "");
	desPanel->SetCompBounds(fileYieldText, 205, 18, 58, 18);
	desPanel->Add(fileYieldText);

	label3 = new GLLabel("mol/ph");
	desPanel->SetCompBounds(label3, 265, 21, 33, 12);
	desPanel->Add(label3);

	label1 = new GLLabel("Avg");
	desPanel->SetCompBounds(label1, 155, 21, 48, 12);
	desPanel->Add(label1);

	label2 = new GLLabel("Use file:");
	desPanel->SetCompBounds(label2, 5, 21, 39, 12);
	desPanel->Add(label2);

	facetMovingToggle = new GLToggle(0, "Moving part");
	paramPanel->SetCompBounds(facetMovingToggle, 9, 132, 74, 16);
	paramPanel->Add(facetMovingToggle);

	facetSuperDest = new GLTextField(0, "");
	paramPanel->SetCompBounds(facetSuperDest, 198, 111, 101, 18);
	paramPanel->Add(facetSuperDest);

	label8 = new GLLabel("Link to:");
	paramPanel->SetCompBounds(label8, 157, 114, 35, 12);
	paramPanel->Add(label8);

	facetStructure = new GLTextField(0, "");
	paramPanel->SetCompBounds(facetStructure, 59, 111, 91, 18);
	paramPanel->Add(facetStructure);

	label7 = new GLLabel("Structure:");
	paramPanel->SetCompBounds(label7, 9, 114, 46, 12);
	paramPanel->Add(label7);

	facetTeleport = new GLTextField(0, "");
	paramPanel->SetCompBounds(facetTeleport, 154, 87, 145, 18);
	paramPanel->Add(facetTeleport);

	label4 = new GLLabel("Teleport to facet:");
	paramPanel->SetCompBounds(label4, 9, 90, 74, 12);
	paramPanel->Add(label4);

	label5 = new GLLabel("Accomodation coefficient:");
	paramPanel->SetCompBounds(label5, 9, 66, 126, 13);
	paramPanel->Add(label5);

	label6 = new GLLabel("Reflection:");
	paramPanel->SetCompBounds(label6, 10, 22, 50, 12);
	paramPanel->Add(label6);

	facetUseDesFile = new GLCombo(0);
	desPanel->SetCompBounds(facetUseDesFile, 50, 18, 95, 20);
	desPanel->Add(facetUseDesFile);
	facetUseDesFile->SetSize(1);
	facetUseDesFile->SetValueAt(0, "No file imported");

	facetAccFactor = new GLTextField(0, "");
	paramPanel->SetCompBounds(facetAccFactor, 154, 63, 145, 18);
	paramPanel->Add(facetAccFactor);

	fileDoseText = new GLTextField(0, "");
	desPanel->SetCompBounds(fileDoseText, 205, 42, 58, 18);
	desPanel->Add(fileDoseText);

	fileFluxText = new GLTextField(0, "");
	desPanel->SetCompBounds(fileFluxText, 50, 42, 55, 18);
	desPanel->Add(fileFluxText);

	label11 = new GLLabel("ph/cm2");
	desPanel->SetCompBounds(label11, 265, 45, 36, 12);
	desPanel->Add(label11);

	label9 = new GLLabel("ph/s/cm2");
	desPanel->SetCompBounds(label9, 105, 45, 44, 12);
	desPanel->Add(label9);

	label12 = new GLLabel("Avg");
	desPanel->SetCompBounds(label12, 155, 45, 49, 12);
	desPanel->Add(label12);

	label10 = new GLLabel("Avg");
	desPanel->SetCompBounds(label10, 5, 45, 44, 12);
	desPanel->Add(label10);

	enableSojournTime = new GLToggle(0, "Wall Sojourn time");
	paramPanel->SetCompBounds(enableSojournTime, 9, 153, 95, 16);
	paramPanel->Add(enableSojournTime);

	sojournLabel3 = new GLLabel("J/mole");
	paramPanel->SetCompBounds(sojournLabel3, 249, 174, 37, 13);
	paramPanel->Add(sojournLabel3);

	sojournE = new GLTextField(0, "");
	paramPanel->SetCompBounds(sojournE, 199, 171, 50, 18);
	paramPanel->Add(sojournE);

	sojournLabel2 = new GLLabel("Hz   Binding E:");
	paramPanel->SetCompBounds(sojournLabel2, 124, 174, 76, 13);
	paramPanel->Add(sojournLabel2);

	sojournLabel1 = new GLLabel("Attempt freq:");
	paramPanel->SetCompBounds(sojournLabel1, 9, 174, 67, 13);
	paramPanel->Add(sojournLabel1);

	sojournFreq = new GLTextField(0, "");
	paramPanel->SetCompBounds(sojournFreq, 74, 171, 50, 18);
	paramPanel->Add(sojournFreq);

	SojournInfoButton = new GLButton(0, "Info");
	paramPanel->SetCompBounds(SojournInfoButton, 228, 147, 69, 20);
	paramPanel->Add(SojournInfoButton);

	label14 = new GLLabel("Theta (grazing angle):");
	angleMapPanel->SetCompBounds(label14, 9, 36, 93, 12);
	angleMapPanel->Add(label14);

	angleMapPhiResText = new GLTextField(0, "");
	angleMapPanel->SetCompBounds(angleMapPhiResText, 115, 77, 46, 18);
	angleMapPanel->Add(angleMapPhiResText);

	angleMapThetaLowresText = new GLTextField(0, "");
	angleMapPanel->SetCompBounds(angleMapThetaLowresText, 115, 33, 46, 18);
	angleMapPanel->Add(angleMapThetaLowresText);

	angleMapImportButton = new GLButton(0, "Import CSV");
	angleMapPanel->SetCompBounds(angleMapImportButton, 140, 122, 64, 20);
	angleMapPanel->Add(angleMapImportButton);

	angleMapExportButton = new GLButton(0, "Export to CSV");
	angleMapPanel->SetCompBounds(angleMapExportButton, 63, 122, 73, 20);
	angleMapPanel->Add(angleMapExportButton);

	angleMapRecordCheckbox = new GLToggle(0, "Record");
	angleMapPanel->SetCompBounds(angleMapRecordCheckbox, 10, 16, 54, 16);
	angleMapPanel->Add(angleMapRecordCheckbox);

	angleMapReleaseButton = new GLButton(0, "Release recorded");
	angleMapPanel->SetCompBounds(angleMapReleaseButton, 205, 122, 93, 20);
	angleMapPanel->Add(angleMapReleaseButton);

	remeshButton = new GLButton(0, "Force remesh");
	aPanel->SetCompBounds(remeshButton, 218, 13, 77, 20);
	aPanel->Add(remeshButton);

	label16 = new GLLabel("part cosine^");
	paramPanel->SetCompBounds(label16, 108, 44, 55, 12);
	paramPanel->Add(label16);

	label15 = new GLLabel("part specular,");
	paramPanel->SetCompBounds(label15, 216, 22, 61, 12);
	paramPanel->Add(label15);

	label13 = new GLLabel("part diffuse,");
	paramPanel->SetCompBounds(label13, 108, 22, 54, 12);
	paramPanel->Add(label13);

	cosineNReflBox = new GLTextField(0, "");
	paramPanel->SetCompBounds(cosineNReflBox, 65, 41, 40, 18);
	paramPanel->Add(cosineNReflBox);

	specularReflBox = new GLTextField(0, "");
	paramPanel->SetCompBounds(specularReflBox, 171, 19, 40, 18);
	paramPanel->Add(specularReflBox);

	diffuseReflBox = new GLTextField(0, "");
	paramPanel->SetCompBounds(diffuseReflBox, 65, 19, 40, 18);
	paramPanel->Add(diffuseReflBox);

	angleMapCopyButton = new GLButton(0, "Copy");
	angleMapPanel->SetCompBounds(angleMapCopyButton, 10, 122, 50, 20);
	angleMapPanel->Add(angleMapCopyButton);

	limitLabel = new GLLabel("values from limit to PI/2");
	angleMapPanel->SetCompBounds(limitLabel, 166, 58, 102, 12);
	angleMapPanel->Add(limitLabel);

	label17 = new GLLabel("values from 0 to");
	angleMapPanel->SetCompBounds(label17, 166, 36, 71, 12);
	angleMapPanel->Add(label17);

	angleMapThetaLimitText = new GLTextField(0, "");
	angleMapPanel->SetCompBounds(angleMapThetaLimitText, 249, 33, 46, 18);
	angleMapPanel->Add(angleMapThetaLimitText);

	label20 = new GLLabel("values from -PI to +PI");
	angleMapPanel->SetCompBounds(label20, 166, 80, 94, 12);
	angleMapPanel->Add(label20);

	label19 = new GLLabel("Phi (azimuth with U):");
	angleMapPanel->SetCompBounds(label19, 10, 80, 91, 12);
	angleMapPanel->Add(label19);

	angleMapThetaHighresText = new GLTextField(0, "");
	angleMapPanel->SetCompBounds(angleMapThetaHighresText, 115, 55, 46, 18);
	angleMapPanel->Add(angleMapThetaHighresText);

	angleMapStatusLabel = new GLLabel("Status: no recorded map");
	angleMapPanel->SetCompBounds(angleMapStatusLabel, 10, 106, 105, 12);
	angleMapPanel->Add(angleMapStatusLabel);

	reflectionExponentBox = new GLTextField(0, "");
	paramPanel->SetCompBounds(reflectionExponentBox, 171, 41, 40, 18);
	paramPanel->Add(reflectionExponentBox);

	SetTitle("Advanced facet parameters");
	// Center dialog
	int wS, hS;
	GLToolkit::GetScreenSize(&wS, &hS);
	int xD = (wS - wD) / 2;
	int yD = (hS - hD) / 2;
	SetBounds(xD, yD, wD, hD);

	cellText->SetEditable(false);
	ramText->SetEditable(false);
	fileDoseText->SetEditable(false);
	fileYieldText->SetEditable(false);
	fileFluxText->SetEditable(false);
	desPanel->SetClosable(true);
	angleMapPanel->SetClosable(true);

	desPanelFullHeight = desPanel->GetHeight();
	angleMapPanelFullHeight = angleMapPanel->GetHeight();

	desPanel->Close();
	angleMapPanel->Close();

	Refresh(std::vector<size_t>());
	Reposition(wD, hD);
	PlaceComponents();

	RestoreDeviceObjects();
}

/** 
* \brief Updates the texture cell / memory size on the panel
* \return void
*/
void FacetAdvParams::UpdateSize() {

	char tmp[64];

	if (enableBtn->GetState()) {

		size_t ram = 0;
		size_t cell = 0;
		size_t nbFacet = geom->GetNbFacet();

		if (recordACBtn->GetState()) {

			for (size_t i = 0; i < nbFacet; i++) {
				Facet *f = geom->GetFacet(i);
				if (f->sh.opacity == 1.0) {
					cell += (size_t)f->GetNbCell();
					ram += (size_t)f->GetTexRamSize(1 + worker->moments.size());
				}
			}
			ram += (((cell - 1)*cell) / 2 + 8 * cell)*((size_t)sizeof(ACFLOAT));

		}
		else {

			for (size_t i = 0; i < nbFacet; i++) {
				Facet *f = geom->GetFacet(i);
				cell += (size_t)f->GetNbCell();
				ram += (size_t)f->GetTexRamSize(1 + worker->moments.size());
			}

		}
		ramText->SetText(FormatMemoryLL(ram));
		sprintf(tmp, "%zd", cell);
		cellText->SetText(tmp);

	}
	else {

		ramText->SetText("0 bytes");
		cellText->SetText("0");

	}

}

/**
* \brief Updates the texture cell / memory size on the panel for changes made to resolution text
* \return void
*/
void FacetAdvParams::UpdateSizeForRatio() {
	if (!geom->IsLoaded()) return;
	double ratio;
	char tmp[64];
	bool boundMap = true;// boundaryBtn->GetState();
	bool recordDir = recordDirBtn->GetState();

	if (!enableBtn->GetState()) {
		ramText->SetText(FormatMemory(0));
		cellText->SetText("0");
		return;
	}

	if (!resolutionText->GetNumber(&ratio)) {
		ramText->SetText("");
		cellText->SetText("");
		return;
	}

	size_t ram = 0;
	size_t cell = 0;
	size_t nbFacet = geom->GetNbFacet();
	if (recordACBtn->GetState()) {

		for (size_t i = 0; i < nbFacet; i++) {
			Facet *f = geom->GetFacet(i);
			//if(f->wp.opacity==1.0) {
			if (f->selected) {
				cell += (size_t)f->GetNbCellForRatio(ratio);
				ram += (size_t)f->GetTexRamSizeForRatio(ratio, boundMap, false, 1 + worker->moments.size());
			}
			else {
				cell += (size_t)f->GetNbCell();
				ram += (size_t)f->GetTexRamSize(1 + worker->moments.size());
			}
			//}
		}
		ram += (((cell - 1)*cell) / 2 + 8 * cell)*((size_t)sizeof(ACFLOAT));

	}
	else {

		for (size_t i = 0; i < nbFacet; i++) {
			Facet *f = geom->GetFacet(i);
			if (f->selected) {
				cell += (size_t)f->GetNbCellForRatio(ratio);
				ram += (size_t)f->GetTexRamSizeForRatio(ratio, boundMap, recordDir, 1 + worker->moments.size());
			}
			else {
				cell += (size_t)f->GetNbCell();
				ram += (size_t)f->GetTexRamSize(1 + worker->moments.size());
			}
		}

	}

	ramText->SetText(FormatMemoryLL(ram));
	sprintf(tmp, "%zd", cell);
	cellText->SetText(tmp);

}

/**
* \brief Refreshes the advanced facet parameters panel completly depending on selection of facets
* \param selection Vector of facet IDs
* \return void
*/
void FacetAdvParams::Refresh(std::vector<size_t> selection) {

	sumArea = sumOutgassing = 0.0;
	sumAngleMapSize = 0;

	bool somethingSelected = selection.size() > 0;
	enableBtn->SetEnabled(somethingSelected);
	recordDesBtn->SetEnabled(somethingSelected);
	recordAbsBtn->SetEnabled(somethingSelected);
	recordReflBtn->SetEnabled(somethingSelected);
	recordTransBtn->SetEnabled(somethingSelected);
	recordACBtn->SetEnabled(somethingSelected);
	recordDirBtn->SetEnabled(somethingSelected);
	angleMapRecordCheckbox->SetEnabled(somethingSelected);
	angleMapPhiResText->SetEditable(somethingSelected);
	angleMapThetaLowresText->SetEditable(somethingSelected);
	angleMapThetaHighresText->SetEditable(somethingSelected);
	angleMapThetaLimitText->SetEditable(somethingSelected);
	showTexture->SetEnabled(somethingSelected);
	showVolume->SetEnabled(somethingSelected);
	resolutionText->SetEditable(somethingSelected);
	lengthText->SetEditable(somethingSelected);
	diffuseReflBox->SetEditable(somethingSelected);
	specularReflBox->SetEditable(somethingSelected);
	cosineNReflBox->SetEditable(somethingSelected);
	reflectionExponentBox->SetEditable(somethingSelected);
	facetAccFactor->SetEditable(somethingSelected);
	facetTeleport->SetEditable(somethingSelected);
	facetStructure->SetEditable(somethingSelected);
	facetSuperDest->SetEditable(somethingSelected);
	facetUseDesFile->SetEditable(somethingSelected);
	facetMovingToggle->SetEnabled(somethingSelected);
	enableSojournTime->SetEnabled(somethingSelected);
	sojournFreq->SetEditable(somethingSelected);
	sojournE->SetEditable(somethingSelected);

	if (!geom->IsLoaded()) return;

	if (!somethingSelected) { //Empty selection, clear
		enableBtn->SetState(0);
		resolutionText->SetText("");
		lengthText->SetText("");
		recordDesBtn->SetState(0);
		recordAbsBtn->SetState(0);
		recordReflBtn->SetState(0);
		recordTransBtn->SetState(0);
		recordACBtn->SetState(0);
		recordDirBtn->SetState(0);
		angleMapRecordCheckbox->SetState(0);
		angleMapPhiResText->SetText("");
		angleMapThetaLowresText->SetText("");
		angleMapThetaHighresText->SetText("");
		angleMapThetaLimitText->SetText("");
		angleMapStatusLabel->SetText("");
		showTexture->SetState(0);
		showVolume->SetState(0);
		facetUseDesFile->SetSelectedValue("");
		diffuseReflBox->SetText("");
		specularReflBox->SetText("");
		cosineNReflBox->SetText("");
		reflectionExponentBox->SetText("");
		facetAccFactor->Clear();
		facetSuperDest->Clear();
		facetMovingToggle->SetState(0);
		facetStructure->Clear();
		facetTeleport->Clear();
		enableSojournTime->SetState(0);
		enableSojournTime->SetText("Wall sojourn time");
		sojournFreq->SetText("");
		sojournE->SetText("");
		std::stringstream label;
		limitLabel->SetText("values from limit to PI/2");
		return;
	}

	Facet* f0 = geom->GetFacet(selection[0]);

	bool isEnabledE = true;
	bool isBoundE = true;
	bool CountDesE = true;
	bool CountAbsE = true;
	bool CountReflE = true;
	bool CountTransE = true;
	bool CountACE = true;
	bool CountDirE = true;
	bool RecordAngleMapE = true;
	bool AngleMapPhiWidthE = true;
	bool AngleMapThetaLowResE = true;
	bool AngleMapThetaHiResE = true;
	bool AngleMapThetaLimitE = true;
	bool hasAngleMapE = true;
	bool TexVisibleE = true;
	bool VolVisibleE = true;
	bool ratioE = true;
	bool teleportE = true;
	bool accFactorE = true;
	bool superDestE = true;
	bool superIdxE = true;
	bool reflectDiffuseE = true;
	bool reflectSpecularE = true;
	bool reflectCosineNE = true;
	bool reflectionExponentE = true;
	bool hasOutgMapE = true;
	bool useOutgMapE = true;
	bool yieldEqual = true;
	bool fluxAEqual = true;
	bool doseAEqual = true;
	bool isMovingE = true;
	bool dynOutgEqual = true;
	bool dynOutgAEqual = true;
	bool hasSojournE = true;
	bool sojournFreqE = true;
	bool sojournEE = true;

	double f0Area = f0->GetArea();
	sumArea = f0Area;
	sumOutgassing = f0->sh.totalOutgassing;
	sumAngleMapSize = f0->angleMapCache.size();
	
	for (size_t i = 1; i < selection.size(); i++) {
		Facet *f = geom->GetFacet(selection[i]);
		double fArea = f->GetArea();
		sumArea += fArea;
		sumOutgassing += f->sh.totalOutgassing;
		sumAngleMapSize += f->angleMapCache.size();
		isEnabledE = isEnabledE && (f0->sh.isTextured == f->sh.isTextured);
		isBoundE = isBoundE && (f0->hasMesh == f->hasMesh);
		CountDesE = CountDesE && f0->sh.countDes == f->sh.countDes;
		CountAbsE = CountAbsE && f0->sh.countAbs == f->sh.countAbs;
		CountReflE = CountReflE && f0->sh.countRefl == f->sh.countRefl;
		CountTransE = CountTransE && f0->sh.countTrans == f->sh.countTrans;
		CountACE = CountACE && f0->sh.countACD == f->sh.countACD;
		CountDirE = CountDirE && f0->sh.countDirection == f->sh.countDirection;
		RecordAngleMapE = RecordAngleMapE && f0->sh.anglemapParams.record == f->sh.anglemapParams.record;
		AngleMapPhiWidthE = AngleMapPhiWidthE && (f0->sh.anglemapParams.phiWidth == f->sh.anglemapParams.phiWidth);
		AngleMapThetaLowResE = AngleMapThetaLowResE && (f0->sh.anglemapParams.thetaLowerRes == f->sh.anglemapParams.thetaLowerRes);
		AngleMapThetaHiResE = AngleMapThetaHiResE && (f0->sh.anglemapParams.thetaHigherRes == f->sh.anglemapParams.thetaHigherRes);
		AngleMapThetaLimitE = AngleMapThetaLimitE && IsEqual(f0->sh.anglemapParams.thetaLimit , f->sh.anglemapParams.thetaLimit);
		hasAngleMapE = hasAngleMapE && (!f0->angleMapCache.empty() == !f->angleMapCache.empty());
		TexVisibleE = TexVisibleE && f0->textureVisible == f->textureVisible;
		VolVisibleE = VolVisibleE && f0->volumeVisible == f->volumeVisible;
		ratioE = ratioE && std::abs(f0->tRatio - f->tRatio) < 1E-8;
		teleportE = teleportE && (f0->sh.teleportDest == f->sh.teleportDest);
		accFactorE = accFactorE && IsEqual(f0->sh.accomodationFactor, f->sh.accomodationFactor);
		superDestE = superDestE && (f0->sh.superDest == f->sh.superDest);
		superIdxE = superIdxE && (f0->sh.superIdx == f->sh.superIdx);
		reflectDiffuseE = reflectDiffuseE && IsEqual(f0->sh.reflection.diffusePart,f->sh.reflection.diffusePart);
		reflectSpecularE = reflectSpecularE && IsEqual(f0->sh.reflection.specularPart, f->sh.reflection.specularPart);
		reflectCosineNE = reflectCosineNE && IsEqual(1.0 - f0->sh.reflection.diffusePart - f0->sh.reflection.specularPart, 1.0 - f->sh.reflection.diffusePart - f->sh.reflection.specularPart);
		reflectionExponentE = reflectionExponentE && IsEqual(f0->sh.reflection.cosineExponent, f->sh.reflection.cosineExponent);
		hasOutgMapE = hasOutgMapE && (f0->hasOutgassingFile == f->hasOutgassingFile);
		useOutgMapE = useOutgMapE && (f0->sh.useOutgassingFile == f->sh.useOutgassingFile);
		dynOutgEqual = dynOutgEqual && IsEqual(f0->sh.totalOutgassing, f->sh.totalOutgassing);
		dynOutgAEqual = dynOutgAEqual && IsEqual(f0->sh.totalOutgassing / f0Area, f->sh.totalOutgassing / fArea);
		yieldEqual = yieldEqual && IsEqual(f0->sh.totalOutgassing / f0->sh.temperature / f0->totalFlux, f->sh.totalOutgassing / f->sh.temperature / f->totalFlux);
		fluxAEqual = fluxAEqual && IsEqual(f0->totalFlux / f0Area, f->totalFlux / fArea);
		doseAEqual = doseAEqual && IsEqual(f0->totalDose / f0Area, f->totalDose / fArea);
		isMovingE = isMovingE && (f0->sh.isMoving == f->sh.isMoving);
		hasSojournE = hasSojournE && (f0->sh.enableSojournTime == f->sh.enableSojournTime);
		sojournFreqE = sojournFreqE && IsEqual(f0->sh.sojournFreq, f->sh.sojournFreq);
		sojournEE = sojournEE && IsEqual(f0->sh.sojournE, f->sh.sojournE);
	}

	/*
	if( nbSel==1 ) {
	Facet *f = f0;
	sprintf(tmp,"Advanced parameters (Facet #%d)",selection[0]+1);
	SetTitle(tmp);
	sprintf(tmp,"%g",maxU);
	uLength->SetText(tmp);
	sprintf(tmp,"%g",maxV);
	vLength->SetText(tmp);
	} else {
	sprintf(tmp,"Advanced parameters (%d selected)",nbSel);
	SetTitle(tmp);
	sprintf(tmp,"%g (Max)",maxU);
	uLength->SetText(tmp);
	sprintf(tmp,"%g (Max)",maxV);
	vLength->SetText(tmp);
	}*/

	enableBtn->AllowMixedState(!isEnabledE); enableBtn->SetState(isEnabledE ? f0->sh.isTextured : 2);
	recordDesBtn->AllowMixedState(!CountDesE); recordDesBtn->SetState(CountDesE ? f0->sh.countDes : 2);
	recordAbsBtn->AllowMixedState(!CountAbsE); recordAbsBtn->SetState(CountAbsE ? f0->sh.countAbs : 2);
	recordReflBtn->AllowMixedState(!CountReflE); recordReflBtn->SetState(CountReflE ? f0->sh.countRefl : 2);
	recordTransBtn->AllowMixedState(!CountTransE); recordTransBtn->SetState(CountTransE ? f0->sh.countTrans : 2);
	recordACBtn->AllowMixedState(!CountACE); recordACBtn->SetState(CountACE ? f0->sh.countACD : 2);
	recordDirBtn->AllowMixedState(!CountDirE); recordDirBtn->SetState(CountDirE ? f0->sh.countDirection : 2);
	angleMapRecordCheckbox->AllowMixedState(!RecordAngleMapE); angleMapRecordCheckbox->SetState(RecordAngleMapE ? f0->sh.anglemapParams.record : 2);
	showTexture->AllowMixedState(!TexVisibleE); showTexture->SetState(TexVisibleE ? f0->textureVisible : 2);
	showVolume->AllowMixedState(!VolVisibleE); showVolume->SetState(VolVisibleE ? f0->volumeVisible : 2);
	facetMovingToggle->AllowMixedState(!isMovingE); facetMovingToggle->SetState(isMovingE ? f0->sh.isMoving : 2);
	enableSojournTime->AllowMixedState(!hasSojournE); enableSojournTime->SetState(hasSojournE ? f0->sh.enableSojournTime : 2);

	if (isEnabledE) {
		if (f0->sh.isTextured) { //All facets have textures
			if (ratioE) { //All facets have textures with same resolution
				resolutionText->SetText(f0->tRatio);
				lengthText->SetText(1.0 / f0->tRatio);
			}
			else { //Mixed resolution
				resolutionText->SetText(isEnabledE ? "..." : "");
				lengthText->SetText(isEnabledE ? "..." : "");
			}
		}
		else { //None of the facets have textures
			resolutionText->SetText("");
			lengthText->SetText("");
		}
	}
	else { //Mixed state
		resolutionText->SetText("");
		lengthText->SetText("");
	}

	if (teleportE) facetTeleport->SetText(f0->sh.teleportDest); else facetTeleport->SetText("...");
	if (accFactorE) facetAccFactor->SetText(f0->sh.accomodationFactor); else facetAccFactor->SetText("...");
	if (reflectDiffuseE) diffuseReflBox->SetText(f0->sh.reflection.diffusePart); else diffuseReflBox->SetText("...");
	if (reflectSpecularE) specularReflBox->SetText(f0->sh.reflection.specularPart); else specularReflBox->SetText("...");
	if (reflectCosineNE) cosineNReflBox->SetText(1.0 - f0->sh.reflection.diffusePart - f0->sh.reflection.specularPart); else cosineNReflBox->SetText("...");
	if (reflectionExponentE) reflectionExponentBox->SetText(f0->sh.reflection.cosineExponent); else reflectionExponentBox->SetText("...");
	if (hasOutgMapE) { //all selected equally HAVE or equally DON'T HAVE outgassing maps
		//mApp->facetFlow->SetEditable(!f0->hasOutgassingFile);
		//mApp->facetFlowArea->SetEditable(!f0->hasOutgassingFile);
		if (!f0->hasOutgassingFile) { //All selected DON'T HAVE outgassing maps
			facetUseDesFile->SetSize(1);
			facetUseDesFile->SetSelectedIndex(0); //no map
			facetUseDesFile->SetSelectedValue("No map loaded");
			facetUseDesFile->SetEditable(false);
			fileFluxText->SetText("");
			fileDoseText->SetText("");
			fileYieldText->SetText("");
		}
		else { //All selected HAVE outgassing maps

			facetUseDesFile->SetSize(2);
			facetUseDesFile->SetValueAt(0, "Use user values");
			facetUseDesFile->SetValueAt(1, "Use des. file");
			facetUseDesFile->SetEditable(true);
			if (useOutgMapE) {
				facetUseDesFile->SetSelectedIndex(f0->sh.useOutgassingFile);
			}
			else {
				facetUseDesFile->SetSelectedValue("...");
			}
			char tmp[64];
			if (fluxAEqual) sprintf(tmp, "%.2E", f0->totalFlux / f0->sh.area); else sprintf(tmp, "...");
			fileFluxText->SetText(tmp);
			if (doseAEqual) sprintf(tmp, "%.2E", f0->totalDose / f0->sh.area); else sprintf(tmp, "...");
			fileDoseText->SetText(tmp);
			if (yieldEqual) sprintf(tmp, "%.2E", f0->sh.totalOutgassing / (1.38E-23*f0->sh.temperature) / f0->totalFlux); else sprintf(tmp, "...");
			fileYieldText->SetText(tmp);
			if (useOutgMapE) {
				mApp->facetFlow->SetEditable(!f0->sh.useOutgassingFile);
				mApp->facetFlowArea->SetEditable(!f0->sh.useOutgassingFile);
				if (f0->sh.useOutgassingFile) {
					sprintf(tmp, "%.1E", sumOutgassing * 10.00); //10.00: Pa*m3/s -> mbar*l/s
					mApp->facetFlow->SetText(tmp);
					sprintf(tmp, "%.1E", sumOutgassing * 10.00 / sumArea);
					mApp->facetFlowArea->SetText(tmp);
				}
				else {
					//Let the main program handle this
				}
			}
			else { //some use it, some not
				facetUseDesFile->SetSelectedValue("...");
				mApp->facetFlow->SetEditable(false);
				mApp->facetFlowArea->SetEditable(false);
			}
		}
	}
	else { //Mixed: some have it, some don't
		facetUseDesFile->SetSelectedIndex(0);
		facetUseDesFile->SetSize(1);
		facetUseDesFile->SetSelectedValue("...");
		facetUseDesFile->SetEditable(false);
		fileFluxText->SetText("");
		fileDoseText->SetText("");
		fileYieldText->SetText("");
	}

	if (superDestE) {
		if (f0->sh.superDest == 0) {
			facetSuperDest->SetText("no");
		}
		else {
			facetSuperDest->SetText(f0->sh.superDest);
		}
	}
	else {
		facetSuperDest->SetText("...");
	}
	if (superIdxE) {
		if (f0->sh.superIdx >= 0) {
			facetStructure->SetText(f0->sh.superIdx + 1);
		}
		else {
			facetStructure->SetText("All");
		}
	}
	else {
		facetStructure->SetText("...");
	}

	if (isMovingE) {
		facetMovingToggle->SetState(f0->sh.isMoving);
		facetMovingToggle->AllowMixedState(false);
	}
	else {
		facetMovingToggle->SetState(2);
		facetMovingToggle->AllowMixedState(true);
	}

	//Angle map
	std::stringstream statusLabelText;
	std::string mapSizeText = mApp->FormatSize(sumAngleMapSize*sizeof(size_t));
	if (hasAngleMapE) {
		if (!f0->angleMapCache.empty())
			statusLabelText << "All selected facets have recorded angle maps (" << mapSizeText << ")";
		else
			statusLabelText << "No recorded angle maps on selected facets.";
	}
	else {
		statusLabelText << "Some selected facets have recorded angle maps (" << mapSizeText << ")";
	}
	angleMapStatusLabel->SetText(statusLabelText.str());

	if (AngleMapPhiWidthE) {
		angleMapPhiResText->SetText((int)f0->sh.anglemapParams.phiWidth);
	}
	else {
		angleMapPhiResText->SetText("...");
	}

	if (AngleMapThetaLowResE) {
		angleMapThetaLowresText->SetText((int)f0->sh.anglemapParams.thetaLowerRes);
	}
	else {
		angleMapThetaLowresText->SetText("...");
	}

	if (AngleMapThetaHiResE) {
		angleMapThetaHighresText->SetText((int)f0->sh.anglemapParams.thetaHigherRes);
	}
	else {
		angleMapThetaHighresText->SetText("...");
	}

	if (AngleMapThetaLimitE) {
		angleMapThetaLimitText->SetText(f0->sh.anglemapParams.thetaLimit);
		std::stringstream label;
		label << "values from " << f0->sh.anglemapParams.thetaLimit << " to PI/2";
		limitLabel->SetText(label.str());
	}
	else {
		angleMapThetaLimitText->SetText("...");
		limitLabel->SetText("values from limit to PI/2");
	}

	if (enableSojournTime->GetState() == 0) {
		enableSojournTime->SetText("Wall sojourn time");
		sojournFreq->SetEditable(false);
		sojournE->SetEditable(false);
	}
	else {
		sojournFreq->SetEditable(true);
		sojournE->SetEditable(true);
	}

	if (sojournFreqE) {
		sojournFreq->SetText(f0->sh.sojournFreq);
	}
	else {
		sojournFreq->SetText("...");
	}
	if (sojournEE) {
		sojournE->SetText(f0->sh.sojournE);
	}
	else {
		sojournE->SetText("...");
	}
	CalcSojournTime();
	UpdateSize();
}

/**
* \brief Sets position of the whole panel depending on it's size
* \param wD Panel width
* \param hD Panel height
* \return void
*/
void FacetAdvParams::Reposition(int wD, int hD) {
	if (wD == 0) wD = this->GetWidth();
	if (hD == 0) hD = this->GetHeight();
	// Position dialog next to Facet parameters
	int facetX, facetY, facetW, facetH;
	mApp->facetPanel->GetBounds(&facetX, &facetY, &facetW, &facetH);
	SetBounds(facetX - wD - 10, Min(facetY + 20, 115), wD, hD); //If below 115, the bottom can be out of screen
}

/**
* \brief Apply new textures on the facets
* \param force If remeshing needs to be forced
* \return bool value 0 if it didnt work 1 if it did
*/
bool FacetAdvParams::ApplyTexture(bool force) {
	bool boundMap = true; // boundaryBtn->GetState();
	double ratio = 0.0;
	std::vector<size_t> selectedFacets = geom->GetSelectedFacets();
	int nbPerformed = 0;
	bool doRatio = false;
	if (enableBtn->GetState() == 1) { //check if valid texture settings are to be applied

		// Check counting mode
		if (!recordDesBtn->GetState() && !recordAbsBtn->GetState() &&
			!recordReflBtn->GetState() && !recordTransBtn->GetState() &&
			!recordACBtn->GetState() && !recordDirBtn->GetState()) {
			GLMessageBox::Display("Please select counting mode", "Error", GLDLG_OK, GLDLG_ICONINFO);
			return false;
		}

		// Resolution
		if (resolutionText->GetNumber(&ratio) && ratio >= 0.0) {
			//Got a valid number
			doRatio = true;
		}
		else if (resolutionText->GetText()!="...") { //Not in mixed "..." state
			GLMessageBox::Display("Invalid texture resolution\nMust be a non-negative number", "Error", GLDLG_OK, GLDLG_ICONERROR);
			return false;
		}
		else {
			//Mixed state: leave doRatio as false
		}
	}

	if (!mApp->AskToReset(worker)) return false;
	progressDlg = new GLProgress("Applying mesh settings", "Please wait");
	progressDlg->SetVisible(true);
	progressDlg->SetProgress(0.0);
	int count = 0;
	for (auto& sel : selectedFacets) {
		Facet *f = geom->GetFacet(sel);
		bool hadAnyTexture = f->sh.countDes || f->sh.countAbs || f->sh.countRefl || f->sh.countTrans || f->sh.countACD || f->sh.countDirection;
		bool hadDirCount = f->sh.countDirection;

		if (enableBtn->GetState() == 0 || (doRatio && ratio == 0.0)) {
			//Let the user disable textures with the main switch or by typing 0 as resolution
			f->sh.countDes = f->sh.countAbs = f->sh.countRefl = f->sh.countTrans = f->sh.countACD = f->sh.countDirection = false;
		}
		else {
			if (recordDesBtn->GetState() < 2) f->sh.countDes = recordDesBtn->GetState();
			if (recordAbsBtn->GetState() < 2) f->sh.countAbs = recordAbsBtn->GetState();
			if (recordReflBtn->GetState() < 2) f->sh.countRefl = recordReflBtn->GetState();
			if (recordTransBtn->GetState() < 2) f->sh.countTrans = recordTransBtn->GetState();
			if (recordACBtn->GetState() < 2) f->sh.countACD = recordACBtn->GetState();
			if (recordDirBtn->GetState() < 2) f->sh.countDirection = recordDirBtn->GetState();
		}

		bool hasAnyTexture = f->sh.countDes || f->sh.countAbs || f->sh.countRefl || f->sh.countTrans || f->sh.countACD || f->sh.countDirection;

		//set textures
		try {
			bool needsRemeshing = force || (hadAnyTexture != hasAnyTexture) || (hadDirCount != f->sh.countDirection) || (doRatio && (!IsZero(geom->GetFacet(sel)->tRatio - ratio)));
			if (needsRemeshing) geom->SetFacetTexture(sel, hasAnyTexture ? ratio : 0.0, hasAnyTexture ? boundMap : false);
		}
		catch (Error &e) {
			GLMessageBox::Display(e.GetMsg(), "Error", GLDLG_OK, GLDLG_ICONWARNING);
			progressDlg->SetVisible(false);
			SAFE_DELETE(progressDlg);
			return false;
		}
		catch (...) {
			GLMessageBox::Display("Unexpected error while setting textures", "Error", GLDLG_OK, GLDLG_ICONWARNING);
			progressDlg->SetVisible(false);
			SAFE_DELETE(progressDlg);
			return false;
		}
		nbPerformed++;
		progressDlg->SetProgress((double)nbPerformed / (double)selectedFacets.size());
	} //main cycle end

	if (progressDlg) progressDlg->SetVisible(false);
	SAFE_DELETE(progressDlg);
	return true;
}

/**
* \brief Apply various values from the panel
* \return bool value 0 if it didnt work 1 if it did
*/
bool FacetAdvParams::Apply() {
	std::vector<size_t> selectedFacets=geom->GetSelectedFacets();
	int nbPerformed = 0;
	/*
	bool boundMap = true; // boundaryBtn->GetState();
	double ratio = 0.0;
	
	
	bool doRatio = false;
	if (enableBtn->GetState() == 1) { //check if valid texture settings are to be applied

		// Check counting mode
		if (!recordDesBtn->GetState() && !recordAbsBtn->GetState() &&
			!recordReflBtn->GetState() && !recordTransBtn->GetState() &&
			!recordACBtn->GetState() && !recordDirBtn->GetState()) {
			GLMessageBox::Display("Please select counting mode", "Error", GLDLG_OK, GLDLG_ICONINFO);
			return false;
		}

		// Resolution
		if (resolutionText->GetNumber(&ratio) && ratio >= 0.0) {
			//Got a valid number
			doRatio = true;
		}
		else if (strcmp(resolutionText->GetText(), "...") != 0)  { //Not in mixed "..." state
			GLMessageBox::Display("Invalid texture resolution\nMust be a non-negative number", "Error", GLDLG_OK, GLDLG_ICONERROR);
			return false;
		}
		else {
			//Mixed state: leave doRatio as false
		}
	}*/

	// Superstructure

	bool structChanged = false; //if a facet gets into a new structure, we have to re-render the geometry

	int superStruct;
	bool doSuperStruct = false;
	auto ssText = facetStructure->GetText();
	if (Contains({ "All","all" }, ssText)) {
		doSuperStruct = true;
		superStruct = -1;
	}
	else if (ssText == "...") {
		//Nothing to do, doSuperStruct is already false
	} 
	else {
		try {
			superStruct = std::stoi(ssText);
			superStruct--; //Internally numbered from 0
			if (superStruct < 0 || superStruct >= geom->GetNbStructure()) {
				throw std::invalid_argument("Invalid superstructure number");
			}
			doSuperStruct = true;
		}
		catch (std::invalid_argument err) {
			GLMessageBox::Display("Invalid superstructure number", "Error", GLDLG_OK, GLDLG_ICONERROR);
			return false;
		}
	}

	// Super structure destination (link)
	int superDest;
	bool doLink = false;
	if (Contains({ "none","no","0" },facetSuperDest->GetText())) {
		doLink = true;
		superDest = 0;
	}
	else if (facetSuperDest->GetNumberInt(&superDest)) {
		if (superDest == (superStruct+1)) {
			GLMessageBox::Display("Link and superstructure can't be the same", "Error", GLDLG_OK, GLDLG_ICONERROR);
			return false;
		}
		else if (superDest < 0 || superDest > geom->GetNbStructure()) {
			GLMessageBox::Display("Link destination points to a structure that doesn't exist", "Error", GLDLG_OK, GLDLG_ICONERROR);
			return false;
		}
		else
			doLink = true;
	}
	else if (facetSuperDest->GetText() == "...") doLink = false;
	else {
		GLMessageBox::Display("Invalid superstructure destination", "Error", GLDLG_OK, GLDLG_ICONERROR);
		return false;
	}

	// teleport
	int teleport;
	bool doTeleport = false;

	if (facetTeleport->GetNumberInt(&teleport)) {
		if (teleport<-1 || teleport>geom->GetNbFacet()) {
			GLMessageBox::Display("Invalid teleport destination\n(If no teleport: set number to 0)", "Error", GLDLG_OK, GLDLG_ICONERROR);
			return false;
		}
		else if (teleport > 0 && geom->GetFacet(teleport - 1)->selected) {
			char tmp[256];
			sprintf(tmp, "The teleport destination of facet #%d can't be itself!", teleport);
			GLMessageBox::Display(tmp, "Error", GLDLG_OK, GLDLG_ICONERROR);
			return false;
		}
		doTeleport = true;
	}
	else {
		if (facetTeleport->GetText() == "...") doTeleport = false;
		else {
			GLMessageBox::Display("Invalid teleport destination\n(If no teleport: set number to 0)", "Error", GLDLG_OK, GLDLG_ICONERROR);
			return false;
		}
	}

	// temp.accomodation factor
	double accfactor;
	bool doAccfactor = false;
	if (facetAccFactor->GetNumber(&accfactor)) {
		if (accfactor<0.0 || accfactor>1.0) {
			GLMessageBox::Display("Facet accomodation factor must be between 0 and 1", "Error", GLDLG_OK, GLDLG_ICONERROR);
			return false;
		}
		doAccfactor = true;
	}
	else {
		if (facetAccFactor->GetText() == "...") doAccfactor = false;
		else {
			GLMessageBox::Display("Invalid accomodation factor number", "Error", GLDLG_OK, GLDLG_ICONERROR);
			return false;
		}
	}

	// Use desorption map
	int useMapA = 0;
	bool doUseMapA = false;
	if (facetUseDesFile->GetSelectedValue()=="...") doUseMapA = false;
	else {
		useMapA = (facetUseDesFile->GetSelectedIndex() == 1);
		bool missingMap = false;
		int missingMapId;
		if (useMapA) {
			for (int i = 0; i < geom->GetNbFacet(); i++) {
				if (geom->GetFacet(i)->selected && !geom->GetFacet(i)->hasOutgassingFile) {
					missingMap = true;
					missingMapId = i;
				}
			}
		}
		if (missingMap) {
			char tmp[256];
			sprintf(tmp, "%d is selected but doesn't have any outgassing map loaded.", missingMapId + 1);
			GLMessageBox::Display(tmp, "Can't use map on all facets", GLDLG_OK, GLDLG_ICONERROR);
			return false;
		}
		doUseMapA = true;
	}

	// sojourn time coefficient 1
	double sojF;
	bool doSojF = false;

	if (sojournFreq->GetNumber(&sojF)) {
		if (sojF <= 0.0) {
			GLMessageBox::Display("Wall sojourn time frequency has to be positive", "Error", GLDLG_OK, GLDLG_ICONERROR);
			return false;
		}
		doSojF = true;
	}
	else {
		if (enableSojournTime->GetState() == 0 || sojournFreq->GetText() == "...") doSojF = false;
		else {
			GLMessageBox::Display("Invalid wall sojourn time frequency", "Error", GLDLG_OK, GLDLG_ICONERROR);
			return false;
		}
	}

	// sojourn time coefficient 2
	double sojE;
	bool doSojE = false;

	if (sojournE->GetNumber(&sojE)) {
		if (sojE <= 0.0) {
			GLMessageBox::Display("Wall sojourn time second coefficient (Energy) has to be positive", "Error", GLDLG_OK, GLDLG_ICONERROR);
			return false;
		}
		doSojE = true;
	}
	else {
		if (enableSojournTime->GetState() == 0 || sojournE->GetText()=="...") doSojE = false;
		else {
			GLMessageBox::Display("Invalid wall sojourn time second coefficient (Energy)", "Error", GLDLG_OK, GLDLG_ICONERROR);
			return false;
		}
	}

	// angle map phi width
	int angleMapWidth;
	bool doAngleMapWidth = false;

	if (angleMapRecordCheckbox->GetState() == 0 || angleMapPhiResText->GetText()== "...") doAngleMapWidth = false;
	else if (angleMapPhiResText->GetNumberInt(&angleMapWidth)) {
		if (angleMapWidth <= 0) {
			GLMessageBox::Display("Angle map width has to be positive", "Error", GLDLG_OK, GLDLG_ICONERROR);
			return false;
		}
		doAngleMapWidth = true;
	}
	else {
		GLMessageBox::Display("Invalid angle map width", "Error", GLDLG_OK, GLDLG_ICONERROR);
		return false;
	}

	// angle map theta (low res) height
	int angleMapLowRes;
	bool doAngleMapLowRes = false;

	if (angleMapRecordCheckbox->GetState() == 0 || angleMapThetaLowresText->GetText()== "...") doAngleMapLowRes = false;
	else if (angleMapThetaLowresText->GetNumberInt(&angleMapLowRes)) {
		if (angleMapLowRes < 0) {
			GLMessageBox::Display("Angle map resolution (below theta limit) has to be positive", "Error", GLDLG_OK, GLDLG_ICONERROR);
			return false;
		}
		doAngleMapLowRes = true;
	}
	else {
		GLMessageBox::Display("Invalid angle map resolution (below theta limit)", "Error", GLDLG_OK, GLDLG_ICONERROR);
		return false;
	}

	// angle map theta (hi res) height
	int angleMapHiRes;
	bool doAngleMapHiRes = false;

	if (angleMapRecordCheckbox->GetState() == 0 || angleMapThetaHighresText->GetText()== "...") doAngleMapHiRes = false;
	else if (angleMapThetaHighresText->GetNumberInt(&angleMapHiRes)) {
		if (angleMapHiRes < 0) {
			GLMessageBox::Display("Angle map resolution (above theta limit) has to be positive", "Error", GLDLG_OK, GLDLG_ICONERROR);
			return false;
		}
		doAngleMapHiRes = true;
	}
	else {
		GLMessageBox::Display("Invalid angle map resolution (above theta limit)", "Error", GLDLG_OK, GLDLG_ICONERROR);
		return false;
	}

	if (doAngleMapHiRes && doAngleMapLowRes && ((angleMapLowRes + angleMapHiRes) == 0)) {
		GLMessageBox::Display("Angle map total theta height (sum of below and above limit) must be positive", "Error", GLDLG_OK, GLDLG_ICONERROR);
		return false;
	}

	// angle map theta limit
	double angleMapThetaLimit;
	bool doAngleMapThetaLimit = false;

	if (angleMapRecordCheckbox->GetState() == 0 || angleMapThetaLimitText->GetText() == "...") doAngleMapThetaLimit = false;
	else if (angleMapThetaLimitText->GetNumber(&angleMapThetaLimit)) {
		if (!(angleMapThetaLimit >= 0 && angleMapThetaLimit<=PI/2.0)) {
			GLMessageBox::Display("Angle map theta limit must be between 0 and PI/2", "Error", GLDLG_OK, GLDLG_ICONERROR);
			return false;
		}
		doAngleMapThetaLimit = true;
	}
	else {
		GLMessageBox::Display("Invalid angle map theta limit", "Error", GLDLG_OK, GLDLG_ICONERROR);
		return false;
	}
	

	// Reflection type
	double diffuseRefl, specularRefl, reflectionExponent;
	bool doDiffuseRefl = false;
	bool doSpecularRefl = false;
	bool doReflExponent = false;
	if (diffuseReflBox->GetNumber(&diffuseRefl)) {
		if (diffuseRefl < 0.0 || diffuseRefl > 1.0) {
			GLMessageBox::Display("Diffuse reflection ratio must be between 0 and 1", "Error", GLDLG_OK, GLDLG_ICONERROR);
			return false;
		}
		doDiffuseRefl = true;
	}
	else if (diffuseReflBox->GetText()!= "...") {
		GLMessageBox::Display("Invalid diffuse reflection ratio", "Error", GLDLG_OK, GLDLG_ICONERROR);
		return false;
	}

	if (specularReflBox->GetNumber(&specularRefl)) {
		if (specularRefl < 0.0 || specularRefl > 1.0) {
			GLMessageBox::Display("Specular reflection ratio must be between 0 and 1", "Error", GLDLG_OK, GLDLG_ICONERROR);
			return false;
		}
		doSpecularRefl = true;
	}
	else if (specularReflBox->GetText()!= "...") {
		GLMessageBox::Display("Invalid specular reflection ratio", "Error", GLDLG_OK, GLDLG_ICONERROR);
		return false;
	}

	if ((diffuseRefl + specularRefl) > 1.0) {
		GLMessageBox::Display("The sum of diffuse and specular reflection ratios cannot be larger than 1", "Error", GLDLG_OK, GLDLG_ICONERROR);
		return false;
	}

	if (reflectionExponentBox->GetNumber(&reflectionExponent)) {
		if (reflectionExponent <= -1.0) {
			GLMessageBox::Display("Cosine^N exponent must be greater than -1", "Error", GLDLG_OK, GLDLG_ICONERROR);
			return false;
		}
		doReflExponent = true;
	}
	else if (reflectionExponentBox->GetText()!= "...") {
		GLMessageBox::Display("Invalid cosine^N reflection exponent", "Error", GLDLG_OK, GLDLG_ICONERROR);
		return false;
	}
	

	//Check complete, let's apply
	//First applying angle map recording before a reset is done
	int angleMapState = angleMapRecordCheckbox->GetState();
	if (angleMapState < 2) {
		for (auto& sel:selectedFacets) {
			geom->GetFacet(sel)->sh.anglemapParams.record=angleMapState;
		}
	}
	
	if (!mApp->AskToReset(worker)) return false;
	progressDlg = new GLProgress("Applying facet parameters", "Please wait");
	progressDlg->SetVisible(true);
	progressDlg->SetProgress(0.0);
	int count = 0;
	for (auto& sel:selectedFacets) {
		Facet *f = geom->GetFacet(sel);
		/*
		bool hadAnyTexture = f->wp.countDes || f->wp.countAbs || f->wp.countRefl || f->wp.countTrans || f->wp.countACD || f->wp.countDirection;
		bool hadDirCount = f->wp.countDirection;
		
		if (enableBtn->GetState() == 0 || ratio == 0.0) {
			//Let the user disable textures with the main switch or by typing 0 as resolution
			f->wp.countDes = f->wp.countAbs = f->wp.countRefl = f->wp.countTrans = f->wp.countACD = f->wp.countDirection = false;
		}
		else {
			if (recordDesBtn->GetState() < 2) f->wp.countDes = recordDesBtn->GetState();
			if (recordAbsBtn->GetState() < 2) f->wp.countAbs = recordAbsBtn->GetState();
			if (recordReflBtn->GetState() < 2) f->wp.countRefl = recordReflBtn->GetState();
			if (recordTransBtn->GetState() < 2) f->wp.countTrans = recordTransBtn->GetState();
			if (recordACBtn->GetState() < 2) f->wp.countACD = recordACBtn->GetState();
			if (recordDirBtn->GetState() < 2) f->wp.countDirection = recordDirBtn->GetState();
		}
		
		bool hasAnyTexture = f->wp.countDes || f->wp.countAbs || f->wp.countRefl || f->wp.countTrans || f->wp.countACD || f->wp.countDirection;
		*/

		if (doTeleport) f->sh.teleportDest = teleport;
		if (doAccfactor) f->sh.accomodationFactor = accfactor;
		if (doDiffuseRefl) f->sh.reflection.diffusePart = diffuseRefl;
		if (doSpecularRefl) f->sh.reflection.specularPart = specularRefl;
		if (doReflExponent) f->sh.reflection.cosineExponent = reflectionExponent;
		if (doSuperStruct) {
			if (f->sh.superIdx != superStruct) {
				f->sh.superIdx = superStruct;
				structChanged = true;
			}
		}
		if (doLink) {
			f->sh.superDest = superDest;
			if (superDest) f->sh.opacity = 1.0; // Force opacity for link facet
		}
		//Moving or not
		if (facetMovingToggle->GetState() < 2) f->sh.isMoving = facetMovingToggle->GetState();
		if (enableSojournTime->GetState() < 2) f->sh.enableSojournTime = enableSojournTime->GetState();
		if (doSojF) f->sh.sojournFreq = sojF;
		if (doSojE) f->sh.sojournE = sojE;

		if (doUseMapA) {
			f->sh.useOutgassingFile = useMapA;
		}

		if (doAngleMapWidth) {
			if (angleMapWidth != f->sh.anglemapParams.phiWidth) {
				//Delete recorded map, will make a new
				f->angleMapCache.clear();
				f->sh.anglemapParams.phiWidth = angleMapWidth;
			}
		}
		if (doAngleMapLowRes) {
			if (angleMapLowRes != f->sh.anglemapParams.thetaLowerRes) {
				//Delete recorded map, will make a new
				f->angleMapCache.clear();
				f->sh.anglemapParams.thetaLowerRes = angleMapLowRes;
			}
		}
		if (doAngleMapHiRes) {
			if (angleMapHiRes != f->sh.anglemapParams.thetaHigherRes) {
				//Delete recorded map, will make a new
				f->angleMapCache.clear();
				f->sh.anglemapParams.thetaHigherRes = angleMapHiRes;
			}
		}
		if (doAngleMapThetaLimit) {
			if (!IsEqual(angleMapThetaLimit,f->sh.anglemapParams.thetaLimit)) {
				//Delete recorded map, will make a new
				f->angleMapCache.clear();
				f->sh.anglemapParams.thetaLimit = angleMapThetaLimit;
			}
		}

		/*
		//set textures
		try {
			bool needsRemeshing = (hadAnyTexture != hasAnyTexture) || (hadDirCount != f->wp.countDirection) || (doRatio && (!IsEqual(geom->GetFacet(sel)->tRatio , ratio)));
			if (needsRemeshing) geom->SetFacetTexture(sel, hasAnyTexture ? ratio : 0.0, hasAnyTexture ? boundMap : false);
		}
		catch (Error &e) {
			GLMessageBox::Display(e.GetMsg(), "Error", GLDLG_OK, GLDLG_ICONWARNING);
			progressDlg->SetVisible(false);
			SAFE_DELETE(progressDlg);
			return false;
		}
		catch (...) {
			GLMessageBox::Display("Unexpected error while setting textures", "Error", GLDLG_OK, GLDLG_ICONWARNING);
			progressDlg->SetVisible(false);
			SAFE_DELETE(progressDlg);
			return false;
		}
		*/
		//if (angleMapRecordCheckbox->GetState() < 2) f->wp.anglemapParams.record = angleMapRecordCheckbox->GetState();
		if (showTexture->GetState() < 2) f->textureVisible = showTexture->GetState();
		if (showVolume->GetState() < 2) f->volumeVisible = showVolume->GetState();
		nbPerformed++;
		progressDlg->SetProgress((double)nbPerformed / (double)selectedFacets.size());
	} //main cycle end
	if (structChanged) geom->BuildGLList(); //Re-render facets

	if (progressDlg) progressDlg->SetVisible(false);
	SAFE_DELETE(progressDlg);

	return ApplyTexture(); //Finally, apply textures
}

/**
* \brief Apply changes to draw settings for a facet
*/
void FacetAdvParams::ApplyDrawSettings() {
	//Apply view settings without stopping the simulation

	double nbSelected = (double)geom->GetNbSelectedFacets();
	double nbPerformed = 0.0;

	for (int i = 0; i < geom->GetNbFacet(); i++) {

		Facet *f = geom->GetFacet(i);
		if (f->selected) {

			if (showTexture->GetState() < 2) f->textureVisible = showTexture->GetState();
			if (showVolume->GetState() < 2) f->volumeVisible = showVolume->GetState();

			nbPerformed += 1.0;
			progressDlg->SetProgress(nbPerformed / nbSelected);
		}

	}
	geom->BuildGLList(); //Re-render facets
}

/**
* \brief Toggles various tickboxes depending on what's active and what's not
* \param src the button that got pressed to call this event
*/
void FacetAdvParams::UpdateToggle(GLComponent *src) {

	/*if (src==boundaryBtn) {
		recordACBtn->SetState(false);
		} else if(src==enableBtn) {
		//boundaryBtn->SetState(enableBtn->GetState());
		} else */

	if (src == recordDesBtn) {
		enableBtn->SetState(true);
		//boundaryBtn->SetState(true);
		recordACBtn->SetState(false);
	}
	else if (src == recordAbsBtn) {
		enableBtn->SetState(true);
		//boundaryBtn->SetState(true);
		recordACBtn->SetState(false);
	}
	else if (src == recordReflBtn) {
		enableBtn->SetState(true);
		//boundaryBtn->SetState(true);
		recordACBtn->SetState(false);
	}
	else if (src == recordTransBtn) {
		enableBtn->SetState(true);
		//boundaryBtn->SetState(true);
		recordACBtn->SetState(false);
	}
	else if (src == recordDirBtn) {
		enableBtn->SetState(true);
		//boundaryBtn->SetState(true);
		recordACBtn->SetState(false);
	}
	else if (src == recordACBtn) {
		if (recordACBtn->GetState()) {
			enableBtn->SetState(true);
			//boundaryBtn->SetState(true);
			recordDesBtn->SetState(false);
			recordAbsBtn->SetState(false);
			recordReflBtn->SetState(false);
			recordTransBtn->SetState(false);
			recordDirBtn->SetState(false);
		}
	}
	else if (src == enableSojournTime) {
		sojournFreq->SetEditable(enableSojournTime->GetState());
		sojournE->SetEditable(enableSojournTime->GetState());
		if (enableSojournTime->GetState() == 0) {
			enableSojournTime->SetText("Wall sojourn time");
		}
		else {
			CalcSojournTime();
		}
	}
	else if (src == angleMapRecordCheckbox) {

	}

	//UpdateSizeForRatio();
}

/**
* \brief Processes events like button clicks for the advanced facet parameters panel.
* \param src the component that got used to call this event
* \param message the type that triggered change (button, text change etc.)
*/
void FacetAdvParams::ProcessMessage(GLComponent *src, int message) {

	switch (message) {

		
	case MSG_BUTTON:
		if (src == quickApply) {

			progressDlg = new GLProgress("Applying view settings", "Please wait");
			progressDlg->SetVisible(true);
			progressDlg->SetProgress(0.5);

			ApplyDrawSettings();

			progressDlg->SetVisible(false);
			SAFE_DELETE(progressDlg);

		}
		
		else if (src==SojournInfoButton) {

			char tmp[] = 
R"(Sojourn time calculated by Frenkel's equation

f: Molecule's surface oscillation frequency [Hz]
E: Adsorption energy [J/mole]
A: Escape probability per oscillation:
A = exp(-E/(R*T))

Probability of sojourn time t:
p(t)= A*f*exp(-A*f*t)

Mean sojourn time:
mean= 1/(A*f) = 1/f*exp(E/(kT))

More info: read report CERN-OPEN-2000-265
from C. Benvenutti http://cds.cern.ch/record/454180
)";
		GLMessageBox::Display(tmp, "Wall sojourn time", GLDLG_OK, GLDLG_ICONINFO);

		}
		else if (src == angleMapExportButton) {
			mApp->ExportAngleMaps();
		}
		else if (src == angleMapCopyButton) {
			mApp->CopyAngleMapToClipboard();
		}
		else if (src == angleMapReleaseButton) {
			mApp->ClearAngleMapsOnSelection();
			Refresh(geom->GetSelectedFacets());
		}
		else if (src == angleMapImportButton) {
			mApp->ImportAngleMaps();
			Refresh(geom->GetSelectedFacets());
		}
		else if (src == remeshButton) {
			ApplyTexture(true);
			worker->Reload();
		}
		
		break;

		
	case MSG_TEXT_UPD:
		mApp->facetApplyBtn->SetEnabled(true);
		if (src == resolutionText) {
			enableBtn->SetState(true);
			UpdateSizeForRatio();
			double res;
			if (resolutionText->GetNumber(&res) && res != 0.0)
				lengthText->SetText(1.0 / res);
			else
				lengthText->SetText("");
		}
		else if (src == lengthText) {
			enableBtn->SetState(true);
			double length;
			if (lengthText->GetNumber(&length) && length != 0.0) {
				resolutionText->SetText(1.0 / length);
				UpdateSizeForRatio();
			}
			else
				resolutionText->SetText("");
		}
		else if (src == sojournFreq || src == sojournE) {
			CalcSojournTime();
		}
		else if (Contains({ diffuseReflBox,specularReflBox },src)) {
			double diffuseRefl, specularRefl;
			if (diffuseReflBox->GetNumber(&diffuseRefl) && specularReflBox->GetNumber(&specularRefl))
				cosineNReflBox->SetText(1.0 - diffuseRefl - specularRefl);
		}
		else if (Contains({ angleMapPhiResText,angleMapThetaLowresText,angleMapThetaHighresText,angleMapThetaLimitText }, src)) {
			angleMapRecordCheckbox->SetState(1);
			if (src == angleMapThetaLimitText) {
				double thetaLimit;
				if (angleMapThetaLimitText->GetNumber(&thetaLimit)) {
					std::stringstream label;
					label << "values from " << thetaLimit << " to PI/2";
					limitLabel->SetText(label.str());
				}
				else {
					limitLabel->SetText("values from limit to PI/2");
				}
			}
		}

		break;

		
	case MSG_TOGGLE:
		UpdateToggle(src);
		mApp->facetApplyBtn->SetEnabled(true);
		break;
	case MSG_TEXT:
		/*if (
			src == resolutionText
			|| src == lengthText
			|| src == facetAccFactor
			|| src == facetTeleport
			|| src == facetSuperDest
			|| src == facetStructure
			|| src == sojournFreq
			|| src == sojournE
			) {*/
			mApp->ApplyFacetParams();
		//}
		break;
	case MSG_COMBO:
		if (src == facetUseDesFile) {
			mApp->facetApplyBtn->SetEnabled(true);
			if (facetUseDesFile->GetSelectedIndex() == 0) {
				//User values
				mApp->facetFlow->SetEditable(true);
				mApp->facetFlowArea->SetEditable(true);
			}
			else { //use desorption file
				mApp->facetFlow->SetEditable(false);
				mApp->facetFlowArea->SetEditable(false);
				//Values from last Refresh();
				char tmp[64];
				sprintf(tmp, "%.2E", sumOutgassing);
				mApp->facetFlow->SetText(tmp);
				sprintf(tmp, "%.2E", sumOutgassing / sumArea);
				mApp->facetFlowArea->SetText(tmp);
			}
		}
		break;
	case MSG_PANELR:
		PlaceComponents();
		break;
	}

	GLWindow::ProcessMessage(src, message);
}

/**
* \brief Text change if Wall sojoourn time option is enabled/disabled
*/
void FacetAdvParams::CalcSojournTime() {
	double sojF,sojE,facetT;
	if (enableSojournTime->GetState() == 0
		|| !(sojournFreq->GetNumber(&sojF))
		|| !(sojournE->GetNumber(&sojE))
		|| !(mApp->facetTemperature->GetNumber(&facetT))) {
		enableSojournTime->SetText("Wall sojourn time");
		return;
	}
	std::ostringstream tmp;
	tmp<< "Wall sojourn time (mean=" << 1.0/(sojF*exp(-sojE /(8.31* facetT))) << " s)";
	enableSojournTime->SetText(tmp.str());
}

/**
* \brief Places Dyanmic desorption area and incident angle distribution properly if closed or open
*/
void FacetAdvParams::PlaceComponents() {
	int desPanelPos, angleMapPanelPos, desPanelHeight, angleMapPanelHeight;
	int x, w;
	desPanel->GetBounds(&x, &desPanelPos, &w, &desPanelHeight);
	desPanel->SetBounds(x, desPanelPos, w, desPanelFullHeight); //height will be overridden if closed
	desPanel->GetBounds(&x, &desPanelPos, &w, &desPanelHeight); //now we get the actual height
	angleMapPanel->SetBounds(x, desPanelPos + desPanelHeight + 8, w, angleMapPanelFullHeight);

	//Update coords below if editing
	angleMapPanel->SetCompBounds(label14, 9, 36, 93, 12);
	angleMapPanel->SetCompBounds(angleMapPhiResText, 115, 77, 46, 18);
	angleMapPanel->SetCompBounds(angleMapThetaLowresText, 115, 33, 46, 18);
	angleMapPanel->SetCompBounds(angleMapImportButton, 140, 122, 64, 20);
	angleMapPanel->SetCompBounds(angleMapExportButton, 63, 122, 73, 20);
	angleMapPanel->SetCompBounds(angleMapRecordCheckbox, 10, 16, 54, 16);
	angleMapPanel->SetCompBounds(angleMapReleaseButton, 207, 122, 93, 20);
	angleMapPanel->SetCompBounds(angleMapCopyButton, 10, 122, 50, 20);
	angleMapPanel->SetCompBounds(limitLabel, 166, 58, 91, 12);
	angleMapPanel->SetCompBounds(label17, 166, 36, 71, 12);
	angleMapPanel->SetCompBounds(angleMapThetaLimitText, 249, 33, 46, 18);
	angleMapPanel->SetCompBounds(label20, 166, 80, 94, 12);
	angleMapPanel->SetCompBounds(label19, 10, 80, 91, 12);
	angleMapPanel->SetCompBounds(angleMapThetaHighresText, 115, 55, 46, 18);
	angleMapPanel->SetCompBounds(angleMapStatusLabel, 10, 106, 105, 12);

	angleMapPanel->GetBounds(&x, &angleMapPanelPos, &w, &angleMapPanelHeight);
	
	int y,h;
	this->GetBounds(&x, &y, &w, &h);
	this->SetBounds(x, y, w, angleMapPanelPos + angleMapPanelHeight + 25);
}