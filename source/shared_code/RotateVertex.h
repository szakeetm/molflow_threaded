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
File:        RotateVertex.h
Description: Rotate vertex around axis dialog
*/
#pragma once

#include "GLApp/GLWindow.h"

class GLButton;
class GLTextField;
class GLLabel;
class GLToggle;
class GLTitledPanel;

class Geometry;
class Worker;

class RotateVertex : public GLWindow {

public:
	// Construction
	RotateVertex(Geometry *geom, Worker *work);
	void ProcessMessage(GLComponent *src, int message);

	// Implementation
private:

	void UpdateToggle(GLComponent *src);

	GLTitledPanel *iPanel;
	GLButton     *moveButton, *copyButton, *cancelButton, *getBaseVertexButton, *getDirVertexButton;
	GLToggle     *l1;
	GLToggle     *l2;
	GLToggle     *l3;
	GLToggle     *l4;
	GLToggle     *l5;
	GLToggle     *l6;
	GLLabel     *lNum;
	
	GLToggle     *l8;
	GLTextField *aText;
	GLTextField *bText;
	GLTextField *cText;
	GLTextField *uText;
	GLTextField *vText;
	GLTextField *wText;
	
	GLTextField *degText, *radText;
	GLLabel		*aLabel;
	GLLabel		*bLabel;
	GLLabel		*cLabel;
	GLLabel		*uLabel;
	GLLabel		*vLabel;
	GLLabel		*wLabel;
	GLLabel		*degLabel, *radLabel;

	int nbFacetS;
	int    axisMode;

	Geometry   *geom;
	Worker	   *work;

};

