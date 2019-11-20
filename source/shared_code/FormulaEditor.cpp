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
#include "FormulaEditor.h"
#include "GLApp/GLToolkit.h"
#include "GLApp/GLMessageBox.h"
#include "GLApp/GLButton.h"
#include "GLApp/GLLabel.h"
#include "GLApp/GLToggle.h"
#include "GLApp/GLTitledPanel.h"
#include "GLApp/GLList.h"
#include "GLApp/MathTools.h"
#include <sstream>
#include <algorithm>

#ifdef MOLFLOW
#include "MolFlow.h"
#endif

#ifdef SYNRAD
#include "SynRad.h"
#endif

extern GLApplication *theApp;
extern std::string formulaSyntax;
extern int formulaSyntaxHeight;

#ifdef MOLFLOW
extern MolFlow *mApp;
#endif

#ifdef SYNRAD
extern SynRad*mApp;
#endif

static const char *flName[] = { "Expression","Name (optional)","Value" };
static const int   flAligns[] = { ALIGN_LEFT,ALIGN_LEFT,ALIGN_LEFT };
static const int   fEdits[] = { EDIT_STRING,EDIT_STRING,0 };

FormulaEditor::FormulaEditor(Worker *w) :GLWindow() {
	columnRatios = { 0.333,0.333,0.333 };

	int wD = 400;
	int hD = 200; //Height extended runtime when formula syntax panel is expanded

	work = w;

	SetTitle("Formula Editor");
	SetIconfiable(true);
	
	SetResizable(true);

	panel1 = new GLTitledPanel("Formula list");
	Add(panel1);

	formulaList = new GLList(0);
	formulaList->SetColumnLabelVisible(true);
	formulaList->SetRowLabelVisible(true);
	formulaList->SetHScrollVisible(false);
	formulaList->SetGrid(true);
	Add(formulaList);

	recalcButton = new GLButton(0, "Recalculate now");
	Add(recalcButton);

	moveUpButton = new GLButton(0, "Move Up");
	moveUpButton->SetEnabled(false);
	Add(moveUpButton);

	moveDownButton = new GLButton(0, "Move Down");
	moveDownButton->SetEnabled(false);
	Add(moveDownButton);

	panel2 = new GLTitledPanel("Format");
	panel2->SetClosable(true);
	panel2->Close();
	Add(panel2);

	descL = new GLLabel(formulaSyntax.c_str());
	descL->SetVisible(false); //Set visible runtime
	Add(descL);

	// Place in top left corner
	//int wS, hS;
	//GLToolkit::GetScreenSize(&wS, &hS);
	int xD = /*(wS - wD - 215)*/ 10;
	int yD = 30;
	SetBounds(xD, yD, wD, hD);

	RestoreDeviceObjects();

}

void FormulaEditor::ProcessMessage(GLComponent *src, int message) {
	switch (message) {
	case MSG_PANELR:
	{
		if (src == panel2) {
			int x, y, w, h;
			GetBounds(&x, &y, &w, &h);
			if (panel2->IsClosed()) {
				SetBounds(x, y, w, h - formulaSyntaxHeight);
				descL->SetVisible(false);
			}
			else {
				SetBounds(x, y, w, h + formulaSyntaxHeight);
				descL->SetVisible(true);
			}
		}
		break;
	}
	case MSG_BUTTON:
		if (src == recalcButton) {
			ReEvaluate();
		}
		else if (src == moveUpButton) {
			int selRow = formulaList->GetSelectedRow();
			if (selRow <= 0) {
				//Interface bug
				DEBUG_BREAK;
				return;
			}
			std::swap(mApp->formulas_n[selRow], mApp->formulas_n[selRow - 1]);
			formulaList->SetSelectedRow(selRow - 1);
			EnableDisableMoveButtons();
			Refresh();
		}
		else if (src == moveDownButton) {
			int selRow = formulaList->GetSelectedRow();
			if (selRow > mApp->formulas_n.size() - 2) {
				//Interface bug
				DEBUG_BREAK;
				return;
			}
			std::swap(mApp->formulas_n[selRow], mApp->formulas_n[selRow + 1]);
			formulaList->SetSelectedRow(selRow + 1);
			EnableDisableMoveButtons();
			Refresh();
		}
		break;
	case MSG_TEXT:
	case MSG_LIST:
	{
		for (size_t row = 0; row < (formulaList->GetNbRow() - 1); row++) { //regular lines

			if (strcmp(formulaList->GetValueAt(0, row), userExpressions[row].c_str()) != 0) { //Expression changed
				if (!(row < mApp->formulas_n.size())) {
					//Interface bug
					DEBUG_BREAK;
					return;
				}
				if (*(formulaList->GetValueAt(0, row)) != 0 || *(formulaList->GetValueAt(1, row)) != 0) //Name or expr. not empty
				{
					mApp->formulas_n[row]->SetExpression(formulaList->GetValueAt(0, row));
					mApp->formulas_n[row]->Parse();
					Refresh();
				}
				else
				{
					SAFE_DELETE(mApp->formulas_n[row]);
					mApp->formulas_n.erase(mApp->formulas_n.begin() + row);
					Refresh();
				}
				EnableDisableMoveButtons();
				break;
			}

			if (strcmp(formulaList->GetValueAt(1, row), userFormulaNames[row].c_str()) != 0) { //Name changed
				if (*(formulaList->GetValueAt(0, row)) != 0 || *(formulaList->GetValueAt(1, row)) != 0) //Name or expr. not empty
				{
					mApp->formulas_n[row]->SetName(formulaList->GetValueAt(1, row));
					Refresh();
				}
				else
				{
					SAFE_DELETE(mApp->formulas_n[row]);
					mApp->formulas_n.erase(mApp->formulas_n.begin() + row);
					Refresh();
				}
				EnableDisableMoveButtons();
				break;
			}

		}
		if (formulaList->GetValueAt(0, formulaList->GetNbRow() - 1) != 0) { //last line
			if (*(formulaList->GetValueAt(0, formulaList->GetNbRow() - 1)) != 0) {
				//Add new line
				GLParser* newF = new GLParser();
				newF->SetExpression(formulaList->GetValueAt(0, formulaList->GetNbRow() - 1));
				newF->SetName("");
				newF->Parse();
				mApp->formulas_n.push_back(newF);
				Refresh();
			}
		}
		else if (formulaList->GetValueAt(1, formulaList->GetNbRow() - 1) != 0) { //last line
			if (*(formulaList->GetValueAt(1, formulaList->GetNbRow() - 1)) != 0) {
				//Add new line
				GLParser* newF = new GLParser();
				newF->SetExpression("");
				newF->SetName(formulaList->GetValueAt(1, formulaList->GetNbRow() - 1));
				newF->Parse();
				mApp->formulas_n.push_back(newF);
				Refresh();
			}
		}
		EnableDisableMoveButtons();
		break;
	}
	case MSG_LIST_COL:
		int x,y,w,h;
		GetBounds(&x, &y, &w, &h);
		double sum = (double)(w - 45);
		std::vector<double> colWidths(3);
		for (size_t i = 0; i < 3; i++) {
			colWidths[i]=(double)formulaList->GetColWidth(i);
		}
		for (size_t i = 0; i < 3; i++) {
			columnRatios[i] = colWidths[i] / sum;
		}
		break;
	}

	GLWindow::ProcessMessage(src, message);
}

void FormulaEditor::SetBounds(int x, int y, int w, int h) {
	int formulaHeight = (panel2->IsClosed() ? 0 : formulaSyntaxHeight);
	panel1->SetBounds(5, 5, w - 10, h - 90 - formulaHeight);
	formulaList->SetBounds(10, 22, w - 20, h - 115 - formulaHeight);
	for (size_t i=0;i<3;i++)
		formulaList->SetColumnWidth(i, (int)(columnRatios[i] * (double)(w - 45)));
	recalcButton->SetBounds(10, h - 80 - formulaHeight, 95, 20);
	moveUpButton->SetBounds(w - 160, h - 80 - formulaHeight, 65, 20);
	moveDownButton->SetBounds(w - 90, h - 80 - formulaHeight, 65, 20);

	panel2->SetBounds(5, h - 50 - formulaHeight, w - 10, 20 + formulaHeight); //Height will be extended runtime
	panel2->SetCompBounds(descL, 10, 15, w-30, formulaHeight);

	SetMinimumSize(400, 150 + formulaHeight);
	GLWindow::SetBounds(x, y, w, h);
}

void FormulaEditor::EnableDisableMoveButtons()
{
	int selRow = formulaList->GetSelectedRow(false);
	if (selRow == -1 || selRow >= ((int)(formulaList->GetNbRow()) - 1)) { //Nothing or very last (empty) row selected
		moveUpButton->SetEnabled(false);
		moveDownButton->SetEnabled(false);
	}
	else if (selRow == 0) { //First row
		moveUpButton->SetEnabled(false);
		moveDownButton->SetEnabled(true);
	}
	else if (selRow == ((int)(formulaList->GetNbRow()) - 2)) { //Last (formula) row
		moveUpButton->SetEnabled(true);
		moveDownButton->SetEnabled(false);
	}
	else {
		moveUpButton->SetEnabled(true);
		moveDownButton->SetEnabled(true);
	}
}

void FormulaEditor::RebuildList() {
	//Rebuild list based on locally stored userExpressions
	int x, y, w, h;
	GetBounds(&x, &y, &w, &h);
	formulaList->SetSize(3, userExpressions.size() + 1);
	for (size_t i = 0; i<3; i++)
		formulaList->SetColumnWidth(i, (int)(columnRatios[i] * (double)(w - 45)));
	formulaList->SetColumnLabels(flName);
	formulaList->SetColumnAligns((int *)flAligns);
	formulaList->SetColumnEditable((int *)fEdits);

	size_t u; double latest = 0.0;

	for (u = 0; u < userExpressions.size(); u++) {
		formulaList->SetValueAt(0, u, userExpressions[u].c_str());
		formulaList->SetValueAt(1, u, userFormulaNames[u].c_str());
	}
}

void FormulaEditor::Refresh() {
	//Load contents of window from global (interface/app) formulas
	size_t nbFormula = mApp->formulas_n.size();
	userExpressions.resize(nbFormula);
	userFormulaNames.resize(nbFormula);
	for (size_t i = 0; i < nbFormula; i++) {
		userExpressions[i] = mApp->formulas_n[i]->GetExpression();
		userFormulaNames[i] = mApp->formulas_n[i]->GetName();
	}
	RebuildList();
	ReEvaluate();
}

void FormulaEditor::ReEvaluate() {
	
	//       NEW CODE
	

	for (size_t i = 0; i < mApp->formulas_n.size(); i++) {

		// Evaluate variables
		int nbVar = mApp->formulas_n[i]->GetNbVariable();
		bool ok = true;
		for (int j = 0; j < nbVar && ok; j++) {
			VLIST *v = mApp->formulas_n[i]->GetVariableAt(j);
			ok = mApp->EvaluateVariable(v);
			if (!ok) {
				std::stringstream tmp;
				tmp << "Invalid variable " << v->name;
				formulaList->SetValueAt(2, i, tmp.str().c_str());
			}
		}

		// Evaluation
		if (ok) { //Variables succesfully evaluated
			double r;
			mApp->formulas_n[i]->hasVariableEvalError = false;
			if (mApp->formulas_n[i]->Evaluate(&r)) {
				std::stringstream tmp;
				tmp << r;
				formulaList->SetValueAt(2, i, tmp.str().c_str());
			}
			else { //Variables OK but the formula itself can't be evaluated
				formulaList->SetValueAt(2, i, mApp->formulas_n[i]->GetErrorMsg());
			}
#ifdef MOLFLOW
			//formulas[i].value->SetTextColor(0.0f, 0.0f, worker.displayedMoment == 0 ? 0.0f : 1.0f);
			formulaList->SetColumnColor(2,mApp->worker.displayedMoment == 0 ? COLOR_BLACK : COLOR_BLUE);
#endif
		}
		else { //Error while evaluating variables
			   //formulas[i].value->SetText("Invalid variable name"); //We set it directly at the error location
		}
	}
}