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
#include <vector>

class GLButton;
class GLTextField;
class GLLabel;
class GLToggle;
class Geometry;
class Worker;
class GLTitledPanel;
class ParticleLoggerItem;

class ParticleLogger : public GLWindow {

public:

  // Construction
  ParticleLogger(Geometry *geom,Worker *work);

  // Implementation
  void ProcessMessage(GLComponent *src,int message);

  void UpdateMemoryEstimate();

  void UpdateStatus();

private:

  Geometry     *geom;
  Worker	   *work;

	GLLabel	*descriptionLabel;
	GLLabel	*label2;
	GLButton	*getSelectedFacetButton;
	GLTextField	*facetNumberTextbox;
	GLLabel	*label3;
	GLTextField	*maxRecordedTextbox;
	GLLabel	*memoryLabel;
	GLButton	*applyButton;
	GLLabel	*statusLabel;
	GLButton	*copyButton;
	GLButton	*exportButton;
	GLToggle	*enableCheckbox;
	GLTitledPanel	*logParamPanel;
	GLTitledPanel	*resultPanel;

	std::string ConvertLogToText(const std::vector<ParticleLoggerItem>& log, const std::string & separator, std::ofstream* file=NULL);
	bool isRunning;
};


