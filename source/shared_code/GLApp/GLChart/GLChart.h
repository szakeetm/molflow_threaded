// Copyright (c) 2011 rubicon IT GmbH
#ifndef _GLCHARTH_
#define _GLCHARTH_

//#include "..\GLWindow.h"
#include "../GLComponent.h"

#include <vector>
//#include "..\GLMenu.h"
//#include "..\GLButton.h"

class GLChart;
class GLAxis;
class SearchInfo;
class GLMenu;
class GLButton;

#include "GLChartConst.h"
#include "DataList.h"
#include "GLDataView.h"
#include "GLAxis.h"
#include "SearchInfo.h"
#include "GLChartOptions.h"
#include "GLDataViewOptions.h"

typedef struct {

  GLCRectangle  rect;
  GLDataView   *view;

} LabelRect;

class GLChart : public GLComponent {

public:

  // Construction
  GLChart(int compId);
  ~GLChart();

  // Components method
  GLAxis *GetXAxis();
  GLAxis *GetY1Axis();
  GLAxis *GetY2Axis();
  void SetMargin(GLCDimension d);
  GLCDimension GetMargin();
  void SetBackground(GLColor c);
  GLColor GetBackground();
  void SetChartBackground(GLColor c);
  GLColor GetChartBackground();
  void SetPaintAxisFirst(bool b);
  bool IsPaintAxisFirst();
  void SetHeaderVisible(bool b);
  void SetHeader(const char *s);
  char *GetHeader();
  void SetDisplayDuration(double v);
  double GetDisplayDuration();
  void SetHeaderColor(GLColor c);
  void SetLabelVisible(bool b);
  bool IsLabelVisible();
  void SetLabelPlacement(int p);
  int GetLabelPlacement();
  void ShowOptionDialog();
  void ShowDataOptionDialog(GLDataView *v);
  void SaveFile();
  void LoadFile();
  void CopyAllToClipboard();
  bool IsZoomed();
  void EnterZoom();
  void ExitZoom();
  void UnselectAll();
  void AddData(GLDataView *v, double x, double y);
  void RemoveDataView(GLDataView *view);

  // Implementation
  void ProcessMessage(GLComponent *src,int message);
  void Paint();
  void ManageEvent(SDL_Event *evt);
  void InvalidateDeviceObjects();
  void RestoreDeviceObjects();

  GLColor GetFirstAvailableColor();

private:

  void paintLabel(GLDataView *v,GLAxis *axis,int x,int y,int w);
  void paintLabelAndHeader();
  void measureGraphItems(int w,int h,GLDataView **views,int nbView);
  void paintZoomButton(int x,int y);
  void paintZoomSelection(int x,int y);
  GLCRectangle buildRect(int x1, int y1, int x2, int y2);
  void mouseDragged(SDL_Event *evt);
  void mouseReleased(SDL_Event *evt);
  void mousePressed(SDL_Event *evt);
  void redrawPanel();
  char **buildPanelString(SearchInfo *si);
  void showPanel(SearchInfo *si);
  int  garbageData(GLDataView *v);
  void setRect(GLCRectangle *r,int x,int y,int w,int h);
  void paintChartBackground(int xOrg,int yOrg);
  bool insideRect(LabelRect *r,int x,int y);
  void showChartMenu(int x,int y);

  char header[256];
  bool headerVisible;
  GLColor headerColor;

  bool ipanelVisible;
  bool paintAxisFirst;
  GLColor chartBackground;

  double displayDuration;

  GLMenu *chartMenu;
  GLMenu *dvMenuY1;
  GLMenu *dvMenuY2;

  bool zoomDrag;
  bool zoomDragAllowed;
  int zoomX;
  int zoomY;
  GLButton *zoomButton;

  int lastX;
  int lastY;
  SearchInfo lastSearch;

  // Measurements stuff
  GLCRectangle headerR;
  GLCRectangle labelR;
  GLCRectangle viewR;
  GLCDimension margin;

  // Label placement
  LabelRect labelRect[MAX_VIEWS];
  bool labelVisible;
  int  labelMode;
  int labelHeight;
  int labelWidth;
  int labelPerLine;
  int nbLabel;
  int headerWidth;
  int axisHeight;
  int axisWidth;
  int y1AxisThickness;
  int y2AxisThickness;
  int xAxisThickness;
  int xAxisUpMargin;

  std::vector<GLColor> colors;

  // Axis
  GLAxis *xAxis;
  GLAxis *y1Axis;
  GLAxis *y2Axis;

  // Config dialog
  GLChartOptions *chartOptions;
  GLDataViewOptions *dvOptions;

};

#endif /* _GLCHARTH_ */
