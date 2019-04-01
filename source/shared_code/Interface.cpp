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
#include "Interface.h"
//#include <direct.h> //_getcwd()
//#include <io.h> // Check for recovery

#include <filesystem>
#include <string>
#include "AppUpdater.h"

//#include "GLApp/GLFileBox.h"
#include "NativeFileDialog/molflow_wrapper/nfd_wrapper.h"
#include "GLApp/GLMessageBox.h"
#include "GLApp/GLInputBox.h"
#include "GLApp/GLSaveDialog.h"
#include "GLApp/GLToolkit.h"
#include "GLApp/MathTools.h" //IDX
#include "RecoveryDialog.h"
#include "Facet_shared.h"

#include "GLApp/GLTextField.h"
#include "GLApp/GLToggle.h"
#include "GLApp/GLLabel.h"
#include "GLApp/GLButton.h"
#include "GLApp/GLTitledPanel.h"
#include "GLApp/GLList.h"
#include "GLApp/GLCombo.h"
#include "GLApp/GLMenuBar.h"
#include "GLApp/GLWindowManager.h"
#include "GLApp/GLToggle.h"

//Windows
#include "GeometryViewer.h"
#include "CollapseSettings.h"
#include "HistogramSettings.h"
#include "HistogramPlotter.h"
#include "MoveVertex.h"
#include "ScaleVertex.h"
#include "ScaleFacet.h"
#include "MoveFacet.h"
#include "CreateShape.h"
#include "ExtrudeFacet.h"
#include "MirrorFacet.h"
#include "MirrorVertex.h"
#include "SplitFacet.h"
#include "BuildIntersection.h"
#include "RotateFacet.h"
#include "RotateVertex.h"
#include "FacetCoordinates.h"
#include "VertexCoordinates.h"
#include "SmartSelection.h"
#include "LoadStatus.h"
#include "SelectDialog.h"
#include "SelectTextureType.h"
#include "AlignFacet.h"
#include "AddVertex.h"
#include "FormulaEditor.h"
#include "ParticleLogger.h"

#include "NativeFileDialog/nfd.h"

//Updater
#include <PugiXML/pugixml.hpp>
#include "File.h" //File utils (Get extension, etc)

#include "versionId.h"


extern Worker worker;
extern std::vector<string> formulaPrefixes;
//extern const char* appTitle;

/*
extern const char *fileLFilters;
extern const char *fileInsFilters;
extern const char *fileSFilters;
extern const char *fileDesFilters;
*/
extern std::string fileLFilters;
extern std::string fileInsFilters;
extern std::string fileSaveFilters;
extern std::string fileSelFilters;
extern std::string fileTexFilters;
extern std::string fileDesFilters;


extern int   cSize;
extern int   cWidth[];
extern const char *cName[];


Interface::Interface() {
	//Get number of cores
	numCPU = std::thread::hardware_concurrency();
	appUpdater = NULL; //We'll initialize later, when the app name and version id is known

	antiAliasing = true;
	whiteBg = false;
	leftHandedView = false;
	autoUpdateFormulas = true;
	compressSavedFiles = true;
	/*double wp.gasMass=28;
	double totalOutgassing=0.0; //total outgassing in Pa*m3/sec (internally everything is in SI units)
	double totalInFlux = 0.0; //total incoming molecules per second. For anisothermal system, it is (totalOutgassing / Kb / T)*/
	autoSaveFrequency = 10.0; //in minutes
	autoSaveSimuOnly = false;
	autosaveFilename = "";
	autoFrameMove = true;

	lastSaveTime = 0.0f;
	lastSaveTimeSimu = 0.0f;
	changedSinceSave = false;
	//lastHeartBeat=0.0f;
	nbDesStart = 0;
	nbHitStart = 0;

	lastUpdate = 0.0;
	//nbFormula = 0;
	nbRecent = 0;

	nbView = 0;
	idView = 0;
	idSelection = 0;

#ifdef _DEBUG
	nbProc = 1;
#else
	Saturate(numCPU, 1, Min(MAX_PROCESS, (size_t)16)); //limit the auto-detected processes to the maximum available, at least one, and max 16 (above it speed improvement not obvious)
	nbProc = numCPU;
#endif

	curViewer = 0;
	strcpy(currentDir, ".");
	strcpy(currentSelDir, ".");
	//memset(formulas, 0, sizeof formulas);

	//formulaSettings = NULL;
	collapseSettings = NULL;
	histogramSettings = NULL;
	histogramPlotter = NULL;
	moveVertex = NULL;
	scaleVertex = NULL;
	scaleFacet = NULL;
	selectDialog = NULL;
	selectTextureType = NULL;
	moveFacet = NULL;
	createShape = NULL;
	extrudeFacet = NULL;
	mirrorFacet = NULL;
	mirrorVertex = NULL;
	splitFacet = NULL;
	buildIntersection = NULL;
	rotateFacet = NULL;
	rotateVertex = NULL;
	alignFacet = NULL;
	addVertex = NULL;
	loadStatus = NULL;
	facetCoordinates = NULL;
	vertexCoordinates = NULL;
	smartSelection = NULL;
	updateCheckDialog = NULL;
	updateFoundDialog = NULL;
	updateLogWindow = NULL;
	particleLogger = NULL;

	m_strWindowTitle = appTitle;
	wnd->SetBackgroundColor(212, 208, 200);
	m_bResizable = true;
	m_minScreenWidth = 800;
	m_minScreenHeight = 600;
	coplanarityTolerance = 1e-8;
	largeAreaThreshold = 1.0;
	planarityThreshold = 1e-5;
}

void Interface::UpdateViewerFlags() {
	viewer[curViewer]->showNormal = showNormal->GetState();
	viewer[curViewer]->showRule = showRule->GetState();
	viewer[curViewer]->showUV = showUV->GetState();
	viewer[curViewer]->showLeak = showLeak->GetState();
	viewer[curViewer]->showHit = showHit->GetState();
	viewer[curViewer]->showLine = showLine->GetState();
	viewer[curViewer]->showVolume = showVolume->GetState();
	viewer[curViewer]->showTexture = showTexture->GetState();
	bool neededTexture = needsTexture;
	CheckNeedsTexture();

	if (!needsTexture && neededTexture) { //We just disabled mesh
		worker.GetGeometry()->ClearFacetTextures();
	}
	else if (needsTexture && !neededTexture) { //We just enabled mesh
		worker.RebuildTextures();
	}
	viewer[curViewer]->showFilter = showFilter->GetState();
	viewer[curViewer]->showVertex = showVertex->GetState();
	viewer[curViewer]->showIndex = showIndex->GetState();
	//worker.Update(0.0);
}

void Interface::ResetSimulation(bool askConfirm) {

	bool ok = true;
	if (askConfirm)
		ok = GLMessageBox::Display("Reset simulation ?", "Question", GLDLG_OK | GLDLG_CANCEL, GLDLG_ICONINFO) == GLDLG_OK;

	if (ok) {
		worker.ResetStatsAndHits(m_fTime);

		nbDesStart = 0;
		nbHitStart = 0;
	}
	UpdatePlotters();
}

void Interface::UpdateStructMenu() {

	char tmp[128];
	Geometry *geom = worker.GetGeometry();

	structMenu->Clear();
	structMenu->Add("New structure...", MENU_VIEW_NEWSTRUCT);
	structMenu->Add("Delete structure...", MENU_VIEW_DELSTRUCT);
	structMenu->Add(NULL); //Separator
	structMenu->Add("Show all", MENU_VIEW_STRUCTURE, SDLK_F1, CTRL_MODIFIER);
	structMenu->Add("Show previous", MENU_VIEW_PREVSTRUCT, SDLK_F11, CTRL_MODIFIER);
	structMenu->Add("Show next", MENU_VIEW_NEXTSTRUCT, SDLK_F12, CTRL_MODIFIER);
	structMenu->Add(NULL); //Separator

	for (int i = 0; i < geom->GetNbStructure(); i++) {
		sprintf(tmp, "Show #%d (%s)", i + 1, geom->GetStructureName(i));
		if (i < 10)
			structMenu->Add(tmp, MENU_VIEW_STRUCTURE + (i + 1), SDLK_F1 + i + 1, CTRL_MODIFIER);
		else
			structMenu->Add(tmp, MENU_VIEW_STRUCTURE + (i + 1));
	}

	structMenu->SetCheck(MENU_VIEW_STRUCTURE + geom->viewStruct + 1, true);

	UpdateTitle();
}

void Interface::UpdateCurrentDir(const char *fileName) {

	strncpy(currentDir, fileName, 1024);
	char *dp = strrchr(currentDir, '\\');
	if (!dp) dp = strrchr(currentDir, '/');
	if (dp) *dp = 0;

}

void Interface::UpdateCurrentSelDir(const char *fileName) {

	strncpy(currentDir, fileName, 1024);
	char *dp = strrchr(currentDir, '\\');
	if (!dp) dp = strrchr(currentDir, '/');
	if (dp) *dp = 0;

}

void Interface::UpdateTitle() {

	std::string title;

	Geometry *geom = worker.GetGeometry();

	if (!geom->IsLoaded()) {
		title = appTitle;
	}
	else {
		if (geom->viewStruct < 0) {
			title = appTitle + " [" + worker.GetCurrentShortFileName() + "]";
		}
		else {
			title = appTitle + " [" + worker.GetCurrentShortFileName() + ": Struct #" + std::to_string(geom->viewStruct + 1) +" " + geom->GetStructureName(geom->viewStruct) +"]";
		}
	}

	SetTitle(title);

}

// Name: FormatInt()
// Desc: Format an integer in K,M,G,..

char* Interface::FormatInt(size_t v, const char *unit)
{

	double x = (double)v;

	static char ret[64];
	if (x < 1E3) {
		sprintf(ret, "%g %s", (double)x, unit);
	}
	else if (x < 1E6) {
		sprintf(ret, "%.1f K%s", x / 1E3, unit);
	}
	else if (x < 1E9) {
		sprintf(ret, "%.2f M%s", x / 1E6, unit);
	}
	else if (x < 1E12) {
		sprintf(ret, "%.2f G%s", x / 1E9, unit);
	}
	else {
		sprintf(ret, "%.2f T%s", x / 1E12, unit);
	}

	return ret;

}

// Name: FormatPS()
// Desc: Format a double in K,M,G,.. per sec

char *Interface::FormatPS(double v, const char *unit)
{

	static char ret[64];
	if (v < 1000.0) {
		sprintf(ret, "%.1f %s/s", v, unit);
	}
	else if (v < 1000000.0) {
		sprintf(ret, "%.1f K%s/s", v / 1000.0, unit);
	}
	else if (v < 1000000000.0) {
		sprintf(ret, "%.1f M%s/s", v / 1000000.0, unit);
	}
	else {
		sprintf(ret, "%.1f G%s/s", v / 1000000000.0, unit);
	}

	return ret;

}

// Name: FormatSize()
// Desc: Format a double in K,M,G,.. per sec

char *Interface::FormatSize(size_t size)
{

	static char ret[64];
	if (size < 1024UL) {
		sprintf(ret, "%zd Bytes", size);
	}
	else if (size < 1048576UL) {
		sprintf(ret, "%.1f KB", (double)size / 1024.0);
	}
	else if (size < 1073741824UL) {
		sprintf(ret, "%.1f MB", (double)size / 1048576.0);
	}
	else {
		sprintf(ret, "%.1f GB", (double)size / 1073741824.0);
	}

	return ret;

}

// Name: FormatTime()
// Desc: Format time in HH:MM:SS

char* Interface::FormatTime(float t) {
	static char ret[64];
	int nbSec = (int)(t + 0.5f);
	sprintf(ret, "%02d:%02d:%02d", nbSec / 3600, (nbSec % 3600) / 60, nbSec % 60);
	return ret;
}

void Interface::LoadSelection(const char *fName) {

	std::string fileName = fName;
	FileReader *f = NULL;

	if (fileName.empty()) {
		fileName = NFD_OpenFile_Cpp(fileSelFilters, "");
		if (fileName.empty()) return;
	}
	

	try {

		Geometry *geom = worker.GetGeometry();
		geom->UnselectAll();
		size_t nbFacet = geom->GetNbFacet();

		f = new FileReader(fileName);
		while (!f->IsEof()) {
			int s = f->ReadInt();
			if (s >= 0 && s < nbFacet) geom->SelectFacet(s);
		}
		geom->UpdateSelection();

		UpdateFacetParams(true);
	}
	catch (Error &e) {

		char errMsg[512];
		sprintf(errMsg, "%s\nFile:%s", e.GetMsg(), fileName.c_str());
		GLMessageBox::Display(errMsg, "Error", GLDLG_OK, GLDLG_ICONERROR);

	}

	SAFE_DELETE(f);
	changedSinceSave = false;

}

void Interface::SaveSelection() {

	FileWriter *f = NULL;
	Geometry *geom = worker.GetGeometry();
	if (geom->GetNbSelectedFacets() == 0) return;
	GLProgress *progressDlg2 = new GLProgress("Saving file", "Please wait");
	progressDlg2->SetProgress(0.5);
	progressDlg2->SetVisible(true);
	//GLWindowManager::Repaint();

	std::string fileName = NFD_SaveFile_Cpp(fileSelFilters, "");
	//FILENAME *fn = GLFileBox::SaveFile(currentSelDir, worker.GetCurrentShortFileName(), "Save selection", fileSelFilters, 0);

	if (!fileName.empty()) {

		try {

			if (FileUtils::GetExtension(fileName).empty()) fileName = fileName + ".sel";

			f = new FileWriter(fileName);
			//int nbSelected = geom->GetNbSelectedFacets();
			size_t nbFacet = geom->GetNbFacet();
			for (size_t i = 0; i < nbFacet; i++) {
				if (geom->GetFacet(i)->selected) f->Write(i, "\n");
			}

		}
		catch (Error &e) {
			char errMsg[512];
			sprintf(errMsg, "%s\nFile:%s", e.GetMsg(), fileName.c_str());
			GLMessageBox::Display(errMsg, "Error", GLDLG_OK, GLDLG_ICONERROR);
		}

		SAFE_DELETE(f);

	}
	progressDlg2->SetVisible(false);
	SAFE_DELETE(progressDlg2);
	changedSinceSave = false;
}

void Interface::ExportSelection() {

	Geometry *geom = worker.GetGeometry();
	if (geom->GetNbSelectedFacets() == 0) {
		GLMessageBox::Display("Empty selection", "Error", GLDLG_OK, GLDLG_ICONERROR);
		return;
	}

	//FILENAME *fn = GLFileBox::SaveFile(currentDir, worker.GetCurrentShortFileName(), "Export selection", fileSFilters, 0);
	std::string fileName = NFD_SaveFile_Cpp(fileSaveFilters, "");
	GLProgress *progressDlg2 = new GLProgress("Saving file...", "Please wait");
	progressDlg2->SetProgress(0.0);
	progressDlg2->SetVisible(true);
	//GLWindowManager::Repaint();
	if (!fileName.empty()) {

		try {
			worker.SaveGeometry(fileName.c_str(), progressDlg2, true, true);
			AddRecent(fileName.c_str());
			//UpdateCurrentDir(fn->fullName);
			//UpdateTitle();
		}
		catch (Error &e) {
			char errMsg[512];
			sprintf(errMsg, "%s\nFile:%s", e.GetMsg(), fileName.c_str());
			GLMessageBox::Display(errMsg, "Error", GLDLG_OK, GLDLG_ICONERROR);
		}

	}

	progressDlg2->SetVisible(false);
	SAFE_DELETE(progressDlg2);
}

// Name: UpdateModelParams()
// Desc: Update displayed model parameter on geometry ghange

void Interface::UpdateModelParams() {

	Geometry *geom = worker.GetGeometry();
	char tmp[256];
	double sumArea = 0;
	facetList->SetSize(cSize, geom->GetNbFacet(), false, true);
	facetList->SetColumnWidths((int*)cWidth);
	facetList->SetColumnLabels((const char**)cName);
	UpdateFacetHits(true);
	UpdateFacetlistSelected();
	AxisAlignedBoundingBox bb = geom->GetBB();

	for (int i = 0; i < geom->GetNbFacet(); i++) {
		Facet *f = geom->GetFacet(i);
		if (f->sh.area>0) sumArea += f->GetArea();
	}

	sprintf(tmp, "V:%zd F:%zd Dim:(%g,%g,%g) Area:%g", geom->GetNbVertex(), geom->GetNbFacet(),
		(bb.max.x - bb.min.x), (bb.max.y - bb.min.y), (bb.max.z - bb.min.z), sumArea);
	geomNumber->SetText(tmp);

}

void Interface::AnimateViewerChange(int next) {

	double xs1, ys1, xs2, ys2;
	double xe1, ye1, xe2, ye2;
	int sx = m_screenWidth - 205;
	int fWidth = m_screenWidth - 215;
	int fHeight = m_screenHeight - 27;
	int Width2 = fWidth / 2 - 1;
	int Height2 = fHeight / 2 - 1;

	// Reset to layout and make all visible

	for (int i = 0; i < MAX_VIEWER; i++)  viewer[i]->SetVisible(true);
	viewer[0]->SetBounds(3, 3, Width2, Height2);
	viewer[1]->SetBounds(6 + Width2, 3, Width2, Height2);
	viewer[2]->SetBounds(3, 6 + Height2, Width2, Height2);
	viewer[3]->SetBounds(6 + Width2, 6 + Height2, Width2, Height2);

	if (modeSolo) {

		// Go from single to layout
		xs1 = (double)3;
		ys1 = (double)3;
		xs2 = (double)fWidth + xs1;
		ys2 = (double)fHeight + ys1;

		switch (next) {
		case 0:
			xe1 = (double)(3);
			ye1 = (double)(3);
			break;
		case 1:
			xe1 = (double)(5 + Width2);
			ye1 = (double)(3);
			break;
		case 2:
			xe1 = (double)(3);
			ye1 = (double)(5 + Height2);
			break;
		case 3:
			xe1 = (double)(5 + Width2);
			ye1 = (double)(5 + Height2);
			break;
		}

		xe2 = (double)(Width2)+xe1;
		ye2 = (double)(Height2)+ye1;

	}
	else {

		// Go from layout to single
		xe1 = (double)3;
		ye1 = (double)3;
		xe2 = (double)fWidth + xe1;
		ye2 = (double)fHeight + ye1;

		switch (next) {
		case 0:
			xs1 = (double)(3);
			ys1 = (double)(3);
			break;
		case 1:
			xs1 = (double)(5 + Width2);
			ys1 = (double)(3);
			break;
		case 2:
			xs1 = (double)(3);
			ys1 = (double)(5 + Height2);
			break;
		case 3:
			xs1 = (double)(5 + Width2);
			ys1 = (double)(5 + Height2);
			break;
		}

		xs2 = (double)(Width2)+xs1;
		ys2 = (double)(Height2)+ys1;

	}

	double t0 = (double)SDL_GetTicks() / 1000.0;
	double t1 = t0;
	double T = 0.15;

	while ((t1 - t0) < T) {
		double t = (t1 - t0) / T;
		int x1 = (int)(xs1 + t*(xe1 - xs1) + 0.5);
		int y1 = (int)(ys1 + t*(ye1 - ys1) + 0.5);
		int x2 = (int)(xs2 + t*(xe2 - xs2) + 0.5);
		int y2 = (int)(ys2 + t*(ye2 - ys2) + 0.5);
		viewer[next]->SetBounds(x1, y1, x2 - x1, y2 - y1);
		wnd->Paint();
		// Overides moving component
		viewer[next]->Paint();
		// Paint modeless
		int n;
		n = GLWindowManager::GetNbWindow();
		GLWindowManager::RepaintRange(1, n);
		t1 = (double)SDL_GetTicks() / 1000.0;
	}

	modeSolo = !modeSolo;
	SelectViewer(next);

}

void Interface::UpdateViewerPanel() {

	showNormal->SetState(viewer[curViewer]->showNormal);
	showRule->SetState(viewer[curViewer]->showRule);
	showUV->SetState(viewer[curViewer]->showUV);
	showLeak->SetState(viewer[curViewer]->showLeak);
	showHit->SetState(viewer[curViewer]->showHit);
	showVolume->SetState(viewer[curViewer]->showVolume);
	showLine->SetState(viewer[curViewer]->showLine);
	showTexture->SetState(viewer[curViewer]->showTexture);
	showFilter->SetState(viewer[curViewer]->showFilter);
	showVertex->SetState(viewer[curViewer]->showVertex);
	showIndex->SetState(viewer[curViewer]->showIndex);

	// Force all views to have the same showColormap
	//viewer[1]->showColormap = viewer[0]->showColormap;
	//viewer[2]->showColormap = viewer[0]->showColormap;
	//viewer[3]->showColormap = viewer[0]->showColormap;
	//worker.GetGeometry()->texColormap = viewer[0]->showColormap;

}

void Interface::SelectViewer(int s) {

	curViewer = s;
	for (int i = 0; i < MAX_VIEWER; i++) viewer[i]->SetSelected(i == curViewer);
	UpdateViewerPanel();

}

void Interface::Place3DViewer() {

	int sx = m_screenWidth - 205;

	// 3D Viewer ----------------------------------------------
	int fWidth = m_screenWidth - 215;
	int fHeight = m_screenHeight - 27;
	int Width2 = fWidth / 2 - 1;
	int Height2 = fHeight / 2 - 1;

	if (modeSolo) {
		for (int i = 0; i < MAX_VIEWER; i++)
			viewer[i]->SetVisible(false);
		viewer[curViewer]->SetBounds(3, 3, fWidth, fHeight);
		viewer[curViewer]->SetVisible(true);
	}
	else {
		for (int i = 0; i < MAX_VIEWER; i++)
			viewer[i]->SetVisible(true);
		viewer[0]->SetBounds(3, 3, Width2, Height2);
		viewer[1]->SetBounds(6 + Width2, 3, Width2, Height2);
		viewer[2]->SetBounds(3, 6 + Height2, Width2, Height2);
		viewer[3]->SetBounds(6 + Width2, 6 + Height2, Width2, Height2);
	}
}

void Interface::UpdateViewers() {
	for (int i = 0; i < MAX_VIEWER; i++)
		viewer[i]->UpdateMatrix();
}

void Interface::SetFacetSearchPrg(bool visible, const char *text) {
	static Uint32 lastUpd = 0;
	Uint32 now = SDL_GetTicks();
	if (!visible || (now - lastUpd > 500)) {
		for (int i = 0; i < MAX_VIEWER; i++) {
			viewer[i]->facetSearchState->SetVisible(visible);
			viewer[i]->facetSearchState->SetText(text);
		}
		GLWindowManager::Repaint();
		lastUpd = now;
	}
}

int Interface::OnExit() {
	SaveConfig();
	if (appUpdater) {
		appUpdater->IncreaseSessionCount();
	}
	remove(autosaveFilename.c_str());
	auto cwd = std::filesystem::current_path();
	auto tempDir = cwd / "tmp";
	std::filesystem::remove_all(tempDir);
	return GL_OK;
}

void Interface::OneTimeSceneInit_shared_pre() {
	GLToolkit::SetIcon32x32("images/app_icon.png");

	for (int i = 0; i < MAX_VIEWER; i++) {
		viewer[i] = new GeometryViewer(i);
		Add(viewer[i]);
	}
	modeSolo = true;
	//nbSt = 0;

	menu = new GLMenuBar(0);
	wnd->SetMenuBar(menu);
	menu->Add("File");
	menu->GetSubMenu("File")->Add("New, empty geometry", MENU_FILE_NEW);
	menu->GetSubMenu("File")->Add("&Load", MENU_FILE_LOAD, SDLK_o, CTRL_MODIFIER);
	menu->GetSubMenu("File")->Add("Load recent");
	menu->GetSubMenu("File")->Add(NULL); //separator
	menu->GetSubMenu("File")->Add("&Insert geometry");
	menu->GetSubMenu("File")->GetSubMenu("Insert geometry")->Add("&To current structure", MENU_FILE_INSERTGEO);
	menu->GetSubMenu("File")->GetSubMenu("Insert geometry")->Add("&To new structure", MENU_FILE_INSERTGEO_NEWSTR);
	menu->GetSubMenu("File")->Add(NULL); //separator
	menu->GetSubMenu("File")->Add("&Save", MENU_FILE_SAVE, SDLK_s, CTRL_MODIFIER);
	menu->GetSubMenu("File")->Add("&Save as", MENU_FILE_SAVEAS);
	menu->GetSubMenu("File")->Add(NULL); //separator
	
	menu->GetSubMenu("File")->Add("Export selected facets", MENU_FILE_EXPORT_SELECTION);
	
	menu->GetSubMenu("File")->Add("Export selected profiles", MENU_FILE_EXPORTPROFILES);	

	menu->GetSubMenu("File")->SetIcon(MENU_FILE_SAVE, 83, 24);
	menu->GetSubMenu("File")->SetIcon(MENU_FILE_SAVEAS, 101, 24);
	menu->GetSubMenu("File")->SetIcon(MENU_FILE_LOAD, 65, 24);//65,24
	//menu->GetSubMenu("File")->SetIcon(MENU_FILE_LOADRECENT,83,24);//83,24

	menu->Add("Selection");
	menu->GetSubMenu("Selection")->Add("Smart Select facets...", MENU_SELECTION_SMARTSELECTION, SDLK_s, ALT_MODIFIER);
	menu->GetSubMenu("Selection")->Add(NULL); // Separator
	menu->GetSubMenu("Selection")->Add("Select All Facets", MENU_FACET_SELECTALL, SDLK_a, CTRL_MODIFIER);
	menu->GetSubMenu("Selection")->Add("Select by Facet Number...", MENU_SELECTION_SELECTFACETNUMBER, SDLK_n, ALT_MODIFIER);
	menu->GetSubMenu("Selection")->Add("Select Sticking", MENU_FACET_SELECTSTICK);
	menu->GetSubMenu("Selection")->Add("Select Transparent", MENU_FACET_SELECTTRANS);
	menu->GetSubMenu("Selection")->Add("Select 2 sided", MENU_FACET_SELECT2SIDE);
	menu->GetSubMenu("Selection")->Add("Select Texture", MENU_FACET_SELECTTEXT);
	menu->GetSubMenu("Selection")->Add("Select by Texture type...", MENU_SELECTION_TEXTURETYPE);
	menu->GetSubMenu("Selection")->Add("Select Profile", MENU_FACET_SELECTPROF);

	menu->GetSubMenu("Selection")->Add(NULL); // Separator
	menu->GetSubMenu("Selection")->Add("Select Abs > 0", MENU_FACET_SELECTABS);
	menu->GetSubMenu("Selection")->Add("Select Hit > 0", MENU_FACET_SELECTHITS);
	menu->GetSubMenu("Selection")->Add("Select large with no hits", MENU_FACET_SELECTNOHITS_AREA);
	menu->GetSubMenu("Selection")->Add(NULL); // Separator

	menu->GetSubMenu("Selection")->Add("Select link facets", MENU_FACET_SELECTDEST);
	menu->GetSubMenu("Selection")->Add("Select teleport facets", MENU_FACET_SELECTTELEPORT);
	menu->GetSubMenu("Selection")->Add("Select non planar facets", MENU_FACET_SELECTNONPLANAR);
	menu->GetSubMenu("Selection")->Add("Select non simple facets", MENU_FACET_SELECTERR);
	//menu->GetSubMenu("Selection")->Add(NULL); // Separator
	//menu->GetSubMenu("Selection")->Add("Load selection",MENU_FACET_LOADSEL);
	//menu->GetSubMenu("Selection")->Add("Save selection",MENU_FACET_SAVESEL);
	menu->GetSubMenu("Selection")->Add("Invert selection", MENU_FACET_INVERTSEL, SDLK_i, CTRL_MODIFIER);
	menu->GetSubMenu("Selection")->Add(NULL); // Separator 

	menu->GetSubMenu("Selection")->Add("Memorize selection to");
	memorizeSelectionsMenu = menu->GetSubMenu("Selection")->GetSubMenu("Memorize selection to");
	memorizeSelectionsMenu->Add("Add new...", MENU_SELECTION_ADDNEW, SDLK_w, CTRL_MODIFIER);
	memorizeSelectionsMenu->Add(NULL); // Separator

	menu->GetSubMenu("Selection")->Add("Select memorized");
	selectionsMenu = menu->GetSubMenu("Selection")->GetSubMenu("Select memorized");

	menu->GetSubMenu("Selection")->Add("Clear memorized", MENU_SELECTION_CLEARSELECTIONS);
	clearSelectionsMenu = menu->GetSubMenu("Selection")->GetSubMenu("Clear memorized");
	clearSelectionsMenu->Add("Clear All", MENU_SELECTION_CLEARALL);
	clearSelectionsMenu->Add(NULL); // Separator

	menu->Add("Tools");

	menu->GetSubMenu("Tools")->Add("Formula editor", MENU_TOOLS_FORMULAEDITOR, SDLK_f, ALT_MODIFIER);
	menu->GetSubMenu("Tools")->Add(NULL); // Separator
	menu->GetSubMenu("Tools")->Add("Texture Plotter ...", MENU_TOOLS_TEXPLOTTER, SDLK_t, ALT_MODIFIER);
	menu->GetSubMenu("Tools")->Add("Profile Plotter ...", MENU_TOOLS_PROFPLOTTER, SDLK_p, ALT_MODIFIER);
	menu->GetSubMenu("Tools")->Add("Histogram Plotter...", MENU_TOOLS_HISTOGRAMPLOTTER);
	menu->GetSubMenu("Tools")->Add(NULL); // Separator
	menu->GetSubMenu("Tools")->Add("Texture scaling...", MENU_EDIT_TSCALING, SDLK_d, CTRL_MODIFIER);
	menu->GetSubMenu("Tools")->Add("Particle logger...", MENU_TOOLS_PARTICLELOGGER);
	//menu->GetSubMenu("Tools")->Add("Histogram settings...", MENU_TOOLS_HISTOGRAMSETTINGS, SDLK_t, CTRL_MODIFIER);
	menu->GetSubMenu("Tools")->Add("Global Settings ...", MENU_EDIT_GLOBALSETTINGS);
	menu->GetSubMenu("Tools")->Add(NULL); // Separator
	menu->GetSubMenu("Tools")->Add("Take screenshot", MENU_TOOLS_SCREENSHOT,SDLK_r, CTRL_MODIFIER);

	menu->GetSubMenu("Tools")->SetIcon(MENU_EDIT_TSCALING, 137, 24);
	menu->GetSubMenu("Tools")->SetIcon(MENU_TOOLS_FORMULAEDITOR, 155, 24);
	menu->GetSubMenu("Tools")->SetIcon(MENU_EDIT_GLOBALSETTINGS, 0, 77);

	menu->Add("Facet");
	menu->GetSubMenu("Facet")->Add("Delete", MENU_FACET_REMOVESEL, SDLK_DELETE, CTRL_MODIFIER);
	menu->GetSubMenu("Facet")->Add("Swap normal", MENU_FACET_SWAPNORMAL, SDLK_n, CTRL_MODIFIER);
	menu->GetSubMenu("Facet")->Add("Shift indices", MENU_FACET_SHIFTVERTEX, SDLK_h, CTRL_MODIFIER);
	menu->GetSubMenu("Facet")->Add("Facet coordinates ...", MENU_FACET_COORDINATES);
	menu->GetSubMenu("Facet")->Add("Move ...", MENU_FACET_MOVE);
	menu->GetSubMenu("Facet")->Add("Scale ...", MENU_FACET_SCALE);
	menu->GetSubMenu("Facet")->Add("Mirror / Project ...", MENU_FACET_MIRROR);
	menu->GetSubMenu("Facet")->Add("Rotate ...", MENU_FACET_ROTATE);
	menu->GetSubMenu("Facet")->Add("Align to ...", MENU_FACET_ALIGN);
	menu->GetSubMenu("Facet")->Add("Extrude ...", MENU_FACET_EXTRUDE);
	menu->GetSubMenu("Facet")->Add("Split ...", MENU_FACET_SPLIT);
	menu->GetSubMenu("Facet")->Add(NULL);
	menu->GetSubMenu("Facet")->Add("Create shape...", MENU_FACET_CREATESHAPE);
	menu->GetSubMenu("Facet")->Add("Create two facets' ...");
	menu->GetSubMenu("Facet")->GetSubMenu("Create two facets' ...")->Add("Difference");
	menu->GetSubMenu("Facet")->GetSubMenu("Create two facets' ...")->GetSubMenu("Difference")->Add("Auto (non-zero)", MENU_FACET_CREATE_DIFFERENCE_AUTO);
	menu->GetSubMenu("Facet")->GetSubMenu("Create two facets' ...")->GetSubMenu("Difference")->Add("First - Second", MENU_FACET_CREATE_DIFFERENCE);
	menu->GetSubMenu("Facet")->GetSubMenu("Create two facets' ...")->GetSubMenu("Difference")->Add("Second - First", MENU_FACET_CREATE_DIFFERENCE2);
	menu->GetSubMenu("Facet")->GetSubMenu("Create two facets' ...")->Add("Union", MENU_FACET_CREATE_UNION);
	menu->GetSubMenu("Facet")->GetSubMenu("Create two facets' ...")->Add("Intersection", MENU_FACET_CREATE_INTERSECTION);
	menu->GetSubMenu("Facet")->GetSubMenu("Create two facets' ...")->Add("XOR", MENU_FACET_CREATE_XOR);
	menu->GetSubMenu("Facet")->Add("Transition between 2", MENU_FACET_LOFT);
	menu->GetSubMenu("Facet")->Add("Build intersection...", MENU_FACET_INTERSECT);
	menu->GetSubMenu("Facet")->Add(NULL);
	menu->GetSubMenu("Facet")->Add("Collapse ...", MENU_FACET_COLLAPSE);
	menu->GetSubMenu("Facet")->Add("Explode", MENU_FACET_EXPLODE);
	

	//menu->GetSubMenu("Facet")->Add("Facet Details ...", MENU_FACET_DETAILS);
	//menu->GetSubMenu("Facet")->Add("Facet Mesh ...",MENU_FACET_MESH);

	//facetMenu = menu->GetSubMenu("Facet");
	//facetMenu->SetEnabled(MENU_FACET_MESH,false);

	menu->GetSubMenu("Facet")->SetIcon(MENU_FACET_COLLAPSE, 173, 24);
	menu->GetSubMenu("Facet")->SetIcon(MENU_FACET_SWAPNORMAL, 191, 24);
	menu->GetSubMenu("Facet")->SetIcon(MENU_FACET_SHIFTVERTEX, 90, 77);
	menu->GetSubMenu("Facet")->SetIcon(MENU_FACET_COORDINATES, 209, 24);
	menu->GetSubMenu("Facet")->SetIcon(MENU_TOOLS_PROFPLOTTER, 227, 24);
	menu->GetSubMenu("Facet")->SetIcon(MENU_FACET_DETAILS, 54, 77);
	//menu->GetSubMenu("Facet")->SetIcon(MENU_FACET_MESH,72,77);
	menu->GetSubMenu("Facet")->SetIcon(MENU_TOOLS_TEXPLOTTER, 108, 77);

	menu->Add("Vertex");
	menu->GetSubMenu("Vertex")->Add("Create Facet from Selected");
	menu->GetSubMenu("Vertex")->GetSubMenu("Create Facet from Selected")->Add("Convex Hull", MENU_VERTEX_CREATE_POLY_CONVEX, SDLK_v, ALT_MODIFIER);
	menu->GetSubMenu("Vertex")->GetSubMenu("Create Facet from Selected")->Add("Keep selection order", MENU_VERTEX_CREATE_POLY_ORDER);
	menu->GetSubMenu("Vertex")->Add("Clear isolated", MENU_VERTEX_CLEAR_ISOLATED);
	menu->GetSubMenu("Vertex")->Add("Remove selected", MENU_VERTEX_REMOVE);
	menu->GetSubMenu("Vertex")->Add("Vertex coordinates...", MENU_VERTEX_COORDINATES);
	menu->GetSubMenu("Vertex")->Add("Move...", MENU_VERTEX_MOVE);
	menu->GetSubMenu("Vertex")->Add("Scale...", MENU_VERTEX_SCALE);
	menu->GetSubMenu("Vertex")->Add("Mirror / Project ...", MENU_VERTEX_MIRROR);
	menu->GetSubMenu("Vertex")->Add("Rotate...", MENU_VERTEX_ROTATE);
	menu->GetSubMenu("Vertex")->Add("Add new...", MENU_VERTEX_ADD);
	menu->GetSubMenu("Vertex")->Add(NULL); // Separator
	menu->GetSubMenu("Vertex")->Add("Select all vertex", MENU_VERTEX_SELECTALL);
	menu->GetSubMenu("Vertex")->Add("Unselect all vertex", MENU_VERTEX_UNSELECTALL);
	menu->GetSubMenu("Vertex")->Add("Select coplanar vertex (visible on screen)", MENU_VERTEX_SELECT_COPLANAR);
	menu->GetSubMenu("Vertex")->Add("Select isolated vertex", MENU_VERTEX_SELECT_ISOLATED);

	menu->Add("View");

	menu->GetSubMenu("View")->Add("Structure");
	structMenu = menu->GetSubMenu("View")->GetSubMenu("Structure");
	UpdateStructMenu();

	menu->GetSubMenu("View")->Add("Full Screen", MENU_VIEW_FULLSCREEN);

	menu->GetSubMenu("View")->Add(NULL); // Separator 

	menu->GetSubMenu("View")->Add("Memorize view to");
	memorizeViewsMenu = menu->GetSubMenu("View")->GetSubMenu("Memorize view to");
	memorizeViewsMenu->Add("Add new...", MENU_VIEW_ADDNEW, SDLK_q, CTRL_MODIFIER);
	memorizeViewsMenu->Add(NULL); // Separator

	menu->GetSubMenu("View")->Add("Select memorized");
	viewsMenu = menu->GetSubMenu("View")->GetSubMenu("Select memorized");

	menu->GetSubMenu("View")->Add("Clear memorized", MENU_VIEW_CLEARVIEWS);
	clearViewsMenu = menu->GetSubMenu("View")->GetSubMenu("Clear memorized");
	clearViewsMenu->Add("Clear All", MENU_VIEW_CLEARALL);

	//menu->GetSubMenu("View")->SetIcon(MENU_VIEW_STRUCTURE_P,0,77);
	menu->GetSubMenu("View")->SetIcon(MENU_VIEW_FULLSCREEN, 18, 77);
	//menu->GetSubMenu("View")->SetIcon(MENU_VIEW_ADD,36,77);

	menu->Add("Test");
	menu->GetSubMenu("Test")->Add("Pipe (L/R=0.0001)", MENU_TEST_PIPE0001);
	menu->GetSubMenu("Test")->Add("Pipe (L/R=1)", MENU_TEST_PIPE1);
	menu->GetSubMenu("Test")->Add("Pipe (L/R=10)", MENU_TEST_PIPE10);
	menu->GetSubMenu("Test")->Add("Pipe (L/R=100)", MENU_TEST_PIPE100);
	menu->GetSubMenu("Test")->Add("Pipe (L/R=1000)", MENU_TEST_PIPE1000);
	menu->GetSubMenu("Test")->Add("Pipe (L/R=10000)", MENU_TEST_PIPE10000);
	//Quick test pipe
	menu->GetSubMenu("Test")->Add(NULL);
	menu->GetSubMenu("Test")->Add("Quick Pipe", MENU_QUICKPIPE, SDLK_q, ALT_MODIFIER);

	geomNumber = new GLTextField(0, NULL);
	geomNumber->SetEditable(false);
	Add(geomNumber);

	togglePanel = new GLTitledPanel("3D Viewer settings");
	togglePanel->SetClosable(true);
	Add(togglePanel);

	showNormal = new GLToggle(0, "Normals");
	togglePanel->Add(showNormal);

	showRule = new GLToggle(0, "Rules");
	togglePanel->Add(showRule);

	showUV = new GLToggle(0, "\201,\202");
	togglePanel->Add(showUV);

	showLeak = new GLToggle(0, "Leaks");
	togglePanel->Add(showLeak);

	showHit = new GLToggle(0, "Hits");
	togglePanel->Add(showHit);

	showLine = new GLToggle(0, "Lines");
	togglePanel->Add(showLine);

	showVolume = new GLToggle(0, "Volume");
	togglePanel->Add(showVolume);

	showTexture = new GLToggle(0, "Texture");
	togglePanel->Add(showTexture);

	showIndex = new GLToggle(0, "Indices");
	togglePanel->Add(showIndex);

	showVertex = new GLToggle(0, "Vertices");
	togglePanel->Add(showVertex);

	simuPanel = new GLTitledPanel("Simulation");
	simuPanel->SetClosable(true);
	Add(simuPanel);

	startSimu = new GLButton(0, "Start/Stop");
	simuPanel->Add(startSimu);

	resetSimu = new GLButton(0, "Reset");
	simuPanel->Add(resetSimu);

	autoFrameMoveToggle = new GLToggle(0, "Auto update scene");
	autoFrameMoveToggle->SetState(autoFrameMove);
	simuPanel->Add(autoFrameMoveToggle);

	forceFrameMoveButton = new GLButton(0, "Update");
	forceFrameMoveButton->SetEnabled(!autoFrameMove);
	simuPanel->Add(forceFrameMoveButton);

	hitLabel = new GLLabel("Hits");
	simuPanel->Add(hitLabel);

	hitNumber = new GLTextField(0, NULL);
	hitNumber->SetEditable(false);
	simuPanel->Add(hitNumber);

	desLabel = new GLLabel("Des.");
	simuPanel->Add(desLabel);

	desNumber = new GLTextField(0, NULL);
	desNumber->SetEditable(false);
	simuPanel->Add(desNumber);

	leakLabel = new GLLabel("Leaks");
	simuPanel->Add(leakLabel);

	leakNumber = new GLTextField(0, NULL);
	leakNumber->SetEditable(false);
	simuPanel->Add(leakNumber);

	sTimeLabel = new GLLabel("Time");
	simuPanel->Add(sTimeLabel);

	sTime = new GLTextField(0, NULL);
	sTime->SetEditable(false);
	simuPanel->Add(sTime);

	facetPanel = new GLTitledPanel("Selected Facet");
	facetPanel->SetClosable(true);
	Add(facetPanel);

	facetSideLabel = new GLLabel("Sides:");
	facetPanel->Add(facetSideLabel);

	facetSideType = new GLCombo(0);
	facetSideType->SetSize(2);
	facetSideType->SetValueAt(0, "1 Sided");
	facetSideType->SetValueAt(1, "2 Sided");
	facetPanel->Add(facetSideType);

	facetTLabel = new GLLabel("Opacity:");
	facetPanel->Add(facetTLabel);
	facetOpacity = new GLTextField(0, NULL);
	facetPanel->Add(facetOpacity);

	facetAreaLabel = new GLLabel("Area (cm\262):");
	facetPanel->Add(facetAreaLabel);
	facetArea = new GLTextField(0, NULL);
	facetPanel->Add(facetArea);

	facetDetailsBtn = new GLButton(0, "Details...");
	facetPanel->Add(facetDetailsBtn);

	facetCoordBtn = new GLButton(0, "Coord.");
	facetPanel->Add(facetCoordBtn);

	facetApplyBtn = new GLButton(0, "Apply");
	facetApplyBtn->SetEnabled(false);
	facetPanel->Add(facetApplyBtn);
}

void Interface::OneTimeSceneInit_shared_post() {
	menu->Add("About");
	menu->GetSubMenu("About")->Add("License", MENU_ABOUT);

	ClearFacetParams();
	LoadConfig();
	UpdateRecentMenu();
	UpdateViewerPanel();
	PlaceComponents();
	CheckNeedsTexture();

	try {
		worker.SetProcNumber(nbProc);
	}
	catch (Error &e) {
		char errMsg[512];
		sprintf(errMsg, "Failed to start working sub-process(es), simulation not available\n%s", e.GetMsg());
		GLMessageBox::Display(errMsg, "Error", GLDLG_OK, GLDLG_ICONERROR);
	}

	EmptyGeometry();

	appUpdater = new AppUpdater(appName, appVersionId, "updater_config.xml");
	int answer = appUpdater->RequestUpdateCheck();
	if (answer == ANSWER_ASKNOW) {
		updateCheckDialog = new UpdateCheckDialog(appName, appUpdater);
		updateCheckDialog->SetVisible(true);
		wereEvents = true;
	}
}

int Interface::RestoreDeviceObjects_shared() {
	Geometry *geom = worker.GetGeometry();
	geom->RestoreDeviceObjects();
	//worker.Update(0.0f);

	// Restore dialog which are not displayed
	// Those which are displayed are invalidated by the window manager
	RVALIDATE_DLG(formulaEditor);
	RVALIDATE_DLG(collapseSettings);
	RVALIDATE_DLG(histogramSettings);
	RVALIDATE_DLG(histogramPlotter);
	RVALIDATE_DLG(moveVertex);
	RVALIDATE_DLG(scaleVertex);
	RVALIDATE_DLG(scaleFacet);
	RVALIDATE_DLG(selectDialog);
	RVALIDATE_DLG(selectTextureType);
	RVALIDATE_DLG(moveFacet);
	RVALIDATE_DLG(createShape);
	RVALIDATE_DLG(extrudeFacet);
	RVALIDATE_DLG(mirrorFacet);
	RVALIDATE_DLG(mirrorVertex);
	RVALIDATE_DLG(splitFacet);
	RVALIDATE_DLG(buildIntersection);
	RVALIDATE_DLG(rotateFacet);
	RVALIDATE_DLG(rotateVertex);
	RVALIDATE_DLG(alignFacet);
	RVALIDATE_DLG(addVertex);
	RVALIDATE_DLG(loadStatus);
	RVALIDATE_DLG(facetCoordinates);
	RVALIDATE_DLG(vertexCoordinates);
	RVALIDATE_DLG(particleLogger);

	RVALIDATE_DLG(updateCheckDialog);
	RVALIDATE_DLG(updateFoundDialog);
	RVALIDATE_DLG(updateLogWindow);

	UpdateTitle();

	return GL_OK;
}

int Interface::InvalidateDeviceObjects_shared() {
	Geometry *geom = worker.GetGeometry();
	geom->InvalidateDeviceObjects();
	//worker.Update(0.0f);

	// Restore dialog which are not displayed
	// Those which are displayed are invalidated by the window manager
	IVALIDATE_DLG(formulaEditor);
	IVALIDATE_DLG(collapseSettings);
	IVALIDATE_DLG(histogramSettings);
	IVALIDATE_DLG(histogramPlotter);
	IVALIDATE_DLG(moveVertex);
	IVALIDATE_DLG(scaleVertex);
	IVALIDATE_DLG(scaleFacet);
	IVALIDATE_DLG(selectDialog);
	IVALIDATE_DLG(selectTextureType);
	IVALIDATE_DLG(moveFacet);
	IVALIDATE_DLG(createShape);
	IVALIDATE_DLG(extrudeFacet);
	IVALIDATE_DLG(mirrorFacet);
	IVALIDATE_DLG(mirrorVertex);
	IVALIDATE_DLG(splitFacet);
	IVALIDATE_DLG(buildIntersection);
	IVALIDATE_DLG(rotateFacet);
	IVALIDATE_DLG(rotateFacet);
	IVALIDATE_DLG(alignFacet);
	IVALIDATE_DLG(addVertex);
	IVALIDATE_DLG(loadStatus);
	IVALIDATE_DLG(facetCoordinates);
	IVALIDATE_DLG(vertexCoordinates);
	IVALIDATE_DLG(particleLogger);

	IVALIDATE_DLG(updateCheckDialog);
	IVALIDATE_DLG(updateFoundDialog);
	IVALIDATE_DLG(updateLogWindow);

	UpdateTitle();

	return GL_OK;
}

bool Interface::ProcessMessage_shared(GLComponent *src, int message) {
	Geometry *geom = worker.GetGeometry();
	char *input;
	char tmp[128];

	switch (message) {

	//MENU --------------------------------------------------------------------
	case MSG_MENU:
		switch (src->GetId()) {
		case MENU_FILE_NEW:
			if (AskToSave()) {
				if (worker.isRunning) worker.Stop_Public();
				EmptyGeometry();
			}
			return true;
		case MENU_FILE_LOAD:
			if (AskToSave()) {
				if (worker.isRunning) worker.Stop_Public();
				LoadFile();
			}
			return true;
		case MENU_FILE_INSERTGEO:
			if (geom->IsLoaded()) {
				if (worker.isRunning) worker.Stop_Public();
				InsertGeometry(false);
			}
			else GLMessageBox::Display("No geometry loaded.", "No geometry", GLDLG_OK, GLDLG_ICONERROR);
			return true;
		case MENU_FILE_INSERTGEO_NEWSTR:
			if (geom->IsLoaded()) {
				if (worker.isRunning) worker.Stop_Public();
				InsertGeometry(true);
			}
			else GLMessageBox::Display("No geometry loaded.", "No geometry", GLDLG_OK, GLDLG_ICONERROR);
			return true;
		case MENU_FILE_SAVEAS:
			if (geom->IsLoaded()) {
				SaveFileAs();
			}
			else GLMessageBox::Display("No geometry loaded.", "No geometry", GLDLG_OK, GLDLG_ICONERROR);
			return true;
		case MENU_FILE_EXPORT_SELECTION:
			ExportSelection();
			return true;
		case MENU_FILE_SAVE:
			if (geom->IsLoaded()) SaveFile();
			else GLMessageBox::Display("No geometry loaded.", "No geometry", GLDLG_OK, GLDLG_ICONERROR);
			return true;
		case MENU_FILE_EXIT:
			if (AskToSave()) Exit();
			return true;

			/*case MENU_EDIT_ADDFORMULA:
				if (!formulaSettings) formulaSettings = new FormulaSettings();
				formulaSettings->Update(NULL, -1);
				formulaSettings->SetVisible(true);
				return true;*/

		case MENU_TOOLS_FORMULAEDITOR:
			if (!geom->IsLoaded()) {
				GLMessageBox::Display("No geometry loaded.", "No geometry", GLDLG_OK, GLDLG_ICONERROR);
				return true;
			}
			if (!formulaEditor || !formulaEditor->IsVisible()) {
				SAFE_DELETE(formulaEditor);
				formulaEditor = new FormulaEditor(&worker);
				formulaEditor->Refresh();
				formulaEditor->SetVisible(true);
			}
			break;
		case MENU_TOOLS_HISTOGRAMSETTINGS:
			if (!histogramSettings || !histogramSettings->IsVisible()) {
				SAFE_DELETE(histogramSettings);
				histogramSettings = new HistogramSettings(geom, &worker);
			}
			histogramSettings->Refresh(geom->GetSelectedFacets());
			histogramSettings->SetVisible(true);
			return true;
		case MENU_TOOLS_HISTOGRAMPLOTTER:
			if (!histogramPlotter || !histogramPlotter->IsVisible()) {
				SAFE_DELETE(histogramPlotter);
				histogramPlotter = new HistogramPlotter(&worker);
			}
			histogramPlotter->Refresh();
			histogramPlotter->SetVisible(true);
			return true;
		case MENU_TOOLS_PARTICLELOGGER:
			if (!particleLogger || !particleLogger->IsVisible()) {
				SAFE_DELETE(particleLogger);
				particleLogger = new ParticleLogger(geom, &worker);
			}
			particleLogger->UpdateStatus();
			particleLogger->SetVisible(true);
			return true;

		case MENU_TOOLS_SCREENSHOT:
		{
			std::ostringstream tmp;

			time_t     now = time(0);
			struct tm  tstruct;
			char       buf[80];
			tstruct = *localtime(&now);
			// Visit http://en.cppreference.com/w/cpp/chrono/c/strftime
			// for more information about date/time format
			strftime(buf, sizeof(buf), "%Y_%m_%d__%H_%M_%S", &tstruct);

			tmp << buf << "_" << worker.GetCurrentShortFileName();
			std::string oriName = tmp.str();
			tmp.str("");
			tmp.clear();
			for (char c : oriName) {
				bool basic_ascii = ((c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z'));
				if (basic_ascii) tmp << c;
				else tmp << '_';
			}
			std::string asciiName = tmp.str();
			tmp.str("");
			tmp.clear();
			tmp << "Screenshots/" << asciiName << ".png";

			
			std::filesystem::create_directory("Screenshots"); //Doesn't do anything if already exists

			int x, y, width, height;
			viewer[curViewer]->GetBounds(&x, &y, &width, &height);

			int leftMargin = 4; //Left bewel
			int rightMargin = 0;
			int topMargin = 0;
			int bottomMargin = 28; //Toolbar

			viewer[curViewer]->RequestScreenshot(tmp.str(), leftMargin, topMargin, width - leftMargin - rightMargin, height - topMargin - bottomMargin);
			return true;
		}
		case MENU_FACET_COLLAPSE:
			if (geom->IsLoaded()) {
				DisplayCollapseDialog();
			}
			else GLMessageBox::Display("No geometry loaded.", "No geometry", GLDLG_OK, GLDLG_ICONERROR);
			return true;
		case MENU_FACET_SWAPNORMAL:
			if (AskToReset()) {
				geom->SwapNormal();
				// Send to sub process
				try { worker.Reload(); }
				catch (Error &e) {
					GLMessageBox::Display(e.GetMsg(), "Error", GLDLG_OK, GLDLG_ICONERROR);
				}
			}
			return true;
		case MENU_FACET_EXTRUDE:
			if (!extrudeFacet || !extrudeFacet->IsVisible()) {
				SAFE_DELETE(extrudeFacet);
				extrudeFacet = new ExtrudeFacet(geom, &worker);
			}
			extrudeFacet->SetVisible(true);
			return true;

		case MENU_FACET_SHIFTVERTEX:
			if (AskToReset()) {
				geom->ShiftVertex();
				// Send to sub process
				worker.Reload();
			}
			return true;
		case MENU_FACET_COORDINATES:

			if (!facetCoordinates) facetCoordinates = new FacetCoordinates();
			facetCoordinates->Display(&worker);
			return true;
		case MENU_FACET_MOVE:
			if (!moveFacet || !moveFacet->IsVisible()) {
				SAFE_DELETE(moveFacet);
				moveFacet = new MoveFacet(geom, &worker);
			}
			moveFacet->SetVisible(true);
			return true;
		case MENU_FACET_SCALE:
			if (geom->IsLoaded()) {
				if (!scaleFacet) scaleFacet = new ScaleFacet(geom, &worker);

				scaleFacet->SetVisible(true);

			}
			else GLMessageBox::Display("No geometry loaded.", "No geometry", GLDLG_OK, GLDLG_ICONERROR);
			return true;
		case MENU_FACET_MIRROR:
			if (!mirrorFacet) mirrorFacet = new MirrorFacet(geom, &worker);
			mirrorFacet->SetVisible(true);
			return true;
		case MENU_FACET_SPLIT:
			if (!splitFacet || !splitFacet->IsVisible()) {
				SAFE_DELETE(splitFacet);
				splitFacet = new SplitFacet(geom, &worker);
				splitFacet->SetVisible(true);
			}
			return true;
		case MENU_FACET_ROTATE:
			if (!rotateFacet) rotateFacet = new RotateFacet(geom, &worker);
			rotateFacet->SetVisible(true);
			return true;
		case MENU_FACET_ALIGN:
			if (!alignFacet) alignFacet = new AlignFacet(geom, &worker);
			alignFacet->MemorizeSelection();
			alignFacet->SetVisible(true);
			return true;

		case MENU_FACET_EXPLODE:
			if (GLMessageBox::Display("Explode selected facet?", "Question", GLDLG_OK | GLDLG_CANCEL, GLDLG_ICONINFO) == GLDLG_OK) {
				if (AskToReset()) {
					int err;
					try {
						err = geom->ExplodeSelected();
					}
					catch (Error &e) {
						GLMessageBox::Display(e.GetMsg(), "Error exploding", GLDLG_OK, GLDLG_ICONERROR);
					}
					if (err == -1) {
						GLMessageBox::Display("Empty selection", "Error", GLDLG_OK, GLDLG_ICONERROR);
					}
					else if (err == -2) {
						GLMessageBox::Display("All selected facets must have a mesh with boudary correction enabled", "Error", GLDLG_OK, GLDLG_ICONERROR);
					}
					else if (err == 0) {

						UpdateModelParams();
						UpdateFacetParams(true);
						// Send to sub process
						try { worker.Reload(); }
						catch (Error &e) {
							GLMessageBox::Display(e.GetMsg(), "Error reloading worker", GLDLG_OK, GLDLG_ICONERROR);
						}
					}
				}
			}
			return true;
		case MENU_FACET_CREATESHAPE:
			if (!createShape) createShape = new CreateShape(geom, &worker);
			createShape->SetVisible(true);
			return true;
		case MENU_SELECTION_SMARTSELECTION:
			if (!smartSelection) smartSelection = new SmartSelection(worker.GetGeometry(), &worker);
			smartSelection->SetVisible(true);
			return true;
		case MENU_FACET_SELECTALL:
			geom->SelectAll();
			UpdateFacetParams(true);
			return true;
		case MENU_FACET_SELECTTRANS:
			geom->UnselectAll();
			for (int i = 0; i < geom->GetNbFacet(); i++)
				if (geom->GetFacet(i)->sh.opacity_paramId!=-1 || (geom->GetFacet(i)->sh.opacity != 1.0 && geom->GetFacet(i)->sh.opacity != 2.0))
					geom->SelectFacet(i);
			geom->UpdateSelection();
			UpdateFacetParams(true);
			return true;
		case MENU_FACET_SELECT2SIDE:
			geom->UnselectAll();
			for (int i = 0; i < geom->GetNbFacet(); i++)
				if (geom->GetFacet(i)->sh.is2sided)
					geom->SelectFacet(i);
			geom->UpdateSelection();
			UpdateFacetParams(true);
			return true;
		case MENU_FACET_SELECTTEXT:
			geom->UnselectAll();
			for (int i = 0; i < geom->GetNbFacet(); i++)
				if (geom->GetFacet(i)->sh.isTextured)
					geom->SelectFacet(i);
			geom->UpdateSelection();
			UpdateFacetParams(true);
			return true;
		case MENU_FACET_SELECTPROF:
			geom->UnselectAll();
			for (int i = 0; i < geom->GetNbFacet(); i++)
				if (geom->GetFacet(i)->sh.isProfile)
					geom->SelectFacet(i);
			geom->UpdateSelection();
			UpdateFacetParams(true);
			return true;

		case MENU_FACET_SELECTNONPLANAR:
		{
			sprintf(tmp, "%g", planarityThreshold);
			//sprintf(title,"Pipe L/R = %g",L/R);
			input = GLInputBox::GetInput(tmp, "Planarity larger than:", "Select non planar facets");
			if (!input) return true;
			if (!sscanf(input, "%lf", &planarityThreshold)) {
				GLMessageBox::Display("Invalid number", "Error", GLDLG_OK, GLDLG_ICONERROR);
				return true;
			}
			geom->UnselectAll();
			std::vector<size_t> nonPlanarFacetids = geom->GetNonPlanarFacets(planarityThreshold);
			for (const auto& i : nonPlanarFacetids)
				geom->SelectFacet(i);
			geom->UpdateSelection();
			UpdateFacetParams(true);
			return true;
		}
		case MENU_FACET_SELECTERR:
			geom->UnselectAll();
			for (int i = 0; i < geom->GetNbFacet(); i++)

				if (geom->GetFacet(i)->nonSimple)
					geom->SelectFacet(i);
			geom->UpdateSelection();
			UpdateFacetParams(true);
			return true;

		case MENU_FACET_SELECTDEST:
			geom->UnselectAll();
			for (int i = 0; i < geom->GetNbFacet(); i++)

				if (geom->GetFacet(i)->sh.superDest != 0)
					geom->SelectFacet(i);
			geom->UpdateSelection();
			UpdateFacetParams(true);
			return true;

		case MENU_FACET_SELECTTELEPORT:
			geom->UnselectAll();
			for (int i = 0; i < geom->GetNbFacet(); i++)

				if (geom->GetFacet(i)->sh.teleportDest != 0)
					geom->SelectFacet(i);
			geom->UpdateSelection();
			UpdateFacetParams(true);
			return true;

		case MENU_FACET_SELECTABS:
			geom->UnselectAll();
			for (int i = 0; i < geom->GetNbFacet(); i++) {
#ifdef MOLFLOW
				if (geom->GetFacet(i)->facetHitCache.nbAbsEquiv > 0)
#endif
#ifdef SYNRAD
					if (geom->GetFacet(i)->facetHitCache.nbAbsEquiv > 0)
#endif
					geom->SelectFacet(i);
			}
			geom->UpdateSelection();
			UpdateFacetParams(true);
			return true;

		case MENU_FACET_SELECTHITS:
			geom->UnselectAll();
			for (int i = 0; i < geom->GetNbFacet(); i++)

#ifdef MOLFLOW
				if (geom->GetFacet(i)->facetHitCache.nbMCHit > 0)
#endif
#ifdef SYNRAD
					if (geom->GetFacet(i)->facetHitCache.nbMCHit > 0)
#endif
					geom->SelectFacet(i);
			geom->UpdateSelection();
			UpdateFacetParams(true);
			return true;

		case MENU_FACET_SELECTNOHITS_AREA:

			sprintf(tmp, "%g", largeAreaThreshold);
			//sprintf(title,"Pipe L/R = %g",L/R);
			input = GLInputBox::GetInput(tmp, "Min.area (cm\262)", "Select large facets without hits");
			if (!input) return true;
			if ((sscanf(input, "%lf", &largeAreaThreshold) <= 0) || (largeAreaThreshold <= 0.0)) {
				GLMessageBox::Display("Invalid number", "Error", GLDLG_OK, GLDLG_ICONERROR);
				return true;
			}
			geom->UnselectAll();
			for (int i = 0; i < geom->GetNbFacet(); i++)
#ifdef MOLFLOW
				if (geom->GetFacet(i)->facetHitCache.nbMCHit == 0 && geom->GetFacet(i)->sh.area >= largeAreaThreshold)
#endif
#ifdef SYNRAD
				if (geom->GetFacet(i)->facetHitCache.nbMCHit == 0 && geom->GetFacet(i)->sh.area >= largeAreaThreshold)
#endif
					geom->SelectFacet(i);
			geom->UpdateSelection();
			UpdateFacetParams(true);
			return true;
		case MENU_FACET_INVERTSEL:
			for (int i = 0; i < geom->GetNbFacet(); i++)
				geom->GetFacet(i)->selected = !geom->GetFacet(i)->selected;
			geom->UpdateSelection();
			UpdateFacetParams(true);
			return true;
		case MENU_SELECTION_SELECTFACETNUMBER:
			if (!selectDialog) selectDialog = new SelectDialog(worker.GetGeometry());
			selectDialog->SetVisible(true);
			return true;
		case MENU_SELECTION_TEXTURETYPE:
			if (!selectTextureType) selectTextureType = new SelectTextureType(&worker);
			selectTextureType->SetVisible(true);
			return true;
		case MENU_FACET_SAVESEL:
			SaveSelection();
			return true;
		case MENU_FACET_LOADSEL:
			LoadSelection();
			return true;
		case MENU_SELECTION_ADDNEW:
			AddSelection();
			return true;
		case  MENU_SELECTION_CLEARALL:
			if (GLMessageBox::Display("Clear all selections ?", "Question", GLDLG_OK | GLDLG_CANCEL, GLDLG_ICONINFO) == GLDLG_OK) {
				ClearAllSelections();
			}
			return true;
		case MENU_VERTEX_UNSELECTALL:
			geom->UnselectAllVertex();
			return true;
		case MENU_VERTEX_SELECTALL:
			geom->SelectAllVertex();
			return true;
		case MENU_VERTEX_SELECT_ISOLATED:
			geom->SelectIsolatedVertices();
			return true;
		case MENU_VERTEX_CLEAR_ISOLATED:
			geom->DeleteIsolatedVertices(false);
			UpdateModelParams();
			if (facetCoordinates) facetCoordinates->UpdateFromSelection();
			if (vertexCoordinates) vertexCoordinates->Update();
			return true;
		case MENU_VERTEX_CREATE_POLY_CONVEX:
			if (AskToReset()) {
				try {
					geom->CreatePolyFromVertices_Convex();
				}
				catch (Error &e) {
					GLMessageBox::Display(e.GetMsg(), "Error creating polygon", GLDLG_OK, GLDLG_ICONERROR);
				}
				worker.Reload();
			}
			return true;
		case MENU_VERTEX_CREATE_POLY_ORDER:
			if (AskToReset()) {
				try {
					geom->CreatePolyFromVertices_Order();
				}
				catch (Error &e) {
					GLMessageBox::Display(e.GetMsg(), "Error creating polygon", GLDLG_OK, GLDLG_ICONERROR);
				}
				worker.Reload();
			}
			return true;
		case MENU_FACET_CREATE_DIFFERENCE:
			CreateOfTwoFacets(ClipperLib::ctDifference, 0);
			return true;
		case MENU_FACET_CREATE_DIFFERENCE2:
			CreateOfTwoFacets(ClipperLib::ctDifference, 1);
			return true;
		case MENU_FACET_CREATE_DIFFERENCE_AUTO:
			CreateOfTwoFacets(ClipperLib::ctDifference, 2);
			return true;
		case MENU_FACET_CREATE_UNION:
			CreateOfTwoFacets(ClipperLib::ctUnion);
			return true;
		case MENU_FACET_CREATE_INTERSECTION:
			CreateOfTwoFacets(ClipperLib::ctIntersection);
			return true;
		case MENU_FACET_CREATE_XOR:
			CreateOfTwoFacets(ClipperLib::ctXor);
			return true;
		case MENU_FACET_LOFT:
			if (geom->GetNbSelectedFacets() != 2) {
				GLMessageBox::Display("Select exactly 2 facets", "Can't create loft", GLDLG_OK, GLDLG_ICONERROR);
				return true;
			}
			if (AskToReset()) {
				geom->CreateLoft();
			}
			worker.Reload();
			UpdateModelParams();
			UpdateFacetlistSelected();
			UpdateViewers();
			return true;
		case MENU_FACET_INTERSECT:
			if (!buildIntersection || !buildIntersection->IsVisible()) {
				SAFE_DELETE(buildIntersection);
				buildIntersection = new BuildIntersection(geom, &worker);
				buildIntersection->SetVisible(true);
			}
			return true;			
		case MENU_VERTEX_SELECT_COPLANAR:
			char *input;
			if (geom->IsLoaded()) {
				if (geom->GetNbSelectedVertex() != 3) {
					GLMessageBox::Display("Select exactly 3 vertices", "Can't define plane", GLDLG_OK, GLDLG_ICONERROR);
					return true;
				}
				sprintf(tmp, "%g", coplanarityTolerance);
				//sprintf(title,"Pipe L/R = %g",L/R);
				input = GLInputBox::GetInput(tmp, "Tolerance (cm)", "Select coplanar vertices");
				if (!input) return true;
				if ((sscanf(input, "%lf", &coplanarityTolerance) <= 0) || (coplanarityTolerance <= 0.0)) {
					GLMessageBox::Display("Invalid number", "Error", GLDLG_OK, GLDLG_ICONERROR);
					return true;
				}
				try { viewer[curViewer]->SelectCoplanar(coplanarityTolerance); }
				catch (Error &e) {
					GLMessageBox::Display(e.GetMsg(), "Error selecting coplanar vertices", GLDLG_OK, GLDLG_ICONERROR);
				}
			}
			else GLMessageBox::Display("No geometry loaded.", "No geometry", GLDLG_OK, GLDLG_ICONERROR);
			return true;
		case MENU_VERTEX_MOVE:
			if (geom->IsLoaded()) {
				if (!moveVertex) moveVertex = new MoveVertex(geom, &worker);

				//moveVertex->DoModal();
				moveVertex->SetVisible(true);

				/*
				UpdateModelParams();
				try { worker.Reload(); } catch(Error &e) {
				GLMessageBox::Display(e.GetMsg(),"Error reloading worker",GLDLG_OK,GLDLG_ICONERROR);
				*/

			}
			else GLMessageBox::Display("No geometry loaded.", "No geometry", GLDLG_OK, GLDLG_ICONERROR);
			return true;
		case MENU_VERTEX_SCALE:
			if (geom->IsLoaded()) {
				if (!scaleVertex) scaleVertex = new ScaleVertex(geom, &worker);
				scaleVertex->SetVisible(true);
			}
			else GLMessageBox::Display("No geometry loaded.", "No geometry", GLDLG_OK, GLDLG_ICONERROR);
			return true;
		case MENU_VERTEX_MIRROR:
			if (!mirrorVertex) mirrorVertex = new MirrorVertex(geom, &worker);
			mirrorVertex->SetVisible(true);
			return true;
		case MENU_VERTEX_ROTATE:
			if (!rotateVertex) rotateVertex = new RotateVertex(geom, &worker);
			rotateVertex->SetVisible(true);
			return true;
		
		case MENU_VERTEX_COORDINATES:

			if (!vertexCoordinates) vertexCoordinates = new VertexCoordinates();
			vertexCoordinates->Display(&worker);
			return true;

		case MENU_VERTEX_ADD:
			if (geom->IsLoaded()) {
				if (!addVertex) addVertex = new AddVertex(geom, &worker);
				addVertex->SetVisible(true);
			}
			else GLMessageBox::Display("No geometry loaded.", "No geometry", GLDLG_OK, GLDLG_ICONERROR);
			return true;

		case MENU_VIEW_FULLSCREEN:
			if (m_bWindowed) {
				ToggleFullscreen();
				PlaceComponents();
			}
			else {
				Resize(1024, 800, true);
			}
			menu->GetSubMenu("View")->SetCheck(MENU_VIEW_FULLSCREEN, !m_bWindowed);
			return true;

		case MENU_VIEW_ADDNEW:
			AddView();
			return true;
			return true;
		case  MENU_VIEW_CLEARALL:
			if (GLMessageBox::Display("Clear all views ?", "Question", GLDLG_OK | GLDLG_CANCEL, GLDLG_ICONINFO) == GLDLG_OK) {
				ClearAllViews();
			}
			return true;
		case MENU_TEST_PIPE0001:
			if (AskToSave()) BuildPipe(0.0001);
			return true;
		case MENU_TEST_PIPE1:
			if (AskToSave()) BuildPipe(1.0);
			return true;
		case MENU_TEST_PIPE10:
			if (AskToSave()) BuildPipe(10.0);
			return true;
		case MENU_TEST_PIPE100:
			if (AskToSave()) BuildPipe(100.0);
			return true;
		case MENU_TEST_PIPE1000:
			if (AskToSave()) BuildPipe(1000.0);
			return true;
		case MENU_TEST_PIPE10000:
			if (AskToSave()) BuildPipe(10000.0);
			return true;
		case MENU_QUICKPIPE:
			if (AskToSave()) BuildPipe(5.0,5);
			return true;
		case MENU_ABOUT:
		{
			std::ostringstream aboutText;
			aboutText << "Program:    " << appName << " " << appVersionName << " (" << appVersionId <<")";
			aboutText << R"(
Authors:     Jean-Luc PONS / Roberto KERSEVAN / Marton ADY
Copyright:   E.S.R.F / CERN   (2019)
Website:    https://cern.ch/molflow

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

Full license text: https://www.gnu.org/licenses/old-licenses/gpl-2.0.en.html
)";
			GLMessageBox::Display(aboutText.str().c_str(), "About", GLDLG_OK, GLDLG_ICONINFO);
			return true;
		}
		}
		// Load recent menu
		if (src->GetId() >= MENU_FILE_LOADRECENT && src->GetId() < MENU_FILE_LOADRECENT + nbRecent) {
			if (AskToSave()) {
				if (worker.isRunning) worker.Stop_Public();
				LoadFile(recents[src->GetId() - MENU_FILE_LOADRECENT]);
			}
			return true;
		}

		// Show structure menu
		else if (src->GetId() >= MENU_VIEW_STRUCTURE && src->GetId() <= MENU_VIEW_STRUCTURE + geom->GetNbStructure()) {
			geom->viewStruct = src->GetId() - MENU_VIEW_STRUCTURE - 1;
			if (src->GetId() > MENU_VIEW_STRUCTURE) geom->UnselectAll();
			UpdateStructMenu();
			return true;
		}
		else if (src->GetId() == MENU_VIEW_NEWSTRUCT) {
			AddStruct();
			UpdateStructMenu();
			return true;
		}
		else if (src->GetId() == MENU_VIEW_DELSTRUCT) {
			DeleteStruct();
			UpdateStructMenu();

			#ifdef MOLFLOW
				worker.CalcTotalOutgassing();
			#endif

			return true;
		}
		else if (src->GetId() == MENU_VIEW_PREVSTRUCT) {
			geom->viewStruct = (int)Previous(geom->viewStruct, geom->GetNbStructure());
			geom->UnselectAll();
			UpdateStructMenu();
			return true;
		}
		else if (src->GetId() == MENU_VIEW_NEXTSTRUCT) {
			geom->viewStruct = (int)Next(geom->viewStruct, geom->GetNbStructure());
			geom->UnselectAll();
			UpdateStructMenu();
			return true;
		}

		// Select selection
		else if (MENU_SELECTION_SELECTIONS + selections.size() > src->GetId() && src->GetId() >= MENU_SELECTION_SELECTIONS) { //Choose selection by number
			SelectSelection(src->GetId() - MENU_SELECTION_SELECTIONS);
			return true;
		}
		else if (src->GetId() == (MENU_SELECTION_SELECTIONS + selections.size())) { //Previous selection
			SelectSelection(Previous(idSelection, selections.size()));
			return true;
		}
		else if (src->GetId() == (MENU_SELECTION_SELECTIONS + selections.size() + 1)) { //Next selection
			SelectSelection(Next(idSelection, selections.size()));
			return true;
		}

		// Clear selection
		else if (src->GetId() >= MENU_SELECTION_CLEARSELECTIONS && src->GetId() < MENU_SELECTION_CLEARSELECTIONS + selections.size()) {
			char tmpname[256];
			sprintf(tmpname, "Clear %s?", selections[src->GetId() - MENU_SELECTION_CLEARSELECTIONS].name.c_str());
			if (GLMessageBox::Display(tmpname, "Confirmation", GLDLG_OK | GLDLG_CANCEL, GLDLG_ICONINFO) == GLDLG_OK) {
				ClearSelection(src->GetId() - MENU_SELECTION_CLEARSELECTIONS);
			}
			return true;
		}

		// Memorize selection
		else if (src->GetId() >= MENU_SELECTION_MEMORIZESELECTIONS && src->GetId() < MENU_SELECTION_MEMORIZESELECTIONS + selections.size()) {
			OverWriteSelection(src->GetId() - MENU_SELECTION_MEMORIZESELECTIONS);
			return true;
		}

		// Select view
		else if (src->GetId() >= MENU_VIEW_VIEWS && src->GetId() < MENU_VIEW_VIEWS + nbView) {
			SelectView(src->GetId() - MENU_VIEW_VIEWS);
			return true;
		}
		// Clear view
		else if (src->GetId() >= MENU_VIEW_CLEARVIEWS && src->GetId() < MENU_VIEW_CLEARVIEWS + nbView) {
			char tmpname[256];
			sprintf(tmpname, "Clear %s?", views[src->GetId() - MENU_VIEW_CLEARVIEWS].name);
			if (GLMessageBox::Display(tmpname, "Confirmation", GLDLG_OK | GLDLG_CANCEL, GLDLG_ICONINFO) == GLDLG_OK) {
				ClearView(src->GetId() - MENU_VIEW_CLEARVIEWS);
			}
			return true;
		}
		// Memorize view
		else if (src->GetId() >= MENU_VIEW_MEMORIZEVIEWS && src->GetId() < MENU_VIEW_MEMORIZEVIEWS + nbView) {
			OverWriteView(src->GetId() - MENU_VIEW_MEMORIZEVIEWS);
			return true;
		}
		break;

		//LIST --------------------------------------------------------------------
	case MSG_LIST:
		if (src == facetList && geom->IsLoaded()) {
			auto selRows = facetList->GetSelectedRows(true);
			geom->UnselectAll();
			for (auto& sel:selRows)
				geom->SelectFacet(sel);
			geom->UpdateSelection();
			UpdateFacetParams();
			return true;
		}
		break;

		//GEOMVIEWER ------------------------------------------------------------------
	case MSG_GEOMVIEWER_MAXIMISE:
	{
		if (src == viewer[0]) {
			AnimateViewerChange(0);
		}
		else if (src == viewer[1]) {
			AnimateViewerChange(1);
		}
		else if (src == viewer[2]) {
			AnimateViewerChange(2);
		}
		else if (src == viewer[3]) {
			AnimateViewerChange(3);
		}
		Place3DViewer();

		bool neededTexture = needsTexture;

		bool neededMesh = needsMesh;
		CheckNeedsTexture();

		if (!needsTexture && neededTexture) { //We just disabled textures
			worker.GetGeometry()->ClearFacetTextures();
		}
		else if (needsTexture && !neededTexture) { //We just enabled textures
			worker.RebuildTextures();
		}

		if (!needsMesh && neededMesh) { //We just disabled mesh
			geom->ClearFacetMeshLists();
		}
		else if (needsMesh && !neededMesh) { //We just enabled mesh
			geom->BuildFacetMeshLists();
		}

		return true;
	}
	case MSG_GEOMVIEWER_SELECT: {
		SelectViewer(src->GetId());
		return true;
	}

		//BUTTON ------------------------------------------------------------------
	case MSG_BUTTON:
		if (src == resetSimu) {
			changedSinceSave = true;
			ResetSimulation();
			return true;
		}
		else if (src == forceFrameMoveButton) {
			updateRequested = true;
			FrameMove();
			return true;
		}
		else if (src == facetCoordBtn) {
			if (!facetCoordinates) facetCoordinates = new FacetCoordinates();
			facetCoordinates->Display(&worker);
			return true;
		}
		break;

		//Panel open/close ---------------------------------------------------------
	case MSG_PANELR:
		PlaceComponents();
		return true;
	}
	return false;
}

void Interface::CheckNeedsTexture()
{
	needsMesh = needsTexture = needsDirection = false;
	for (int i = 0;i < MAX_VIEWER;i++) {
		needsMesh = needsMesh || (viewer[i]->IsVisible() && viewer[i]->showMesh);
		needsTexture = needsTexture || (viewer[i]->IsVisible() && viewer[i]->showTexture);
		needsDirection = needsDirection || (viewer[i]->IsVisible() && viewer[i]->showDir);
	}
}

//SELECTIONS

void Interface::SelectView(int v) {
	viewer[curViewer]->SetCurrentView(views[v]);
}

void Interface::SelectSelection(size_t v) {
	Geometry *geom = worker.GetGeometry();
	geom->SetSelection(selections[v].selection, viewer[0]->GetWindow()->IsShiftDown(), viewer[0]->GetWindow()->IsCtrlDown());
	idSelection = v;
}

void Interface::ClearSelectionMenus() {
	memorizeSelectionsMenu->Clear();
	memorizeSelectionsMenu->Add("Add new...", MENU_SELECTION_ADDNEW, SDLK_w, CTRL_MODIFIER);
	memorizeSelectionsMenu->Add(NULL); // Separator
	clearSelectionsMenu->Clear();
	clearSelectionsMenu->Add("Clear All", MENU_SELECTION_CLEARALL);
	clearSelectionsMenu->Add(NULL); // Separator
	selectionsMenu->Clear();
}

void Interface::RebuildSelectionMenus() {
	ClearSelectionMenus();
	size_t i;
	for (i = 0; i < selections.size(); i++) {
		if (i <= 8) {
			selectionsMenu->Add(selections[i].name.c_str(), MENU_SELECTION_SELECTIONS + (int)i, SDLK_1 + (int)i, ALT_MODIFIER);
		}
		else {
			selectionsMenu->Add(selections[i].name.c_str(), MENU_SELECTION_SELECTIONS + (int)i); //no place for ALT+shortcut
		}
		clearSelectionsMenu->Add(selections[i].name.c_str(), MENU_SELECTION_CLEARSELECTIONS + (int)i);
		memorizeSelectionsMenu->Add(selections[i].name.c_str(), MENU_SELECTION_MEMORIZESELECTIONS + (int)i);
	}
	selectionsMenu->Add(NULL); //Separator
	selectionsMenu->Add("Select previous", MENU_SELECTION_SELECTIONS + (int)i, SDLK_F11, ALT_MODIFIER);
	selectionsMenu->Add("Select next", MENU_SELECTION_SELECTIONS + (int)i + 1, SDLK_F12, ALT_MODIFIER);
}

void Interface::AddSelection(SelectionGroup s) {
	selections.push_back(s);
	RebuildSelectionMenus();
}

void Interface::ClearSelection(size_t idClr) {
	selections.erase(selections.begin() + idClr);
	RebuildSelectionMenus();
}

void Interface::ClearAllSelections() {
	selections.clear();
	ClearSelectionMenus();
}

void Interface::OverWriteSelection(size_t idOvr) {
	Geometry *geom = worker.GetGeometry();
	char *selectionName = GLInputBox::GetInput(selections[idOvr].name.c_str(), "Selection name", "Enter selection name");
	if (!selectionName) return;

	selections[idOvr].selection = geom->GetSelectedFacets();
	selections[idOvr].name = selectionName;
	RebuildSelectionMenus();
}

void Interface::AddSelection() {
	Geometry *geom = worker.GetGeometry();
	std::stringstream tmp;
	tmp << "Selection #" << (selections.size() + 1);
	char *selectionName = GLInputBox::GetInput(tmp.str().c_str(), "Selection name", "Enter selection name");
	if (!selectionName) return;

	SelectionGroup newSelection;
	newSelection.selection = geom->GetSelectedFacets();
	newSelection.name = selectionName;
	selections.push_back(newSelection);
	RebuildSelectionMenus();
}

//VIEWS

void Interface::ClearViewMenus() {
	memorizeViewsMenu->Clear();
	memorizeViewsMenu->Add("Add new...", MENU_VIEW_ADDNEW, SDLK_q, CTRL_MODIFIER);
	memorizeViewsMenu->Add(NULL); // Separator
	clearViewsMenu->Clear();
	clearViewsMenu->Add("Clear All", MENU_VIEW_CLEARALL);
	clearViewsMenu->Add(NULL); // Separator
	viewsMenu->Clear();
}

void Interface::RebuildViewMenus() {
	ClearViewMenus();
	std::vector<int> fKeys = { SDLK_F1,SDLK_F2,SDLK_F3,SDLK_F5,SDLK_F6,SDLK_F7,SDLK_F8,SDLK_F9,SDLK_F10,SDLK_F11,SDLK_F12 }; //Skip ALT+F4 shortcut :)
	for (int i = 0; i < nbView; i++) {
		if (fKeys.empty()) {
			viewsMenu->Add(views[i].name, MENU_VIEW_VIEWS + i);
		}
		else {
			viewsMenu->Add(views[i].name, MENU_VIEW_VIEWS + i, fKeys[0], ALT_MODIFIER);
			fKeys.erase(fKeys.begin());
		}
		clearViewsMenu->Add(views[i].name, MENU_VIEW_CLEARVIEWS + i);
		memorizeViewsMenu->Add(views[i].name, MENU_VIEW_MEMORIZEVIEWS + i);
	}
}

void Interface::AddView(const char *viewName, AVIEW v) {

	if (nbView < MAX_VIEW) {
		views[nbView] = v;
		views[nbView].name = strdup(viewName);
		nbView++;
	}
	else {
		SAFE_FREE(views[0].name);
		for (int i = 0; i < MAX_VIEW - 1; i++) views[i] = views[i + 1];
		views[MAX_VIEW - 1] = v;
		views[MAX_VIEW - 1].name = strdup(viewName);
	}
	RebuildViewMenus();
}

void Interface::ClearView(int idClr) {
	SAFE_FREE(views[idClr].name);
	for (int i = idClr; i < nbView - 1; i++) views[i] = views[i + 1];
	nbView--;
	RebuildViewMenus();
}

void Interface::ClearAllViews() {
	for (int i = 0; i < nbView; i++) SAFE_FREE(views[i].name);
	nbView = 0;
	ClearViewMenus();
}

void Interface::OverWriteView(int idOvr) {
	Geometry *geom = worker.GetGeometry();
	char *viewName = GLInputBox::GetInput(views[idOvr].name, "View name", "Enter view name");
	if (!viewName) return;

	views[idOvr] = viewer[curViewer]->GetCurrentView();
	views[idOvr].name = strdup(viewName);
	RebuildViewMenus();
}

void Interface::AddView() {
	Geometry *geom = worker.GetGeometry();
	char tmp[32];
	sprintf(tmp, "View #%d", nbView + 1);
	char *viewName = GLInputBox::GetInput(tmp, "View name", "Enter view name");
	if (!viewName) return;

	if (nbView < MAX_VIEW) {
		views[nbView] = viewer[curViewer]->GetCurrentView();
		views[nbView].name = strdup(viewName);
		nbView++;
	}
	else {
		SAFE_FREE(views[0].name);
		for (int i = 0; i < MAX_VIEW - 1; i++) views[i] = views[i + 1];
		views[MAX_VIEW - 1] = viewer[curViewer]->GetCurrentView();
		views[MAX_VIEW - 1].name = strdup(viewName);
	}
	RebuildViewMenus();
}

void Interface::RemoveRecent(const char *fileName) {

	if (!fileName) return;

	bool found = false;
	int i = 0;
	while (!found && i < nbRecent) {
		found = strcmp(fileName, recents[i]) == 0;
		if (!found) i++;
	}
	if (!found) return;

	SAFE_FREE(recents[i]);
	for (int j = i; j < nbRecent - 1; j++)
		recents[j] = recents[j + 1];
	nbRecent--;

	// Update menu
	GLMenu *m = menu->GetSubMenu("File")->GetSubMenu("Load recent");
	m->Clear();
	for (i = nbRecent - 1; i >= 0; i--)
		m->Add(recents[i], MENU_FILE_LOADRECENT + i);
	SaveConfig();
}

void Interface::AddRecent(const char *fileName) {

	// Check if already exists
	bool found = false;
	int i = 0;
	while (!found && i < nbRecent) {
		found = strcmp(fileName, recents[i]) == 0;
		if (!found) i++;
	}
	if (found) {
		for (int j = i; j < nbRecent - 1; j++) {
			recents[j] = recents[j + 1];
		}
		recents[nbRecent - 1] = strdup(fileName);
		// Update menu
		GLMenu *m = menu->GetSubMenu("File")->GetSubMenu("Load recent");
		m->Clear();
		for (int i = nbRecent - 1; i >= 0; i--)
			m->Add(recents[i], MENU_FILE_LOADRECENT + i);
		SaveConfig();
		return;
	}

	// Add the new recent file
	if (nbRecent < MAX_RECENT) {
		recents[nbRecent] = strdup(fileName);
		nbRecent++;
	}
	else {
		// Shift
		SAFE_FREE(recents[0]);
		for (int i = 0; i < MAX_RECENT - 1; i++)
			recents[i] = recents[i + 1];
		recents[MAX_RECENT - 1] = strdup(fileName);
	}

	// Update menu
	GLMenu *m = menu->GetSubMenu("File")->GetSubMenu("Load recent");
	m->Clear();
	for (int i = nbRecent - 1; i >= 0; i--)
		m->Add(recents[i], MENU_FILE_LOADRECENT + i);
	SaveConfig();
}

void Interface::AddStruct() {
	Geometry *geom = worker.GetGeometry();
	char tmp[32];
	sprintf(tmp, "Structure #%zd", geom->GetNbStructure() + 1);
	char *structName = GLInputBox::GetInput(tmp, "Structure name", "Enter name of new structure");
	if (!structName) return;
	geom->AddStruct(structName);
	// Send to sub process
	try { worker.Reload(); }
	catch (Error &e) {
		GLMessageBox::Display(e.GetMsg(), "Error", GLDLG_OK, GLDLG_ICONERROR);
		return;
	}
}

void Interface::DeleteStruct() {
	Geometry *geom = worker.GetGeometry();
	char *structNum = GLInputBox::GetInput("", "Structure number", "Number of structure to delete:");
	if (!structNum) return;
	int structNumInt;
	if (!sscanf(structNum, "%d", &structNumInt)) {
		GLMessageBox::Display("Invalid structure number");
		return;
	}
	if (structNumInt<1 || structNumInt>geom->GetNbStructure()) {
		GLMessageBox::Display("Invalid structure number");
		return;
	}
	bool hasFacets = false;
	for (int i = 0; i < geom->GetNbFacet() && !hasFacets; i++) {
		if (geom->GetFacet(i)->sh.superIdx == (structNumInt - 1)) hasFacets = true;
	}
	if (hasFacets) {
		int rep = GLMessageBox::Display("This structure has facets. They will be deleted with the structure.", "Structure delete", GLDLG_OK | GLDLG_CANCEL, GLDLG_ICONWARNING);
		if (rep != GLDLG_OK) return;
	}
	if (!AskToReset()) return;
	geom->DelStruct(structNumInt - 1);
	// Send to sub process
	try { worker.Reload(); }
	catch (Error &e) {
		GLMessageBox::Display(e.GetMsg(), "Error", GLDLG_OK, GLDLG_ICONERROR);
		return;
	}
}

void Interface::DisplayCollapseDialog() {
	Geometry *geom = worker.GetGeometry();
	if (!collapseSettings) collapseSettings = new CollapseSettings();
	collapseSettings->SetGeometry(geom, &worker);
	collapseSettings->SetVisible(true);
}

void Interface::RenumberSelections(const std::vector<int> &newRefs) {
	for (int i = 0; i < selections.size(); i++) {
		for (int j = 0; i>=0 && i < selections.size() && j>=0 && j < selections[i].selection.size(); j++) {
			if (selections[i].selection[j] >= newRefs.size() || newRefs[selections[i].selection[j]] == -1) { //remove from selection
				selections[i].selection.erase(selections[i].selection.begin()+j);
				j--; //Do again the element as now it's the next
				if (selections[i].selection.size() == 0) {
					ClearSelection(i); //last facet removed from selection
					i--;
				}
				
			}
			else { //renumber
				selections[i].selection[j] = newRefs[selections[i].selection[j]];
			}
		}
	}
}

void Interface::RenumberFormulas(std::vector<int> *newRefs) {
	for (auto& f:formulas_n) {
		if (OffsetFormula(f->GetExpression(), 0, -1, newRefs)) {
			f->Parse();
		}
	}
	if (formulaEditor && formulaEditor->IsVisible()) formulaEditor->Refresh();
}

/*

// Name: ProcessFormulaButtons()
// Desc: Handle forumla button event

void Interface::ProcessFormulaButtons(GLComponent *src) {

	// Search formula buttons
	bool found = false;
	int i = 0;
	while (!found && i < nbFormula) {
		found = (src == formulas[i].setBtn);
		if (!found) i++;
	}
	if (found) {
		if (!formulaSettings) formulaSettings = new FormulaSettings();
		formulaSettings->Update(formulas[i].parser, i);
		formulaSettings->SetVisible(true);
	}
}
*/

/*void Interface::AddFormula(GLParser *f, bool doUpdate) {

	if (f) {
		if (nbFormula < MAX_FORMULA) {
			formulas[nbFormula].parser = f;
			std::string formulaName = f->GetName();
			if (formulaName.empty()) formulaName = f->GetExpression();
			formulas[nbFormula].name = new GLLabel(formulaName.c_str());
			Add(formulas[nbFormula].name);
			formulas[nbFormula].value = new GLTextField(0, "");
			formulas[nbFormula].value->SetEditable(false);
			Add(formulas[nbFormula].value);
			formulas[nbFormula].setBtn = new GLButton(0, "...");
			Add(formulas[nbFormula].setBtn);
			nbFormula++;
			PlaceComponents();
			if (doUpdate) UpdateFormula();
		}
		else {
			SAFE_DELETE(f);
		}
	}

}*/

void Interface::AddFormula(const char *fName, const char *formula) {

	/*GLParser *f = new GLParser();
	f->SetExpression(formula);
	f->SetName(fName);
	f->Parse();
	AddFormula(f, false);*/

	GLParser *f2 = new GLParser();
	f2->SetExpression(formula);
	f2->SetName(fName);
	f2->Parse();
	formulas_n.push_back(f2);
}

void Interface::ClearFormulas() {

	/*for (int i = 0; i < nbFormula; i++) {
		wnd->PostDelete(formulas[i].name);
		wnd->PostDelete(formulas[i].value);
		wnd->PostDelete(formulas[i].setBtn);
		formulas[i].name = NULL;
		formulas[i].value = NULL;
		formulas[i].setBtn = NULL;
		SAFE_DELETE(formulas[i].parser);
	}
	nbFormula = 0;

	PlaceComponents();
	*/

	for (auto& f : formulas_n)
		SAFE_DELETE(f);
	formulas_n.clear();
	if (formulaEditor) formulaEditor->Refresh();

}

/*
void Interface::UpdateFormulaName(int i) {
		std::string formulaName = formulas[i].parser->GetName();
		if (formulaName.empty()) formulaName = formulas[i].parser->GetExpression();
		formulas[i].name->SetText(formulaName.c_str());
		UpdateFormula();
}
*/

/*
void Interface::DeleteFormula(int i) {
			// Delete
			wnd->PostDelete(formulas[i].name);
			wnd->PostDelete(formulas[i].value);
			wnd->PostDelete(formulas[i].setBtn);
			formulas[i].name = NULL;
			formulas[i].value = NULL;
			formulas[i].setBtn = NULL;
			SAFE_DELETE(formulas[i].parser);
			for (int j = i; j < nbFormula - 1; j++) {
				formulas[j] = formulas[j + 1];
			}
			nbFormula--;
			PlaceComponents();
			wnd->DoPostDelete(); //forces redraw
			//UpdateFormula(); //no new values needed
}
*/

bool Interface::OffsetFormula(char* expression, int offset, int filter, std::vector<int> *newRefs) {
	//will increase or decrease facet numbers in a formula
	//only applies to facet numbers larger than "filter" parameter
	//If *newRefs is not NULL, a vector is passed containing the new references
	bool changed = false;

	string newExpr = expression; //convert char* to string

	size_t pos = 0; //analyzed until this position
	while (pos < newExpr.size()) { //while not end of expression

		vector<size_t> location; //for each prefix, we store where it was found

		for (size_t j = 0; j < formulaPrefixes.size(); j++) { //try all expressions
			location.push_back(newExpr.find(formulaPrefixes[j], pos));
		}
		size_t minPos = string::npos;
		size_t maxLength = 0;
		for (size_t j = 0; j < formulaPrefixes.size(); j++)  //try all expressions, find first prefix location
			if (location[j] < minPos) minPos = location[j];
		for (size_t j = 0; j < formulaPrefixes.size(); j++)  //try all expressions, find longest prefix at location
			if (location[j] == minPos && formulaPrefixes[j].size() > maxLength) maxLength = formulaPrefixes[j].size();
		int digitsLength = 0;
		if (minPos != string::npos) { //found expression, let's find tailing facet number digits
			while ((minPos + maxLength + digitsLength) < newExpr.length() && newExpr[minPos + maxLength + digitsLength] >= '0' && newExpr[minPos + maxLength + digitsLength] <= '9')
				digitsLength++;
			if (digitsLength > 0) { //there was a digit after the prefix
				int facetNumber;
				if (sscanf(newExpr.substr(minPos + maxLength, digitsLength).c_str(), "%d", &facetNumber)) {
					if (newRefs == NULL) { //Offset mode
						if ((facetNumber - 1) > filter) {
							char tmp[10];
							sprintf(tmp, "%d", facetNumber + offset);
							newExpr.replace(minPos + maxLength, digitsLength, tmp);
							changed = true;
						}
						else if ((facetNumber - 1) == filter) {
							newExpr.replace(minPos + maxLength, digitsLength, "0");
							changed = true;
						}
					}
					else { //newRefs mode
						if ((facetNumber - 1) >= (*newRefs).size() || (*newRefs)[facetNumber - 1] == -1) { //Facet doesn't exist anymore
							newExpr.replace(minPos + maxLength, digitsLength, "0");
							changed = true;
						}
						else { //Update facet number
							char tmp[12];
							sprintf(tmp, "%d", (*newRefs)[facetNumber - 1]+1);
							newExpr.replace(minPos + maxLength, digitsLength, tmp);
							changed = true;
						}
					}
				}
			}
		}
		if (minPos != string::npos) pos = minPos + maxLength + digitsLength;
		else pos = minPos;
	}
	strcpy(expression, newExpr.c_str());
	return changed;
}

int Interface::Resize(size_t width, size_t height, bool forceWindowed) {
	int r = GLApplication::Resize(width, height, forceWindowed);
	PlaceComponents();
	worker.RebuildTextures();
	return r;
}

void Interface::UpdateFacetlistSelected() {
	Geometry *geom = worker.GetGeometry();
	auto selectedFacets = geom->GetSelectedFacets();
	//facetList->SetSelectedRows(selection,nbSelected,true);
	if (selectedFacets.size() > 1000) {
		facetList->ReOrder();
		facetList->SetSelectedRows(selectedFacets, false);
	}
	else {
		facetList->SetSelectedRows(selectedFacets, true);
	}
}

int Interface::GetVariable(const char *name, const char *prefix) {

	char tmp[256];
	int  idx;
	int lgthP = (int)strlen(prefix);
	int lgthN = (int)strlen(name);

	if (lgthP >= lgthN) {
		return -1;
	}
	else {
		strcpy(tmp, name);
		tmp[lgthP] = 0;

		if (iequals(tmp, prefix)) {
			strcpy(tmp, name + lgthP);
			int conv = sscanf(tmp, "%d", &idx);
			if (conv) {
				return idx;
			}
			else {
				return -1;
			}
		}
	}
	return -1;

}

/*
void Interface::UpdateFormula() {

	char tmp[256];

	Geometry *geom = worker.GetGeometry();

	for (int i = 0; i < nbFormula; i++) {

		
		GLParser *f = formulas[i].parser;
		//f->Parse(); //If selection group changed
		

					// Evaluate variables
		int nbVar = f->GetNbVariable();
		bool ok = true;
		for (int j = 0; j < nbVar && ok; j++) {
			VLIST *v = f->GetVariableAt(j);
			ok = EvaluateVariable(v);
			if (!ok) {
				std::stringstream tmp;
				tmp << "Invalid variable " << v->name;
				formulas[i].value->SetText(tmp.str());
			}
		}

		// Evaluation
		if (ok) { //Variables succesfully evaluated
			double r;
			if (f->Evaluate(&r)) {
				sprintf(tmp, "%g", r);
				formulas[i].value->SetText(tmp);
			}
			else { //Variables OK but the formula itself can't be evaluated
				formulas[i].value->SetText(f->GetErrorMsg());
			}
#ifdef MOLFLOW
			formulas[i].value->SetTextColor(0.0f, 0.0f, worker.displayedMoment == 0 ? 0.0f : 1.0f);
#endif
		}
		else { //Error while evaluating variables
			//formulas[i].value->SetText("Invalid variable name"); //We set it directly at the error location
		}
	}
}
*/

bool Interface::AskToSave() {
	if (!changedSinceSave) return true;
	int ret = GLSaveDialog::Display("Save current geometry first?", "File not saved", GLDLG_SAVE | GLDLG_DISCARD | GLDLG_CANCEL_S, GLDLG_ICONINFO);
	if (ret == GLDLG_SAVE) {
		//FILENAME *fn = GLFileBox::SaveFile(currentDir, worker.GetCurrentShortFileName(), "Save File", fileSFilters, 0);
		std::string fn = NFD_SaveFile_Cpp(fileSaveFilters, "");
		if (!fn.empty()) {
			GLProgress *progressDlg2 = new GLProgress("Saving file...", "Please wait");
			progressDlg2->SetVisible(true);
			progressDlg2->SetProgress(0.0);
			//GLWindowManager::Repaint();
			try {
				worker.SaveGeometry(fn.c_str(), progressDlg2);
				changedSinceSave = false;
				UpdateCurrentDir(fn.c_str());
				UpdateTitle();
				AddRecent(fn.c_str());
			}
			catch (Error &e) {
				char errMsg[512];
				sprintf(errMsg, "%s\nFile:%s", e.GetMsg(), fn.c_str());
				GLMessageBox::Display(errMsg, "Error", GLDLG_OK, GLDLG_ICONERROR);
				RemoveRecent(fn.c_str());
			}
			progressDlg2->SetVisible(false);
			SAFE_DELETE(progressDlg2);
			return true;
		}
		else return false;
	}
	else if (ret == GLDLG_DISCARD) return true;
	return false;
}

void Interface::CreateOfTwoFacets(ClipperLib::ClipType type, int reverseOrder) {
	Geometry *geom = worker.GetGeometry();
	if (geom->IsLoaded()) {
		try {
			if (AskToReset()) {
				//geom->CreateDifference();
				geom->ClipSelectedPolygons(type,reverseOrder);
			}
		}
		catch (Error &e) {
			GLMessageBox::Display(e.GetMsg(), "Error creating polygon", GLDLG_OK, GLDLG_ICONERROR);
		}
		//UpdateModelParams();
		try { worker.Reload(); }
		catch (Error &e) {
			GLMessageBox::Display(e.GetMsg(), "Error reloading worker", GLDLG_OK, GLDLG_ICONERROR);
		}
	}
	else GLMessageBox::Display("No geometry loaded.", "No geometry", GLDLG_OK, GLDLG_ICONERROR);
}

void Interface::UpdateRecentMenu(){
	// Update menu
	GLMenu *m = menu->GetSubMenu("File")->GetSubMenu("Load recent");
	m->Clear();
	for (int i = nbRecent - 1; i >= 0; i--)
		m->Add(recents[i], MENU_FILE_LOADRECENT + i);
}

void Interface::SaveFileAs() {

	//FILENAME *fn = GLFileBox::SaveFile(currentDir, worker.GetCurrentShortFileName(), "Save File", fileSFilters, 0);
	std::string fn = NFD_SaveFile_Cpp(fileSaveFilters, "");
	
	GLProgress *progressDlg2 = new GLProgress("Saving file...", "Please wait");
	progressDlg2->SetProgress(0.0);
	progressDlg2->SetVisible(true);
	//GLWindowManager::Repaint();  
	if (!fn.empty()) {

		try {

			worker.SaveGeometry(fn.c_str(), progressDlg2);
			ResetAutoSaveTimer();
			changedSinceSave = false;
			UpdateCurrentDir(worker.fullFileName);
			UpdateTitle();
			AddRecent(worker.fullFileName);
		}
		catch (Error &e) {
			char errMsg[512];
			sprintf(errMsg, "%s\nFile:%s", e.GetMsg(), fn.c_str());
			GLMessageBox::Display(errMsg, "Error", GLDLG_OK, GLDLG_ICONERROR);
			RemoveRecent(fn.c_str());
		}

	}

	progressDlg2->SetVisible(false);
	SAFE_DELETE(progressDlg2);
}

void Interface::ExportTextures(int grouping, int mode) {

	Geometry *geom = worker.GetGeometry();
	if (geom->GetNbSelectedFacets() == 0) {
		GLMessageBox::Display("Empty selection", "Error", GLDLG_OK, GLDLG_ICONERROR);
		return;
	}
	if (!worker.IsDpInitialized()) {
		GLMessageBox::Display("Worker Dataport not initialized yet", "Error", GLDLG_OK, GLDLG_ICONERROR);
		return;
	}

	//FILENAME *fn = GLFileBox::SaveFile(currentDir, NULL, "Save File", fileTexFilters, 0);
	std::string fn = NFD_SaveFile_Cpp(fileTexFilters, "");
	if (!fn.empty()) {

		try {
			worker.ExportTextures(fn.c_str(), grouping, mode, true, true);
			//UpdateCurrentDir(fn->fullName);
			//UpdateTitle();
		}
		catch (Error &e) {
			char errMsg[512];
			sprintf(errMsg, "%s\nFile:%s", e.GetMsg(), fn.c_str());
			GLMessageBox::Display(errMsg, "Error", GLDLG_OK, GLDLG_ICONERROR);
		}

	}

}

void Interface::DoEvents(bool forced)
{
	static int lastChkEvent = 0;
	static int lastRepaint = 0;
	int time = SDL_GetTicks();
	if (forced || (time - lastChkEvent > 200)) { //Don't check for inputs more than 5 times a second
		SDL_Event sdlEvent;
		SDL_PollEvent(&sdlEvent);
		UpdateEventCount(&sdlEvent);
		/*if (GLWindowManager::ManageEvent(&sdlEvent)) {
		// Relay to GLApp EventProc
		mApp->EventProc(&sdlEvent);
		}*/
		GLWindowManager::ManageEvent(&sdlEvent);
		lastChkEvent = time;
	}
	if (forced || (time - lastRepaint > 500)) { //Don't redraw more than every 500 msec
		GLWindowManager::Repaint();
		GLToolkit::CheckGLErrors("GLApplication::Paint()");
		lastRepaint = time;
	}

}

bool Interface::AskToReset(Worker *work) {
	if (work == NULL) work = &worker;
	if (work->globalHitCache.globalHits.nbMCHit > 0) {
		int rep = GLMessageBox::Display("This will reset simulation data.", "Geometry change", GLDLG_OK | GLDLG_CANCEL, GLDLG_ICONWARNING);
		if (rep == GLDLG_OK) {
			work->ResetStatsAndHits(m_fTime);
			nbDesStart = 0;
			nbHitStart = 0;

			//resetSimu->SetEnabled(false);
			UpdatePlotters();
			return true;
		}
		else return false;
	}
	else return true;
}

int Interface::FrameMove()
{
	char tmp[256];
	Geometry *geom = worker.GetGeometry();

	//Autosave routines
	bool timeForAutoSave = false;
	if (geom->IsLoaded()) {
		if (autoSaveSimuOnly) {
			if (worker.isRunning) {
				if (((worker.simuTime + (m_fTime - worker.startTime)) - lastSaveTimeSimu) >= (float)autoSaveFrequency*60.0f) {
					timeForAutoSave = true;
				}
			}
		}
		else {
			if ((m_fTime - lastSaveTime) >= (float)autoSaveFrequency*60.0f) {
				timeForAutoSave = true;
			}
		}
	}
	
	if (worker.isRunning) {
		if (m_fTime - lastUpdate >= 1.0f) {

			sprintf(tmp, "Running: %s", FormatTime(worker.simuTime + (m_fTime - worker.startTime)));
			sTime->SetText(tmp);
			wereEvents = true; //Will repaint

			UpdateStats(); //Update m_fTime
			lastUpdate = m_fTime;

			if (updateRequested || autoFrameMove) {

				forceFrameMoveButton->SetEnabled(false);
				forceFrameMoveButton->SetText("Updating...");
				//forceFrameMoveButton->Paint();
				//GLWindowManager::Repaint();

				updateRequested = false;

				// Update hits
				try {
					worker.Update(m_fTime);
				}
				catch (Error &e) {
					GLMessageBox::Display(e.GetMsg(), "Error (Stop)", GLDLG_OK, GLDLG_ICONERROR);
				}
				// Simulation monitoring
				UpdatePlotters();

				// Formulas
				//if (autoUpdateFormulas) UpdateFormula();
				if (autoUpdateFormulas && formulaEditor && formulaEditor->IsVisible()) formulaEditor->ReEvaluate();
				if (particleLogger && particleLogger->IsVisible()) particleLogger->UpdateStatus();
				//lastUpdate = GetTick(); //changed from m_fTime: include update duration

				// Update timing measurements
				if (worker.globalHitCache.globalHits.nbMCHit != lastNbHit || worker.globalHitCache.globalHits.nbDesorbed != lastNbDes) {
					double dTime = (double)(m_fTime - lastMeasTime);
					hps = (double)(worker.globalHitCache.globalHits.nbMCHit - lastNbHit) / dTime;
					dps = (double)(worker.globalHitCache.globalHits.nbDesorbed - lastNbDes) / dTime;
					if (lastHps != 0.0) {
						hps = 0.2*(hps)+0.8*lastHps;
						dps = 0.2*(dps)+0.8*lastDps;
					}
					lastHps = hps;
					lastDps = dps;
					lastNbHit = worker.globalHitCache.globalHits.nbMCHit;
					lastNbDes = worker.globalHitCache.globalHits.nbDesorbed;
					lastMeasTime = m_fTime;
				}
			}
		}

#ifdef MOLFLOW
		if (worker.calcAC) {
			sprintf(tmp, "Calc AC: %s (%zd %%)", FormatTime(worker.simuTime + (m_fTime - worker.startTime)),
				worker.calcACprg);
		}
		else {
#endif
			
#ifdef MOLFLOW
		}
#endif

		forceFrameMoveButton->SetEnabled(!autoFrameMove);
		forceFrameMoveButton->SetText("Update");
	}
	else {
		if (worker.simuTime > 0.0) {
			hps = (double)(worker.globalHitCache.globalHits.nbMCHit - nbHitStart) / worker.simuTime;
			dps = (double)(worker.globalHitCache.globalHits.nbDesorbed - nbDesStart) / worker.simuTime;
		}
		else {
			hps = 0.0;
			dps = 0.0;
		}
		sprintf(tmp, "Stopped: %s", FormatTime(worker.simuTime));
		sTime->SetText(tmp);
	}

	
	// Facet parameters and hits
	if (viewer[0]->SelectionChanged() ||
		viewer[1]->SelectionChanged() ||
		viewer[2]->SelectionChanged() ||
		viewer[3]->SelectionChanged()) {
		UpdateFacetParams(true);
	}
	UpdateFacetHits();
	//Autosave
	if (timeForAutoSave) AutoSave();

	//Check if app updater has found updates
	if (appUpdater && appUpdater->IsUpdateAvailable()) {
		if (!updateLogWindow) {
			updateLogWindow = new UpdateLogWindow(this);
		}
		if (!updateFoundDialog) {
			updateFoundDialog = new UpdateFoundDialog(appName, appVersionName, appUpdater, updateLogWindow);
			updateFoundDialog->SetVisible(true);
			wereEvents = true;
		}
	}

	if (worker.globalHitCache.nbLeakTotal) {
		sprintf(tmp, "%g (%.4f%%)", (double)worker.globalHitCache.nbLeakTotal, (double)(worker.globalHitCache.nbLeakTotal)*100.0 / (double)worker.globalHitCache.globalHits.nbDesorbed);
		leakNumber->SetText(tmp);
	}
	else {
		leakNumber->SetText("None");
	}
	resetSimu->SetEnabled(!worker.isRunning&&worker.globalHitCache.globalHits.nbDesorbed > 0);

	if (worker.isRunning) {
		startSimu->SetText("Pause");
		//startSimu->SetFontColor(255, 204, 0);
	}
	else if (worker.globalHitCache.globalHits.nbMCHit > 0) {
		startSimu->SetText("Resume");
		//startSimu->SetFontColor(0, 140, 0);
	}
	else {
		startSimu->SetText("Begin");
		//startSimu->SetFontColor(0, 140, 0);
	}

	/*
	// Sleep a bit to avoid unwanted CPU load
	if (viewer[0]->IsDragging() ||
		viewer[1]->IsDragging() ||
		viewer[2]->IsDragging() ||
		viewer[3]->IsDragging() || !worker.running)
	{
		SDL_Delay(22); //was 22
	}
	else
	{
		SDL_Delay(60); //was 60
	}
	*/
	
	double delayTime = 0.03 - (wereEvents?fPaintTime:0.0) - fMoveTime;
	if (delayTime > 0) SDL_Delay((int)(1000.0*delayTime)); //Limits framerate at about 60fps
	return GL_OK;
}

void Interface::ResetAutoSaveTimer() {
	UpdateStats(); //updates m_fTime
	if (autoSaveSimuOnly) lastSaveTimeSimu = worker.simuTime + (m_fTime - worker.startTime);
	else lastSaveTime = m_fTime;
}

bool Interface::AutoSave(bool crashSave) {
	if (!changedSinceSave) return true;
	GLProgress *progressDlg2 = new GLProgress("Peforming autosave...", "Please wait");
	progressDlg2->SetProgress(0.0);
	progressDlg2->SetVisible(true);
	//GLWindowManager::Repaint();

	std::string shortFn(worker.GetCurrentShortFileName());
	std::string newAutosaveFilename = appName + "_Autosave";
	if (shortFn != "") newAutosaveFilename += "(" + shortFn + ")";
#ifdef MOLFLOW
	newAutosaveFilename += ".zip";
#endif
#ifdef SYNRAD
	newAutosaveFilename += ".syn7z";
#endif
	char fn[1024];
	strcpy(fn, newAutosaveFilename.c_str());
	try {
		worker.SaveGeometry(fn, progressDlg2, false, false, true, crashSave);
		//Success:
		if (autosaveFilename != "" && autosaveFilename != newAutosaveFilename) remove(autosaveFilename.c_str());
		autosaveFilename = newAutosaveFilename;
		ResetAutoSaveTimer(); //deduct saving time from interval
	}
	catch (Error &e) {
		GLMessageBox::Display(std::string(e.GetMsg()) + "\n" + fn, "Autosave error", { "OK" }, GLDLG_ICONERROR);
		progressDlg2->SetVisible(false);
		SAFE_DELETE(progressDlg2);
		ResetAutoSaveTimer();
		return false;
	}
	//lastSaveTime=(worker.simuTime+(m_fTime-worker.startTime));
	progressDlg2->SetVisible(false);
	SAFE_DELETE(progressDlg2);
	wereEvents = true;
	return true;
}

void Interface::CheckForRecovery() {
	// Check for autosave files in current dir.
	auto curPath = std::filesystem::current_path(); //string (POSIX) or wstring (Windows)
	for (const auto & p : std::filesystem::directory_iterator(curPath)) {
		
		std::string path_str = p.path().u8string();
		std::string fileName_str = p.path().filename().u8string();

		if (beginsWith(fileName_str, appName + "_Autosave")) {
			std::ostringstream msg;
			msg << "Autosave file found:\n" << fileName_str << "\n";
			int rep = RecoveryDialog::Display(msg.str().c_str(), "Autosave recovery", GLDLG_LOAD | GLDLG_SKIP, GLDLG_DELETE);
			if (rep == GLDLG_LOAD) {
				LoadFile(path_str);
				RemoveRecent(path_str.c_str());
			}
			else if (rep == GLDLG_CANCEL) return;
			else if (rep == GLDLG_SKIP) continue;
			else if (rep == GLDLG_DELETE) remove(p.path());
		}
	}
}