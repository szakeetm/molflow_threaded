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
#include "GLApp/GLParser.h"
class GLButton;
class GLLabel;
class GLTextField;
class GLToggle;
class GLTitledPanel;
class GLList;
class Worker;
#include <vector>
#include <string>

class FormulaEditor : public GLWindow {

public:

  // Construction
  FormulaEditor(Worker *work);
  void RebuildList();
  void Refresh();

  void ReEvaluate();

  // Implementation
  void ProcessMessage(GLComponent *src,int message);

  void SetBounds(int x, int y, int w, int h);
  

private:

  Worker	   *work;

  GLButton    *recalcButton;
  GLButton		*moveUpButton;
  GLButton		*moveDownButton;
  GLLabel     *l1;
  GLLabel     *descL;
  GLList      *formulaList;
  GLTitledPanel *panel1;
  GLTitledPanel *panel2;

  int descLabelHeight;

  std::vector<std::string> userExpressions,userFormulaNames;
  std::vector<double> columnRatios;

  void EnableDisableMoveButtons();

};
