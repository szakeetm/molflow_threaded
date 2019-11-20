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
#include <vector>
#include "GLApp/GLTabWindow.h"
#include "GLApp/GLChart/GLChartConst.h"
class GLChart;
class GLLabel;
class GLCombo;
class GLButton;
class GLDataView;
class GLToggle;
class GLTextField;
class Worker;
class Geometry;

#define HISTOGRAM_MODE_BOUNCES 0
#define HISTOGRAM_MODE_DISTANCE 1
#ifdef MOLFLOW
#define HISTOGRAM_MODE_TIME 2
#endif

class HistogramMode {
public:
	GLChart * chart;
	std::vector<GLDataView*>  views;
	std::string name;
	std::string XaxisLabel;
};

class HistogramPlotter : public GLTabWindow {

public:

  // Construction
  HistogramPlotter(Worker *w);

  // Component method
  void Refresh();
  void Update(float appTime,bool force=false);
  void Reset();

  // Implementation
  void ProcessMessage(GLComponent *src,int message);
  void SetBounds(int x,int y,int w,int h);

private:

  void addView(int facetId);
  void remView(int facetId);
  void refreshChart();

  std::tuple<std::vector<double>*, double, double,size_t> GetHistogramValues(int facetId, size_t modeId);

  Worker      *worker;

  GLLabel *label1, *normLabel;

  //GLCombo     *modeCombo; //Bounce / distance / time
  GLCombo     *histCombo; //Which facets
  GLCombo     *yScaleCombo; //Absolute / normalized
  GLButton    *selButton;
  GLButton    *addButton;
  GLButton    *removeButton;
  GLButton    *removeAllButton;
  GLButton    *histogramSettingsButton;

  GLToggle *logXToggle,*logYToggle;

  float        lastUpdate;

  std::vector<HistogramMode> modes; //Bounces, Flight distance, Flight time (for Molflow)

};
