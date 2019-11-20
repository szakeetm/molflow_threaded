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
#include "GLApp/GLWindow.h"
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


class PressureEvolution : public GLWindow {

public:

  // Construction
  PressureEvolution(Worker *w);

  // Component method
  void Refresh();
  void Update(float appTime,bool force=false);
  void Reset();

  // Implementation
  void ProcessMessage(GLComponent *src,int message);
  void SetBounds(int x,int y,int w,int h);

private:

  void addView(size_t facetId);
  void remView(size_t viewId);
  void refreshChart();

  Worker      *worker;

  GLChart     *chart;
  GLLabel *label1, *normLabel;


  GLCombo     *profCombo;
  GLCombo     *yScaleCombo;
  GLButton    *selButton;
  GLButton    *addButton;
  GLButton    *removeButton;
  GLButton    *removeAllButton;

  GLToggle *logXToggle,*logYToggle;

  std::vector<GLDataView*>  views;
  std::vector<GLColor>    colors;
  float        lastUpdate;

};
