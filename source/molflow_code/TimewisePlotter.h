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
#ifndef _TIMEWISEPLOTTERH_
#define _TIMEWISEPLOTTERH_

#include "GLApp/GLWindow.h"
#include "GLApp/GLChart/GLChartConst.h"
#include <vector>
class GLChart;
class GLLabel;
class GLCombo;
class GLButton;
class GLParser;
class GLDataView;
class GLToggle;
class GLTextField;
class Worker;
class Geometry;

class TimewisePlotter : public GLWindow {

public:

  // Construction
  TimewisePlotter();

  // Component method
  void Display(Worker *w);
  void Refresh();
  void Update(float appTime,bool force=false);
  void UpdateMoment();
  void Reset();

  // Implementation
  void ProcessMessage(GLComponent *src,int message);
  void SetBounds(int x,int y,int w,int h);
  void refreshViews();

private:
  bool ParseMoments();
  void ParseToken(std::string token);
  void FormatParseText();
  void addView(int facet);
  void remView(int facet);
  
  Worker      *worker;
  GLButton    *dismissButton;
  GLLabel     *warningLabel;
  GLChart     *chart;
  GLCombo     *profCombo;
  GLLabel     *normLabel;
  GLCombo     *normCombo;
  GLButton    *selButton;
  GLTextField *momentsText;
  GLLabel     *momLabel,*momentsLabel;
  GLToggle    *logYToggle,*constantFlowToggle,*correctForGas;

  std::vector<size_t> displayedMoments;

  GLDataView  *views[50];

  int          nbView;
  float        lastUpdate;

};

#endif /* _TIMEWISEPLOTTERH_ */
