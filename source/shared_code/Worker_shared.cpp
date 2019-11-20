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
#define NOMINMAX
//#include <Windows.h>

#include "Worker.h"
#include "Facet_shared.h"
#include "GLApp/GLApp.h"
#include "GLApp/GLMessageBox.h"
#include "GLApp/MathTools.h" //Min max
#include "GLApp/GLList.h"
#include <math.h>
#include <stdlib.h>
//#include <Process.h>
#include "GLApp/GLUnitDialog.h"
#include "LoadStatus.h"
#include "Simulation.h"
#ifdef MOLFLOW
#include "MolFlow.h"
#include "MolflowGeometry.h"
#include "FacetAdvParams.h"
#endif

#ifdef SYNRAD
#include "SynRad.h"
#include "SynradGeometry.h"
#endif

//#include <direct.h>
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

Worker::~Worker() {
	//CLOSEDP(dpHit);
	//CLOSEDP(dpControl);
	//CLOSEDP(dpLog);
	delete geom;
}

Geometry *Worker::GetGeometry() {
	return geom;
}

bool Worker::IsDpInitialized() {

	return /*(dpHit != NULL);*/ true;
}

char *Worker::GetCurrentFileName() {
	return fullFileName;
}

char *Worker::GetCurrentShortFileName() {

	static char ret[512];
	char *r = strrchr(fullFileName, '/');
	if (!r) r = strrchr(fullFileName, '\\');
	if (!r) strcpy(ret, fullFileName);
	else {
		r++;
		strcpy(ret, r);
	}

	return ret;

}

/*
char *Worker::GetShortFileName(char* longFileName) {

	static char ret[512];
	char *r = strrchr(longFileName, '/');
	if (!r) r = strrchr(longFileName, '\\');
	if (!r) strcpy(ret, longFileName);
	else   {
		r++;
		strcpy(ret, r);
	}

	return ret;

}
*/

void Worker::SetCurrentFileName(const char *fileName) {

	strcpy(fullFileName, fileName);
}

void Worker::ExportTextures(const char *fileName, int grouping, int mode, bool askConfirm, bool saveSelected) {



	// Read a file
	FILE *f = NULL;

	bool ok = true;
	
	//NativeFileDialog already asks confirmation
	/*
	if (askConfirm) {
		if (FileUtils::Exist(fileName)) {
			sprintf(tmp, "Overwrite existing file ?\n%s", fileName);
			ok = (GLMessageBox::Display(tmp, "Question", GLDLG_OK | GLDLG_CANCEL, GLDLG_ICONWARNING) == GLDLG_OK);
		}
	}
	*/
	
	if (ok) {
		f = fopen(fileName, "w");
		if (!f) {
			char tmp[256];
			sprintf(tmp, "Cannot open file for writing %s", fileName);
			throw Error(tmp);

		}
#ifdef MOLFLOW
		geom->ExportTextures(f, grouping, mode, /*dpHit*/ results, saveSelected, wp.sMode);
#endif
#ifdef SYNRAD
		geom->ExportTextures(f, grouping, mode, no_scans, /*dpHit*/ results, saveSelected);
#endif
		fclose(f);
	}

}

/*
void Worker::SendLeakCache(Dataport* dpHit) { //From worker.globalhitCache to dpHit
	if (dpHit) {
		LockMutex(hitsMutex);
		GlobalHitBuffer *gHits = (GlobalHitBuffer *)dpHit->buff;
		size_t nbCopy = Min(LEAKCACHESIZE, globalHitCache.leakCacheSize);
		gHits->leakCache = globalHitCache.leakCache;
		gHits->lastLeakIndex = nbCopy-1;
		gHits->leakCacheSize = globalHitCache.leakCacheSize;
		hitsMutex.unlock();
	}
}

void Worker::SendHitCache(Dataport* dpHit) { //From worker.globalhitCache to dpHit
	if (dpHit) {
		LockMutex(hitsMutex);
		GlobalHitBuffer *gHits = (GlobalHitBuffer *)dpHit->buff;
		size_t nbCopy = Min(HITCACHESIZE, globalHitCache.hitCacheSize);
		gHits->hitCache = globalHitCache.hitCache;
		gHits->lastHitIndex = nbCopy - 1;
		gHits->hitCacheSize = globalHitCache.hitCacheSize;
		hitsMutex.unlock();
	}
}
*/

void Worker::Stop_Public() {
	// Stop
	InnerStop(mApp->m_fTime);
	try {
		Stop();
		Update(mApp->m_fTime);
	}
	catch (Error &e) {
		GLMessageBox::Display(e.GetMsg(), "Error (Stop)", GLDLG_OK, GLDLG_ICONERROR);
	}
}

void  Worker::ReleaseHits() {
	results.mutex.unlock();
}

GlobalSimuState* Worker::GetHits() {
	try {
		if (needsReload) RealReload();
	}
	catch (Error &e) {
		GLMessageBox::Display(e.GetMsg(), "Error (Stop)", GLDLG_OK, GLDLG_ICONERROR);
	}
	if (LockMutex(results.mutex)) {
		if (results.initialized) {
			return &results;
		}
		else {
			ReleaseMutex(results.mutex);
			return NULL;
		}
	}
	else return NULL;
}

std::vector<ParticleLoggerItem>& Worker::GetLog() {
	try {
		if (needsReload) RealReload();
	}
	catch (Error &e) {
		GLMessageBox::Display(e.GetMsg(), "Error (Stop)", GLDLG_OK, GLDLG_ICONERROR);
	}
	LockMutex(logMutex);
	return log;
}

void Worker::ReleaseLog() {
	ReleaseMutex(logMutex);
}

void Worker::ThrowSubProcError(std::string message) {
	throw Error(message.c_str());
}

void Worker::ThrowSubProcError(const char *message) {

	char errMsg[1024];
	if (!message)
		sprintf(errMsg, "Bad response from sub process(es):\n%s", GetErrorDetails());
	else
		sprintf(errMsg, "%s\n%s", message, GetErrorDetails());
	throw Error(errMsg);

}

void Worker::Reload() {
	needsReload = true;
}

/*
void Worker::SetMaxDesorption(size_t max) {

	try {
		ResetStatsAndHits(0.0);

		desorptionLimit = max;
		Reload();

	}
	catch (Error &e) {
		GLMessageBox::Display(e.GetMsg(), "Error", GLDLG_OK, GLDLG_ICONERROR);
	}

}
*/

const char *Worker::GetErrorDetails() {

	std::ostringstream errorText;

	LockMutex(workerControl.mutex);
	for (size_t i = 0; i < ontheflyParams.nbProcess; i++) {
		//char tmp[512];
		std::thread::id defaultThreadId;
		auto tid = workerControl.threads[i].get_id();
		if (tid != defaultThreadId) {
			size_t st = workerControl.states[i];
			if (st == PROCESS_ERROR) {
				errorText << "[#" << i << "] Thread [" << tid << "] " << workerControl.states[i] << ": " << workerControl.statusStr[i] << "\n";
			}
			else {
				errorText << "[#" << i << "] Thread [" << tid << "] " << workerControl.states[i] << "\n";
			}
		}
		else {
			errorText << "[#" << i << "] Thread [???] Not started\n";
		}
	}
	ReleaseMutex(workerControl.mutex);

	return errorText.str().c_str();
}

bool Worker::Wait(size_t readyState, LoadStatus *statusWindow) {

	abortRequested = false;
	bool finished = false;
	bool error = false;

	int waitTime = 0;
	allDone = true;

	// Wait for completion
	while (!finished && !abortRequested) {

		finished = true;
		//AccessDataport(dpControl);
		LockMutex(workerControl.mutex);
		//WorkerControl *shMaster = (WorkerControl *)dpControl->buff;

		for (size_t i = 0; i < ontheflyParams.nbProcess; i++) {

			finished = finished & (workerControl.states[i] == readyState || workerControl.states[i] == PROCESS_ERROR || workerControl.states[i] == PROCESS_DONE);
			if (workerControl.states[i] == PROCESS_ERROR) {
				error = true;
			}
			allDone = allDone & (workerControl.states[i] == PROCESS_DONE);
		}
		//ReleaseDataport(dpControl);
		ReleaseMutex(workerControl.mutex);

		if (!finished) {

			if (statusWindow) {
				if (waitTime >= 500) {
					statusWindow->SetVisible(true);
				}
				statusWindow->SMPUpdate();
				mApp->DoEvents(); //Do a few refreshes during waiting for subprocesses
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(250));
			waitTime += 250;
		}
	}

	if (statusWindow) {
		statusWindow->SetVisible(false);
		statusWindow->EnableStopButton();
	}
	return finished && !error;

}

bool Worker::ExecuteAndWait(int command, size_t readyState, size_t param) {

	//if(!dpControl) return false;

	// Send command
	LockMutex(workerControl.mutex);
	//AccessDataport(dpControl);
	//WorkerControl *shMaster = (WorkerControl *)dpControl->buff;
	for (size_t i = 0; i < ontheflyParams.nbProcess; i++) {
		workerControl.states[i] = command;
		workerControl.cmdParam[i] = param;
	}
	ReleaseMutex(workerControl.mutex);
	//ReleaseDataport();

	std::this_thread::sleep_for(std::chrono::milliseconds(100));

	if (!mApp->loadStatus) mApp->loadStatus = new LoadStatus(this);
	bool result = Wait(readyState, mApp->loadStatus);
	//SAFE_DELETE(statusWindow);
	return result;
}

void Worker::ResetStatsAndHits(float appTime) {

	if (calcAC) {
		GLMessageBox::Display("Reset not allowed while calculating AC", "Error", GLDLG_OK, GLDLG_ICONERROR);
		return;
	}
	stopTime = 0.0f;
	startTime = 0.0f;
	simuTime = 0.0f;
	isRunning = false;
	if (ontheflyParams.nbProcess == 0)
		return;

	try {
		ResetWorkerStats();
		if (!ExecuteAndWait(COMMAND_RESET, PROCESS_READY))
			ThrowSubProcError();
		ClearHits(false);
		Update(appTime);
	}
	catch (Error &e) {
		GLMessageBox::Display(e.GetMsg(), "Error", GLDLG_OK, GLDLG_ICONERROR);
	}
}

void Worker::Stop() {

	if (ontheflyParams.nbProcess == 0)
		throw Error("No sub process found. (Simulation not available)");

	if (!ExecuteAndWait(COMMAND_PAUSE, PROCESS_READY))
		ThrowSubProcError();
}

void Worker::KillAll(bool keepDpHit) {
	if (ontheflyParams.nbProcess > 0) {
		std::unique_ptr<GLProgress> prg(new GLProgress("Stopping old threads...", "Restarting threads"));
		prg->SetVisible(true);
		bool result = ExecuteAndWait(COMMAND_EXIT, PROCESS_KILLED); //Wait until either the user request abort or subprocesses exit nicely
		for (size_t i = 0; i < ontheflyParams.nbProcess; i++) {
			prg->SetProgress((double)i / (double)ontheflyParams.nbProcess);
			if (workerControl.states[i] == PROCESS_KILLED) {
				workerControl.threads[i].join();
			}
			else {
				//Force kill
				auto myHandle = workerControl.threads[i].native_handle();
#ifdef _WIN32
				//Windows
				TerminateThread(myHandle, 1);
#else
				//Linux
				pthread_cancel(myHandle);
#endif
			}
			SAFE_DELETE(workerControl.simuPointers[i]);
		}
		prg->SetVisible(false);
	}
	if (!keepDpHit) results.Reset();

	ontheflyParams.nbProcess = 0;
	
}

void Worker::SetProcNumber(size_t n, bool keepDpHit) {

	// Kill all sub process
	KillAll(keepDpHit);

	LockMutex(workerControl.mutex);
	//Can't delete because contains mutex
	std::vector<size_t>(MAX_PROCESS).swap(workerControl.cmdParam);
	std::vector<size_t>(MAX_PROCESS).swap(workerControl.cmdParam2);
	std::vector<size_t>(MAX_PROCESS).swap(workerControl.states);
	std::vector<std::string>(MAX_PROCESS).swap(workerControl.statusStr);
	std::vector<std::thread>(MAX_PROCESS).swap(workerControl.threads);
	std::vector<Simulation*>(MAX_PROCESS).swap(workerControl.simuPointers);
	ReleaseMutex(workerControl.mutex);

	// Launch n subprocess
	for (size_t i = 0; i < n; i++) {
		
			workerControl.simuPointers[i] = new Simulation(this); //Create Simulation() instance
			workerControl.threads[i] = std::move(std::thread(&Simulation::mainLoop, workerControl.simuPointers[i], (int)i)); //Launch main loop
			//Set priority to idle
			auto myHandle = workerControl.threads[i].native_handle();
#ifdef _WIN32
			SetThreadPriority(myHandle, THREAD_PRIORITY_IDLE);
#else
			int policy;
			struct sched_param param;

			pthread_getschedparam(myHandle, &policy, &param);
			param.sched_priority = sched_get_priority_min(policy);
			pthread_setschedparam(myHandle, policy, &param);
			//Check! Some documentation says it's always 0
#endif
			//workerControl.threads[i].detach();
			//int a = i;
		
	}

	ontheflyParams.nbProcess = n;

	if (!mApp->loadStatus) mApp->loadStatus = new LoadStatus(this);
	bool result = Wait(PROCESS_READY, mApp->loadStatus);
	if (!result)
		ThrowSubProcError("Sub process(es) starting failure");
}

void Worker::RebuildTextures() {
	GlobalSimuState* resPtr;
	if (needsReload) RealReload();
	if (resPtr = GetHits()) {
		GlobalSimuState& results = *resPtr;
		if (mApp->needsTexture || mApp->needsDirection) try { geom->BuildFacetTextures(results, mApp->needsTexture, mApp->needsDirection, wp.sMode); }
		catch (Error &e) {
			ReleaseHits();
			throw e;
		}
		ReleaseHits();
	}
}

size_t Worker::GetProcNumber() {
	return ontheflyParams.nbProcess;
}

void Worker::Update(float appTime) {
	//Refreshes interface cache:
	//Global statistics, leak/hits, global histograms
	//Facet hits, facet histograms, facet angle maps
	//No cache for profiles, textures, directions (plotted directly from shared memory hit buffer)


	if (needsReload) RealReload();

	// Check calculation ending
	bool done = true;
	bool error = true;

	if (LockMutex(workerControl.mutex)) {
		int i = 0;
		for (int i = 0; i < ontheflyParams.nbProcess && done; i++) {
			done = done && (workerControl.states[i] == PROCESS_DONE);
			error = error && (workerControl.states[i] == PROCESS_ERROR);
#ifdef MOLFLOW
			if (workerControl.states[i] == PROCESS_RUNAC) calcACprg = workerControl.cmdParam[i];
#endif
		}
		ReleaseMutex(workerControl.mutex);
	}


	// End of simulation reached (Stop GUI)
	if ((error || done) && isRunning && appTime != 0.0f) {
		InnerStop(appTime);
		if (error) ThrowSubProcError();
	}

	// Retrieve hit count recording from the shared memory


	if (LockMutex(results.mutex)) {

		mApp->changedSinceSave = true;
		// Globals
		globalHitCache = results.globalHits;

		// Global hits and leaks
#ifdef MOLFLOW
		bool needsAngleMapStatusRefresh = false;
#endif

#ifdef SYNRAD

		if (nbDesorption && nbTrajPoints) {
			no_scans = (double)nbDesorption / (double)nbTrajPoints;
		}
		else {
			no_scans = 1.0;
		}
#endif
		if (results.globalHistograms.size() > 0) globalHistogramCache = results.globalHistograms[displayedMoment];

		// Refresh local facet hit cache for the displayed moment
		size_t nbFacet = geom->GetNbFacet();
		for (size_t i = 0; i < nbFacet; i++) {
			Facet *f = geom->GetFacet(i);
#ifdef SYNRAD
			memcpy(&(f->facetHitCache), buffer + f->sh.hitOffset, sizeof(FacetHitBuffer));
#endif
#ifdef MOLFLOW
			//memcpy(&(f->facetHitCache), buffer + f->sh.hitOffset + displayedMoment * sizeof(FacetHitBuffer), sizeof(FacetHitBuffer));
			f->facetHitCache = results.facetStates[i].momentResults[displayedMoment].hits;

			if (f->sh.anglemapParams.record) { //Recording, so needs to be updated
				if (f->selected && f->angleMapCache.empty()) needsAngleMapStatusRefresh = true; //Will update facetadvparams panel 
				//Retrieve angle map from hits dp
				f->angleMapCache = results.facetStates[i].recordedAngleMapPdf;
			}

#endif

			f->facetHistogramCache = results.facetStates[i].momentResults[displayedMoment].histogram;
		}
		try {
			if (mApp->needsTexture || mApp->needsDirection) geom->BuildFacetTextures(results, mApp->needsTexture, mApp->needsDirection, wp.sMode);
		}
		catch (Error &e) {
			GLMessageBox::Display(e.GetMsg(), "Error building texture", GLDLG_OK, GLDLG_ICONERROR);
			ReleaseMutex(results.mutex);
			return;
		}
#ifdef MOLFLOW
		if (mApp->facetAdvParams && mApp->facetAdvParams->IsVisible() && needsAngleMapStatusRefresh)
			mApp->facetAdvParams->Refresh(geom->GetSelectedFacets());
#endif
		ReleaseMutex(results.mutex);
	}

}

void Worker::GetProcStatus(std::vector<size_t>& states, std::vector<std::string>& statusStrings) {

	if (ontheflyParams.nbProcess == 0) return;

	LockMutex(workerControl.mutex);
	states = workerControl.states;
	statusStrings = workerControl.statusStr;
	ReleaseMutex(workerControl.mutex);

}

std::vector<std::vector<std::string>> Worker::ImportCSV_string(FileReader *file) {
	std::vector<std::vector<string>> table; //reset table
	do {
		std::vector<std::string> row;
		std::string line = file->ReadLine();
		std::stringstream token;
		size_t cursor = 0;
		size_t length = line.length();
		while (cursor < length) {
			char c = line[cursor];
			if (c == ',') {
				row.push_back(token.str());
				token.str(""); token.clear();
			}
			else {
				token << c;
			}
			cursor++;
		}
		if (token.str().length() > 0) row.push_back(token.str());

		table.push_back(row);
	} while (!file->IsEof());
	return table;
}

std::vector<std::vector<double>> Worker::ImportCSV_double(FileReader *file) {
	std::vector<std::vector<double>> table;
	do {
		std::vector<double> currentRow;
		do {
			currentRow.push_back(file->ReadDouble());
			if (!file->IsEol()) file->ReadKeyword(",");
		} while (!file->IsEol());
		table.push_back(currentRow);
	} while (!file->IsEof());
	return table;
}

void Worker::ChangeSimuParams() { //Send simulation mode changes to subprocesses without reloading the whole geometry
	if (ontheflyParams.nbProcess == 0 || !geom->IsLoaded()) return;
	if (needsReload) RealReload(); //Sync (number of) regions


	GLProgress *progressDlg = new GLProgress("Waiting for mutex...", "Waiting to connect to log...");
	progressDlg->SetVisible(true);
	progressDlg->SetProgress(0.0);
	/*
	// Create the temporary geometry shared structure
	size_t loadSize = sizeof(OntheflySimulationParams);
	*/
#ifdef SYNRAD
	//loadSize += regions.size() * sizeof(bool); //Show photons or not
#endif

	//To do: only close if parameters changed
	//CLOSEDP(dpLog);
	LockMutex(logMutex);
	log.clear();
	ReleaseMutex(logMutex);
	/*
	progressDlg->SetMessage("Waiting for subprocesses to release log dataport...");
	if (!ExecuteAndWait(COMMAND_RELEASEDPLOG, isRunning ? PROCESS_RUN : PROCESS_READY, isRunning ? PROCESS_RUN : PROCESS_READY)) {
		char errMsg[1024];
		sprintf(errMsg, "Subprocesses didn't release dpLog handle:\n%s", GetErrorDetails());
		GLMessageBox::Display(errMsg, "Warning (Updateparams)", GLDLG_OK, GLDLG_ICONWARNING);

		progressDlg->SetVisible(false);
		SAFE_DELETE(progressDlg);
		return;
	}
	*/
	/*
	if (ontheflyParams.enableLogging) {
		size_t logDpSize = sizeof(size_t) + ontheflyParams.logLimit * sizeof(ParticleLoggerItem);
		dpLog = CreateDataport(logDpName, logDpSize);
		if (!dpLog) {
			progressDlg->SetVisible(false);
			SAFE_DELETE(progressDlg);
			throw Error("Failed to create 'dpLog' dataport.\nMost probably out of memory.\nReduce number of logged particles in Particle Logger.");
		}
		//Fills values with 0
	}*/
	/*
	Dataport *loader = CreateDataport(loadDpName, loadSize);
	if (!loader) {
		progressDlg->SetVisible(false);
		SAFE_DELETE(progressDlg);
		throw Error("Failed to create 'loader' dataport.\nMost probably out of memory.\nReduce number of subprocesses or texture size.");
	}
	progressDlg->SetMessage("Accessing dataport...");
	AccessDataportTimed(loader, 1000);
	progressDlg->SetMessage("Assembling parameters to pass...");

	BYTE* buffer = (BYTE*)loader->buff;
	WRITEBUFFER(ontheflyParams, OntheflySimulationParams);
	*/


#ifdef SYNRAD
	/*
	for (size_t i = 0; i < regions.size(); i++) {
		WRITEBUFFER(regions[i].params.showPhotons, bool);
	}
	*/
#endif
	/*
	progressDlg->SetMessage("Releasing dataport...");
	ReleaseDataport(loader);
	*/
	// Pass to workers
	progressDlg->SetMessage("Waiting for subprocesses to read mode...");
	if (!ExecuteAndWait(COMMAND_UPDATEPARAMS, isRunning ? PROCESS_RUN : PROCESS_READY, isRunning ? PROCESS_RUN : PROCESS_READY)) {
		//CLOSEDP(loader);
		char errMsg[1024];
		sprintf(errMsg, "Failed to send params to sub process:\n%s", GetErrorDetails());
		GLMessageBox::Display(errMsg, "Warning (Updateparams)", GLDLG_OK, GLDLG_ICONWARNING);

		progressDlg->SetVisible(false);
		SAFE_DELETE(progressDlg);
		return;
	}

	//progressDlg->SetMessage("Closing dataport...");
	//CLOSEDP(loader);
	progressDlg->SetVisible(false);
	SAFE_DELETE(progressDlg);

#ifdef SYNRAD
	//Reset leak and hit cache
	leakCacheSize = 0;
	SetLeakCache(leakCache, &leakCacheSize, dpHit); //will only write leakCacheSize
	hitCacheSize = 0;
	SetHitCache(hitCache, &hitCacheSize, dpHit); //will only write hitCacheSize
#endif
}