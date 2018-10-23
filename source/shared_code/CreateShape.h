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

#define MODE_RECTANGLE 0
#define MODE_CIRCLE 1
#define MODE_RACETRACK 2


class GLWindow;
class GLButton;
class GLTextField;
class GLTitledPanel;
class GLToggle;
class GLLabel;
class GLIcon;

class Geometry;
class Worker;

class CreateShape : public GLWindow {

public:

  // Construction
  CreateShape(Geometry *geom,Worker *work);

  // Implementation
  void ProcessMessage(GLComponent *src,int message);

private:

	void EnableDisableControls();

  Geometry     *geom;
  Worker	   *work;

	GLTitledPanel	*shapePanel;
	GLToggle	*racetrackCheckbox;
	GLToggle	*ellipseCheckbox;
	GLToggle	*rectangleCheckbox;
	GLTitledPanel	*positionPanel;
	GLLabel	*normalStatusLabel;
	GLButton	*normalVertexButton;
	GLButton	*facetNormalButton;
	GLLabel	*axisStatusLabel;
	GLButton	*axisVertexButton;
	GLButton	*axisFacetUButton;
	GLLabel	*centerStatusLabel;
	GLButton	*centerVertexButton;
	GLButton	*facetCenterButton;
	GLTextField	*normalZtext;
	GLLabel	*label9;
	GLTextField	*normalYtext;
	GLLabel	*label10;
	GLTextField	*normalXtext;
	GLLabel	*label11;
	GLLabel	*label12;
	GLTextField	*axisZtext;
	GLLabel	*label5;
	GLTextField	*axisYtext;
	GLLabel	*label6;
	GLTextField	*axisXtext;
	GLLabel	*label7;
	GLLabel	*label8;
	GLTextField	*centerZtext;
	GLLabel	*label4;
	GLTextField	*centerYtext;
	GLLabel	*label3;
	GLTextField	*centerXtext;
	GLLabel	*label2;
	GLLabel	*label1;
	GLTitledPanel	*sizePanel;
	GLButton	*fullCircleButton;
	GLTextField	*nbstepsText;
	GLLabel	*label22;
	GLLabel	*label20;
	GLTextField	*racetrackToplengthText;
	GLLabel	*label21;
	GLLabel	*label18;
	GLTextField	*axis2LengthText;
	GLLabel	*label19;
	GLLabel	*label17;
	GLTextField	*axis1LengthText;
	GLLabel	*label16;
	GLButton	*createButton;

	GLIcon *rectangleIcon, *circleIcon, *racetrackIcon;
	size_t mode;
};