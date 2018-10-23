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
#include <vector>

class GLWindow;
class GLButton;
class GLTextField;
class GLTitledPanel;
class GLLabel;
class GLToggle;

class Geometry;
class Worker;

class HistogramSettings : public GLWindow {

public:

  // Construction
  HistogramSettings(Geometry *s,Worker *w);

  // Component methods
  void Refresh(const std::vector<size_t>& selectedFacetIds);
  void EnableDisableControls();
  bool Apply();

  // Implementation
  void ProcessMessage(GLComponent *src,int message);

private:

  Geometry     *geom;
  Worker	   *work;

  GLTitledPanel *globalSettingsPanel, *facetSettingsPanel;
  GLTextField *globalHitLimitText,*facetHitLimitText;
  GLTextField *globalHitBinsizeText,*facetHitBinsizeText;
  GLTextField *globalDistanceLimitText,*facetDistanceLimitText;
  GLTextField *globalDistanceBinsizeText, *facetDistanceBinsizeText;
  GLTextField *globalTimeLimitText, *facetTimeLimitText;
  GLTextField *globalTimeBinsizeText, *facetTimeBinsizeText;
  GLToggle *globalRecordBounceToggle,*facetRecordBounceToggle;
  GLToggle *globalRecordDistanceToggle, *facetRecordDistanceToggle;
#ifdef MOLFLOW
  GLToggle *globalRecordTimeToggle, *facetRecordTimeToggle;
#endif
  GLLabel  *globalMemoryEstimateLabel,*facetMemoryEstimateLabel;

  GLButton *applyButton, *openViewerButton;

};