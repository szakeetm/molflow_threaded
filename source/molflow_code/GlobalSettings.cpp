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

//#include "GLApp/GLLabel.h"
#include "GlobalSettings.h"
#include "GLApp/GLToolkit.h"
#include "GLApp/GLToggle.h"
#include "GLApp/GLMessageBox.h"
#include "GLApp/GLList.h"
#include "GLApp/GLInputBox.h"
#include "Facet_shared.h"
#include "Geometry_shared.h"
#include "GLApp/MathTools.h"
#include "GLApp/GLButton.h"
#include "GLApp/GLTextField.h"
#include "GLApp/GLLabel.h"
#include "GLApp/GLToggle.h"
#include "GLApp/GLTitledPanel.h"
#include "Buffer_shared.h"
#include "AppUpdater.h"
#ifdef MOLFLOW
#include "MolFlow.h"
#endif

#ifndef _WIN32
//getpid in linux
#include <sys/types.h>
#include <unistd.h>
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

static const int   plWidth[] = { 60,40,70,70,335 };
static const char *plName[] = { "#","PID","Mem Usage","Mem Peak",/*"CPU",*/"Status" };
static const int   plAligns[] = { ALIGN_LEFT,ALIGN_LEFT,ALIGN_LEFT,ALIGN_LEFT,ALIGN_LEFT };

GlobalSettings::GlobalSettings(Worker *w) :GLWindow() {

	worker = w;
	int wD = 580;
	int hD = 525;

	SetTitle("Global Settings");
	SetIconfiable(true);

	GLTitledPanel *settingsPanel = new GLTitledPanel("Program settings");
	settingsPanel->SetBounds(5, 2, 270, 242);
	Add(settingsPanel);

	GLLabel *asLabel = new GLLabel("Autosave frequency (minutes):");
	asLabel->SetBounds(16, 22, 80, 19);
	settingsPanel->Add(asLabel);

	autoSaveText = new GLTextField(0, "");
	autoSaveText->SetBounds(170, 20, 30, 19);
	settingsPanel->Add(autoSaveText);

	chkSimuOnly = new GLToggle(0, "Autosave only when simulation is running");
	chkSimuOnly->SetBounds(15, 47, 160, 19);
	settingsPanel->Add(chkSimuOnly);

	chkCompressSavedFiles = new GLToggle(0, "Use .zip as default extension (otherwise .xml)");
	chkCompressSavedFiles->SetBounds(15, 72, 100, 19);
	settingsPanel->Add(chkCompressSavedFiles);

	chkCheckForUpdates = new GLToggle(0, "Check for updates at startup");
	chkCheckForUpdates->SetBounds(15, 97, 160, 19);
	settingsPanel->Add(chkCheckForUpdates);

	chkAutoUpdateFormulas = new GLToggle(0, "Auto refresh formulas");
	chkAutoUpdateFormulas->SetBounds(15, 122, 160, 19);
	settingsPanel->Add(chkAutoUpdateFormulas);

	chkAntiAliasing = new GLToggle(0, "Anti-Aliasing");
	chkAntiAliasing->SetBounds(15, 147, 160, 19);
	settingsPanel->Add(chkAntiAliasing);

	chkWhiteBg = new GLToggle(0, "White Background");
	chkWhiteBg->SetBounds(15, 172, 160, 19);
	settingsPanel->Add(chkWhiteBg);

	leftHandedToggle = new GLToggle(0, "Left-handed coord. system");
	leftHandedToggle->SetBounds(15, 197, 160, 19);
	settingsPanel->Add( leftHandedToggle);

	highlightNonplanarToggle = new GLToggle(0, "Highlight non-planar facets");
	highlightNonplanarToggle->SetBounds(15, 222, 160, 19);
	settingsPanel->Add(highlightNonplanarToggle);

	GLTitledPanel *simuSettingsPanel = new GLTitledPanel("Simulation settings");
	simuSettingsPanel->SetBounds(280, 2, 290, 242);
	Add(simuSettingsPanel);

	GLLabel *massLabel = new GLLabel("Gas molecular mass (g/mol):");
	massLabel->SetBounds(290, 22, 150, 19);
	simuSettingsPanel->Add(massLabel);

	gasMassText = new GLTextField(0, "");
	gasMassText->SetBounds(460, 20, 100, 19);
	simuSettingsPanel->Add(gasMassText);

	enableDecay = new GLToggle(0, "Gas half life (s):");
	enableDecay->SetBounds(290, 47, 150, 19);
	simuSettingsPanel->Add(enableDecay);

	halfLifeText = new GLTextField(0, "");
	halfLifeText->SetBounds(460, 45, 100, 19);
	simuSettingsPanel->Add(halfLifeText);
	
	GLLabel *outgassingLabel = new GLLabel("Final outgassing rate (mbar*l/sec):");
	outgassingLabel->SetBounds(290, 72, 150, 19);
	simuSettingsPanel->Add(outgassingLabel);

	outgassingText = new GLTextField(0, "");
	outgassingText->SetBounds(460, 70, 100, 19);
	outgassingText->SetEditable(false);
	simuSettingsPanel->Add(outgassingText);

	GLLabel *influxLabel = new GLLabel("Total desorbed molecules:");
	influxLabel->SetBounds(290, 97, 150, 19);
	simuSettingsPanel->Add(influxLabel);

	influxText = new GLTextField(0, "");
	influxText->SetBounds(460, 95, 100, 19);
	influxText->SetEditable(false);
	simuSettingsPanel->Add(influxText);

	recalcButton = new GLButton(0, "Recalc. outgassing");
	recalcButton->SetBounds(460, 123, 100, 19);
	simuSettingsPanel->Add(recalcButton);

	lowFluxToggle = new GLToggle(0, "Enable low flux mode");
	lowFluxToggle->SetBounds(290, 150, 120, 19);
	simuSettingsPanel->Add(lowFluxToggle);

	lowFluxInfo = new GLButton(0, "?");
	lowFluxInfo->SetBounds(420, 150, 20, 19);
	simuSettingsPanel->Add(lowFluxInfo);

	GLLabel *cutoffLabel = new GLLabel("Cutoff ratio:");
	cutoffLabel->SetBounds(310, 176, 80, 19);
	simuSettingsPanel->Add(cutoffLabel);

	cutoffText = new GLTextField(0, "");
	cutoffText->SetBounds(370, 175, 70, 19);
	cutoffText->SetEditable(false);
	simuSettingsPanel->Add(cutoffText);

	applyButton = new GLButton(0, "Apply above settings");
	applyButton->SetBounds(wD / 2 - 65, 248, 130, 19);
	Add(applyButton);

	/*chkNonIsothermal = new GLToggle(0,"Non-isothermal system (textures only, experimental)");
	chkNonIsothermal->SetBounds(315,125,100,19);
	Add(chkNonIsothermal);*/

	GLTitledPanel *panel3 = new GLTitledPanel("Process control");
	panel3->SetBounds(5, 284, wD - 10, hD - 285);
	Add(panel3);

	processList = new GLList(0);
	processList->SetHScrollVisible(true);
	processList->SetSize(5, MAX_PROCESS+1);
	processList->SetColumnWidths((int*)plWidth);
	processList->SetColumnLabels((const char **)plName);
	processList->SetColumnAligns((int *)plAligns);
	processList->SetColumnLabelVisible(true);
	processList->SetBounds(10, 278, wD - 20, hD - 355);
	panel3->Add(processList);

	char tmp[128];
	sprintf(tmp, "Number of CPU cores:     %zd", mApp->numCPU);
	GLLabel *coreLabel = new GLLabel(tmp);
	coreLabel->SetBounds(10, hD - 74, 120, 19);
	panel3->Add(coreLabel);

	GLLabel *l1 = new GLLabel("Number of subprocesses:");
	l1->SetBounds(10, hD - 49, 120, 19);
	panel3->Add(l1);

	nbProcText = new GLTextField(0, "");
	nbProcText->SetEditable(true);
	nbProcText->SetBounds(135, hD - 51, 30, 19);
	panel3->Add(nbProcText);

	restartButton = new GLButton(0, "Apply and restart processes");
	restartButton->SetBounds(170, hD - 51, 150, 19);
	panel3->Add(restartButton);

	maxButton = new GLButton(0, "Change MAX desorbed molecules");
	maxButton->SetBounds(wD - 195, hD - 51, 180, 19);
	panel3->Add(maxButton);

	

	// Center dialog
	int wS, hS;
	GLToolkit::GetScreenSize(&wS, &hS);
	int xD = (wS - wD) / 2;
	int yD = (hS - hD) / 2;
	SetBounds(xD, yD, wD, hD);

	RestoreDeviceObjects();

	lastUpdate = 0;
	//for (size_t i = 0; i < MAX_PROCESS; i++) lastCPUTime[i] = -1.0f;
	//memset(lastCPULoad, 0, MAX_PROCESS*sizeof(float));
}

void GlobalSettings::Update() {

	char tmp[256];
	chkAntiAliasing->SetState(mApp->antiAliasing);
	chkWhiteBg->SetState(mApp->whiteBg);
	highlightNonplanarToggle->SetState(mApp->highlightNonplanarFacets);
	leftHandedToggle->SetState(mApp->leftHandedView);
	//chkNonIsothermal->SetState(nonIsothermal);
	UpdateOutgassing();

	gasMassText->SetText(worker->wp.gasMass);

	enableDecay->SetState(worker->wp.enableDecay);
	halfLifeText->SetText(worker->wp.halfLife);
	halfLifeText->SetEditable(worker->wp.enableDecay);

	cutoffText->SetText(worker->ontheflyParams.lowFluxCutoff);
	cutoffText->SetEditable(worker->ontheflyParams.lowFluxMode);
	lowFluxToggle->SetState(worker->ontheflyParams.lowFluxMode);

	autoSaveText->SetText(mApp->autoSaveFrequency);
	chkSimuOnly->SetState(mApp->autoSaveSimuOnly);
	if (mApp->appUpdater) { //Updater initialized
		chkCheckForUpdates->SetState(mApp->appUpdater->IsUpdateCheckAllowed());
	}
	else {
		chkCheckForUpdates->SetState(0);
		chkCheckForUpdates->SetEnabled(false);
	}
	chkAutoUpdateFormulas->SetState(mApp->autoUpdateFormulas);
	chkCompressSavedFiles->SetState(mApp->compressSavedFiles);

	size_t nb = worker->GetProcNumber();
	sprintf(tmp, "%zd", nb);
	nbProcText->SetText(tmp);
}

void GlobalSettings::SMPUpdate() {
	int time = SDL_GetTicks();

	if (!IsVisible() || IsIconic()) return;
	size_t nb = worker->GetProcNumber();
	if (processList->GetNbRow() != (nb + 1)) processList->SetSize(5, nb + 1,true);

	if( time-lastUpdate>333 ) {

	char tmp[512];
	//PROCESS_INFO pInfo;
	std::vector<size_t>  states(MAX_PROCESS);
	std::vector<std::string> statusStrings(MAX_PROCESS);
	worker->GetProcStatus(states, statusStrings);

	processList->ResetValues();

	//Interface
#ifdef _WIN32
	size_t currPid = GetCurrentProcessId();
#else
	size_t currPid = getpid();
#endif
	//GetProcInfo(currpid, &pInfo);
	processList->SetValueAt(0, 0, "Interface");
	sprintf(tmp, "%zd", currPid);
	processList->SetValueAt(1, 0, tmp, (int)currPid);
	sprintf(tmp, "? MB"/*, (double)pInfo.mem_use / (1024.0*1024.0)*/);
	//processList->SetValueAt(2, 0, tmp);
	sprintf(tmp, "? MB"/*, (double)pInfo.mem_peak / (1024.0*1024.0)*/);
	//processList->SetValueAt(3, 0, tmp);
	//sprintf(tmp, "%d %%", (int)pInfo.cpu_time);
	//processList->SetValueAt(4, 0, tmp);

	for (int i = 0;i<nb;i++) {
		//size_t pid = worker->GetPID(i);
		auto pid = worker->workerControl.threads[i].get_id();
		sprintf(tmp, "Thread %d", i + 1);
		processList->SetValueAt(0, i + 1, tmp);
		std::ostringstream tmpstr; tmpstr << pid;
		processList->SetValueAt(1, i + 1, tmpstr.str().c_str());
		if (pid == std::thread::id()) {
			processList->SetValueAt(2, i + 1, "0 KB");
			processList->SetValueAt(3, i + 1, "0 KB");
			//processList->SetValueAt(4,i+1,"0 %");
			processList->SetValueAt(4, i + 1, "Dead");
		}
		else {
			sprintf(tmp, "? MB"/*, (double)pInfo.mem_use / (1024.0*1024.0)*/);
			//processList->SetValueAt(2, i + 1, tmp);
			sprintf(tmp, "? MB"/*, (double)pInfo.mem_peak / (1024.0*1024.0)*/);
			//processList->SetValueAt(3, i + 1, tmp);
			//sprintf(tmp, "%d %%", (int)pInfo.cpu_time);
			//processList->SetValueAt(4, i+1, tmp);
			/*
			// CPU usage
			if( lastCPUTime[i]!=-1.0f ) {
			float dTime = appTime-lastUpdate;
			float dCPUTime = (float)pInfo.cpu_time-lastCPUTime[i];
			float cpuLoad = dCPUTime/dTime;
			lastCPULoad[i] = 0.85f*cpuLoad + 0.15f*lastCPULoad[i];
			int percent = (int)(100.0f*lastCPULoad[i] + 0.5f);
			if(percent<0) percent=0;
			sprintf(tmp,"%d %%",percent);
			processList->SetValueAt(4,i,tmp);
			} else {
			processList->SetValueAt(4,i,"---");
			}
			lastCPUTime[i] = (float)pInfo.cpu_time;
			*/

			// State/Status
			std::stringstream tmp; tmp << "[" << prStates[states[i]] << "] " << statusStrings[i];
			processList->SetValueAt(4, i + 1, tmp.str().c_str());

		}
	}
	lastUpdate = SDL_GetTicks();
	}

}

void GlobalSettings::RestartProc() {

	int nbProc;
	if (!nbProcText->GetNumberInt(&nbProc)) {
		GLMessageBox::Display("Invalid process number", "Error", GLDLG_OK, GLDLG_ICONERROR);
	}
	else {
			if (nbProc <= 0 || nbProc > MAX_PROCESS) {
				GLMessageBox::Display("Invalid process number [1..32]", "Error", GLDLG_OK, GLDLG_ICONERROR);
			}
			else {
				try {
					if (worker->isRunning) worker->Stop_Public();
					worker->SetProcNumber(nbProc,true);
					worker->RealReload(true);
					mApp->SaveConfig();
				}
				catch (Error &e) {
					GLMessageBox::Display(e.GetMsg(), "Error", GLDLG_OK, GLDLG_ICONERROR);
				}
			}
	}

}

void GlobalSettings::ProcessMessage(GLComponent *src, int message) {

	switch (message) {
	case MSG_BUTTON:

		if (src == recalcButton) {
			if (mApp->AskToReset()) {
				try {
					worker->RealReload();
				}
				catch (Error &e) {
					GLMessageBox::Display(e.GetMsg(), "Recalculation failed: Couldn't reload Worker", GLDLG_OK, GLDLG_ICONWARNING);
				}
			}
		}
		else if (src == restartButton) {
			RestartProc();
		}
		else if (src == maxButton) {
			if (worker->GetGeometry()->IsLoaded()) {
				char tmp[128];
				sprintf(tmp, "%zd", worker->ontheflyParams.desorptionLimit);
				char *val = GLInputBox::GetInput(tmp, "Desorption max (0=>endless)", "Edit MAX");
				if (val) {
					size_t maxDes;
					if (sscanf(val, "%zd", &maxDes) == 0) {
						GLMessageBox::Display("Invalid 'maximum desorption' number", "Error", GLDLG_OK, GLDLG_ICONERROR);
					}
					else {
						worker->ontheflyParams.desorptionLimit = maxDes;
						worker->ChangeSimuParams(); //Sync with subprocesses
					}
				}
			}
			else {
				GLMessageBox::Display("No geometry loaded.", "No geometry", GLDLG_OK, GLDLG_ICONERROR);
			}
		}
		else if (src == applyButton) {
			mApp->antiAliasing = chkAntiAliasing->GetState();
			mApp->whiteBg = chkWhiteBg->GetState();
			mApp->highlightNonplanarFacets = highlightNonplanarToggle->GetState();
			mApp->leftHandedView = (bool)leftHandedToggle->GetState();
			for (int i = 0; i < MAX_VIEWER; i++) {
				mApp->viewer[i]->UpdateMatrix();
				mApp->viewer[i]->UpdateLabelColors();
			}
			mApp->wereEvents = true;
			bool updateCheckPreference = chkCheckForUpdates->GetState();
			if (mApp->appUpdater) {
				if (mApp->appUpdater->IsUpdateCheckAllowed() != updateCheckPreference) {
					mApp->appUpdater->SetUserUpdatePreference(updateCheckPreference);
				}
			}
			mApp->autoUpdateFormulas = chkAutoUpdateFormulas->GetState();
			mApp->compressSavedFiles = chkCompressSavedFiles->GetState();
			mApp->autoSaveSimuOnly = chkSimuOnly->GetState();
			double gm;
			if (!gasMassText->GetNumber(&gm) || !(gm > 0.0)) {
				GLMessageBox::Display("Invalid gas mass", "Error", GLDLG_OK, GLDLG_ICONERROR);
				return;
			}
			if (std::abs(gm - worker->wp.gasMass) > 1e-7) {
				if (mApp->AskToReset()) {
					worker->needsReload = true;
					worker->wp.gasMass = gm;
					if (worker->GetGeometry()->IsLoaded()) { //check if there are pumps
						bool hasPump = false;
						size_t nbFacet = worker->GetGeometry()->GetNbFacet();
						for (size_t i = 0; (i<nbFacet) && (!hasPump); i++) {
							if (worker->GetGeometry()->GetFacet(i)->sh.sticking>0.0) {
								hasPump = true;
							}
						}
						if (hasPump) GLMessageBox::Display("Don't forget the pumps: update pumping speeds and/or recalculate sticking factors.", "You have changed the gas mass.", GLDLG_OK, GLDLG_ICONINFO);
					}
				}
			}

			double hl;
			if (enableDecay->GetState() && (!halfLifeText->GetNumber(&hl) || !(hl > 0.0))) {
				GLMessageBox::Display("Invalid half life", "Error", GLDLG_OK, GLDLG_ICONERROR);
				return;
			}
			if ((enableDecay->GetState()==1) != worker->wp.enableDecay || ((enableDecay->GetState()==1) && IsEqual(hl, worker->wp.halfLife))) {
				if (mApp->AskToReset()) {
					worker->needsReload = true;
					worker->wp.enableDecay = enableDecay->GetState();
					if (worker->wp.enableDecay) worker->wp.halfLife = hl;
				}
			}

			double cutoffnumber;
			if (!cutoffText->GetNumber(&cutoffnumber) || !(cutoffnumber>0.0 && cutoffnumber<1.0)) {
				GLMessageBox::Display("Invalid cutoff ratio, must be between 0 and 1", "Error", GLDLG_OK, GLDLG_ICONWARNING);
				return;
			}

			if (!IsEqual(worker->ontheflyParams.lowFluxCutoff, cutoffnumber) || (int)worker->ontheflyParams.lowFluxMode != lowFluxToggle->GetState()) {
				worker->ontheflyParams.lowFluxCutoff = cutoffnumber;
				worker->ontheflyParams.lowFluxMode = lowFluxToggle->GetState();
				worker->ChangeSimuParams();
			}

			double autosavefreq;
			if (!autoSaveText->GetNumber(&autosavefreq) || !(autosavefreq > 0.0)) {
				GLMessageBox::Display("Invalid autosave frequency", "Error", GLDLG_OK, GLDLG_ICONERROR);
				return;
			}
			mApp->autoSaveFrequency = autosavefreq;

			return;

		}
		else if (src == lowFluxInfo) {
			GLMessageBox::Display("Low flux mode helps to gain more statistics on low pressure parts of the system, at the expense\n"
				"of higher pressure parts. If a traced particle reflects from a high sticking factor surface, regardless of that probability,\n"
				"a reflected test particle representing a reduced flux will still be traced. Therefore test particles can reach low flux areas more easily, but\n"
				"at the same time tracing a test particle takes longer. The cutoff ratio defines what ratio of the originally generated flux\n"
				"can be neglected. If, for example, it is 0.001, then, when after subsequent reflections the test particle carries less than 0.1%\n"
				"of the original flux, it will be eliminated. A good advice is that if you'd like to see pressure across N orders of magnitude, set it to 1E-N"
				, "Low flux mode", GLDLG_OK, GLDLG_ICONINFO);
			return;
		}
		break;

	case MSG_TEXT:
		ProcessMessage(applyButton, MSG_BUTTON);
		break;

	case MSG_TOGGLE:
		if (src == enableDecay) {
			halfLifeText->SetEditable(enableDecay->GetState());
		} else if (src == lowFluxToggle) {
			cutoffText->SetEditable(lowFluxToggle->GetState());
		}
		break;
	}

	GLWindow::ProcessMessage(src, message);
}

void GlobalSettings::UpdateOutgassing() {
	char tmp[128];
	sprintf(tmp, "%g", worker->wp.gasMass);
	gasMassText->SetText(tmp);
	sprintf(tmp, "%g", worker->wp.finalOutgassingRate_Pa_m3_sec * 10.00); //10: conversion Pa*m3/sec -> mbar*l/s
	outgassingText->SetText(tmp);
	sprintf(tmp, "%.3E", worker->wp.totalDesorbedMolecules);
	influxText->SetText(tmp);
}