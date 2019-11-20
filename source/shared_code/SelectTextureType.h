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
#pragma once
#include "GLApp/GLWindow.h"

class Worker;
class Geometry;
class GLTextField;
class GLToggle;
class GLTitledPanel;
class GLButton;

// Buttons
#ifndef GLDLG_CANCEL
#define GLDLG_CANCEL		0x0002
#endif

#ifndef GLDLG_SELECT
#define GLDLG_SELECT		0x0004
#endif

#ifndef GLDLG_SELECT_ADD
#define GLDLG_SELECT_ADD	0x0008
#endif

#ifndef GLDLG_SELECT_REM
#define GLDLG_SELECT_REM	0x0016
#endif

class SelectTextureType : public GLWindow {

public:
  // Display a modal dialog and return the code of the pressed button
  SelectTextureType(Worker *w);
  int  rCode;
  void ProcessMessage(GLComponent *src,int message);
private:
  Geometry     *geom;
  Worker	   *work;
  GLButton
	  *selectButton,
	  *addSelectButton,
	  *remSelectButton;
  GLTextField
	  *ratioText,
	  *ratioMinText,
	  *ratioMaxText;
  GLToggle
	  *ratioToggle,
	  *ratioMinMaxToggle,
	  *desorbToggle,
	  *absorbToggle,
	  *reflectToggle,
	  *transparentToggle,
	  *directionToggle;
  GLTitledPanel
	  *resolutionpanel,
	  *textureTypePanel;
};
