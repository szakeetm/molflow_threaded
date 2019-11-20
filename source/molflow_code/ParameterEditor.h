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

/*
  File:        ParameterEditor.h
  Description: Moments Editor
*/
#ifndef _PARAMETEREDITORH_
#define _PARAMETEREDITORH_

#include "GLApp/GLWindow.h"
#include "Parameter.h"
class GLButton;
class GLCombo;
class GLChart;
class GLTextField;
class GLToggle;
class GLTitledPanel;
class GLList;
class GLDataView;

class Worker;

class ParameterEditor : public GLWindow {

public:

  // Construction
  ParameterEditor(Worker *work);
  void Refresh();
  void UpdateCombo();
  void RebuildList(bool autoSize=true, bool refillValues=true);
  // Implementation
  void ProcessMessage(GLComponent *src,int message);
  void PrepareForNewParam();
  

private:

  Worker	   *work;

  GLCombo *selectorCombo;
  GLButton *newButton, *deleteButton,/* *copyButton ,*/ *pasteButton, *loadCSVbutton, *plotButton, *applyButton;
  GLList *list;
  GLTextField *nameField;
  GLTitledPanel *editorPanel;
  GLChart *plotArea;
  GLDataView *dataView;
  GLToggle *logXtoggle;
  GLToggle *logYtoggle;

  Parameter tempParam;
  std::vector<std::pair<std::string, std::string>> userValues;
  
  void CopyToClipboard();
  void PasteFromClipboard();
  void LoadCSV();
  void Plot();
  void UpdateUserValues();
  bool ValidateInput();
  
};

#endif /* _PARAMETEREDITORH_ */
