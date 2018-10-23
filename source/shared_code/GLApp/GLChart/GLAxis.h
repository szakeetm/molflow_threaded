// Copyright (c) 2011 rubicon IT GmbH
#ifndef _GLCHARTAXISH_
#define _GLCHARTAXISH_

#include <string>

typedef struct {

  char *value;
  GLCDimension size;
  double pos;
  int offsetX;
  int offsetY;

} LabelInfo;

class GLAxis {

public:

  // Construction
  GLAxis(GLComponent *parent, int orientation);
  ~GLAxis();

  // Components method
  void     SetAxisColor(GLColor c);
  GLColor GetAxisColor();
  void   SetPercentScrollback(double d);
  double GetPercentScrollback();
  void   SetLabelFormat(int l);
  int    GetLabelFormat();
  void   SetGridVisible(bool b);
  void   SetFitXAxisToDisplayDuration(bool b);
  bool   IsFitXAxisToDisplayDuration();
  bool   IsGridVisible();
  void   SetDrawOpposite(bool b);
  bool   IsDrawOpposite();
  void   SetSubGridVisible(bool b);
  bool   IsSubGridVisible();
  void   SetGridStyle(int s);
  int    GetGridStyle();
  void   SetAnnotation(int a);
  int    GetAnnotation();
  void   SetVisible(bool b);
  bool   IsVisible();
  bool   IsZoomed();
  bool   IsXY();
  void   SetMinimum(double d);
  double GetMinimum();
  void   SetMaximum(double d);
  double GetMaximum();
  double GetMin();
  double GetMax();
  bool   IsAutoScale();
  void   SetAutoScale(bool b);
  int    GetScale();
  void   SetScale(int s);
  void   SetOrientation(int orientation);
  int    GetOrientation();
  void   Zoom(int x1, int x2);
  void   Unzoom();
  int    GetTick();
  double GetTickSpacing();
  void   SetTickSpacing(double spacing);
  void   SetTick(int s);
  void   SetTickLength(int lgth);
  int    GetTickLength();
  char  *GetName();
  void   SetName(const char *s);
  void   SetPosition(int o);
  int    GetPosition();
  void   AddDataView(GLDataView *v);
  void   AddDataViewAt(int index,GLDataView *v);
  GLDataView *GetDataView(int index);
  void   RemoveDataView(GLDataView *v);
  bool   CheckRemoveDataView(GLDataView *v);
  void   ClearDataView();
  GLDataView **GetViews();
  int    GetViewNumber();
  void   SetInverted(bool i);
  bool   IsInverted();
  GLCRectangle GetBoundRect();
  char  *ToScientific(double d);
  char  *ToScientificInt(double d);
  std::string FormatValue(double vt, double prec);
  char  *FormatTimeValue(double vt);
  bool   IsHorizontal();
  void SetAxisDuration(double d);
  void ComputeXScale(GLDataView **views,int sz);
  int GetFontHeight();
  int GetLabelFontDimension();
  int GetFontOverWidth();
  int GetThickness();
  void MeasureAxis(int desiredWidth, int desiredHeight);
  GLCPoint transform(double x, double y, GLAxis *xAxis);
  SearchInfo *SearchNearest(int x, int y, GLAxis *xAxis);
  void computeLabels(double length);
  void DrawFast(GLCPoint lp, GLCPoint p, GLDataView *v);
  static void PaintMarker(GLColor c,int mType, int mSize, int x, int y);
  static void DrawSampleLine(int x, int y, GLDataView *v);
  void PaintDataViews(GLAxis *xAxis, int xOrg, int yOrg);
  GLColor ComputeMediumColor(GLColor c1, GLColor c2);
  void  PaintAxis(int x0, int y0, GLAxis *xAxis, int xOrg, int yOrg, GLColor back,bool oppositeVisible);
  void  PaintAxisDirect(int x0, int y0,GLColor back,int tr,int la);
  void  PaintAxisOpposite(int x0, int y0,GLColor back,int tr,int la);
  void  PaintAxisOppositeDouble(int x0, int y0,GLColor back,int tr,int la);
  bool  IsZeroAlwaysVisible();
  void  SetZeroAlwaysVisible(bool zeroAlwaysVisible);
  char *GetDateFormat();
  void  SetDateFormat (const char *dateFormat);
  static void  Invalidate();
  static void  Revalidate();

private:

  int    getDV(GLDataView *v);
  bool   insideRect(GLCRectangle *r,GLCPoint *p);
  void   computeDateformat(int maxLab);
  std::string suppressZero(const char *n);
  double computeHighTen(double d);
  double computeLowTen(double d);
  void   computeAutoScale();
  int    getLength();
  SearchInfo *searchNearestNormal(int x, int y, GLAxis *xAxis);
  SearchInfo *searchNearestXY(int x, int y, GLAxis *xAxis);
  void paintDataViewNormal(GLDataView *v, GLAxis *xAxis, int xOrg, int yOrg);
  void paintDataViewXY(GLDataView *v, GLDataView *w, GLAxis *xAxis, int xOrg, int yOrg);
  void paintYOutTicks(GLColor c,int x0, double ys, int y0, int la, int tr,int off,bool grid);
  void paintXOutTicks(GLColor c,int y0, double xs, int x0, int la, int tr,int off,bool grid);
  int  getTickShift(int width);
  int  getTickShiftOpposite(int width);
  void paintYTicks(GLColor c,int i, int x0, double y, int la, int tr,int off,bool grid);
  void paintXTicks(GLColor c,int i, int y0, double x, int la, int tr,int off,bool grid);
  int  computeBarWidth(GLDataView *v, GLAxis *xAxis);
  void paintDataViewBar(GLDataView *v,int barWidth,int y0,int x,int y);
  void paintDataViewPolyline(GLDataView *v,int nb,int yOrg,int *pointX,int *pointY);
  int  distance2(int x1, int y1, int x2, int y2);
  void paintBarBorder(int barWidth, int y0, int x, int y);
  void paintBar(int barWidth, GLColor background, int fillStyle, int y0, int x, int y);
  void addLabel(const char *lab, int w, int h, double pos,int offX=0,int offY=0);
  void clearLabel();
  void clip(int x,int y,int width,int height);
  static void drawLine(GLColor c,int dash,int lWidth,int x1,int y1,int x2,int y2);
  static GLuint initMarker(const char *name);
  static void paintMarkerTex(GLuint mTex,int x,int y,int width,int height,int r,int g,int b);

  bool visible;
  double min;
  double max;
  double minimum;
  double maximum;
  bool autoScale;
  int scale;
  GLColor labelColor;
  int labelFormat;
  LabelInfo labels[MAX_VIEWS];
  int nbLabel;
  int orientation;  // Axis orientation/position
  int dOrientation; // Default orientation (cannot be _ORG)
  bool subtickVisible;
  GLCDimension csize;
  char name[256];
  int annotation;
  GLDataView *dataViews[MAX_VIEWS];
  int nbView;
  double ln10;
  bool gridVisible;
  bool subGridVisible;
  int gridStyle;
  GLCRectangle boundRect;
  bool lastAutoScate;
  bool isZoomed;
  double percentScrollback;
  double axisDuration;
  std::string useFormat;
  double desiredPrec;
  bool drawOpposite;
  int tickLength;
  int subtickLength;
  int fontOverWidth;
  bool inverted;
  double tickStep;    // In pixel
  double minTickStep;
  int subTickStep; // 0 => NONE , -1 => Log step , 1.. => Linear step
  bool fitXAxisToDisplayDuration;
  
  bool zeroAlwaysVisible;
  bool  autoLabeling;

  char *dateFormat;

  GLComponent *parent;

};

#endif /* _GLCHARTAXISH_ */