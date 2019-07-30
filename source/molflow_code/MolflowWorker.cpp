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
#define NOMINMAX
//#include <Windows.h>
#include "MolflowGeometry.h"
#include "Worker.h"
#include "GLApp/GLApp.h"
#include "GLApp/GLMessageBox.h"
#include <math.h>
#include <stdlib.h>
//#include <Process.h>
#include "GLApp/GLUnitDialog.h"
#include "GLApp/MathTools.h"
#include "Facet_shared.h"
#include "IntersectAABB_shared.h"
//#include "Simulation.h" //SHELEM
#include "GlobalSettings.h"
#include "FacetAdvParams.h"
#include <fstream>
#include <istream>

#include <filesystem>

#include <cereal/archives/binary.hpp>
#include <cereal/types/utility.hpp>
//#include <cereal/archives/xml.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/string.hpp>
#include <fstream>

#ifdef MOLFLOW
#include "MolFlow.h"
#endif

#ifdef SYNRAD
#include "SynRad.h"
#endif

#include "ziplib/ZipArchive.h"
#include "ziplib/ZipArchiveEntry.h"
#include "ziplib/ZipFile.h"
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

/**
* \brief Default constructor for a worker
*/
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
	wp.timeWindowSize = 1E-10; //Dirac-delta desorption pulse at t=0
	wp.useMaxwellDistribution = true;
	wp.calcConstantFlow = true;
	wp.gasMass = 28.0;
	wp.enableDecay = false;
	wp.halfLife = 1;
	wp.finalOutgassingRate = wp.finalOutgassingRate_Pa_m3_sec = wp.totalDesorbedMolecules = 0.0;
	wp.motionType = 0;
	wp.sMode = MC_MODE;

	//Common init
	//pid = _getpid();
	/*sprintf(ctrlDpName, "MFLWCTRL%d", pid);
	sprintf(loadDpName, "MFLWLOAD%d", pid);
	sprintf(hitsDpName, "MFLWHITS%d", pid);
	sprintf(logDpName, "MFLWLOG%d", pid);*/

	ontheflyParams.nbProcess = 0;
	ontheflyParams.enableLogging = false;
	ontheflyParams.desorptionLimit = 0;
	ontheflyParams.lowFluxCutoff = 1E-7;
	ontheflyParams.lowFluxMode = false;

	ResetWorkerStats();
	geom = new MolflowGeometry();

	/*dpControl = NULL;
	dpHit = NULL;
	dpLog = NULL;*/

	startTime = 0.0f;
	stopTime = 0.0f;
	simuTime = 0.0f;

	isRunning = false;
	calcAC = false;
	strcpy(fullFileName, "");
}

/**
* \brief Getter to retrieve the geometry on this worker
* \return pointer of the geometry object
*/
MolflowGeometry* Worker::GetMolflowGeometry() {
	return geom;
}

/**
* \brief Function for saving geometry to a set file
* \param fileName output file name with extension
* \param prg GLProgress window where loading is visualised
* \param askConfirm if a window should be opened to ask if the file should be overwritten
* \param saveSelected if a selection is to be saved
* \param autoSave if automatic saving is enabled
* \param crashSave if save on crash is enabled
*/
void Worker::SaveGeometry(std::string fileName, GLProgress *prg, bool askConfirm, bool saveSelected, bool autoSave, bool crashSave) {

	try {
		if (needsReload && (!crashSave && !saveSelected)) RealReload();
	}
	catch (Error &e) {
		char errMsg[512];
		sprintf(errMsg, "Error reloading worker. Trying safe save (geometry only):\n%s", e.GetMsg());
		GLMessageBox::Display(errMsg, "Error", GLDLG_OK, GLDLG_ICONERROR);
		crashSave = true;
	}
	std::string compressCommandLine;
	std::string fileNameWithGeo; //file name with .geo extension (instead of .geo7z)
	std::string fileNameWithGeo7z;
	std::string fileNameWithXML;
	std::string fileNameWithZIP;
	std::string fileNameWithoutExtension; //file name without extension
	
	std::string ext = FileUtils::GetExtension(fileName);
	std::string path = FileUtils::GetPath(fileName);
	
	bool ok = true;
	if (ext.empty()) {
		fileName = fileName + (mApp->compressSavedFiles ? ".zip" : ".xml");
		ext = FileUtils::GetExtension(fileName);
		if (!autoSave && FileUtils::Exist(fileName)) {
			char tmp[1024];
			sprintf(tmp, "Overwrite existing file ?\n%s", fileName.c_str());
			if (askConfirm) ok = (GLMessageBox::Display(tmp, "Question", GLDLG_OK | GLDLG_CANCEL, GLDLG_ICONWARNING) == GLDLG_OK);
		}
	}

	// Read a file
	
	FileWriter *f = NULL;
	bool isTXT = Contains({ "txt","TXT" },ext);
	bool isSTR = Contains({ "str","STR" }, ext);
	bool isGEO = ext == "geo";
	bool isGEO7Z = ext == "geo7z";
	bool isXML = ext == "xml";
	bool isXMLzip = ext == "zip";

	if (isTXT || isGEO || isGEO7Z || isSTR || isXML || isXMLzip) {
#ifdef _WIN32
		//Check (using native handle) if background compressor is still alive
		if ((isGEO7Z) && WAIT_TIMEOUT == WaitForSingleObject(mApp->compressProcessHandle, 0)) {
			GLMessageBox::Display("Compressing a previous save file is in progress. Wait until that finishes "
				"or close process \"compress.exe\"\nIf this was an autosave attempt,"
				"you have to lower the autosave frequency.", "Can't save right now.", GLDLG_OK, GLDLG_ICONERROR);
			return;
		}
#endif
		if (isGEO) {
			fileNameWithoutExtension=fileName.substr(0,fileName.length()-4);
			fileNameWithGeo7z = fileName + "7z";			
		}
		else if (isGEO7Z) {
			fileNameWithoutExtension = fileName.substr(0, fileName.length() - 6);
			fileNameWithGeo = fileName.substr(0, fileName.length() - 2);
			fileNameWithGeo7z = fileName;
			std::ostringstream tmp;
			tmp << "A .geo file of the same name exists. Overwrite that file ?\n" << fileNameWithGeo;
			if (!autoSave && FileUtils::Exist(fileNameWithGeo)) {
				ok = (GLMessageBox::Display(tmp.str().c_str(), "Question", GLDLG_OK | GLDLG_CANCEL, GLDLG_ICONWARNING) == GLDLG_OK);
			}
		}

		if (isXML || isXMLzip) {
			fileNameWithoutExtension = fileName.substr(0,fileName.length() - 4);			
			fileNameWithXML = fileNameWithoutExtension + ".xml";
			fileNameWithZIP = fileNameWithoutExtension + ".zip";
		}
		if (isXMLzip) {
			char tmp[1024];
			sprintf(tmp, "An .xml file of the same name exists. Overwrite that file ?\n%s", fileNameWithZIP.c_str());
			if (!autoSave && FileUtils::Exist(fileNameWithXML)) {

				ok = (GLMessageBox::Display(tmp, "Question", GLDLG_OK | GLDLG_CANCEL, GLDLG_ICONWARNING) == GLDLG_OK);
			}
		}		

		if (ok) {
			if (isSTR) {
				geom->SaveSTR(results, saveSelected);
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
					GLMessageBox::Display(e.GetMsg(), "Error writing file.", GLDLG_OK, GLDLG_ICONERROR);
					return;
				}
				
				if (isTXT) geom->SaveTXT(f, results, saveSelected);
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
					geom->SaveGEO(f, prg, results, this, saveSelected, crashSave);
				}
				else if (isXML || isXMLzip) {
					{
						xml_document saveDoc;
						geom->SaveXML_geometry(saveDoc, this, prg, saveSelected);
						xml_document geom_only; geom_only.reset(saveDoc);
						bool success = false; //success: simulation state could be saved
						if (!crashSave && !saveSelected) {
							try {
								LockMutex(results.mutex);
								success = geom->SaveXML_simustate(saveDoc, this, results, prg, saveSelected);
								ReleaseMutex(results.mutex);
							}
							catch (Error &e) {
								SAFE_DELETE(f);
								ReleaseMutex(results.mutex);
								GLMessageBox::Display(e.GetMsg(), "Error saving simulation state.", GLDLG_OK, GLDLG_ICONERROR);
								return;
							}
						}

						prg->SetMessage("Writing xml file...");
						if (success) {
							if (!saveDoc.save_file(fileNameWithXML.c_str())) throw Error("Error writing XML file."); //successful save
						}
						else {
							if (!geom_only.save_file(fileNameWithXML.c_str())) throw Error("Error writing XML file."); //simu state error
						}
					} //saveDoc goes out of scope for the compress duration
					if (isXMLzip) {
						prg->SetProgress(0.75);
						prg->SetMessage("Compressing xml to zip...");
						//mApp->compressProcessHandle=CreateThread(0, 0, ZipThreadProc, 0, 0, 0);
						
						//Zipper library
						if (FileUtils::Exist(fileNameWithZIP)) remove(fileNameWithZIP.c_str());
						ZipFile::AddFile(fileNameWithZIP, fileNameWithXML,FileUtils::GetFilename(fileNameWithXML));
						//At this point, if no error was thrown, the compression is successful
						remove(fileNameWithXML.c_str());


						/*
						//Old ZipUtils library, Windows only
						HZIP hz = CreateZip(fileNameWithZIP.c_str(), 0);
						if (!hz) {
							throw Error("Error creating ZIP file");
						}
						if (!ZipAdd(hz, FileUtils::GetFilename(fileNameWithXML).c_str(), fileNameWithXML.c_str())) remove(fileNameWithXML.c_str());
						else {
							CloseZip(hz);
							throw Error("Error compressing ZIP file.");
						}
						CloseZip(hz);
						*/
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

#ifdef _WIN32
		std::string compressorName = "compress.exe";
#else
		std::string compressorName = "./compress";
#endif

		if (FileUtils::Exist(compressorName)) { //compress GEO file to GEO7Z using 7-zip launcher "compress.exe"
			std::ostringstream tmp;
			tmp<<compressorName << " \"" << fileNameWithGeo << "\" Geometry.geo";
#ifdef _WIN32
			size_t procId = StartProc(tmp.str().c_str(),STARTPROC_BACKGROUND);
			mApp->compressProcessHandle = OpenProcess(PROCESS_ALL_ACCESS, true, (unsigned long)procId);
#else
			//In Linux, compressing to old format will be blocking
			system(tmp.str().c_str());
#endif

			fileName = fileNameWithGeo7z;
		}
		else {
			GLMessageBox::Display("compress.exe (part of Molfow) not found.\n Will save as uncompressed GEO file.", "Compressor not found", GLDLG_OK, GLDLG_ICONERROR);
			fileName = fileNameWithGeo;
		}
	}
	else if (ok && isGEO) fileName = fileNameWithGeo;
	if (!autoSave && !saveSelected) {
		SetCurrentFileName(fileName.c_str());
		mApp->UpdateTitle();
	}
}

/**
* \brief Function for saving of the profile data (simulation)
* \param fileName output file name
*/
void Worker::ExportProfiles(const char *fn) {

	std::string fileName = fn;
	char tmp[512];

	// Read a file
	FILE *f = NULL;

	if (FileUtils::GetExtension(fileName).empty()) {
		fileName += ".csv"; //set to default CSV format
		if (FileUtils::Exist(fileName)) {
			sprintf(tmp, "Overwrite existing file ?\n%s", fileName.c_str());
			if (GLMessageBox::Display(tmp, "Question", GLDLG_OK | GLDLG_CANCEL, GLDLG_ICONWARNING) != GLDLG_OK) return;
		}
	}
	bool isTXT = FileUtils::GetExtension(fileName) == "txt";


		f = fopen(fileName.c_str(), "w");
		if (!f) {
			char tmp[256];
			sprintf(tmp, "Cannot open file for writing %s", fileName.c_str());
			throw Error(tmp);
		}
		geom->ExportProfiles(f, isTXT, results, this);
		fclose(f);
	
}

/**
* \brief Function for exportation of the angle maps
* \param facetList vector of all facets necessary for the export
* \param fileName output file name
*/
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

/*void Worker::ImportDesorption(const char *fileName) {
	//if (needsReload) RealReload();

	// Read a file
	FileReader *f=new FileReader(fileName);
	geom->ImportDesorption(f,dpHit);
	SAFE_DELETE(f);
	changedSinceSave=true;
	Reload();
	}*/

	/**
	* \brief Function for loading of the geometry
	* \param fileName input file name
	* \param insert if geometry will be inserted into an existing geometry view
	* \param newStr if a new structure needs to be created
	*/
void Worker::LoadGeometry(const std::string& fileName,bool insert,bool newStr) {
	if (!insert) {
		needsReload = true;
	}
	else {
		RealReload();
	}
	//char CWD[MAX_PATH];
	//_getcwd(CWD, MAX_PATH);

	std::string ext=FileUtils::GetExtension(fileName);

	if (ext == "")

		throw Error("LoadGeometry(): No file extension, can't determine type");

	// Read a file
	FileReader *f = NULL;
	GLProgress *progressDlg = new GLProgress("Reading file...", "Please wait");
	progressDlg->SetVisible(true);
	progressDlg->SetProgress(0.0);

	ResetWorkerStats();

	if (!insert) {
		//Clear hits and leaks cache
		ResetMoments();
		
		//default values
		wp.enableDecay = false;
		wp.gasMass = 28;
	}

	/*
	bool isASE = (stricmp(ext, "ase") == 0);
	bool isSTR = (stricmp(ext, "str") == 0);
	bool isSTL = (stricmp(ext, "stl") == 0);
	bool isTXT = (stricmp(ext, "txt") == 0);
	bool isGEO = (stricmp(ext, "geo") == 0);
	bool isGEO7Z = (stricmp(ext, "geo7z") == 0);
	bool isSYN = (stricmp(ext, "syn") == 0);
	bool isSYN7Z = (stricmp(ext, "syn7z") == 0);
	bool isXML = (stricmp(ext, "xml") == 0);
	bool isXMLzip = (stricmp(ext, "zip") == 0);
	*/

	if (ext == "txt" || ext == "TXT") {

		try {
			
			if (insert) mApp->changedSinceSave = true;

			f = new FileReader(fileName);
			
			if (!insert) {
				geom->LoadTXT(f, progressDlg, this);
				SAFE_DELETE(f);
				//RealReload();
				strcpy(fullFileName, fileName.c_str());
			}
			else { //insert

				geom->InsertTXT(f, progressDlg, newStr);
				SAFE_DELETE(f);
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
				progressDlg->SetMessage("Reading geometry...");
				f = new FileReader(fileName);
				if (!insert) {
					geom->LoadSTL(f, progressDlg, scaleFactor);
					SAFE_DELETE(f);
					strcpy(fullFileName, fileName.c_str());
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
			f = new FileReader(fileName);
			progressDlg->SetVisible(true);
			geom->LoadSTR(f, progressDlg);
			SAFE_DELETE(f);
			//RealReload();

			strcpy(fullFileName, fileName.c_str());
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
					f = ExtractFrom7zAndOpen(fileName, "Geometry.syn");
				}
				else {
					f = new FileReader(fileName);  //original file opened
				}
			
			if (!insert) {
				progressDlg->SetMessage("Resetting worker...");
			
				geom->LoadSYN(f, progressDlg, &version, this);
				SAFE_DELETE(f);
				ontheflyParams.desorptionLimit = 0;
			}
			else { //insert
				geom->InsertSYN(f, progressDlg, newStr);
				SAFE_DELETE(f);
			}

			progressDlg->SetMessage("Reloading worker with new geometry...");
			Reload();
			if (!insert) strcpy(fullFileName, fileName.c_str());
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
		int version;
		progressDlg->SetVisible(true);
		try {
			if (ext == "geo7z") {
				//decompress file
				progressDlg->SetMessage("Decompressing file...");
				f = ExtractFrom7zAndOpen(fileName, "Geometry.geo");
			}
			else { //not geo7z
				f = new FileReader(fileName); //geo file, open it directly
			}
			if (insert) mApp->changedSinceSave = true;

			if (!insert) {

				geom->LoadGEO(f, progressDlg, &version, this);
				
				progressDlg->SetMessage("Reloading worker with new geometry...");
				RealReload(); //for the loading of textures
				if (version >= 8) geom->LoadProfileGEO(f, results, version);
				SendToHitBuffer(); //Global hit counters and hit/leak cache
				SendFacetHitCounts(/*dpHit*/); // From facetHitCache to dpHit's const.flow counter
				
				progressDlg->SetMessage("Loading textures...");
				LoadTexturesGEO(f, version);
				strcpy(fullFileName, fileName.c_str());
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

				ZipArchive::Ptr zip = ZipFile::Open(fileName);
				if (zip == nullptr) {
					throw Error("Can't open ZIP file");
				}
				size_t numitems = zip->GetEntriesCount();
				bool notFoundYet = true;
				for (int i = 0; i < numitems && notFoundYet; i++) { //extract first xml file found in ZIP archive
					auto zipItem = zip->GetEntry(i);
					std::string zipFileName = zipItem->GetName();

					if (FileUtils::GetExtension(zipFileName) == "xml") { //if it's an .xml file
						notFoundYet = false;

						FileUtils::CreateDir("tmp");// If doesn't exist yet

						std::string tmpFileName = "tmp/" + zipFileName;
						ZipFile::ExtractFile(fileName, zipFileName, tmpFileName);
						progressDlg->SetMessage("Reading and parsing XML file...");

						parseResult = loadXML.load_file(tmpFileName.c_str()); //load and parse it
					}
				}
				if (notFoundYet) {
					throw Error("Didn't find any XML file in the ZIP file.");
				}

				/*
				//Old ZipUtils library
				HZIP hz = OpenZip(fileName.c_str(), 0);
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
				*/

			} else parseResult = loadXML.load_file(fileName.c_str()); //parse xml file directly

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
				geom->UpdateName(fileName.c_str());

				progressDlg->SetMessage("Reloading worker with new geometry...");
				try {
					RealReload(); //To create the dpHit dataport for the loading of textures, profiles, etc...
					strcpy(fullFileName, fileName.c_str());

					if (ext == "xml" || ext == "zip")
						progressDlg->SetMessage("Restoring simulation state...");
					geom->LoadXML_simustate(loadXML, results, this, progressDlg);
					SendToHitBuffer(); //Send hits without sending facet counters, as they are directly written during the load process (mutiple moments)
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
			strcpy(fullFileName, fileName.c_str());

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

/**
* \brief Function for loading textures from a GEO file
* \param f input file handle
* \param version version of the GEO data description
*/
void Worker::LoadTexturesGEO(FileReader *f, int version) {	
		GLProgress *progressDlg = new GLProgress("Loading textures", "Please wait");
		progressDlg->SetProgress(0.0);
		try {
			progressDlg->SetVisible(true);
			geom->LoadTexturesGEO(f, progressDlg, results, version);
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

/**
* \brief Function that updates various variables when stopping a simulation
* \param appTime current time of the application
*/
void Worker::InnerStop(float appTime) {

	stopTime = appTime;
	simuTime += appTime - startTime;
	isRunning = false;
	calcAC = false;

}

/**
* \brief Function that starts exactly one simulation step for AC (angular coefficient) mode
*/
void Worker::OneACStep() {

	if (ontheflyParams.nbProcess == 0)
		throw Error("No sub process found. (Simulation not available)");

	if (!isRunning)  {
		if (!ExecuteAndWait(COMMAND_STEPAC, PROCESS_RUN, AC_MODE))
			ThrowSubProcError();
	}

}

/**
* \brief Function that executes one step in AC (angular coefficient) mode and updates the interface
* \param appTime current time of the application
*/
void Worker::StepAC(float appTime) {

	try {
		OneACStep();
		Update(appTime);
	}
	catch (Error &e) {
		GLMessageBox::Display(e.GetMsg(), "Error (Stop)", GLDLG_OK, GLDLG_ICONERROR);
	}

}

/**
* \brief Function that handles starting and stopping of the simulation
* \param appTime current time of the application
* \param sMode simulation mode (MC/AC)
*/
void Worker::StartStop(float appTime , size_t sMode) {

	if (isRunning)  {

		// Stop
		InnerStop(appTime);
		try {
			Stop();
			Update(appTime);
		}

		catch (Error &e) {
			GLMessageBox::Display(e.GetMsg(), "Error (Stop)", GLDLG_OK, GLDLG_ICONERROR);
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

			wp.sMode = sMode;

			Start();

		}
		catch (Error &e) {
			isRunning = false;
			GLMessageBox::Display(e.GetMsg(), "Error (Start)", GLDLG_OK, GLDLG_ICONERROR);
			return;
		}

			// Particular case when simulation ends before getting RUN state
			if (allDone) {
				Update(appTime);
				GLMessageBox::Display("Max desorption reached", "Information (Start)", GLDLG_OK, GLDLG_ICONINFO);
			}

	}

}

/**
* \brief Function that inserts a list of new paramters at the beginning of the catalog parameters
* \param newParams vector containing new parameters to be inserted
*/
void Worker::InsertParametersBeforeCatalog(const std::vector<Parameter>& newParams)
{
	auto iter = parameters.begin();
	for (; iter != parameters.end() && !iter->fromCatalog; iter++);
	parameters.insert(iter, newParams.begin(), newParams.end()); //Insert to front (before catalog parameters)
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
				WorkerControl *master = (WorkerControl *)dpControl->buff;
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
		if ((error || done) && running && appTime != 0.0f) {
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
				nbMCHit = gHits->globalHits.nbMCHit;
				nbAbsEquiv = gHits->globalHits.nbAbsEquiv;
				nbDesorption = gHits->globalHits.nbDesorbed;				
				distTraveled_total = gHits->distTraveled_total;
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
					f->facetHitCache=(*((FacetHitBuffer*)(buffer + f->wp.hitOffset+displayedMoment*sizeof(FacetHitBuffer))));
				}
				try {
					if (mApp->needsTexture || mApp->needsDirection) geom->BuildFacetTextures(buffer,mApp->needsTexture,mApp->needsDirection);
				}
				catch (Error &e) {
					GLMessageBox::Display(e.GetMsg(), "Error building texture", GLDLG_OK, GLDLG_ICONERROR);
					hitsMutex.unlock();
					return;
				}
				hitsMutex.unlock();
			}

		}
	//}

}
*/

/**
* \brief Function that updates the global hit counter with the cached value + releases the mutex
* \param skipFacetHits TODO: check if not necessary anymore
* 
* Send total and facet hit counts to subprocesses
*/
void Worker::SendToHitBuffer(bool skipFacetHits ) {
	GlobalSimuState* hits = GetHits();
	if (!hits) throw Error("Failed to initialize 'hits' dataport");
	hits->globalHits = globalHitCache;
	ReleaseHits();
}
/*
void Worker::ComputeAC(float appTime) {
	try {
		if (needsReload) RealReload();
	}
	catch (Error &e) {
		GLMessageBox::Display(e.GetMsg(), "Error (Stop)", GLDLG_OK, GLDLG_ICONERROR);
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
/*
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
*/

/**
* \brief Function that reloads the whole simulation (resets simulation, rebuilds ray tracing etc) and synchronises subprocesses to main process
* \param sendOnly if only certain parts should be reloaded (geometry reloading / ray tracing tree)
*/
void Worker::RealReload(bool sendOnly) { //Sharing geometry with workers
	GLProgress *progressDlg = new GLProgress("Performing preliminary calculations on geometry...", "Passing Geometry to workers");
	progressDlg->SetVisible(true);
	progressDlg->SetProgress(0.0);
	
	if (!sendOnly) {
		//Do preliminary calculations
		try {
			PrepareToRun();
		}
		catch (Error &e) {
			progressDlg->SetVisible(false);
			SAFE_DELETE(progressDlg);
			throw Error(e.GetMsg());
		}

		if (ontheflyParams.nbProcess == 0) return;

		progressDlg->SetMessage("Asking subprocesses to clear geometry...");


		// Clear geometry
		LockMutex(results.mutex);
		results.initialized = false;
		ReleaseMutex(results.mutex);
		LockMutex(logMutex);
		log.clear();
		ReleaseMutex(logMutex);
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
		progressDlg->SetMessage("Creating particle log...");

		if (ontheflyParams.enableLogging) {
			try {
				LockMutex(logMutex);
				std::vector<ParticleLoggerItem>(ontheflyParams.logLimit).swap(log);
				ReleaseMutex(logMutex);
			}
			catch (...) {
				progressDlg->SetVisible(false);
				SAFE_DELETE(progressDlg);
					throw Error("Failed to create 'dpLog' dataport.\nMost probably out of memory.\nReduce number of logged particles in Particle Logger.");
			}
		}
	}


	progressDlg->SetMessage("Constructing memory structure to store results...");
	if (!sendOnly) {
		results.Resize(*this);
	}
	emptyResultTemplate = GlobalSimuState();
	emptyResultTemplate.Resize(*this);
	//Construct subprocess structures and calculate their AABB
	std::vector<SubProcessSuperStructure>(GetGeometry()->GetNbStructure()).swap(subprocessStructures); //Create structures
	size_t nbF = GetGeometry()->GetNbFacet();
	progressDlg->SetMessage("Preparing facets for simulation...");
	try {
		for (size_t i = 0; i < nbF; i++) {
			progressDlg->SetProgress((double)i / (double)nbF);
			SubprocessFacet f;
			f.globalId = i;
			f.facetRef = GetGeometry()->GetFacet(i);
			f.InitializeOnLoad(subprocessStructures.size());
			if (f.facetRef->sh.superIdx == -1) { //Facet in all structures
				for (auto& s : subprocessStructures) {
					s.facets.push_back(f);
				}
			}
			else {
				subprocessStructures[f.facetRef->sh.superIdx].facets.push_back(f);
			}
		}
		}
		catch (Error &e) {
		progressDlg->SetVisible(false);
		SAFE_DELETE(progressDlg);
		throw Error(e.GetMsg());
		}
	size_t maxDepth = 0;
	progressDlg->SetMessage("Constructing ray-tracing volume hierarchy...");
	for (auto& s : subprocessStructures) {
		std::vector<SubprocessFacet*> facetPointers; facetPointers.reserve(s.facets.size());
		for (auto& f : s.facets) {
			facetPointers.push_back(&f);
		}
		s.aabbTree = BuildAABBTree(facetPointers, 0, maxDepth);
	}

	// Load geometry
	progressDlg->SetMessage("Waiting for subprocesses to load geometry...");
	if (!ExecuteAndWait(COMMAND_LOAD, PROCESS_READY)) {
		char errMsg[1024];
		sprintf(errMsg, "Failed to send geometry to sub process:\n%s", GetErrorDetails());

		progressDlg->SetVisible(false);
		SAFE_DELETE(progressDlg);
		throw Error(errMsg);

	}
	needsReload = false;
	progressDlg->SetVisible(false);
	SAFE_DELETE(progressDlg);
}

/**
* \brief Serialization function for a binary cereal archive for the worker attributes
* \return output string stream containing the result of the archiving
*/
std::ostringstream Worker::SerializeForLoader()
{
	//std::ofstream os("out.xml");
	std::ostringstream result;
	cereal::BinaryOutputArchive outputarchive(result);

	outputarchive(
		CEREAL_NVP(wp),
		CEREAL_NVP(ontheflyParams),
		CEREAL_NVP(CDFs),
		CEREAL_NVP(IDs),
		CEREAL_NVP(parameters),
		CEREAL_NVP(temperatures),
		CEREAL_NVP(moments),
		CEREAL_NVP(desorptionParameterIDs)
	); //Worker

	geom->SerializeForLoader(outputarchive);

	return result;
}

/**
* \brief Resets the hit counter and can reload the simulation
* \param noReload if the whole simulation should not be reloaded
*/
void Worker::ClearHits(bool noReload) {
	try {
		if (!noReload && needsReload) RealReload();
	}
	catch (Error &e) {
		GLMessageBox::Display(e.GetMsg(), "Error (Stop)", GLDLG_OK, GLDLG_ICONERROR);
		return;
	}
	/*
	if (dpHit) {
		LockMutex(hitsMutex);
		memset(dpHit->buff, 0, geom->GetHitsSize(&moments)); //Also clears hits, leaks
		hitsMutex.unlock();
	}
	*/
	results.Reset();

}

/**
* \brief Resets workers global hit cache
* Reset function mainly used for initialisation / reload procedures
*/
void Worker::ResetWorkerStats() {

	memset(&globalHitCache, 0, sizeof(GlobalHitBuffer));


}

/**
* \brief Starts the simulation process
*/
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

	if (!(wp.totalDesorbedMolecules>0.0))
		throw Error("Total outgassing is zero.");

	if (ontheflyParams.nbProcess == 0)

		throw Error("No sub process found. (Simulation not available)");

	if (!ExecuteAndWait(COMMAND_START, PROCESS_RUN, wp.sMode))
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

/**
* \brief Adds a time serie to moments and returns the number of elements
* \param newMoments vector containing a list of new moments that should be added
* \return number of new moments that got added
*/
int Worker::AddMoment(std::vector<double> newMoments) {
	int nb = (int)newMoments.size();
	for (int i = 0; i < nb; i++)
		moments.push_back(newMoments[i]);
	return nb;
}

/**
* \brief Parses a user input and returns a vector of time moments
* \param userInput string of format "%lf,%lf,%lf" describing start, interval and end for a list of new moments
* \return vector containing parsed moments
*/
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

/**
* \brief Resets/clears all moment variables
*/
void Worker::ResetMoments() {
	displayedMoment = 0;
	moments.clear();
	userMoments.clear();
}

/**
* \brief Returns how many physical molecules one test particle represents
* \param moment if there is constant flow or time-dependent mode
* \return amount of physical molecules represented by one test particle
*/
double Worker::GetMoleculesPerTP(size_t moment) {
	if (globalHitCache.globalHits.nbDesorbed == 0) return 0; //avoid division by 0
	if (moment == 0) {
		//Constant flow
		//Each test particle represents a certain real molecule influx per second
		return wp.finalOutgassingRate / globalHitCache.globalHits.nbDesorbed;
	}
	else {
		//Time-dependent mode
		//Each test particle represents a certain absolute number of real molecules
		return (wp.totalDesorbedMolecules / wp.timeWindowSize) / globalHitCache.globalHits.nbDesorbed;
	}
}

/* //Commenting out as deprecated
void Worker::ImportDesorption_DES(const char *fileName) {
	//if (needsReload) RealReload();
	// Read a file
	FileReader *f = new FileReader(fileName);
	geom->ImportDesorption_DES(f);
	SAFE_DELETE(f);
	mApp->changedSinceSave = true;
	Reload();
}
*/

/**
* \brief Importing desorption data from a SYN file
* \param fileName name of the input file
* \param source what the source to calculate the dose is
* \param time time to calculate the dose
* \param mode mode used for outgassing calculation
* \param eta0 coefficient for outgassing calculation in mode==1
* \param alpha exponent for outgassing calculation in mode==1
* \param cutoffdose cutoff dose for outgassing calculation in mode==1
* \param convDistr distribution for outgassing calculation in mode==2
* \param prg GLProgress window where visualising of the import progress is shown
*/
void Worker::ImportDesorption_SYN(const char *fileName, const size_t &source, const double &time,
	const size_t &mode, const double &eta0, const double &alpha, const double &cutoffdose,
	const std::vector<std::pair<double, double>> &convDistr,
	GLProgress *prg) {
	std::string ext = FileUtils::GetExtension(fileName);
	if (!Contains({"syn7z","syn"},ext)) throw Error("ImportDesorption_SYN(): Invalid file extension [Only syn, syn7z]");

	// Read a file

	FileReader *f = NULL;

	GLProgress *progressDlg = new GLProgress("Analyzing SYN file...", "Please wait");
	progressDlg->SetProgress(0.0);
	progressDlg->SetVisible(true);
	bool isSYN7Z = (iequals(ext, "syn7z") );
	bool isSYN = (iequals(ext, "syn"));

	if (isSYN || isSYN7Z) {
		progressDlg->SetVisible(true);
		try {
			if (isSYN7Z) {
				f = ExtractFrom7zAndOpen(fileName, "Geometry.syn");
			}
			else {
				f = new FileReader(fileName);  //original file opened
			} 

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

/**
* \brief To analyse desorption data from a SYN file
* \param fileName name of the input file
* \param nbFacet number of facets in the file
* \param nbTextured number of textured facets in the file
* \param nbDifferent (TODO: check usage)
*/
void Worker::AnalyzeSYNfile(const char *fileName, size_t *nbFacet, size_t *nbTextured, size_t *nbDifferent) {
	std::string ext = FileUtils::GetExtension(fileName);
	bool isSYN = (ext == "syn") || (ext == "SYN");
	bool isSYN7Z = (ext == "syn7z") || (ext == "SYN7Z");
	
	if (!(isSYN || isSYN7Z))
		throw Error("AnalyzeSYNfile(): Invalid file extension [Only syn, syn7z]");

	// Read a file
	FileReader *f = NULL;

	GLProgress *progressDlg = new GLProgress("Analyzing SYN file...", "Please wait");
	progressDlg->SetProgress(0.0);
	progressDlg->SetVisible(true);

	if (isSYN || isSYN7Z) {
		progressDlg->SetVisible(true);
		try {
			if (isSYN7Z) {
				//decompress file
				progressDlg->SetMessage("Decompressing file...");
				f = ExtractFrom7zAndOpen(fileName, "Geometry.syn");
			}
			else {
				f = new FileReader(fileName);  //original file opened
			}

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

/**
* \brief Extract a 7z file and return the file handle
* \param fileName name of the input file
* \param geomName name of the geometry file
* \return handle to opened decompressed file
*/
FileReader* Worker::ExtractFrom7zAndOpen(const std::string & fileName, const std::string & geomName)
{
	std::ostringstream cmd;
	std::string sevenZipName = "7za";

#ifdef _WIN32
	//Necessary push/pop trick to support UNC (network) paths in Windows command-line
	auto CWD = FileUtils::get_working_path();
	cmd << "cmd /C \"pushd \"" << CWD << "\"&&";
	cmd << "7za.exe x -t7z -aoa \"" << fileName << "\" -otmp";
#else
	cmd << "./7za x -t7z -aoa \"" << fileName << "\" -otmp";
#endif

#ifdef _WIN32
	cmd << "&&popd\"";
#endif
	system(cmd.str().c_str());

	std::string toOpen, prefix;
	std::string shortFileName = FileUtils::GetFilename(fileName);
#ifdef _WIN32
	prefix = CWD + "\\tmp\\";
#else
	prefix = "tmp/";
#endif
	toOpen = prefix + geomName;
	if (!FileUtils::Exist(toOpen)) toOpen = prefix + (shortFileName).substr(0, shortFileName.length() - 2); //Inside the zip, try original filename with extension changed from geo7z to geo

	return new FileReader(toOpen); //decompressed file opened
}

/**
* \brief Do calculations necessary before launching simulation
* determine latest moment
* Generate integrated desorption functions
* match parameters
* Generate speed distribution functions
* Angle map
*/
void Worker::PrepareToRun() {

	//determine latest moment
	wp.latestMoment = 1E-10;
	for (size_t i = 0; i<moments.size(); i++)
		if (moments[i]>wp.latestMoment) wp.latestMoment = moments[i];
	wp.latestMoment += wp.timeWindowSize / 2.0;

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
			if (f->angleMapCache.empty()) {
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

		/* //First worker::update will do it
		if (f->sh.anglemapParams.record) {
			if (!f->sh.anglemapParams.hasRecorded) {
				//Initialize angle map
				f->angleMapCache = (size_t*)malloc(f->sh.anglemapParams.GetDataSize());
				if (!f->angleMapCache) {
					std::stringstream tmp;
					tmp << "Not enough memory for incident angle map on facet " << i + 1;
					throw Error(tmp.str().c_str());
				}
				//Set values to zero
				memset(f->angleMapCache, 0, f->sh.anglemapParams.GetDataSize());
				f->sh.anglemapParams.hasRecorded = true;
				if (f->selected) needsAngleMapStatusRefresh = true;
			}
		}
		*/
	}

	if (mApp->facetAdvParams && mApp->facetAdvParams->IsVisible() && needsAngleMapStatusRefresh)
		mApp->facetAdvParams->Refresh(geom->GetSelectedFacets());

	CalcTotalOutgassing();
	
}

/**
* \brief Get ID (if it exists) of the Commulative Distribution Function (CFD) for a particular temperature (bin)
* \param temperature temperature for the CFD
* \return ID of the CFD
*/
int Worker::GetCDFId(double temperature) {

	int i;
	for (i = 0; i<(int)temperatures.size() && (std::abs(temperature - temperatures[i])>1E-5); i++); //check if we already had this temperature
	if (i >= (int)temperatures.size()) i = -1; //not found
	return i;
}

/**
* \brief Generate a new Commulative Distribution Function (CFD) for a particular temperature (bin)
* \param temperature for the CFD
* \return Previous size of temperatures vector, which determines new ID
*/
int Worker::GenerateNewCDF(double temperature){
	size_t i = temperatures.size();
	temperatures.push_back(temperature);
	CDFs.push_back(Generate_CDF(temperature, wp.gasMass, CDF_SIZE));
	return (int)i;
}

/**
* \brief Generate a new ID (integrated desorption) for desorption parameter for time-dependent simulations
* \param paramId parameter ID
* \return Previous size of IDs vector, which determines new id in the vector
*/
int Worker::GenerateNewID(int paramId){
	size_t i = desorptionParameterIDs.size();
	desorptionParameterIDs.push_back(paramId);
	IDs.push_back(Generate_ID(paramId));
	return (int)i;
}

/**
* \brief Get ID (if it exists) of the integrated desorption (ID) function for a particular paramId
* \param paramId parameter ID
* \return Id of the integrated desorption function
*/
int Worker::GetIDId(int paramId) {

	int i;
	for (i = 0; i < (int)desorptionParameterIDs.size() && (paramId != desorptionParameterIDs[i]); i++); //check if we already had this parameter Id
	if (i >= (int)desorptionParameterIDs.size()) i = -1; //not found
	return i;

}

/**
* \brief Compute the outgassing of all source facet depending on the mode (file, regular, time-dependent) and set it to the global settings
*/
void Worker::CalcTotalOutgassing() {
	// Compute the outgassing of all source facet
	wp.totalDesorbedMolecules = wp.finalOutgassingRate_Pa_m3_sec = wp.finalOutgassingRate = 0.0;
	Geometry *g = GetGeometry();

	for (int i = 0; i < g->GetNbFacet(); i++) {
		Facet *f = g->GetFacet(i);
		if (f->sh.desorbType != DES_NONE) { //there is a kind of desorption
			if (f->sh.useOutgassingFile) { //outgassing file
				for (int l = 0; l < (f->sh.outgassingMapWidth*f->sh.outgassingMapHeight); l++) {
					wp.totalDesorbedMolecules += wp.latestMoment * f->outgassingMap[l] / (1.38E-23*f->sh.temperature);
					wp.finalOutgassingRate += f->outgassingMap[l] / (1.38E-23*f->sh.temperature);
					wp.finalOutgassingRate_Pa_m3_sec += f->outgassingMap[l];
				}
			}
			else { //regular outgassing
				if (f->sh.outgassing_paramId == -1) { //constant outgassing
					wp.totalDesorbedMolecules += wp.latestMoment * f->sh.outgassing / (1.38E-23*f->sh.temperature);
					wp.finalOutgassingRate += f->sh.outgassing / (1.38E-23*f->sh.temperature);  //Outgassing molecules/sec
					wp.finalOutgassingRate_Pa_m3_sec += f->sh.outgassing;
				}
				else { //time-dependent outgassing
					wp.totalDesorbedMolecules += IDs[f->sh.IDid].back().second / (1.38E-23*f->sh.temperature);
					size_t lastIndex = parameters[f->sh.outgassing_paramId].GetSize() - 1;
					double finalRate_mbar_l_s = parameters[f->sh.outgassing_paramId].GetY(lastIndex);
					wp.finalOutgassingRate += finalRate_mbar_l_s *0.100 / (1.38E-23*f->sh.temperature); //0.1: mbar*l/s->Pa*m3/s
					wp.finalOutgassingRate_Pa_m3_sec += finalRate_mbar_l_s *0.100;
				}
			}
		}
	}
	if (mApp->globalSettings) mApp->globalSettings->UpdateOutgassing();

}

/**
* \brief Generate cumulative distribution function (CFD) for the velocity
* \param gasTempKelvins gas temperature in Kelvin
* \param gasMassGramsPerMol molar gas mass in grams per mol
* \param size amount of points/bins of the CFD
* \return CFD as a Vector containing a pair of double values (x value = speed_bin, y value = cumulated value)
*/
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

/**
* \brief Generate integrated desorption (ID) function
* \param paramId parameter identifier
* \return ID as a Vector containing a pair of double values (x value = moment, y value = desorption value)
*/
std::vector<std::pair<double, double>> Worker::Generate_ID(int paramId){
	std::vector<std::pair<double, double>> ID;
	//First, let's check at which index is the latest moment
	size_t indexBeforeLastMoment;
	for (indexBeforeLastMoment = 0; indexBeforeLastMoment < parameters[paramId].GetSize() &&
		(parameters[paramId].GetX(indexBeforeLastMoment) < wp.latestMoment); indexBeforeLastMoment++);
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

	//wp.latestMoment
	double valueAtlatestMoment = parameters[paramId].InterpolateY(wp.latestMoment,false);
	if (IsEqual(valueAtlatestMoment , parameters[paramId].GetY(indexBeforeLastMoment))) //two equal values follow, simple integration by multiplying
		ID.push_back(std::make_pair(wp.latestMoment,
		ID.back().second +
		(wp.latestMoment - parameters[paramId].GetX(indexBeforeLastMoment))*parameters[paramId].GetY(indexBeforeLastMoment)*0.100));
	else { //difficult case, we'll integrate by dividing two 5equal sections
		for (double delta = 0.0; delta < 1.0001; delta += 0.05) {
			double delta_t = wp.latestMoment - parameters[paramId].GetX(indexBeforeLastMoment);
			double time = parameters[paramId].GetX(indexBeforeLastMoment) + delta*delta_t;
			double avg_value = (parameters[paramId].GetY(indexBeforeLastMoment)*0.100 + parameters[paramId].InterpolateY(time, false)*0.100) / 2.0;
			ID.push_back(std::make_pair(time,
				ID.back().second +
				0.05*delta_t*avg_value));
		}
	}

	return ID;

}

/**
* \brief Get ID of a parameter (if it exists) for a corresponding name
* \param name name of the parameter that shall be looked up
* \return ID corresponding to the found parameter
*/
int Worker::GetParamId(const std::string name) {
	int foundId = -1;
	for (int i = 0; foundId == -1 && i < (int)parameters.size(); i++)
		if (name.compare(parameters[i].name) == 0) foundId = i;
	return foundId;
}


/*size_t Worker::ZipThreadProc(char* fileNameWithXML, char* fileNameWithXMLzip)
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

/**
* \brief Saves current facet hit counter from cache to results
*/
void Worker::SendFacetHitCounts(/*Dataport* dpHit*/) {
	LockMutex(results.mutex);
	size_t nbFacet = geom->GetNbFacet();
	for (size_t i = 0; i < nbFacet; i++) {
		Facet *f = geom->GetFacet(i);
		//*((FacetHitBuffer*)((BYTE*)dpHit->buff + f->sh.hitOffset )) = f->facetHitCache; //Only const.flow
		results.facetStates[i].momentResults[0].hits = f->facetHitCache; //Steady state
	}
	ReleaseMutex(results.mutex);
}
