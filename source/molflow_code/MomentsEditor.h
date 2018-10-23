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

/*
  File:        MomentsEditor.h
  Description: Moments Editor
*/
#ifndef _MOMENTSEDITORH_
#define _MOMENTSEDITORH_

#include "GLApp/GLWindow.h"
class GLButton;
class GLLabel;
class GLTextField;
class GLToggle;
class GLTitledPanel;
class GLList;

#include "Worker.h"

class MomentsEditor : public GLWindow {

public:

  // Construction
  MomentsEditor(Worker *work);
  void RebuildList();
  void Refresh();

  // Implementation
  void ProcessMessage(GLComponent *src,int message);
  

private:

  Worker	   *work;

  GLButton    *setButton;
  GLButton    *cancelButton;
  GLButton    *clearButton;
  GLButton    *pasteButton;
  GLLabel     *l1;
  GLList      *momentsList;
  GLTextField *windowSizeText;
  GLToggle    *useMaxwellToggle;
  GLToggle    *calcConstantFlow;
  GLTitledPanel *panel1;
  GLTitledPanel *panel2;

  
  std::vector<double> moments;
  std::vector<std::string> userMoments;

  int AddMoment(std::vector<double> newMoments); //Adds a time serie to moments and returns the number of elements
  void PasteClipboard();
  std::vector<double> ParseMoment(std::string userInput); //Parses a user input and returns a vector of time moments

};

#endif /* _MOMENTSEDITORH_ */
