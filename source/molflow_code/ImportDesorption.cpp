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
#include "ImportDesorption.h"
#include "GLApp/GLToolkit.h"
#include "GLApp/GLWindowManager.h"
#include "GLApp/GLMessageBox.h"
#include "MolFlow.h"
#include "File.h"
//#include "GLApp/GLFileBox.h"
#include "NativeFileDialog/molflow_wrapper/nfd_wrapper.h"
#include "GLApp/GLButton.h"
#include "GLApp/GLTextField.h"
#include "GLApp/GLLabel.h"
#include "GLApp/GLToggle.h"
#include "GLApp/GLTitledPanel.h"
#include "GLApp/GLCombo.h"

#include "Geometry_shared.h"

#define MODE_NOCONV 0
#define MODE_EQUATION 1
#define MODE_FILE 2

extern MolFlow *mApp;

ImportDesorption::ImportDesorption():GLWindow() {

	int wD = 400;
	int hD = 400;

	SetTitle("Import SYN textures as desorption");

	filePanel = new GLTitledPanel("Source file with textures");
	filePanel->SetBounds(5,5,wD-10,120);
	Add(filePanel);

	GLLabel *l1=new GLLabel("SYN file:");
	l1->SetBounds(15,25,40,21);
	Add(l1);

	synFileName=new GLTextField(0,"");
	synFileName->SetBounds(60,24,245,19);
	Add(synFileName);

	loadSynButton=new GLButton(0,"Load...");
	loadSynButton->SetBounds(wD-90,24,80,19);
	Add(loadSynButton);

    analysisResultLabel=new GLLabel("Load a SYN or SYN7Z file to analyze its textures.");
	analysisResultLabel->SetBounds(60,50,200,70);
	Add(analysisResultLabel);

	reloadButton = new GLButton(0, "Reanalyze");
	reloadButton->SetBounds(wD - 90, 49, 80, 19);
	Add(reloadButton);

	useCurrentButton=new GLButton(0,"Use current file");
	useCurrentButton->SetBounds(wD-90,74,80,19);
	Add(useCurrentButton);

	importPanel=new GLTitledPanel("Convert and Import");
	importPanel->SetBounds(5,130,wD-10,220);
	Add(importPanel);

	GLLabel *l2=new GLLabel("Dose=");
	l2->SetBounds(15,150,40,21);
	Add(l2);

	sourceSelectCombo=new GLCombo(0);
	sourceSelectCombo->SetSize(3);
	sourceSelectCombo->SetValueAt(0,"MC hits");
	sourceSelectCombo->SetValueAt(1,"Flux");
	sourceSelectCombo->SetValueAt(2,"Power");
	sourceSelectCombo->SetSelectedIndex(1);
	sourceSelectCombo->SetBounds(60,150,60,19);
	Add(sourceSelectCombo);
	
	GLLabel *l3=new GLLabel("*");
	l3->SetBounds(125,152,10,21);
	Add(l3);

	timeField=new GLTextField(0,"1");
	timeField->SetBounds(135,150,50,19);
	Add(timeField);

	GLLabel *l4=new GLLabel("seconds");
	l4->SetBounds(190,152,10,21);
	Add(l4);

	r1=new GLToggle(0,"Outgassing [mbar.l/s] equals original texture values");
	r1->SetBounds(15,200,150,21);
	Add(r1);

	r2=new GLToggle(0,"Yield=");
	r2->SetBounds(15,225,50,21);
	r2->SetState(true);mode=MODE_EQUATION; //Set default mode
	Add(r2);

	eta0Text=new GLTextField(0,"1E-2");
	eta0Text->SetBounds(70,224,50,19);
	Add(eta0Text);

	GLLabel *l5=new GLLabel("molecules/photon until");
	l5->SetBounds(120,225,100,21);
	Add(l5);

	cutoffText = new GLTextField(0, "1E20");
	cutoffText->SetBounds(235, 225, 50, 19);
	Add(cutoffText);

	GLLabel *l6 = new GLLabel("photons/cm\262 dose...");
	l6->SetBounds(285, 225, 50, 21);
	Add(l6);

	GLLabel *l7 = new GLLabel("...then power function with");
	l7->SetBounds(70, 250, 100, 21);
	Add(l7);

	alphaText=new GLTextField(0,"-0.666");
	alphaText->SetBounds(200,250,60,19);
	Add(alphaText);

	GLLabel *l8 = new GLLabel("/decade slope");
	l8->SetBounds(260, 250, 100, 21);
	Add(l8);

	GLLabel *l9 = new GLLabel("(if dose>cutoff, eta=eta0*(dose/cutoff)^alpha, otherwise eta=eta0)");
	l9->SetBounds(70, 275, 300, 21);
	Add(l9);

	r3=new GLToggle(0,"Use molecule yield file:");
	r3->SetBounds(15,300,100,21);
	Add(r3);

	convFileName=new GLTextField(0,"");
	convFileName->SetBounds(30,299,220,19);
	convFileName->SetEditable(false);
	Add(convFileName);

	loadConvButton=new GLButton(0,"Load...");
	loadConvButton->SetBounds(260,300,80,19);
	//loadConvButton->SetEnabled(false);
	Add(loadConvButton);

	convInfoButton=new GLButton(0,"info");
	convInfoButton->SetBounds(345,300,40,19);
	Add(convInfoButton);

	convAnalysisLabel=new GLLabel("");
	convAnalysisLabel->SetBounds(30,325,150,21);
	Add(convAnalysisLabel);

	setButton = new GLButton(0,"Import for selected facets");
	setButton->SetBounds(wD-165,hD-45,160,21);
	Add(setButton);

	cancelButton = new GLButton(0,"Dismiss");
	cancelButton->SetBounds(wD-260,hD-45,90,21);
	Add(cancelButton);

	// Center dialog
	int wS,hS;
	GLToolkit::GetScreenSize(&wS,&hS);
	int xD = (wS-wD)/2;
	int yD = (hS-hD)/2;
	SetBounds(xD,yD,wD,hD);

	RestoreDeviceObjects();

	geom = NULL;

}

void ImportDesorption::SetGeometry(Geometry *s,Worker *w) {

	geom = s;
	work = w;

}

void ImportDesorption::ProcessMessage(GLComponent *src,int message) {

	switch(message) {
	case MSG_BUTTON:

		if(src==cancelButton) {

			GLWindow::ProcessMessage(NULL,MSG_CLOSE);

		} else if (src==loadConvButton) {
			/*
			//load file dialog
			FILENAME *convFile=GLFileBox::OpenFile(mApp->currentDir,NULL,"Open conversion file","All files\0*.*\0",2);
			if (!convFile) return;
			if (!convFile->fullName) return;
			*/
			std::string fileName = NFD_OpenFile_Cpp("", "");
			if (fileName.empty()) return;

			//load file
			try{
				LoadConvFile(fileName.c_str());
			} catch (Error &e) {
				char errMsg[512];
				sprintf(errMsg,"%s\nFile:%s",e.GetMsg(), fileName.c_str());
				GLMessageBox::Display(errMsg,"Error loading conversion file",GLDLG_OK,GLDLG_ICONERROR);
				return;
			}
			if (convDistr.size()>0) {
			convFileName->SetText(fileName);
			char tmp[256];
			sprintf(tmp,"%d points loaded.",(int)convDistr.size());
			convAnalysisLabel->SetText(tmp);
			} else {
				GLMessageBox::Display("Couldn't load conversion file","Error",GLDLG_OK,GLDLG_ICONERROR);
				return;
			}
			mode=MODE_FILE;
			EnableDisableComponents();
		} else if (src==loadSynButton || src==useCurrentButton || src==reloadButton) {
			//load file dialog
			/*
			FILENAME synFileTmp;
			FILENAME *synFilePtr=NULL;
			*/
			std::string fileName;
			if (src == loadSynButton) {
				/*synFilePtr = GLFileBox::OpenFile(mApp->currentDir, NULL, "Open source SYN file",
					"SynRad+ files\0*.syn;*.syn7z\0All files\0*.*\0", 2);*/
				fileName = NFD_OpenFile_Cpp("syn,syn7z", "");
			}
			else if (src==useCurrentButton) {
				fileName = work->fullFileName;
			}
			else if (src == reloadButton) {
				fileName = synFileName->GetText();
			}
			if (fileName.length()==0) return;
			//load file
			size_t nbFacet,nbTextured,nbDifferent;
			try{
				work->AnalyzeSYNfile(fileName.c_str(),&nbFacet,
					&nbTextured,&nbDifferent);
			} catch (Error &e) {
				char errMsg[512];
				sprintf(errMsg,"%s\nFile:%s",e.GetMsg(),fileName.c_str());
				GLMessageBox::Display(errMsg,"Error loading source file",GLDLG_OK,GLDLG_ICONERROR);
				return;
			}
			
			synFileName->SetText(fileName);
			char tmp[512];
			sprintf(tmp,"SYN file analysis results:\n"
				"%zd facets\n"
				"%zd with textures that can be imported\n"
				,nbFacet,nbTextured);
			analysisResultLabel->SetText(tmp);
			mApp->UpdateFacetParams(true); //Update selection
		} else if (src==setButton) {

			//validate user input
			std::string synFile=synFileName->GetText();
			std::string convFile=convFileName->GetText();
			double time,eta0,alpha,cutoffdose;

			if( !FileUtils::Exist(synFile.c_str())) {
				GLMessageBox::Display("Referenced SYN file doesn't exist","Error",GLDLG_OK,GLDLG_ICONERROR);
				return;
			}

			if( mode==MODE_FILE&&!FileUtils::Exist(convFile.c_str())) {
				GLMessageBox::Display("Referenced conversion file doesn't exist","Error",GLDLG_OK,GLDLG_ICONERROR);
				return;
			}

			if( !timeField->GetNumber(&time) || !(time>=0.0)) {
				GLMessageBox::Display("Invalid number in TIME field.","Error",GLDLG_OK,GLDLG_ICONERROR);
				return;
			}

			if( mode==MODE_EQUATION && !eta0Text->GetNumber(&eta0) || !(eta0>=0.0)) {
				GLMessageBox::Display("Invalid number in ETA0 (conversion coefficient) field.","Error",GLDLG_OK,GLDLG_ICONERROR);
				return;
			}
			if (mode == MODE_EQUATION && !alphaText->GetNumber(&alpha)) {
				GLMessageBox::Display("Invalid number in ALPHA (conversion exponent) field.","Error",GLDLG_OK,GLDLG_ICONERROR);
				return;
			}

			if (mode == MODE_EQUATION && !cutoffText->GetNumber(&cutoffdose)) {
				GLMessageBox::Display("Invalid cutoff dose number.", "Error", GLDLG_OK, GLDLG_ICONERROR);
				return;
			}

			if ((mode==MODE_FILE)&&(!(convDistr.size()>0))) {
				GLMessageBox::Display("No valid conversion file loaded","Error",GLDLG_OK,GLDLG_ICONERROR);
				return;
			}
			doseSource=sourceSelectCombo->GetSelectedIndex();
			//Everything fine, let's go!
			if (!mApp->AskToReset(work)) return;
			GLProgress *progressDlg = new GLProgress("Importing...","Please wait");
			progressDlg->SetProgress(0.0);
			progressDlg->SetVisible(true);
			
			try{
				work->ImportDesorption_SYN(synFile.c_str(),doseSource,time,mode,eta0,alpha,cutoffdose,convDistr,progressDlg);
			} catch (Error &e) {
				char errMsg[512];
				sprintf(errMsg,"%s\nFile:%s",e.GetMsg(),synFile.c_str());
				GLMessageBox::Display(errMsg,"Error",GLDLG_OK,GLDLG_ICONERROR);
			}
			work->CalcTotalOutgassing();
			mApp->UpdateFacetParams();
			// Send to sub process
			try { work->Reload(); } catch(Error &e) {
				GLMessageBox::Display(e.GetMsg(),"Error reloading worker",GLDLG_OK,GLDLG_ICONERROR);
			}

			progressDlg->SetVisible(false);
			SAFE_DELETE(progressDlg);
			GLWindow::ProcessMessage(NULL,MSG_CLOSE);
		} else if (src==convInfoButton) {
			GLMessageBox::Display("A conversion file should contain several lines for interpolation\n"
				"Each line should contain two numbers separated by a space or tab:\n"
				"First number: Photon Dose (in photons/cm\262)\n"
				"Second number: Molecules/Photon (at that dose value).\n"
				"Good luck!","Conversion file info",GLDLG_OK,GLDLG_ICONINFO);
		}
		break;
	case MSG_TOGGLE:
		if (src==r1) {
			mode=MODE_NOCONV;
		} else if (src==r2) {
			mode=MODE_EQUATION;
		} else if (src==r3) {
			mode=MODE_FILE;
		}
		EnableDisableComponents();
		break;
	}

	GLWindow::ProcessMessage(src,message);
}

void ImportDesorption::LoadConvFile(const char* fileName) {
	FileReader *f = NULL;
	try {
		f=new FileReader(fileName);
		convDistr=std::vector<std::pair<double,double>>();
		while (!f->IsEof()) {
			double x=f->ReadDouble();
			double y=f->ReadDouble();
			convDistr.push_back(std::make_pair(x,y));
		}
		if (!(convDistr.size()>0)) throw Error("Couldn't load any points");
		//conversionDistr=new Distribution2D(nbPoints);
		
		f->SeekStart(); //restart from the beginning
		SAFE_DELETE(f);
	}  catch (Error &e) {
		SAFE_DELETE(f);
		char errMsg[512];
		sprintf(errMsg,"%s\nFile:%s",e.GetMsg(),fileName);
		GLMessageBox::Display(errMsg,"Error",GLDLG_OK,GLDLG_ICONERROR);
	}
}

void ImportDesorption::EnableDisableComponents(){
	r1->SetState(mode==MODE_NOCONV);
		r2->SetState(mode==MODE_EQUATION);
		eta0Text->SetEditable(mode==MODE_EQUATION);
		alphaText->SetEditable(mode==MODE_EQUATION);
		cutoffText->SetEditable(mode == MODE_EQUATION);
		r3->SetState(mode==MODE_FILE);
		convFileName->SetEditable(mode==MODE_FILE);
		loadConvButton->SetEnabled(mode==MODE_FILE);
};
