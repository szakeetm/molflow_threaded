/*
  File:        SmpStatus.h
  Description: SMP status dialog
  Program:     SynRad
  Author:      R. KERSEVAN / M ADY
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

#ifndef _SMPSTATUSH_
#define _SMPSTATUSH_

#include "GLApp/GLWindow.h"

class GLList;
class GLTextField;
class GLButton;
class GLCombo;

class Worker;

class SmpStatus : public GLWindow {

public:

  // Construction
  SmpStatus();

  // Component method
  void Display(Worker *w);
  void Update(float appTime);

  // Implementation
  void ProcessMessage(GLComponent *src,int message);

private:

  void RestartProc();

  Worker      *worker;
  GLList      *processList;
  GLButton    *dismissButton;
  GLButton    *restartButton;
  GLButton    *maxButton;
  GLTextField *nbProcText;

  float lastUpdate;
  float lastCPUTime[MAX_PROCESS];
  float lastCPULoad[MAX_PROCESS];

};

#endif /* _SMPSTATUSH_ */
