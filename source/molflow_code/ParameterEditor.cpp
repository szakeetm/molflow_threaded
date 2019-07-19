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
#include "ParameterEditor.h"
#include "File.h"
#include "GLApp/GLToolkit.h"
#include "GLApp/GLMessageBox.h"
#include "GLApp/GLLabel.h"
#include "GLApp/GLButton.h"
#include "GLApp/GLCombo.h"
#include "GLApp/GLChart/GLChart.h"
#include "GLApp/GLTextField.h"
#include "GLApp/GLToggle.h"
#include "GLApp/GLTitledPanel.h"
#include "GLApp/GLList.h"
#include "GLApp/MathTools.h"

#include "MolFlow.h"
//#include "GLApp/GLFileBox.h"
#include "NativeFileDialog/molflow_wrapper/nfd_wrapper.h"
#include <sstream>

extern MolFlow *mApp;

  static const int   flWidth[] = {107,107};
  static const char *flName[] = {"Time (s)","Value"};
  static const int   flAligns[] = { ALIGN_LEFT,ALIGN_LEFT };
  static const int   fEdits[] = { EDIT_STRING,EDIT_STRING };

ParameterEditor::ParameterEditor(Worker *w):GLWindow() {

  int wD = 700;
  int hD = 400;

  work=w;
  userValues = std::vector<std::pair<std::string, std::string>>();
  dataView = new GLDataView();

  int hSpace = 10;
  int vSpace = 5;
  int col1 = 10;
  int col2 = 250;
  int cursorX = col1;
  int cursorY = 5;
  int buttonWidth = 110;
  int toggleWidth = 40;
  int labelWidth = 30;
  int compHeight = 20;
  int panelHeight = 345;
  int listHeight = 265;

  SetTitle("Edit parameters");

  selectorCombo = new GLCombo(0);
  selectorCombo->SetBounds(cursorX, cursorY, col2 - cursorX - 5, compHeight);
  Add(selectorCombo);

  cursorX = col2;
  deleteButton = new GLButton(2, "Delete");
  deleteButton->SetBounds(cursorX, cursorY, buttonWidth, compHeight);
  deleteButton->SetEnabled(false);
  Add(deleteButton);

  /*
  cursorX += buttonWidth+hSpace;
  newButton = new GLButton(1, "Create new");
  newButton->SetBounds(cursorX, cursorY, buttonWidth, compHeight);
  Add(newButton);*/

  cursorX = col1;
  cursorY += compHeight+vSpace;
  editorPanel=new GLTitledPanel("Editor");
  editorPanel->SetBounds(5, cursorY, wD - 10, panelHeight);
  Add(editorPanel);

  cursorY += compHeight;
  GLLabel *nameLabel = new GLLabel("Name:");
  nameLabel->SetBounds(cursorX, cursorY, labelWidth, compHeight);
  Add(nameLabel);

  cursorX += labelWidth + hSpace;
  nameField = new GLTextField(3, "Param1");
  nameField->SetBounds(cursorX, cursorY, col2 - col1 - labelWidth - 2 * hSpace, compHeight);
  Add(nameField);

  cursorX = col2;
  plotArea = new GLChart(0);
  plotArea->SetBounds(cursorX, cursorY, wD - col2 - hSpace, compHeight + vSpace + listHeight);
  plotArea->SetBorder(BORDER_BEVEL_IN);
  plotArea->GetY1Axis()->SetGridVisible(true);
  plotArea->SetLabelVisible(false);
  dataView->SetMarker(MARKER_DOT);
  dataView->SetLineWidth(2);
  dataView->SetMarkerSize(8);
  plotArea->GetY1Axis()->AddDataView(dataView);
  plotArea->GetXAxis()->SetGridVisible(true);
  plotArea->GetY1Axis()->SetAutoScale(true);
  plotArea->GetY2Axis()->SetAutoScale(true);
  plotArea->GetY1Axis()->SetAnnotation(VALUE_ANNO);
  plotArea->GetXAxis()->SetAnnotation(VALUE_ANNO);
  Add(plotArea);

  cursorX = col1;
  cursorY += compHeight + vSpace;
  list = new GLList(0);
  list->SetBounds(cursorX, cursorY, col2-col1-hSpace, listHeight);
  list->SetColumnLabelVisible(true);
  list->SetGrid(true);
  //list->SetSelectionMode(BOX_CELL);
  Add(list);

  cursorX = col1;
  cursorY += listHeight + vSpace;
  /*copyButton = new GLButton(0,"Copy to clipboard");
  copyButton->SetBounds(cursorX, cursorY, buttonWidth, compHeight);
  Add(copyButton);

  cursorX += buttonWidth + hSpace;*/
  pasteButton = new GLButton(0,"Paste from clipboard");
  pasteButton->SetBounds(cursorX,cursorY,buttonWidth,compHeight);
  //pasteButton->SetEnabled(false);
  Add(pasteButton);

  cursorX += buttonWidth + hSpace;
  loadCSVbutton = new GLButton(0, "Load CSV file");
  loadCSVbutton->SetBounds(cursorX, cursorY, buttonWidth, compHeight);
  
  Add(loadCSVbutton);
  cursorX += buttonWidth + hSpace;

  logXtoggle = new GLToggle(0, "LogX");
  logXtoggle->SetBounds(cursorX, cursorY, toggleWidth, compHeight);
  Add(logXtoggle);

  cursorX += toggleWidth;
  logYtoggle = new GLToggle(0, "LogY");
  logYtoggle->SetBounds(cursorX, cursorY, toggleWidth, compHeight);
  Add(logYtoggle);

  cursorX = wD-2*buttonWidth-2*hSpace;
  plotButton = new GLButton(0, "Plot");
  plotButton->SetBounds(cursorX, cursorY, buttonWidth, compHeight);
  
  Add(plotButton);

  cursorX += buttonWidth + hSpace;
  applyButton = new GLButton(0, "Apply");
  applyButton->SetBounds(cursorX, cursorY, buttonWidth, compHeight);
  
  Add(applyButton);

  Refresh();
  
  // Center dialog
  int wS,hS;
  GLToolkit::GetScreenSize(&wS,&hS);
  int xD = (wS-wD)/2;
  int yD = (hS-hD)/2;
  SetBounds(xD,yD,wD,hD);
  

  RestoreDeviceObjects();
  
}

void ParameterEditor::Refresh()
{
	UpdateCombo();
	UpdateUserValues();
	RebuildList();
	if (work->parameters.size() > 0) { //If there is at least one parameter, show values and plot it 
		ValidateInput();
		Plot();
	}
}

void ParameterEditor::ProcessMessage(GLComponent *src,int message) {
  switch(message) {
    case MSG_BUTTON:
		if (Contains({ applyButton,deleteButton }, src) ) {
			int i = selectorCombo->GetSelectedIndex();
			if (i > 0 && i-1 < work->parameters.size()) {
				if (work->parameters[i-1].fromCatalog) {
					GLMessageBox::Display("Parameters imported from the catalog (.csv files in the parameter_catalog folder) are read-only.");
					return;
				}
			}
		}
		if (src==applyButton) {
			if (ValidateInput() && mApp->AskToReset()) { //ValidateInput() constructs tempParam
				if (selectorCombo->GetSelectedIndex() >= 1 && selectorCombo->GetSelectedIndex() < selectorCombo->GetNbRow()) {//existing param
					work->parameters[selectorCombo->GetSelectedIndex()-1] = tempParam;
					UpdateCombo(); //If name changed
					UpdateUserValues();
					Plot();
				} else if (selectorCombo->GetSelectedIndex() == 0) { //new Param
					//work->parameters.push_back(tempParam); //Inserted newly defined parameter as last
					size_t insertPos = work->InsertParametersBeforeCatalog({ tempParam }); //Inserts before catalog
					UpdateCombo();
					selectorCombo->SetSelectedIndex((int)insertPos); //Set to newly added parameter
					deleteButton->SetEnabled(true);
					UpdateUserValues();
					Plot();
				}
				RebuildList(); //Show parsed and sorted values
				work->Reload(); //Mark for re-sync with workers
			}
		} else if (src == deleteButton) {
			if (selectorCombo->GetSelectedValue() == "New...") return; //Delete button shouldn't have been enabled
			if (mApp->AskToReset()) {
				work->parameters.erase(work->parameters.begin() + selectorCombo->GetSelectedIndex()-1);
				UpdateCombo();
				bool paramSelected = selectorCombo->GetSelectedIndex() > 0;
				UpdateUserValues();
				RebuildList();
				if (paramSelected) {
					ValidateInput();
					Plot();
				}
				else {
					dataView->Reset();
				}
				deleteButton->SetEnabled(paramSelected); //has at least one valid parameter
			}
		} else if (src == plotButton) {
			ValidateInput();
			Plot();
		} else if (src==pasteButton) {
			PasteFromClipboard();
		} else if (src==loadCSVbutton) {
			LoadCSV();
		}
		/*else if (src == copyButton) {
			CopyToClipboard();
		}*/ //Removed as does the same as right-click and Copy All
		break;
	case MSG_TEXT:

	case MSG_LIST:
		for (int row=0;row<(list->GetNbRow()-1);row++) {
			if ((strcmp(list->GetValueAt(0, row), userValues[row].first.c_str()) != 0) ||
				(strcmp(list->GetValueAt(1, row), userValues[row].second.c_str()) != 0)) { //something changed
				if (*(list->GetValueAt(0, row)) != 0 ||
					*(list->GetValueAt(1, row)) != 0) //there is some content
					userValues[row]=std::make_pair(list->GetValueAt(0, row),list->GetValueAt(1, row)); //update pair
				else{ //no content
					userValues.erase(userValues.begin()+row); //erase
					RebuildList();
				}
				break;
			}
		}
		if ((list->GetValueAt(0,list->GetNbRow()-1)!=0 && (*(list->GetValueAt(0, list->GetNbRow() - 1)) != 0)) ||
			(list->GetValueAt(1,list->GetNbRow()-1)!=0 && (*(list->GetValueAt(1, list->GetNbRow() - 1)) != 0))) {
			//Add new line
			std::string first = (list->GetValueAt(0, list->GetNbRow() - 1) != 0) ? list->GetValueAt(0, list->GetNbRow() - 1) : "";
			std::string second = (list->GetValueAt(1, list->GetNbRow() - 1) != 0) ? list->GetValueAt(1, list->GetNbRow() - 1) : "";
			userValues.push_back(std::make_pair(first,second)); //add moment
			RebuildList();
		}
		break;
	case MSG_COMBO:
		if (selectorCombo->GetSelectedIndex() >= 1 && selectorCombo->GetSelectedIndex() < selectorCombo->GetNbRow()) { //existing param
			UpdateUserValues();
			ValidateInput();
			Plot();
			deleteButton->SetEnabled(true);
		}
		else {
			if (selectorCombo->GetSelectedIndex() != -1) { //New param
				PrepareForNewParam();
			}
			deleteButton->SetEnabled(false);
		}
		RebuildList();
		break;
	case MSG_TOGGLE:
		if (src==logXtoggle) plotArea->GetXAxis()->SetScale(logXtoggle->GetState());
		else if (src == logYtoggle) plotArea->GetY1Axis()->SetScale(logYtoggle->GetState());
		break;
}

  GLWindow::ProcessMessage(src,message);
}

void ParameterEditor::PrepareForNewParam() {
	//Clear table
	userValues = std::vector<std::pair<std::string, std::string>>();
	RebuildList();
	//Clear plot
	dataView->Reset();
	//Set default name for new param
	nameField->SetText("Param"+std::to_string(work->parameters.size()+1));
}

void ParameterEditor::UpdateCombo() {
	selectorCombo->SetSize((int)work->parameters.size()+1);
	selectorCombo->SetValueAt(0, "New...");
	for (size_t i = 0; i < work->parameters.size(); i++) {
		selectorCombo->SetValueAt((int)i + 1, work->parameters[i].name.c_str());
	}

	if (selectorCombo->GetSelectedIndex() < 1) selectorCombo->SetSelectedIndex(selectorCombo->GetNbRow()>1 ? 1 : 0); //Select first param, otherwise "New..."
	else if (selectorCombo->GetSelectedIndex() > (int)selectorCombo->GetNbRow() - 1) selectorCombo->SetSelectedIndex((int)selectorCombo->GetNbRow() - 1); //Select last (probably just deleted the last)
	if (selectorCombo->GetSelectedIndex() > 0) deleteButton->SetEnabled(true);
	if (selectorCombo->GetSelectedIndex()==0) PrepareForNewParam(); //If "New..." selected
}

void ParameterEditor::PasteFromClipboard() {
	list->PasteClipboardText(true,false,0); //Paste clipboard text, allow adding more rows, have one extra line in the end
	//Fill uservalues vector with pasted text
	userValues = std::vector<std::pair<std::string, std::string>>();
	for (int row = 0;row<(list->GetNbRow());row++) {
		if (*(list->GetValueAt(0, row)) != 0 ||
			*(list->GetValueAt(1, row)) != 0) { //There is some content
			userValues.push_back ( std::make_pair(list->GetValueAt(0, row), list->GetValueAt(1, row)) ); //update pair
		}
	}
	RebuildList();
}

void ParameterEditor::LoadCSV() {

	std::string fn = NFD_OpenFile_Cpp("csv", "");
	if (fn.empty()) return;

	std::vector<std::vector<std::string>> table;
	try {
		FileReader *f = new FileReader(fn);
		table = work->ImportCSV_string(f);
		SAFE_DELETE(f);
	}
	catch (Error &e) {
		char errMsg[512];
		sprintf(errMsg, "Failed to load CSV file.\n%s",e.GetMsg());
		GLMessageBox::Display(errMsg, "Error", GLDLG_OK, GLDLG_ICONERROR);
		return;
	}
	if (table.size() == 0) {
		GLMessageBox::Display("No full rows found in file", "Error", GLDLG_OK, GLDLG_ICONERROR);
		return;
	}
	if (table[0].size() != 2) {
		char tmp[256];
		sprintf(tmp, "First row has %zd columns instead of the expected 2. Load anyway?",table[0].size());
		if (GLDLG_OK != GLMessageBox::Display(tmp, "Warning", GLDLG_OK|GLDLG_CANCEL, GLDLG_ICONWARNING)) return;
	}
	if (table.size() > 30) {
		char tmp[256];
		sprintf(tmp, "CSV file has a large number (%zd) of rows. Load anyway?", table.size());
		if (GLDLG_OK != GLMessageBox::Display(tmp, "Warning", GLDLG_OK|GLDLG_CANCEL, GLDLG_ICONWARNING)) return;
	}
	userValues = std::vector<std::pair<std::string, std::string>>();
	for (auto& row :table) {
			std::string val1, val2;
			if (row.size()>=1) val1 = row[0];
			if (row.size()>=2) val2 = row[1];
			if (val1!="" || val2!="") userValues.push_back(std::make_pair(val1, val2)); //commit pair if not empty
	}
	RebuildList();
}

void ParameterEditor::CopyToClipboard() {
	list->CopyAllToClipboard();
}

void ParameterEditor::Plot() {
	dataView->Reset();
	for (size_t i = 0; i < tempParam.GetSize(); i++) {
		dataView->Add(tempParam.GetX(i), tempParam.GetY(i));
	}
	dataView->CommitChange();
}

void ParameterEditor::RebuildList(bool autoSize, bool refillValues) {

	if (autoSize) list->SetSize(2, userValues.size()+1);
	list->SetColumnWidths((int*)flWidth);
	list->SetColumnLabels(flName);
	list->SetColumnAligns((int *)flAligns);
	list->SetColumnEditable((int *)fEdits);
	//list->cEdits[0] = list->cEdits[1]= EDIT_NUMBER;

	//char tmp[128];
	if (refillValues) {
		for (int row = 0; row < (int)userValues.size(); row++) {
			list->SetValueAt(0, row, userValues[row].first.c_str());
			list->SetValueAt(1, row, userValues[row].second.c_str());
		}
		//last line, possibility to enter new value
		list->SetValueAt(0, (int)userValues.size(), "");
		list->SetValueAt(1, (int)userValues.size(), "");
	}
}

bool ParameterEditor::ValidateInput() {
	//Validate name
	std::string tempName = nameField->GetText();

	if (!beginsWith(tempName, "[catalog]")) {
		if (tempName.length() == 0) {
			GLMessageBox::Display("Parameter name can't be empty", "Invalid parameter definition", GLDLG_OK, GLDLG_ICONWARNING);
			return false;
		}
		if (selectorCombo->GetSelectedIndex() == 0) {
			for (auto& p : work->parameters) {
				if (tempName.compare(p.name) == 0) {
					GLMessageBox::Display("This parameter name is already used", "Invalid parameter definition", GLDLG_OK, GLDLG_ICONWARNING);
					return false;
				}
			}
		}
		if (!((tempName[0] >= 65 && tempName[0] <= 90) || (tempName[0] >= 97 && tempName[0] <= 122))) {
			GLMessageBox::Display("Parameter name must begin with a letter", "Invalid parameter definition", GLDLG_OK, GLDLG_ICONWARNING);
			return false;
		}
	}
	
	/*
	for (int i = 0; i < 1; i++) {
		StringClass test;
	}*/

	tempParam=Parameter();
	tempParam.name = tempName;

	bool atLeastOne = false;
	for (size_t row = 0; row < userValues.size(); row++) {
		double valueX, valueY;
		try {
			valueX = ::atof(userValues[row].first.c_str());
		} catch (std::exception err){
			char tmp[256];
			sprintf(tmp, "Can't parse value \"%s\" in row %zd, first column:\n%s", userValues[row].first.c_str(), row+1, err.what());
			GLMessageBox::Display(tmp, "Invalid parameter definition", GLDLG_OK, GLDLG_ICONWARNING);
			return false;
		}
		try {
			valueY = ::atof(userValues[row].second.c_str());
		}
		catch (std::exception err){
			char tmp[256];
			sprintf(tmp, "Can't parse value \"%s\" in row %zd, second column:\n%s", userValues[row].second.c_str(), row+1, err.what());
			GLMessageBox::Display(tmp, "Invalid parameter definition", GLDLG_OK, GLDLG_ICONWARNING);
			return false;
		}
		tempParam.AddPair(valueX, valueY, true); //insert in correct position
		atLeastOne = true;
	}
	if (!atLeastOne) {
		GLMessageBox::Display("At least one value is required", "Invalid parameter definition", GLDLG_OK, GLDLG_ICONWARNING);
		return false;
	}

	for (size_t i = 0; i < tempParam.GetSize();i++) {
		for (size_t j = i+1; j < tempParam.GetSize(); j++) {
			if (IsEqual(tempParam.GetX(i) , tempParam.GetX(j))) {
				std::stringstream msg;
				msg << "There are two values for t=" << tempParam.GetX(i) << "s.";
				GLMessageBox::Display(msg.str().c_str(), "Invalid parameter definition", GLDLG_OK, GLDLG_ICONWARNING);
				return false;
			}
		}
	}

	return true;
}

void ParameterEditor::UpdateUserValues() {
	userValues = std::vector<std::pair<std::string, std::string>>();
	//nameField->SetText("");
	if (selectorCombo->GetSelectedIndex()>0 && selectorCombo->GetSelectedIndex()-1 <(int)work->parameters.size()) {
		Parameter *getParam = &work->parameters[selectorCombo->GetSelectedIndex()-1];

		for (int row = 0; row < (int)getParam->GetSize(); row++) {
			/*std::ostringstream str1, str2;
			//Make it more precise!
			str1 << getParam->values[row].first;
			str2 << getParam->values[row].second;
			userValues.push_back(std::make_pair(str1.str(), str2.str()));*/
			//userValues.push_back(std::make_pair(std::to_string(getParam->values[row].first), std::to_string(getParam->values[row].second)));
			char tmp1[32];
			char tmp2[32];
			sprintf(tmp1, "%.10g", getParam->GetX(row));
			sprintf(tmp2, "%.10g", getParam->GetY(row));
			userValues.push_back(std::make_pair(tmp1,tmp2));
		}
		nameField->SetText(getParam->name.c_str());
	}
}