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
#ifndef _LOADSTATUSH_
#define _LOADSTATUSH_

#include "GLApp/GLWindow.h"

class Worker;
class GLButton;
class GLLabel;
class GLTitledPanel;
class GLList;

class LoadStatus : public GLWindow {

public:

  // Construction
  LoadStatus(Worker *w);
  void EnableStopButton();
  void RefreshNbProcess();
  ~LoadStatus();
  void SMPUpdate();

  // Implementation
  void ProcessMessage(GLComponent *src,int message);

private:

  GLList      *processList;
  GLLabel     *memInfoLabel;
  GLButton    *cancelButton;
  Worker      *worker;
  
};

#endif /* _LOADSTATUSH_ */
