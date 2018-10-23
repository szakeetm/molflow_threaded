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
#pragma once

#include "GLApp/GLWindow.h"
#include <vector>
class GLToggle;
class GLButton;
class GLTextField;
class GLTitledPanel;
class GLLabel;
class GLProgress;
class GLCombo;
class Geometry;
class Worker;

class FacetAdvParams : public GLWindow {

public:

	// Construction
	FacetAdvParams(Worker *w);

	// Component method
	void Refresh(std::vector<size_t> selection);
	void Reposition(int wD = 0, int hD = 0);

	// Implementation
	void ProcessMessage(GLComponent *src, int message);
	bool ApplyTexture(bool force=false);
	bool Apply();

private:

	void CalcSojournTime();
	void PlaceComponents();
	void UpdateSize();
	void UpdateSizeForRatio();
	void UpdateToggle(GLComponent *src);
	void ApplyDrawSettings(); //Apply View Settings without stopping the simulation

	Worker   *worker;
	Geometry *geom;
	int       fIdx;
	double sumArea, sumOutgassing;
	size_t sumAngleMapSize;
	int desPanelFullHeight, angleMapPanelFullHeight;

	GLProgress* progressDlg;

	//Designer generated code below
	GLTitledPanel	*aPanel;
	GLTextField	*lengthText;
	GLLabel	*perCm;
	GLTextField	*resolutionText;
	GLLabel	*l5;
	GLToggle	*enableBtn;
	GLToggle	*recordDesBtn;
	GLLabel	*perCell;
	GLToggle	*recordDirBtn;
	GLToggle	*recordTransBtn;
	GLToggle	*recordReflBtn;
	GLToggle	*recordACBtn;
	GLToggle	*recordAbsBtn;
	GLTitledPanel	*mPanel;
	GLToggle	*showTexture;
	GLToggle	*showVolume;
	GLTextField	*cellText;
	GLLabel	*l8;
	GLTextField	*ramText;
	GLLabel	*l7;
	GLTitledPanel	*vPanel;
	GLButton	*quickApply;
	GLTitledPanel	*desPanel;
	GLTextField	*fileYieldText;
	GLLabel	*label3;
	GLLabel	*label1;
	GLLabel	*label2;
	GLTitledPanel	*paramPanel;
	GLTitledPanel	*angleMapPanel;
	GLToggle	*facetMovingToggle;
	GLTextField	*facetSuperDest;
	GLLabel	*label8;
	GLTextField	*facetStructure;
	GLLabel	*label7;
	GLTextField	*facetTeleport;
	GLLabel	*label4;
	GLLabel	*label5;
	GLLabel	*label6;
	GLCombo	*facetUseDesFile;
	GLTextField	*facetAccFactor;
	GLTextField	*fileDoseText;
	GLTextField	*fileFluxText;
	GLLabel	*label11;
	GLLabel	*label9;
	GLLabel	*label12;
	GLLabel	*label10;
	GLToggle	*enableSojournTime;
	GLLabel	*sojournLabel3;
	GLTextField	*sojournE;
	GLLabel	*sojournLabel2;
	GLLabel	*sojournLabel1;
	GLTextField	*sojournFreq;
	GLButton	*SojournInfoButton;
	GLLabel	*label14;
	GLTextField	*angleMapPhiResText;
	GLTextField	*angleMapThetaLowresText;
	GLButton	*angleMapImportButton;
	GLButton	*angleMapExportButton;
	GLToggle	*angleMapRecordCheckbox;
	GLButton	*angleMapReleaseButton;
	GLButton	*remeshButton;
	GLLabel	*label16;
	GLLabel	*label15;
	GLLabel	*label13;
	GLTextField	*cosineNReflBox;
	GLTextField	*specularReflBox;
	GLTextField	*diffuseReflBox;
	GLButton	*angleMapCopyButton;
	GLLabel	*limitLabel;
	GLLabel	*label17;
	GLTextField	*angleMapThetaLimitText;
	GLLabel	*label20;
	GLLabel	*label19;
	GLTextField	*angleMapThetaHighresText;
	GLLabel	*angleMapStatusLabel;
	GLTextField	*reflectionExponentBox;
};
