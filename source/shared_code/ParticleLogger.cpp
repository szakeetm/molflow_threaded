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
#include "ParticleLogger.h"
#include "GLApp/GLTitledPanel.h"
#include "GLApp/GLToolkit.h"
#include "GLApp/GLWindowManager.h"
#include "GLApp/GLMessageBox.h"

#include "GLApp/GLButton.h"
#include "GLApp/GLTextField.h"
#include "GLApp/GLLabel.h"
#include "GLApp/GLToggle.h"
//#include "GLApp/GLFileBox.h"
#include "NativeFileDialog/molflow_wrapper/nfd_wrapper.h"

#include "Geometry_shared.h"
#include "Facet_shared.h"

#include <fstream>

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

ParticleLogger::ParticleLogger(Geometry *g, Worker *w) :GLWindow() {

	int wD = 367;
	int hD = 345;
	logParamPanel = new GLTitledPanel("Recording settings");
	logParamPanel->SetBounds(17, 79, 322, 149);
	Add(logParamPanel);
	resultPanel = new GLTitledPanel("Result");
	resultPanel->SetBounds(17, 234, 322, 84);
	Add(resultPanel);
	descriptionLabel = new GLLabel("This tool allows to record all test particles hitting a chosen facet.\nOnly one facet can be recorded at a time.\nThe recording must be exported, it is not saved with the file.");
	descriptionLabel->SetBounds(13, 18, 805, 13);
	Add(descriptionLabel);

	label2 = new GLLabel("Facet number:");
	logParamPanel->SetCompBounds(label2, 9, 52, 75, 13);
	logParamPanel->Add(label2);

	getSelectedFacetButton = new GLButton(0, "<- Get selected");
	logParamPanel->SetCompBounds(getSelectedFacetButton, 201, 51, 109, 23);
	logParamPanel->Add(getSelectedFacetButton);

	facetNumberTextbox = new GLTextField(0, "");
	logParamPanel->SetCompBounds(facetNumberTextbox, 89, 51, 106, 20);
	logParamPanel->Add(facetNumberTextbox);

	label3 = new GLLabel("Max recorded:");
	logParamPanel->SetCompBounds(label3, 9, 78, 75, 13);
	logParamPanel->Add(label3);

	maxRecordedTextbox = new GLTextField(0, "100000");
	logParamPanel->SetCompBounds(maxRecordedTextbox, 89, 75, 106, 20);
	logParamPanel->Add(maxRecordedTextbox);

	memoryLabel = new GLLabel("memory");
	logParamPanel->SetCompBounds(memoryLabel, 202, 78, 43, 13);
	logParamPanel->Add(memoryLabel);

	applyButton = new GLButton(0, "Apply");
	logParamPanel->SetCompBounds(applyButton, 120, 110, 75, 23);
	logParamPanel->Add(applyButton);

	statusLabel = new GLLabel("No recording.");
	resultPanel->SetCompBounds(statusLabel, 9, 16, 35, 13);
	resultPanel->Add(statusLabel);

	copyButton = new GLButton(0, "Copy to clipboard");
	resultPanel->SetCompBounds(copyButton, 12, 45, 139, 23);
	resultPanel->Add(copyButton);

	exportButton = new GLButton(0, "Export to CSV");
	resultPanel->SetCompBounds(exportButton, 157, 45, 153, 23);
	resultPanel->Add(exportButton);

	enableCheckbox = new GLToggle(0, "Enable logging");
	logParamPanel->SetCompBounds(enableCheckbox, 11, 19, 96, 17);
	logParamPanel->Add(enableCheckbox);

	SetTitle("ParticleLogger");
	// Center dialog
	//int wS, hS;
	//GLToolkit::GetScreenSize(&wS, &hS);
	//int xD = (wS - wD) / 2;
	//int yD = (hS - hD) / 2;
	SetBounds(10, 40, wD, hD); //Top left

	RestoreDeviceObjects();

	UpdateMemoryEstimate();
	geom = g;
	work = w;
	isRunning = false;

}

void ParticleLogger::ProcessMessage(GLComponent *src, int message) {

	switch (message) {
	case MSG_BUTTON:
		if (!isRunning) {
			if (src == applyButton) {
				int facetId, nbRec;
				if (!facetNumberTextbox->GetNumberInt(&facetId) || facetId<1 || facetId>geom->GetNbFacet()) {
					GLMessageBox::Display("Invalid facet number", "Error", GLDLG_OK, GLDLG_ICONERROR);
					return;
				}
				facetId--;
				if (!maxRecordedTextbox->GetNumberInt(&nbRec) || nbRec < 1) {
					GLMessageBox::Display("Invalid max rec. number", "Error", GLDLG_OK, GLDLG_ICONERROR);
					return;
				}
				work->ontheflyParams.enableLogging = (enableCheckbox->GetState() == 1);
				work->ontheflyParams.logFacetId = facetId;
				work->ontheflyParams.logLimit = nbRec;
				work->ChangeSimuParams();
				UpdateStatus();
			}
			else if (src == getSelectedFacetButton) {
				auto selFacets = geom->GetSelectedFacets();
				if (selFacets.size() != 1) {
					GLMessageBox::Display("Select exactly one facet", "Error", GLDLG_OK, GLDLG_ICONERROR);
					return;
				}
				facetNumberTextbox->SetText(selFacets[0] + 1);
				enableCheckbox->SetState(1);
			}
			else if (src == exportButton) {
				//Export to CSV
				
				std::vector<ParticleLoggerItem>& log = work->GetLog();
				//FILENAME *fn = GLFileBox::SaveFile(NULL, NULL, "Save log", "All files\0*.*\0", NULL);
				std::string fn = NFD_SaveFile_Cpp("csv", "");
				if (!fn.empty()) {
					bool ok = true;
					
					std::string formattedFileName = fn;
					if (FileUtils::GetExtension(formattedFileName) == "")
					{
						formattedFileName = formattedFileName + ".csv";

						if (FileUtils::Exist(formattedFileName)) {
							std::ostringstream tmp;
							tmp << "Overwrite existing file ?\n" << formattedFileName;
							ok = (GLMessageBox::Display(tmp.str().c_str(), "Question", GLDLG_OK | GLDLG_CANCEL, GLDLG_ICONWARNING) == GLDLG_OK);
						}
					}
					
					if (ok) {
						std::ofstream file(formattedFileName);
						exportButton->SetText("Abort");
						isRunning = true;
						ConvertLogToText(work->log, ",", &file);
						isRunning = false;
						exportButton->SetText("Export to CSV");
						file.close();
					}
				}
				work->ReleaseLog();
			}
			else if (src == copyButton) {
				//Copy to clipboard
				auto log = work->GetLog();
				copyButton->SetText("Abort");
				isRunning = true;
				std::string clipBoardText = ConvertLogToText(log, "\t");
				isRunning = false;
				copyButton->SetText("Copy to clipboard");
				work->ReleaseLog();
				bool ok = sizeof(clipBoardText[0])*clipBoardText.length() < 50 * 1024 * 1024;
				if (!ok) {
					std::ostringstream msg;
					msg << "Careful! You're putting " << mApp->FormatSize(sizeof(clipBoardText[0])*clipBoardText.length()) << " to the clipboard.";
					msg << "\nMaybe it's a better idea to save it as a file. Try anyway?";
					int retVal = GLMessageBox::Display(msg.str(), "Large log size", { "Yes","Cancel" }, GLDLG_ICONWARNING);
					ok = retVal == 0;
				}
				if (ok) GLToolkit::CopyTextToClipboard(clipBoardText);
			}
		}
		else {
			if (src == copyButton) copyButton->SetText("Copy To Clipboard");
			else if (src == exportButton) exportButton->SetText("Export to CSV");
			isRunning = false;
			work->abortRequested = true;
		}
		break;
	case MSG_TEXT_UPD:
		UpdateMemoryEstimate();
		break;
	}

	GLWindow::ProcessMessage(src, message);
}

void ParticleLogger::UpdateMemoryEstimate() {
	int nbRec;
	if (maxRecordedTextbox->GetNumberInt(&nbRec) && nbRec > 0) {
		memoryLabel->SetText(mApp->FormatSize(2 * nbRec * sizeof(ParticleLoggerItem)));
	}
}

void ParticleLogger::UpdateStatus() {

	auto log = work->GetLog();
	if (log.size() == 0) {
		statusLabel->SetText("No recording.");
	}
	else {
		std::ostringstream tmp;
		tmp << log.size() << " particles logged";
		statusLabel->SetText(tmp.str());
	}
	work->ReleaseLog();
}

std::string ParticleLogger::ConvertLogToText(const std::vector<ParticleLoggerItem>& log, const std::string& separator, std::ofstream* targetFile) {
	std::ostringstream targetString;
	std::ostringstream tmp;
	bool directWriteMode = targetFile != NULL;
	//Header
	work->abortRequested = false;
	

	tmp << "Pos_X_[cm]" << separator 
		<< "Pos_Y_[cm]" << separator
		<< "Pos_Z_[cm]" << separator
		<< "Pos_u" << separator
		<< "Pos_v" << separator
		<< "Dir_X" << separator 
		<< "Dir_Y" << separator
		<< "Dir_Z" << separator
		<< "Dir_theta_[rad]" << separator
		<< "Dir_phi_[rad]" << separator
		<< "LowFluxRatio" << separator;

#ifdef MOLFLOW
	tmp << "Velocity_[m/s]" << separator
		<< "HitTime_[s]" << separator
		<< "ParticleDecayMoment_[s]" << separator;
#endif // MOLFLOW
#ifdef SYNRAD
	tmp << "Energy_[eV]" << separator
		<< "Flux_[photon/s]" << separator
		<< "Power_[W]" << separator;
#endif // SYNRAD
	tmp << "\n";

	if (directWriteMode)
		*targetFile << tmp.str();
	else
		targetString << tmp.str();
	tmp.clear();
	tmp.str(std::string());

	//Lines
	GLProgress* prg = new GLProgress("Assembling text", "Particle logger");
	prg->SetVisible(true);
	for (size_t i = 0; !work->abortRequested && i < log.size(); i++) {
		prg->SetProgress((double)i / (double)log.size());
		mApp->DoEvents(); //To catch eventual abort button click
		Facet* f = work->GetGeometry()->GetFacet(work->ontheflyParams.logFacetId);
		Vector3d hitPos = f->sh.O + log[i].facetHitPosition.u*f->sh.U + log[i].facetHitPosition.v*f->sh.V;
		
		double u = sin(log[i].hitTheta)*cos(log[i].hitPhi);
		double v = sin(log[i].hitTheta)*sin(log[i].hitPhi);
		double n = cos(log[i].hitTheta);
		
		Vector3d hitDir = u*f->sh.nU + v*f->sh.nV + n* f->sh.N;

		tmp << hitPos.x << separator
			<< hitPos.y << separator
			<< hitPos.z << separator
			<< log[i].facetHitPosition.u << separator
			<< log[i].facetHitPosition.v << separator
			<< hitDir.x << separator
			<< hitDir.y << separator
			<< hitDir.z << separator
			<< log[i].hitTheta << separator
			<< log[i].hitPhi << separator
			<< log[i].oriRatio << separator;

#ifdef MOLFLOW
		tmp << log[i].velocity << separator
			<< log[i].time << separator
			<< log[i].particleDecayMoment << separator;
#endif // MOLFLOW
#ifdef SYNRAD
		tmp << log[i].energy << separator
			<< log[i].dF << separator
			<< log[i].dP << separator;
#endif // SYNRAD

		tmp << "\n";

		if (directWriteMode)
			*targetFile << tmp.str();
		else
			targetString << tmp.str();
		tmp.clear();
		tmp.str(std::string());

	}
	prg->SetVisible(false);
	SAFE_DELETE(prg);
	return targetString.str();
}