/*
File:        Worker.cpp
Description: Sub processes handling
Program:     MolFlow
Author:      R. KERSEVAN / J-L PONS / M ADY
Copyright:   E.S.R.F / CERN

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/

#define NOMINMAX
#include <Windows.h>
#include "MolflowGeometry.h"
#include "Worker.h"
#include "GLApp/GLApp.h"
#include "GLApp/GLMessageBox.h"
#include <math.h>
#include <stdlib.h>
#include <Process.h>
#include "GLApp/GLUnitDialog.h"
#include "GLApp/MathTools.h"
#include "Facet_shared.h"
//#include "Simulation.h" //SHELEM
#include "GlobalSettings.h"
#include "FacetAdvParams.h"
#include <fstream>

#ifdef MOLFLOW
#include "MolFlow.h"
#endif

#ifdef SYNRAD
#include "SynRad.h"
#endif

#include <direct.h>
#include "ZipUtils/zip.h"
#include "ZipUtils/unzip.h"
#include "File.h" //File utils (Get extension, etc)

/*
//Leak detection
#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
#define new DEBUG_NEW
#endif
*/
using namespace pugi;

#ifdef MOLFLOW
extern MolFlow *mApp;
#endif

#ifdef SYNRAD
extern SynRad*mApp;
#endif

Worker::Worker() {
	
	//Molflow specific
	temperatures = std::vector<double>();
	moments = std::vector<double>();
	desorptionParameterIDs = std::vector<size_t>();
	userMoments = std::vector<std::string>(); //strings describing moments, to be parsed
	CDFs = std::vector<std::vector<std::pair<double, double>>>();
	IDs = std::vector<std::vector<std::pair<double, double>>>();
	parameters = std::vector<Parameter>();
	needsReload = true;  //When main and subprocess have different geometries, needs to reload (synchronize)
	displayedMoment = 0; //By default, steady-state is displayed
	timeWindowSize = 0.1;
	useMaxwellDistribution = true;
	calcConstantFlow = true;
	distTraveledTotal_total = 0.0;
	distTraveledTotal_fullHitsOnly = 0.0;
	gasMass = 28.0;
	enableDecay = false;
	halfLife = 1;
	finalOutgassingRate = finalOutgassingRate_Pa_m3_sec = totalDesorbedMolecules = 0.0;
	motionType = 0;
	globalHistogramParams.record = false;

	//Common init
	pid = _getpid();
	sprintf(ctrlDpName, "MFLWCTRL%d", pid);
	sprintf(loadDpName, "MFLWLOAD%d", pid);
	sprintf(hitsDpName, "MFLWHITS%d", pid);

	nbProcess = 0;
	desorptionLimit = 0;

	ResetWorkerStats();
	geom = new MolflowGeometry();

	dpControl = NULL;
	dpHit = NULL;
	hitCacheSize = 0;
	nbHit = 0;
	nbLeakTotal = 0;
	leakCacheSize = 0;
	startTime = 0.0f;
	stopTime = 0.0f;
	simuTime = 0.0f;

	isRunning = false;
	calcAC = false;
	strcpy(fullFileName, "");
}

MolflowGeometry* Worker::GetMolflowGeometry() {
	return geom;
}

void Worker::SaveGeometry(char *fileName, GLProgress *prg, bool askConfirm, bool saveSelected, bool autoSave, bool crashSave) {

	try {
		if (needsReload && (!crashSave && !saveSelected)) RealReload();
	}
	catch (Error &e) {
		char errMsg[512];
		sprintf(errMsg, "Error reloading worker. Trying safe save (geometry only):\n%s", e.GetMsg());
		GLMessageBox::Display(errMsg, "Error", GLDLG_OK, GLDLG_ICONERROR);
		crashSave = true;
	}
	char tmp[10000]; //compress.exe command line
	char fileNameWithGeo[2048]; //file name with .geo extension (instead of .geo7z)
	char fileNameWithGeo7z[2048];
	char fileNameWithXML[2048];
	char fileNameWithZIP[2048];
	char fileNameWithoutExtension[2048]; //file name without extension
	//char *ext = fileName+strlen(fileName)-4;
	char *ext, *dir;

	dir = strrchr(fileName, '\\');
	ext = strrchr(fileName, '.');

	if (!(ext) || !(*ext == '.') || ((dir) && (dir > ext))) {
		sprintf(fileName, mApp->compressSavedFiles ? "%s.zip" : "%s.xml", fileName); //set to default XML/ZIP format
		ext = strrchr(fileName, '.');
	}

	ext++;

	// Read a file
	bool ok = true;
	FileWriter *f = NULL;
	bool isTXT = _stricmp(ext, "txt") == 0;
	bool isSTR = _stricmp(ext, "str") == 0;
	bool isGEO = _stricmp(ext, "geo") == 0;
	bool isGEO7Z = _stricmp(ext, "geo7z") == 0;
	bool isXML = _stricmp(ext, "xml") == 0;
	bool isXMLzip = _stricmp(ext, "zip") == 0;

	if (isTXT || isGEO || isGEO7Z || isSTR || isXML || isXMLzip) {

		if ((isGEO7Z) && WAIT_TIMEOUT == WaitForSingleObject(mApp->compressProcessHandle, 0)) {
			GLMessageBox::Display("Compressing a previous save file is in progress. Wait until that finishes "
				"or close process \"compress.exe\"\nIf this was an autosave attempt,"
				"you have to lower the autosave frequency.", "Can't save right now.", GLDLG_OK, GLDLG_ICONERROR);
			return;
		}
		if (isGEO) {
			memcpy(fileNameWithoutExtension, fileName, sizeof(char)*(strlen(fileName) - 4));
			fileNameWithoutExtension[strlen(fileName) - 4] = '\0';
			sprintf(fileNameWithGeo7z, "%s7z", fileName);
			memcpy(fileNameWithGeo, fileName, (strlen(fileName) + 1)*sizeof(char));
		}
		else if (isGEO7Z) {
			memcpy(fileNameWithoutExtension, fileName, sizeof(char)*(strlen(fileName) - 6));
			fileNameWithoutExtension[strlen(fileName) - 6] = '\0';
			memcpy(fileNameWithGeo, fileName, sizeof(char)*(strlen(fileName) - 2));
			fileNameWithGeo[strlen(fileName) - 2] = '\0';
			memcpy(fileNameWithGeo7z, fileName, (1 + strlen(fileName))*sizeof(char));
			sprintf(tmp, "A .geo file of the same name exists. Overwrite that file ?\n%s", fileNameWithGeo);
			if (!autoSave && FileUtils::Exist(fileNameWithGeo)) {
				ok = (GLMessageBox::Display(tmp, "Question", GLDLG_OK | GLDLG_CANCEL, GLDLG_ICONWARNING) == GLDLG_OK);
			}
		}

		if (isXML || isXMLzip) {

			memcpy(fileNameWithoutExtension, fileName, sizeof(char)*(strlen(fileName) - 4));
			fileNameWithoutExtension[strlen(fileName) - 4] = '\0';
			sprintf(fileNameWithXML, "%s.xml", fileNameWithoutExtension);
			sprintf(fileNameWithZIP, "%s.zip", fileNameWithoutExtension);
		}
		if (isXMLzip) {
			sprintf(tmp, "An .xml file of the same name exists. Overwrite that file ?\n%s", fileNameWithZIP);
			if (!autoSave && FileUtils::Exist(fileNameWithXML)) {

				ok = (GLMessageBox::Display(tmp, "Question", GLDLG_OK | GLDLG_CANCEL, GLDLG_ICONWARNING) == GLDLG_OK);
			}
		}

		if (!autoSave && ok && FileUtils::Exist(fileName)) {
			sprintf(tmp, "Overwrite existing file ?\n%s", fileName);
			if (askConfirm) ok = (GLMessageBox::Display(tmp, "Question", GLDLG_OK | GLDLG_CANCEL, GLDLG_ICONWARNING) == GLDLG_OK);
		}

		if (ok) {
			if (isSTR) {
				geom->SaveSTR(dpHit, saveSelected);
			}

			else {
				try {
					if (isGEO7Z) {
						f = new FileWriter(fileNameWithGeo); //We first write a GEO file, then compress it to GEO7Z later
					}
					else if (!(isXML || isXMLzip))

						f = new FileWriter(fileName);
				}

				catch (Error &e) {
					SAFE_DELETE(f);
					GLMessageBox::Display((char*)e.GetMsg(), "Error writing file.", GLDLG_OK, GLDLG_ICONERROR);
					return;
				}
				geom->loaded_desorptionLimit = desorptionLimit;
				if (isTXT) geom->SaveTXT(f, dpHit, saveSelected);
				else if (isGEO || isGEO7Z) {
					/*
					// Retrieve leak cache
					int nbLeakSave, nbHHitSave;
					LEAK leakCache[LEAKCACHESIZE];
					if (!crashSave && !saveSelected) GetLeak(leakCache, &nbLeakSave);

					// Retrieve hit cache (lines and dots)
					HIT hitCache[HITCACHESIZE];
					if (!crashSave && !saveSelected) GetHHit(hitCache, &nbHHitSave);
					*/
					geom->SaveGEO(f, prg, dpHit, this->userMoments, this, saveSelected, leakCache, &leakCacheSize, hitCache, &hitCacheSize, crashSave);
				}
				else if (isXML || isXMLzip) {
					xml_document saveDoc;
					geom->SaveXML_geometry(saveDoc, this, prg, saveSelected);
					xml_document geom_only; geom_only.reset(saveDoc);
					bool success = false; //success: simulation state could be saved
					if (!crashSave && !saveSelected) {
						try {
							AccessDataport(dpHit);
							BYTE *buffer;
							buffer = (BYTE *)dpHit->buff;
							GlobalHitBuffer *gHits;
							gHits = (GlobalHitBuffer *)buffer;
							/*
							int nbLeakSave, nbHHitSave;
							LEAK leakCache[LEAKCACHESIZE];
							GetLeak(leakCache, &nbLeakSave);
							HIT hitCache[HITCACHESIZE];
							GetHHit(hitCache, &nbHHitSave);
							*/

							success = geom->SaveXML_simustate(saveDoc, this, buffer, gHits, leakCacheSize, hitCacheSize, leakCache, hitCache, prg, saveSelected);
							ReleaseDataport(dpHit);
						}
						catch (Error &e) {
							SAFE_DELETE(f);
							ReleaseDataport(dpHit);
							GLMessageBox::Display((char*)e.GetMsg(), "Error saving simulation state.", GLDLG_OK, GLDLG_ICONERROR);
							return;
						}
					}

					prg->SetMessage("Writing xml file...");
					if (success) {
						if (!saveDoc.save_file(fileNameWithXML)) throw Error("Error writing XML file."); //successful save
					}
					else {
						if (!geom_only.save_file(fileNameWithXML)) throw Error("Error writing XML file."); //simu state error
					}

					if (isXMLzip) {
						prg->SetProgress(0.75);
						prg->SetMessage("Compressing xml to zip...");
						//mApp->compressProcessHandle=CreateThread(0, 0, ZipThreadProc, 0, 0, 0);
						HZIP hz = CreateZip(fileNameWithZIP, 0);
						if (!hz) {
							throw Error("Error creating ZIP file");
						}
						if (!ZipAdd(hz, GetShortFileName(fileNameWithXML), fileNameWithXML)) remove(fileNameWithXML);
						else {
							CloseZip(hz);
							throw Error("Error compressing ZIP file.");
						}
						CloseZip(hz);
					}
				}
			}
			/*if (!autoSave && !saveSelected) {
				strcpy(fullFileName, fileName);
				remove("Molflow_AutoSave.zip");
				}*/
		}
	}
	else {
		SAFE_DELETE(f);
		throw Error("SaveGeometry(): Invalid file extension [only xml,zip,geo,geo7z,txt or str]");
	}

	SAFE_DELETE(f);

	//File written, compress it if the user wanted to
	if (ok && isGEO7Z) {

		if (FileUtils::Exist("compress.exe")) { //compress GEO file to GEO7Z using 7-zip launcher "compress.exe"
			sprintf(tmp, "compress.exe \"%s\" Geometry.geo", fileNameWithGeo);
			int procId = StartProc(tmp,STARTPROC_BACKGROUND);

			mApp->compressProcessHandle = OpenProcess(PROCESS_ALL_ACCESS, true, procId);
			fileName = fileNameWithGeo7z;
		}
		else {
			GLMessageBox::Display("compress.exe (part of Molfow) not found.\n Will save as uncompressed GEO file.", "Compressor not found", GLDLG_OK, GLDLG_ICONERROR);
			fileName = fileNameWithGeo;
		}
	}
	else if (ok && isGEO) fileName = fileNameWithGeo;
	if (!autoSave && !saveSelected) {
		SetFileName(fileName);
		mApp->UpdateTitle();
	}
}

void Worker::ExportProfiles(char *fileName) {

	char tmp[512];

	// Read a file
	FILE *f = NULL;

	char *ext, *dir;

	dir = strrchr(fileName, '\\');
	ext = strrchr(fileName, '.');

	if (!(ext) || !(*ext == '.') || ((dir) && (dir > ext))) {
		sprintf(fileName, "%s.csv", fileName); //set to default CSV format
		ext = strrchr(fileName, '.');
	}
	ext++;
	bool isTXT = _stricmp(ext, "txt") == 0;

	bool ok = true;

	if (FileUtils::Exist(fileName)) {
		sprintf(tmp, "Overwrite existing file ?\n%s", fileName);
		ok = (GLMessageBox::Display(tmp, "Question", GLDLG_OK | GLDLG_CANCEL, GLDLG_ICONWARNING) == GLDLG_OK);
	}

	if (ok) {
		f = fopen(fileName, "w");
		if (!f) {
			char tmp[256];
			sprintf(tmp, "Cannot open file for writing %s", fileName);
			throw Error(tmp);
		}
		geom->ExportProfiles(f, isTXT, dpHit, this);
		fclose(f);
	}
}

void Worker::ExportAngleMaps(std::vector<size_t> facetList, std::string fileName) {
	bool overwriteAll = false;
	for (size_t facetIndex : facetList) {
		std::string saveFileName;
			if (facetList.size() == 1) {
				saveFileName = FileUtils::StripExtension(fileName) + ".csv";
			}
			else {
				std::stringstream tmp;
				tmp << FileUtils::StripExtension(fileName) << "_facet" << facetIndex + 1 << ".csv";
				saveFileName = tmp.str();
			}

		bool ok = true;

		if (FileUtils::Exist(saveFileName)) {
			if (!overwriteAll) {
				std::vector<std::string> buttons = { "Cancel", "Overwrite" };
				if (facetList.size() > 1) buttons.push_back("Overwrite All");
				int answer = GLMessageBox::Display("Overwrite existing file ?\n" + saveFileName, "Question", buttons, GLDLG_ICONWARNING);
				if (answer == 0) break; //User cancel
				overwriteAll = (answer == 2);
			}
		}

		std::ofstream file;
		file.open(saveFileName);
			if (!file.is_open()) {
				std::string tmp = "Cannot open file for writing " + saveFileName;
				throw Error(tmp.c_str());
			}
			file << geom->GetFacet(facetIndex) -> GetAngleMap(1);
			file.close();
	}
}

/*void Worker::ImportDesorption(char *fileName) {
	//if (needsReload) RealReload();

	// Read a file
	FileReader *f=new FileReader(fileName);
	geom->ImportDesorption(f,dpHit);
	SAFE_DELETE(f);
	changedSinceSave=true;
	Reload();
	}*/

void Worker::LoadGeometry(char *fileName,bool insert,bool newStr) {
	if (!insert) {
		needsReload = true;
	}
	else {
		RealReload();
	}
	char CWD[MAX_PATH];
	_getcwd(CWD, MAX_PATH);

	std::string ext=FileUtils::GetExtension(fileName);

	if (ext == "")

		throw Error("LoadGeometry(): No file extension, can't determine type");

	// Read a file
	FileReader *f = NULL;
	GLProgress *progressDlg = new GLProgress("Reading file...", "Please wait");
	progressDlg->SetVisible(true);
	progressDlg->SetProgress(0.0);

	if (!insert) {
		//Clear hits and leaks cache
		memset(hitCache, 0, sizeof(HIT)*HITCACHESIZE);
		memset(leakCache, 0, sizeof(LEAK)*LEAKCACHESIZE);
		ResetMoments();
		//default values
		enableDecay = false;
		gasMass = 28;
	}

	/*
	bool isASE = (_stricmp(ext, "ase") == 0);
	bool isSTR = (_stricmp(ext, "str") == 0);
	bool isSTL = (_stricmp(ext, "stl") == 0);
	bool isTXT = (_stricmp(ext, "txt") == 0);
	bool isGEO = (_stricmp(ext, "geo") == 0);
	bool isGEO7Z = (_stricmp(ext, "geo7z") == 0);
	bool isSYN = (_stricmp(ext, "syn") == 0);
	bool isSYN7Z = (_stricmp(ext, "syn7z") == 0);
	bool isXML = (_stricmp(ext, "xml") == 0);
	bool isXMLzip = (_stricmp(ext, "zip") == 0);
	*/

	if (ext == "txt" || ext == "TXT") {

		try {
			if (!insert) ResetWorkerStats();
			else mApp->changedSinceSave = true;

			f = new FileReader(fileName);
			
			if (!insert) {
				geom->LoadTXT(f, progressDlg);
				SAFE_DELETE(f);
				nbHit = geom->loaded_nbHit;
				nbDesorption = geom->loaded_nbDesorption;
				nbAbsorption = geom->loaded_nbAbsorption;
				desorptionLimit = geom->loaded_desorptionLimit;
				nbLeakTotal = geom->loaded_nbLeak;
				//RealReload();
				strcpy(fullFileName, fileName);
			}
			else { //insert

				geom->InsertTXT(f, progressDlg, newStr);
				SAFE_DELETE(f);
				nbHit = 0;
				nbDesorption = 0;
				desorptionLimit = 0;
				nbLeakTotal = 0;
				Reload();
			}
		}

		catch (Error &e) {
			if (!insert) geom->Clear();
			SAFE_DELETE(f);
			progressDlg->SetVisible(false);
			SAFE_DELETE(progressDlg);
			throw e;
		}

	}
	else if (ext == "stl" || ext == "STL") {
		try {
			int ret = GLUnitDialog::Display("", "Choose STL file units:", GLDLG_MM | GLDLG_CM | GLDLG_M | GLDLG_INCH | GLDLG_FOOT | GLDLG_CANCEL_U, GLDLG_ICONNONE);
			double scaleFactor = 1.0;
			switch (ret) {
			case GLDLG_MM:
				scaleFactor = 0.1;
				break;
			case GLDLG_CM:
				scaleFactor = 1.0;
				break;
			case GLDLG_M:
				scaleFactor = 100;
				break;
			case GLDLG_INCH:
				scaleFactor = 2.54;
				break;
			case GLDLG_FOOT:
				scaleFactor = 30.48;
				break;
			}
			if (ret != GLDLG_CANCEL_U) {
				progressDlg->SetMessage("Resetting worker...");
				progressDlg->SetVisible(true);
				ResetWorkerStats();				
				progressDlg->SetMessage("Reading geometry...");
				f = new FileReader(fileName);
				if (!insert) {
					geom->LoadSTL(f, progressDlg, scaleFactor);
					SAFE_DELETE(f);
					strcpy(fullFileName, fileName);
					mApp->DisplayCollapseDialog();
				}
				else { //insert
					mApp->changedSinceSave = true;
					geom->InsertSTL(f, progressDlg, scaleFactor, newStr);
					SAFE_DELETE(f);
					Reload();
				}
			}
		}
		catch (Error &e) {
			if (!insert) geom->Clear();
			SAFE_DELETE(f);
			progressDlg->SetVisible(false);
			SAFE_DELETE(progressDlg);
			throw e;

		}

	}
	else if (ext == "str" || ext == "STR") {
		if (insert) throw Error("STR file inserting is not supported.");
		try {
			ResetWorkerStats();
			f = new FileReader(fileName);
			progressDlg->SetVisible(true);
			geom->LoadSTR(f, progressDlg);
			SAFE_DELETE(f);
			//RealReload();

			strcpy(fullFileName, fileName);
		}

		catch (Error &e) {
			geom->Clear();
			SAFE_DELETE(f);
			progressDlg->SetVisible(false);
			SAFE_DELETE(progressDlg);
			throw e;
		}

	}
	else if (ext=="syn" || ext=="syn7z") { //Synrad file
		int version;
		progressDlg->SetVisible(true);
		try {
			if (ext=="syn7z") {
				//decompress file
				progressDlg->SetMessage("Decompressing file...");
				char tmp[1024];
				//char fileOnly[512];
				sprintf(tmp, "cmd /C \"pushd \"%s\"&&7za.exe x -t7z -aoa \"%s\" -otmp&&popd\"", CWD, fileName);
				system(tmp);
				f = new FileReader((std::string)CWD + "\\tmp\\Geometry.syn"); //Open extracted file
			} else f = new FileReader(fileName); //syn file, open it directly
			
			if (!insert) {
				progressDlg->SetMessage("Resetting worker...");
				ResetWorkerStats();
			
				geom->LoadSYN(f, progressDlg, &version);
				SAFE_DELETE(f);
				desorptionLimit = 0;
			}
			else { //insert
				geom->InsertSYN(f, progressDlg, newStr);
				SAFE_DELETE(f);
				nbLeakTotal = 0;
				nbHit = 0;
				nbDesorption = 0;
				desorptionLimit = geom->loaded_desorptionLimit;
			}

			progressDlg->SetMessage("Reloading worker with new geometry...");
			Reload();
			if (!insert) strcpy(fullFileName, fileName);
		}

		catch (Error &e) {
			if (!insert) geom->Clear();

			SAFE_DELETE(f);
			//if (isSYN7Z) remove(tmp2);

			progressDlg->SetVisible(false);
			SAFE_DELETE(progressDlg);
			throw e;
		}

	}
	else if (ext=="geo" || ext=="geo7z") {
		//char tmp2[1024];
		std::string toOpen;
		int version;
		progressDlg->SetVisible(true);
		try {
			if (ext == "geo7z") {
				//decompress file
				progressDlg->SetMessage("Decompressing file...");
				char tmp[1024];
				//char fileOnly[512];
				sprintf(tmp, "cmd /C \"pushd \"%s\"&&7za.exe x -t7z -aoa \"%s\" -otmp&&popd\"", CWD, fileName);

				system(tmp);

				toOpen = (std::string)CWD + "\\tmp\\Geometry.geo"; //newer geo7z format: contain Geometry.geo
				if (!FileUtils::Exist(toOpen)) toOpen = ((std::string)fileName).substr(0, strlen(fileName) - 2); //Inside the zip, try original filename with extension changed from geo7z to geo
				f = new FileReader(toOpen);
			}
			else { //not geo7z
				toOpen = fileName;
				f = new FileReader(fileName); //geo file, open it directly
			}
			progressDlg->SetMessage("Resetting worker...");
			ResetWorkerStats();
			if (insert) mApp->changedSinceSave = true;

			LEAK loaded_leakCache[LEAKCACHESIZE];
			size_t loaded_nbLeak;
			HIT hitCache[HITCACHESIZE];
			if (!insert) {

				geom->LoadGEO(f, progressDlg, loaded_leakCache, &loaded_nbLeak, hitCache, &hitCacheSize, &version, this);
				
				//copy temp values from geom to worker:
				nbLeakTotal = geom->loaded_nbLeak;
				nbHit = geom->loaded_nbHit;
				nbDesorption = geom->loaded_nbDesorption;
				desorptionLimit = geom->loaded_desorptionLimit;
				nbAbsorption = geom->loaded_nbAbsorption;
				distTraveledTotal_total = geom->distTraveledTotal_total;
				distTraveledTotal_fullHitsOnly = geom->distTraveledTotal_fullHitsOnly;
				progressDlg->SetMessage("Reloading worker with new geometry...");
				RealReload(); //for the loading of textures
				if (version >= 8) geom->LoadProfile(f, dpHit, version);
				SetLeakCache(loaded_leakCache, &loaded_nbLeak, dpHit);
				SetHitCache(hitCache, &hitCacheSize, dpHit);
				SendHits(); //Global and facet hit counters
				
				progressDlg->SetMessage("Loading textures...");
				LoadTexturesGEO(f, version);
				strcpy(fullFileName, fileName);
			}
			else { //insert
				mApp->changedSinceSave = true;
				geom->InsertGEO(f, progressDlg, newStr);
				Reload();
			}
			SAFE_DELETE(f);
		}

		catch (Error &e) {
			if (!insert) geom->Clear();
			SAFE_DELETE(f);
			//if (isGEO7Z) remove(tmp2);
			progressDlg->SetVisible(false);
			SAFE_DELETE(progressDlg);
			throw e;
		}

	}
	else if (ext == "xml" || ext=="zip" ) { //XML file, optionally in ZIP container
		xml_document loadXML;
		xml_parse_result parseResult;
		progressDlg->SetVisible(true);
		try {
			if (ext=="zip") { //compressed in ZIP container
				//decompress file
				progressDlg->SetMessage("Decompressing file...");

				HZIP hz = OpenZip(fileName, 0);
				if (!hz) {
					throw Error("Can't open ZIP file");
				}
				ZIPENTRY ze; GetZipItem(hz, -1, &ze); int numitems = ze.index;
				bool notFoundYet = true;
				for (int i = 0; i < numitems && notFoundYet; i++) { //extract first xml file found in ZIP archive
					GetZipItem(hz, i, &ze);
					std::string zipFileName = ze.name;

					if (FileUtils::GetExtension(zipFileName) == "xml") { //if it's an .xml file
						notFoundYet = false;
						std::string tmpFileName = "tmp/" + zipFileName;
						UnzipItem(hz, i, tmpFileName.c_str()); //unzip it to tmp directory
						CloseZip(hz);
						progressDlg->SetMessage("Reading and parsing XML file...");
						parseResult = loadXML.load_file(tmpFileName.c_str()); //load and parse it
					}
				}
				if (notFoundYet) {
					CloseZip(hz);
					throw Error("Didn't find any XML file in the ZIP file.");
				}

			} else parseResult = loadXML.load_file(fileName); //parse xml file directly

			ResetWorkerStats();
			if (!parseResult) {
				//Parse error
				std::stringstream err;
				err << "XML parsed with errors.\n";
				err << "Error description: " << parseResult.description() << "\n";
				err << "Error offset: " << parseResult.offset << "\n";
				throw Error(err.str().c_str());
			}

			progressDlg->SetMessage("Building geometry...");
			if (!insert) {
				geom->LoadXML_geom(loadXML, this, progressDlg);
				geom->UpdateName(fileName);

				progressDlg->SetMessage("Reloading worker with new geometry...");
				try {
					RealReload(); //To create the dpHit dataport for the loading of textures, profiles, etc...
					strcpy(fullFileName, fileName);

					if (ext == "xml" || ext == "zip")
						progressDlg->SetMessage("Restoring simulation state...");
					geom->LoadXML_simustate(loadXML, dpHit, this, progressDlg);
					SendHits(true); //Send hits without sending facet counters, as they are directly written during the load process
					RebuildTextures();
				}
				catch (Error &e) {
					GLMessageBox::Display(e.GetMsg(), "Error while loading simulation state", GLDLG_CANCEL, GLDLG_ICONWARNING);
				}
			}
			else { //insert
				geom->InsertXML(loadXML, this, progressDlg, newStr);
				mApp->changedSinceSave = true;
				ResetWorkerStats();

				Reload();
			}
		}
		catch (Error &e) {
			if (!insert) geom->Clear();
			progressDlg->SetVisible(false);
			SAFE_DELETE(progressDlg);
			throw e;
		}

	}
	else if (ext=="ase" || ext=="ASE") {
		if (insert) throw Error("ASE file inserting is not supported.");
		try {
			ResetWorkerStats();
			f = new FileReader(fileName);
			progressDlg->SetVisible(true);
			geom->LoadASE(f, progressDlg);
			SAFE_DELETE(f);
			//RealReload();
			strcpy(fullFileName, fileName);

		}
		catch (Error &e) {
			geom->Clear();
			SAFE_DELETE(f);
			progressDlg->SetVisible(false);
			SAFE_DELETE(progressDlg);
			throw e;
		}
	}
	else {
		progressDlg->SetVisible(false);
		SAFE_DELETE(progressDlg);
		throw Error("LoadGeometry(): Invalid file extension [Only xml,zip,geo,geo7z,syn.syn7z,txt,ase,stl or str]");
	}
	if (!insert)
	{
		CalcTotalOutgassing();
		/*
		
		//Refresh is already done by the caller Molflow::LoadFile()

		if (mApp->timeSettings) mApp->timeSettings->RefreshMoments(); //Sets displayed moment to 0
		if (mApp->momentsEditor) mApp->momentsEditor->Refresh();
		if (mApp->parameterEditor) mApp->parameterEditor->UpdateCombo();
		if (mApp->timewisePlotter) mApp->timewisePlotter->Refresh();
		*/
	}

	progressDlg->SetVisible(false);
	SAFE_DELETE(progressDlg);
	if (insert) {
		mApp->UpdateFacetlistSelected();
		mApp->UpdateViewers();
	}
}

void Worker::LoadTexturesGEO(FileReader *f, int version) {	
		GLProgress *progressDlg = new GLProgress("Loading textures", "Please wait");
		progressDlg->SetProgress(0.0);
		try {
			progressDlg->SetVisible(true);
			geom->LoadTextures(f, progressDlg, dpHit, version);
			RebuildTextures();
		}
		catch (Error &e) {
			char tmp[256];
			sprintf(tmp, "Couldn't load some textures. To avoid continuing a partially loaded state, it is recommended to reset the simulation.\n%s", e.GetMsg());
			GLMessageBox::Display(tmp, "Error while loading textures.", GLDLG_OK, GLDLG_ICONWARNING);
		}
		progressDlg->SetVisible(false);
		SAFE_DELETE(progressDlg);	
}

void Worker::InnerStop(float appTime) {

	stopTime = appTime;
	simuTime += appTime - startTime;
	isRunning = false;
	calcAC = false;

}

void Worker::OneStep() {

	if (nbProcess == 0)
		throw Error("No sub process found. (Simulation not available)");

	if (!isRunning)  {
		if (!ExecuteAndWait(COMMAND_STEPAC, PROCESS_RUN, AC_MODE))
			ThrowSubProcError();
	}

}

void Worker::StepAC(float appTime) {

	try {
		OneStep();
		Update(appTime);
	}
	catch (Error &e) {
		GLMessageBox::Display((char *)e.GetMsg(), "Error (Stop)", GLDLG_OK, GLDLG_ICONERROR);
	}

}

void Worker::StartStop(float appTime, int sMode) {

	if (isRunning)  {

		// Stop
		InnerStop(appTime);
		try {
			Stop();
			Update(appTime);
		}

		catch (Error &e) {
			GLMessageBox::Display((char *)e.GetMsg(), "Error (Stop)", GLDLG_OK, GLDLG_ICONERROR);
			return;
		}
	}
	else {

		// Start
		try {
			if (needsReload) RealReload(); //Synchronize subprocesses to main process
			startTime = appTime;
			isRunning = true;
			calcAC = false;

			this->sMode = sMode;

			Start();

		}
		catch (Error &e) {
			isRunning = false;
			GLMessageBox::Display((char *)e.GetMsg(), "Error (Start)", GLDLG_OK, GLDLG_ICONERROR);
			return;
		}

			// Particular case when simulation ends before getting RUN state
			if (allDone) {
				Update(appTime);
				GLMessageBox::Display("Max desorption reached", "Information (Start)", GLDLG_OK, GLDLG_ICONINFO);
			}

	}

}

/* //Moved to worker_shared.cpp

void Worker::Update(float appTime) {
	if (needsReload) RealReload();
	//if (!needsReload) {
		// Check calculation ending
		bool done = true;
		bool error = true;
		if (dpControl) {
			if (AccessDataport(dpControl)) {
				int i = 0;
				SHCONTROL *master = (SHCONTROL *)dpControl->buff;
				for (int proc = 0; proc < nbProcess && done; proc++) {
					done = done && (master->states[proc] == PROCESS_DONE);
					error = error && (master->states[proc] == PROCESS_ERROR);
#ifdef MOLFLOW
					if (master->states[proc] == PROCESS_RUNAC) calcACprg = master->cmdParam[proc];
#endif
				}
				ReleaseDataport(dpControl);
			}
		}

		// End of simulation reached (Stop GUI)
		if ((error || done) && isRunning && appTime != 0.0f) {
			InnerStop(appTime);
			if (error) ThrowSubProcError();
		}

		// Retrieve hit count recording from the shared memory
		if (dpHit) {

			if (AccessDataport(dpHit)) {
				BYTE *buffer = (BYTE *)dpHit->buff;

				mApp->changedSinceSave = true;
				// Globals
				SHGHITS *gHits = (SHGHITS *)buffer;

				// Copy Global hits and leaks
				nbHit = gHits->total.hit.nbHit;
				nbAbsorption = gHits->total.hit.nbAbsorbed;
				nbDesorption = gHits->total.hit.nbDesorbed;				
				distTraveledTotal_total = gHits->distTraveledTotal_total;
				distTraveledTotal_fullHitsOnly = gHits->distTraveledTotal_fullHitsOnly;
				

				nbLeakTotal = gHits->nbLeakTotal;
				hitCacheSize = gHits->hitCacheSize;
				memcpy(hitCache, gHits->hitCache, sizeof(HIT)*hitCacheSize);
				leakCacheSize = gHits->leakCacheSize;
				memcpy(leakCache, gHits->leakCache, sizeof(LEAK)*leakCacheSize); //will display only first leakCacheSize leaks

				// Refresh local facet hit cache for the displayed moment
				int nbFacet = geom->GetNbFacet();
				for (int i = 0; i < nbFacet; i++) {
					Facet *f = geom->GetFacet(i);
					f->counterCache=(*((FacetHitBuffer*)(buffer + f->sh.hitOffset+displayedMoment*sizeof(FacetHitBuffer))));
				}
				try {
					if (mApp->needsTexture || mApp->needsDirection) geom->BuildFacetTextures(buffer,mApp->needsTexture,mApp->needsDirection);
				}
				catch (Error &e) {
					GLMessageBox::Display((char *)e.GetMsg(), "Error building texture", GLDLG_OK, GLDLG_ICONERROR);
					ReleaseDataport(dpHit);
					return;
				}
				ReleaseDataport(dpHit);
			}

		}
	//}

}
*/

void Worker::SendHits(bool skipFacetHits ) {
	//if (!needsReload) {
	if (dpHit) {
		if (AccessDataport(dpHit)) {

			GlobalHitBuffer *gHits = (GlobalHitBuffer *)dpHit->buff;

			gHits->total.hit.nbHit = nbHit;
			gHits->nbLeakTotal = nbLeakTotal;
			gHits->total.hit.nbDesorbed = nbDesorption;
			gHits->total.hit.nbAbsorbed = nbAbsorption;
			gHits->distTraveledTotal_total = distTraveledTotal_total;
			gHits->distTraveledTotal_fullHitsOnly = distTraveledTotal_fullHitsOnly;

			if (!skipFacetHits) {

				size_t nbFacet = geom->GetNbFacet();
				for (size_t i = 0; i < nbFacet; i++) {
					Facet *f = geom->GetFacet(i);
					/*for (size_t m = 0;m <= moments.size();m++)*/
					*((FacetHitBuffer*)((BYTE*)dpHit->buff + f->sh.hitOffset /* + m * sizeof(FacetHitBuffer) */)) = f->counterCache;
				}

			}
			ReleaseDataport(dpHit);

		}
		else {
			throw Error("Failed to initialize 'hits' dataport");
		}
	}
}

void Worker::ComputeAC(float appTime) {
	try {
		if (needsReload) RealReload();
	}
	catch (Error &e) {
		GLMessageBox::Display((char *)e.GetMsg(), "Error (Stop)", GLDLG_OK, GLDLG_ICONERROR);
		return;
	}
	if (isRunning)
		throw Error("Already running");

	// Send correction map to sub process
	// (correction map contains area a surface elements)
	size_t maxElem = geom->GetMaxElemNumber();
	if (!maxElem)
		throw Error("Mesh with boundary correction must be enabled on all polygons");
	size_t dpSize = maxElem*sizeof(SHELEM_OLD);

	Dataport *loader = CreateDataport(loadDpName, dpSize);
	if (!loader)
		throw Error("Failed to create 'loader' dataport");
	/*
	AccessDataport(loader);
	geom->CopyElemBuffer((BYTE *)loader->buff);
	ReleaseDataport(loader);
	//CopyElemBuffer needs fix
	*/

	// Load Elem area and send AC matrix calculation order
	// Send command
	if (!ExecuteAndWait(COMMAND_LOADAC, PROCESS_RUNAC, dpSize)) {
		CLOSEDP(loader);
		char errMsg[1024];
		sprintf(errMsg, "Failed to send geometry to sub process:\n%s", GetErrorDetails());
		GLMessageBox::Display(errMsg, "Warning (LoadAC)", GLDLG_OK, GLDLG_ICONWARNING);
		return;
	}

	CLOSEDP(loader);

	isRunning = true;
	calcAC = true;
	startTime = appTime;

}

void Worker::RealReload() { //Sharing geometry with workers
	GLProgress *progressDlg = new GLProgress("Performing preliminary calculations on geometry...", "Passing Geometry to workers");
	progressDlg->SetVisible(true);
	progressDlg->SetProgress(0.0);
	
	//Do preliminary calculations
	try {
		PrepareToRun();
	}
	catch (Error &e) {
		progressDlg->SetVisible(false);
		SAFE_DELETE(progressDlg);
		throw Error(e.GetMsg());
	}

	if (nbProcess == 0) return;
	
	progressDlg->SetMessage("Asking subprocesses to clear geometry...");

	// Clear geometry
	CLOSEDP(dpHit);
	if (!ExecuteAndWait(COMMAND_CLOSE, PROCESS_READY))
	{
		progressDlg->SetVisible(false);
		SAFE_DELETE(progressDlg);
		ThrowSubProcError();
	}

	if (!geom->IsLoaded()) {
		progressDlg->SetVisible(false);
		SAFE_DELETE(progressDlg);
		return;
	}

	// Create the temporary geometry shared structure
	progressDlg->SetMessage("Creating dataport...");

	size_t loadSize = geom->GetGeometrySize();
	Dataport *loader = CreateDataport(loadDpName, loadSize);
	if( !loader )
		throw Error("Failed to create 'loader' dataport.\nMost probably out of memory.\nReduce number of subprocesses or texture size.");
	progressDlg->SetMessage("Accessing dataport...");
	AccessDataportTimed(loader, (DWORD)(3000 + nbProcess*loadSize / 10000));
	progressDlg->SetMessage("Assembling geometry to pass...");
	geom->CopyGeometryBuffer((BYTE *)loader->buff);
	progressDlg->SetMessage("Releasing dataport...");
	ReleaseDataport(loader);

	size_t hitSize = geom->GetHitsSize(&moments);
	dpHit = CreateDataport(hitsDpName, hitSize);
	ClearHits(true);
	if (!dpHit) {
		CLOSEDP(loader);
		//GLMessageBox::Display("Failed to create 'hits' dataport: not enough memory.", "Warning (Load)", GLDLG_OK, GLDLG_ICONERROR);
		//return false;

		progressDlg->SetVisible(false);
		SAFE_DELETE(progressDlg);
		throw Error("Failed to create 'hits' dataport: out of memory.");
	}

	// Compute number of max desorption per process
	if (AccessDataportTimed(dpControl, DWORD(3000 + nbProcess*loadSize / 10000))) {
		SHCONTROL *master = (SHCONTROL *)dpControl->buff;
		llong common = desorptionLimit / (llong)nbProcess;
		int remain = (int)(desorptionLimit % (llong)nbProcess);
		for (size_t i = 0; i < nbProcess; i++) {
			master->cmdParam2[i] = common;
			if (i < remain) master->cmdParam2[i]++;
		}
		ReleaseDataport(dpControl);
	}

	// Load geometry
	progressDlg->SetMessage("Waiting for subprocesses to load geometry...");
	if (!ExecuteAndWait(COMMAND_LOAD, PROCESS_READY, loadSize)) {
		CLOSEDP(loader);
		char errMsg[1024];
		sprintf(errMsg, "Failed to send geometry to sub process:\n%s", GetErrorDetails());
		//GLMessageBox::Display(errMsg, "Warning (Load)", GLDLG_OK, GLDLG_ICONWARNING);
		//return false;

		progressDlg->SetVisible(false);
		SAFE_DELETE(progressDlg);
		throw Error(errMsg);

	}

	//Old send hits location
	progressDlg->SetMessage("Closing dataport...");
	CLOSEDP(loader);
	needsReload = false;
	progressDlg->SetVisible(false);
	SAFE_DELETE(progressDlg);
}

void Worker::ClearHits(bool noReload) {
	try {
		if (!noReload && needsReload) RealReload();
	}
	catch (Error &e) {
		GLMessageBox::Display((char *)e.GetMsg(), "Error (Stop)", GLDLG_OK, GLDLG_ICONERROR);
		return;
	}
	if (dpHit) {
		AccessDataport(dpHit);
		memset(dpHit->buff, 0, geom->GetHitsSize(&moments));
		ReleaseDataport(dpHit);
	}

}

void Worker::ResetWorkerStats() {

	nbAbsorption = 0;
	nbDesorption = 0;
	nbHit = 0;
	nbLeakTotal = 0;
	distTraveledTotal_total = 0.0;
	distTraveledTotal_fullHitsOnly = 0.0;

}

void Worker::Start() {

	// Check that at least one desortion facet exists
	bool found = false;
	size_t nbF = geom->GetNbFacet();
	size_t i = 0;
	while (i<nbF && !found) {
		found = (geom->GetFacet(i)->sh.desorbType != DES_NONE);
		if (!found) i++;
	}

	if (!found)
		throw Error("No desorption facet found");

	if (!(totalDesorbedMolecules>0.0))
		throw Error("Total outgassing is zero.");

	if (nbProcess == 0)

		throw Error("No sub process found. (Simulation not available)");

	if (!ExecuteAndWait(COMMAND_START, PROCESS_RUN, sMode))
		ThrowSubProcError();
}

/*
std::string execCMD(char* cmd) {
FILE* pipe = _popen(cmd, "r");
if (!pipe) return "ERROR";
char buffer[128];
std::string result = "";
while(!feof(pipe)) {
if(fgets(buffer, 128, pipe) != NULL)
result += buffer;
}
_pclose(pipe);
return result;
}
*/

int Worker::AddMoment(std::vector<double> newMoments) {
	int nb = (int)newMoments.size();
	for (int i = 0; i < nb; i++)
		moments.push_back(newMoments[i]);
	return nb;
}

std::vector<double> Worker::ParseMoment(std::string userInput) {
	std::vector<double> parsedResult;
	double begin, interval, end;

	int nb = sscanf(userInput.c_str(), "%lf,%lf,%lf", &begin, &interval, &end);
	if (nb == 1 && (begin >= 0.0)) {
		//One moment
		parsedResult.push_back(begin);
		//} else if (nb==3 && (begin>0.0) && (end>begin) && (interval<(end-begin)) && ((end-begin)/interval<300.0)) {
	}
	else if (nb == 3 && (begin >= 0.0) && (end > begin) && (interval < (end - begin))) {
		//Range
		for (double time = begin; time <= end; time += interval)
			parsedResult.push_back(time);
	}
	return parsedResult;
}

void Worker::ResetMoments() {
	displayedMoment = 0;
	moments = std::vector<double>();
	userMoments = std::vector<std::string>();
}

double Worker::GetMoleculesPerTP(size_t moment)
//Returns how many physical molecules one test particle represents
{
	if (nbDesorption == 0) return 0; //avoid division by 0
	if (moment == 0) {
		//Constant flow
		//Each test particle represents a certain real molecule influx per second
		return finalOutgassingRate / nbDesorption;
	}
	else {
		//Time-dependent mode
		//Each test particle represents a certain absolute number of real molecules
		return (totalDesorbedMolecules / timeWindowSize) / nbDesorption;
	}
}

/* //Commenting out as deprecated
void Worker::ImportDesorption_DES(char *fileName) {
	//if (needsReload) RealReload();
	// Read a file
	FileReader *f = new FileReader(fileName);
	geom->ImportDesorption_DES(f);
	SAFE_DELETE(f);
	mApp->changedSinceSave = true;
	Reload();
}
*/

void Worker::ImportDesorption_SYN(char *fileName, const size_t &source, const double &time,
	const size_t &mode, const double &eta0, const double &alpha, const double &cutoffdose,
	const std::vector<std::pair<double, double>> &convDistr,
	GLProgress *prg) {
	char *ext, *filebegin;
	char tmp2[1024];
	ext = strrchr(fileName, '.');
	char CWD[MAX_PATH];
	_getcwd(CWD, MAX_PATH);
	if (ext == NULL || (!(_stricmp(ext, ".syn7z") == 0)) && (!(_stricmp(ext, ".syn") == 0)))
		throw Error("ImportDesorption_SYN(): Invalid file extension [Only syn, syn7z]");
	ext++;

	// Read a file

	FileReader *f = NULL;

	GLProgress *progressDlg = new GLProgress("Analyzing SYN file...", "Please wait");
	progressDlg->SetProgress(0.0);
	progressDlg->SetVisible(true);
	bool isSYN7Z = (_stricmp(ext, "syn7z") == 0);
	bool isSYN = (_stricmp(ext, "syn") == 0);

	if (isSYN || isSYN7Z) {
		progressDlg->SetVisible(true);
		try {
			if (isSYN7Z) {
				//decompress file
				progressDlg->SetMessage("Decompressing file...");
				char tmp[1024];
				char fileOnly[512];
				sprintf(tmp, "cmd /C \"pushd \"%s\"&&7za.exe x -t7z -aoa \"%s\" -otmp&&popd\"", CWD, fileName);
				system(tmp);

				filebegin = strrchr(fileName, '\\');
				if (filebegin) filebegin++;
				else filebegin = fileName;
				memcpy(fileOnly, filebegin, sizeof(char)*(strlen(filebegin) - 2)); //remove ..7z from extension
				fileOnly[strlen(filebegin) - 2] = '\0';
				sprintf(tmp2, "%s\\tmp\\Geometry.syn", CWD);
				f = new FileReader(tmp2); //decompressed file opened
			}

			if (!isSYN7Z) f = new FileReader(fileName);  //original file opened

			geom->ImportDesorption_SYN(f, source, time, mode, eta0, alpha, cutoffdose, convDistr, prg);
			CalcTotalOutgassing();
			SAFE_DELETE(f);

		}
		catch (Error &e) {

			SAFE_DELETE(f);
			progressDlg->SetVisible(false);
			SAFE_DELETE(progressDlg);
			throw e;
		}
		progressDlg->SetVisible(false);
		SAFE_DELETE(progressDlg);
	}
}

void Worker::AnalyzeSYNfile(char *fileName, size_t *nbFacet, size_t *nbTextured, size_t *nbDifferent) {
	char *ext, *filebegin;
	char tmp2[1024];
	ext = strrchr(fileName, '.');
	char CWD[MAX_PATH];
	_getcwd(CWD, MAX_PATH);
	if (ext == NULL || (!(_stricmp(ext, ".syn7z") == 0)) && (!(_stricmp(ext, ".syn") == 0)))
		throw Error("AnalyzeSYNfile(): Invalid file extension [Only syn, syn7z]");
	ext++;

	// Read a file
	FileReader *f = NULL;

	GLProgress *progressDlg = new GLProgress("Analyzing SYN file...", "Please wait");
	progressDlg->SetProgress(0.0);
	progressDlg->SetVisible(true);
	bool isSYN7Z = (_stricmp(ext, "syn7z") == 0);
	bool isSYN = (_stricmp(ext, "syn") == 0);

	if (isSYN || isSYN7Z) {
		progressDlg->SetVisible(true);
		try {
			if (isSYN7Z) {
				//decompress file
				progressDlg->SetMessage("Decompressing file...");
				char tmp[1024];
				char fileOnly[512];
				sprintf(tmp, "cmd /C \"pushd \"%s\"&&7za.exe x -t7z -aoa \"%s\" -otmp&&popd\"", CWD, fileName);
				system(tmp);

				filebegin = strrchr(fileName, '\\');
				if (filebegin) filebegin++;
				else filebegin = fileName;
				memcpy(fileOnly, filebegin, sizeof(char)*(strlen(filebegin) - 2)); //remove ..7z from extension
				fileOnly[strlen(filebegin) - 2] = '\0';
				sprintf(tmp2, "%s\\tmp\\Geometry.syn", CWD);
				f = new FileReader(tmp2); //decompressed file opened
			}

			if (!isSYN7Z) f = new FileReader(fileName);  //original file opened

			geom->AnalyzeSYNfile(f, progressDlg, nbFacet, nbTextured, nbDifferent, progressDlg);

			SAFE_DELETE(f);

		}
		catch (Error &e) {
			SAFE_DELETE(f);
			progressDlg->SetVisible(false);
			SAFE_DELETE(progressDlg);
			throw e;
		}
		progressDlg->SetVisible(false);
		SAFE_DELETE(progressDlg);

	}
}

void Worker::PrepareToRun() {

	//determine latest moment
	latestMoment = 1E-10;
	for (size_t i = 0; i<moments.size(); i++)
		if (moments[i]>latestMoment) latestMoment = moments[i];
	latestMoment += timeWindowSize / 2.0;

	Geometry *g = GetGeometry();
	//Generate integrated desorption functions

	temperatures = std::vector<double>();
	desorptionParameterIDs = std::vector<size_t>();
	CDFs = std::vector<std::vector<std::pair<double, double>>>();
	IDs = std::vector<std::vector<std::pair<double, double>>>();

	bool needsAngleMapStatusRefresh = false;

	for (size_t i = 0; i < g->GetNbFacet(); i++) {
		Facet *f = g->GetFacet(i);

		//match parameters
		if (f->userOutgassing.length() > 0) {
			int id = GetParamId(f->userOutgassing);
			if (id == -1) { //parameter not found
				char tmp[256];
				sprintf(tmp, "Facet #%zd: Outgassing parameter \"%s\" isn't defined.", i + 1, f->userOutgassing.c_str());
				throw Error(tmp);
			}
			else f->sh.outgassing_paramId = id;
		}
		else f->sh.outgassing_paramId = -1;

		if (f->userOpacity.length() > 0) {
			int id = GetParamId(f->userOpacity);
			if (id == -1) { //parameter not found
				char tmp[256];
				sprintf(tmp, "Facet #%zd: Opacity parameter \"%s\" isn't defined.", i + 1, f->userOpacity.c_str());
				throw Error(tmp);
			}
			else f->sh.opacity_paramId = id;
		}
		else f->sh.opacity_paramId = -1;

		if (f->userSticking.length() > 0) {
			int id = GetParamId(f->userSticking);
			if (id == -1) { //parameter not found
				char tmp[256];
				sprintf(tmp, "Facet #%zd: Sticking parameter \"%s\" isn't defined.", i + 1, f->userSticking.c_str());
				throw Error(tmp);
			}
			else f->sh.sticking_paramId = id;
		}
		else f->sh.sticking_paramId = -1;

		if (f->sh.outgassing_paramId >= 0) { //if time-dependent desorption
			int id = GetIDId(f->sh.outgassing_paramId);
			if (id >= 0)
				f->sh.IDid = id; //we've already generated an ID for this temperature
			else
				f->sh.IDid = GenerateNewID(f->sh.outgassing_paramId);
		}

		//Generate speed distribution functions
		int id = GetCDFId(f->sh.temperature);
		if (id >= 0)
			f->sh.CDFid = id; //we've already generated a CDF for this temperature
		else
			f->sh.CDFid = GenerateNewCDF(f->sh.temperature);

		//Angle map
		if (f->sh.desorbType == DES_ANGLEMAP) {
			if (!f->sh.anglemapParams.hasRecorded) {
				char tmp[256];
				sprintf(tmp, "Facet #%zd: Uses angle map desorption but doesn't have a recorded angle map.", i + 1);
				throw Error(tmp);
			}
			if (f->sh.anglemapParams.record) {
				char tmp[256];
				sprintf(tmp, "Facet #%zd: Can't RECORD and USE angle map desorption at the same time.", i + 1);
				throw Error(tmp);
			}
		}

		if (f->sh.anglemapParams.record) {
			if (!f->sh.anglemapParams.hasRecorded) {
				//Initialize angle map
				f->angleMapCache = (size_t*)malloc(f->sh.anglemapParams.phiWidth * (f->sh.anglemapParams.thetaLowerRes + f->sh.anglemapParams.thetaHigherRes) * sizeof(size_t));
				if (!f->angleMapCache) {
					std::stringstream tmp;
					tmp << "Not enough memory for incident angle map on facet " << i + 1;
					throw Error(tmp.str().c_str());
				}
				//Set values to zero
				memset(f->angleMapCache, 0, f->sh.anglemapParams.phiWidth * (f->sh.anglemapParams.thetaLowerRes + f->sh.anglemapParams.thetaHigherRes) * sizeof(size_t));
				f->sh.anglemapParams.hasRecorded = true;
				if (f->selected) needsAngleMapStatusRefresh = true;
			}
		}
	}

	if (mApp->facetAdvParams && mApp->facetAdvParams->IsVisible() && needsAngleMapStatusRefresh)
		mApp->facetAdvParams->Refresh(geom->GetSelectedFacets());

	CalcTotalOutgassing();
	
}

int Worker::GetCDFId(double temperature) {

	int i;
	for (i = 0; i<(int)temperatures.size() && (abs(temperature - temperatures[i])>1E-5); i++); //check if we already had this temperature
	if (i >= (int)temperatures.size()) i = -1; //not found
	return i;
}

int Worker::GenerateNewCDF(double temperature){
	size_t i = temperatures.size();
	temperatures.push_back(temperature);
	CDFs.push_back(Generate_CDF(temperature, gasMass, CDF_SIZE));
	return (int)i;
}

int Worker::GenerateNewID(int paramId){
	size_t i = desorptionParameterIDs.size();
	desorptionParameterIDs.push_back(paramId);
	IDs.push_back(Generate_ID(paramId));
	return (int)i;
}

int Worker::GetIDId(int paramId) {

	int i;
	for (i = 0; i < (int)desorptionParameterIDs.size() && (paramId != desorptionParameterIDs[i]); i++); //check if we already had this parameter Id
	if (i >= (int)desorptionParameterIDs.size()) i = -1; //not found
	return i;

}

void Worker::CalcTotalOutgassing() {
	// Compute the outgassing of all source facet
	totalDesorbedMolecules = finalOutgassingRate_Pa_m3_sec = finalOutgassingRate = 0.0;
	Geometry *g = GetGeometry();

	for (int i = 0; i < g->GetNbFacet(); i++) {
		Facet *f = g->GetFacet(i);
		if (f->sh.desorbType != DES_NONE) { //there is a kind of desorption
			if (f->sh.useOutgassingFile) { //outgassing file
				for (int l = 0; l < (f->sh.outgassingMapWidth*f->sh.outgassingMapHeight); l++) {
					totalDesorbedMolecules += latestMoment * f->outgassingMap[l] / (1.38E-23*f->sh.temperature);
					finalOutgassingRate += f->outgassingMap[l] / (1.38E-23*f->sh.temperature);
					finalOutgassingRate_Pa_m3_sec += f->outgassingMap[l];
				}
			}
			else { //regular outgassing
				if (f->sh.outgassing_paramId == -1) { //constant outgassing
					totalDesorbedMolecules += latestMoment * f->sh.outgassing / (1.38E-23*f->sh.temperature);
					finalOutgassingRate += f->sh.outgassing / (1.38E-23*f->sh.temperature);  //Outgassing molecules/sec
					finalOutgassingRate_Pa_m3_sec += f->sh.outgassing;
				}
				else { //time-dependent outgassing
					totalDesorbedMolecules += IDs[f->sh.IDid].back().second / (1.38E-23*f->sh.temperature);
					size_t lastIndex = parameters[f->sh.outgassing_paramId].GetSize() - 1;
					double finalRate_mbar_l_s = parameters[f->sh.outgassing_paramId].GetY(lastIndex);
					finalOutgassingRate += finalRate_mbar_l_s *0.100 / (1.38E-23*f->sh.temperature); //0.1: mbar*l/s->Pa*m3/s
					finalOutgassingRate_Pa_m3_sec += finalRate_mbar_l_s *0.100;
				}
			}
		}
	}
	if (mApp->globalSettings) mApp->globalSettings->UpdateOutgassing();

}

std::vector<std::pair<double, double>> Worker::Generate_CDF(double gasTempKelvins, double gasMassGramsPerMol, size_t size){
	std::vector<std::pair<double, double>> cdf; cdf.reserve(size);
	double Kb = 1.38E-23;
	double R = 8.3144621;
	double a = sqrt(Kb*gasTempKelvins / (gasMassGramsPerMol*1.67E-27)); //distribution a parameter. Converting molar mass to atomic mass

	//Generate cumulative distribution function
	double mostProbableSpeed = sqrt(2 * R*gasTempKelvins / (gasMassGramsPerMol / 1000.0));
	double binSize = 4.0*mostProbableSpeed / (double)size; //distribution generated between 0 and 4*V_prob
	/*double coeff1=1.0/sqrt(2.0)/a;
	double coeff2=sqrt(2.0/PI)/a;
	double coeff3=1.0/(2.0*pow(a,2));

	for (size_t i=0;i<size;i++) {
	double x=(double)i*binSize;
	cdf.push_back(std::make_pair(x,erf(x*coeff1)-coeff2*x*exp(-pow(x,2)*coeff3)));
	}*/
	for (size_t i = 0; i < size; i++) {
		double x = (double)i*binSize;
		double x_square_per_2_a_square = pow(x, 2) / (2 * pow(a, 2));
		cdf.push_back(std::make_pair(x, 1 - exp(-x_square_per_2_a_square)*(x_square_per_2_a_square + 1)));

	}

	/* //UPDATE: not generating inverse since it was introducing sampling problems at the large tail for high speeds
	//CDF created, let's generate its inverse
	std::vector<std::pair<double,double>> inverseCDF;inverseCDF.reserve(size);
	binSize=1.0/(double)size; //Divide probability to bins
	for (size_t i=0;i<size;i++) {
	double p=(double)i*binSize;
	//inverseCDF.push_back(std::make_pair(p,InterpolateX(p,cdf,true)));
	inverseCDF.push_back(std::make_pair(p, InterpolateX(p, cdf, false)));

	}
	return inverseCDF;
	*/
	return cdf;
}

std::vector<std::pair<double, double>> Worker::Generate_ID(int paramId){
	std::vector<std::pair<double, double>> ID;
	//First, let's check at which index is the latest moment
	size_t indexBeforeLastMoment;
	for (indexBeforeLastMoment = 0; indexBeforeLastMoment < parameters[paramId].GetSize() &&
		(parameters[paramId].GetX(indexBeforeLastMoment) < latestMoment); indexBeforeLastMoment++);
		if (indexBeforeLastMoment >= parameters[paramId].GetSize()) indexBeforeLastMoment = parameters[paramId].GetSize() - 1; //not found, set as last moment

	//Construct integral from 0 to latest moment
	//Zero
	ID.push_back(std::make_pair(0.0, 0.0));

	//First moment
	ID.push_back(std::make_pair(parameters[paramId].GetX(0),
		parameters[paramId].GetX(0)*parameters[paramId].GetY(0)*0.100)); //for the first moment (0.1: mbar*l/s -> Pa*m3/s)

	//Intermediate moments
	for (size_t pos = 1; pos <= indexBeforeLastMoment; pos++) {
		if (IsEqual(parameters[paramId].GetY(pos) , parameters[paramId].GetY(pos-1))) //two equal values follow, simple integration by multiplying
			ID.push_back(std::make_pair(parameters[paramId].GetX(pos),
			ID.back().second +
			(parameters[paramId].GetX(pos) - parameters[paramId].GetX(pos-1))*parameters[paramId].GetY(pos)*0.100));
		else { //difficult case, we'll integrate by dividing to 20 equal sections
			for (double delta = 0.05; delta < 1.0001; delta += 0.05) {
				double delta_t = parameters[paramId].GetX(pos) - parameters[paramId].GetX(pos-1);
				double time = parameters[paramId].GetX(pos-1) + delta*delta_t;
				double avg_value = (parameters[paramId].InterpolateY(time - 0.05*delta_t,false) + parameters[paramId].InterpolateY(time,false))*0.100 / 2.0;
				ID.push_back(std::make_pair(time,
					ID.back().second +
					0.05*delta_t*avg_value));
			}
		}
	}

	//latestMoment
	double valueAtLatestMoment = parameters[paramId].InterpolateY(latestMoment,false);
	if (IsEqual(valueAtLatestMoment , parameters[paramId].GetY(indexBeforeLastMoment))) //two equal values follow, simple integration by multiplying
		ID.push_back(std::make_pair(latestMoment,
		ID.back().second +
		(latestMoment - parameters[paramId].GetX(indexBeforeLastMoment))*parameters[paramId].GetY(indexBeforeLastMoment)*0.100));
	else { //difficult case, we'll integrate by dividing two 5equal sections
		for (double delta = 0.0; delta < 1.0001; delta += 0.05) {
			double delta_t = latestMoment - parameters[paramId].GetX(indexBeforeLastMoment);
			double time = parameters[paramId].GetX(indexBeforeLastMoment) + delta*delta_t;
			double avg_value = (parameters[paramId].GetY(indexBeforeLastMoment)*0.100 + parameters[paramId].InterpolateY(time, false)*0.100) / 2.0;
			ID.push_back(std::make_pair(time,
				ID.back().second +
				0.05*delta_t*avg_value));
		}
	}

	return ID;

}

int Worker::GetParamId(const std::string name) {
	int foundId = -1;
	for (int i = 0; foundId == -1 && i < (int)parameters.size(); i++)
		if (name.compare(parameters[i].name) == 0) foundId = i;
	return foundId;
}


/*DWORD Worker::ZipThreadProc(char* fileNameWithXML, char* fileNameWithXMLzip)
{
HZIP hz = CreateZip(fileNameWithXMLzip, 0);
if (!hz) {
throw Error("Error creating ZIP file");
}
if (!ZipAdd(hz, GetShortFileName(fileNameWithXML), fileNameWithXML)) remove(fileNameWithXML);
else {
CloseZip(hz);
throw Error("Error compressing ZIP file.");
}
CloseZip(hz);
return 0;
}*/