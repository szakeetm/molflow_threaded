// Copyright (c) 2011 rubicon IT GmbH
#include "GLChart.h"
#include "../GLToolkit.h"
#include "../GLApp.h"
//#include <malloc.h>
#include <float.h>
#include <math.h>
#include <time.h>
#define cimg_use_png 1
#include <CImg/CImg.h>
using namespace cimg_library;
#include <cstring> //strcpy, etc

// Static handle to marker texture
GLuint dotTex = 0;
GLuint triTex = 0;
GLuint diaTex = 0;

// Time formatting

static const double timePrecs[] = {
  1 * SECO, 5 * SECO, 10 * SECO, 30 * SECO,
  1 * MINU, 5 * MINU, 10 * MINU, 30 * MINU,
  1 * HOUR, 3 * HOUR, 6 * HOUR, 12 * HOUR,
  1 * DAY, 7 * DAY, 1 * MONTH, 1 * YEAR, 5 * YEAR,
  10 * YEAR
};

static const char *timeFormats[] = {
    "%H:%M:%S","%H:%M:%S","%H:%M:%S","%H:%M:%S",
    "%H:%M:%S","%H:%M:%S","%H:%M:%S","%H:%M",
    "%H:%M","%H:%M","%H:%M","%a %H:%M",
    "%a %d","%d/%m/%y","%b %y","%Y","%Y","%Y"};

static const int nbTimeF = 18;

static const char *labelFomats[] = {"%g", "", "%02d:%02d:%02d", "%d", "%X", "%b"};

static const double logStep[] = {0.301, 0.477, 0.602, 0.699, 0.778, 0.845, 0.903, 0.954};

extern GLApplication *theApp;

// Inner class to handle XY correlation

class XYData {

public:

  DataList *d1;
  DataList *d2;  // View plotted on the xAxis

  XYData(DataList *d1, DataList *d2) {
    this->d1 = d1;
    this->d2 = d2;
  }

  // Find the next point for XY mode
  void toNextXYPoint() {

    // Correlation mode
    d1 = d1->next;
    while (d1 != NULL && d2 != NULL && d2->next != NULL && d2->next->x <= d1->x) d2 = d2->next;

  }

  // Go to starting time position
  void initFirstPoint() {
    if (d1->x < d2->x) {
      while (d1 != NULL && d1->x < d2->x) d1 = d1->next;
    } else {
      while (d2 != NULL && d2->next != NULL && d2->next->x < d1->x) d2 = d2->next;
    }
  }

  bool isValid() {
    return (d1 != NULL && d2 != NULL);
  }

};

GLAxis::GLAxis(GLComponent *parent,int orientation) {

  this->parent = parent;
  min = 0.0;
  max = 100.0;
  minimum = 0.0;
  maximum = 100.0;
  autoScale = false;
  scale = LINEAR_SCALE;
  csize.width = 0;
  csize.height = 0;
  annotation = VALUE_ANNO;
  axisDuration = MAX_VALUE;
  zeroAlwaysVisible = false;
  autoLabeling = true;
  dateFormat = (char *)FR_DATE_FORMAT;
  nbLabel = 0;
  labelColor.r = 0;
  labelColor.g = 0;
  labelColor.b = 0;
  strcpy(name,"");
  this->orientation = orientation;
  dOrientation = orientation;
  inverted = !IsHorizontal();
  nbView = 0;
  ln10 = log(10.0);
  gridVisible = false;
  subGridVisible = false;
  gridStyle = STYLE_DOT;
  labelFormat = AUTO_FORMAT;
  subtickVisible = true;
  memset(&boundRect,0,sizeof(GLCRectangle));
  isZoomed = false;
  percentScrollback = 0.0;
  visible = true;
  drawOpposite = true;
  tickLength = 6;
  subtickLength = tickLength/2;
  fontOverWidth = 0;
  minTickStep = 50.0;
  fitXAxisToDisplayDuration = true;
  autoLabeling = true;

}

GLAxis::~GLAxis() {
  Invalidate();
  clearLabel();
}

void GLAxis::Invalidate() {
  DELETE_TEX(dotTex);
  DELETE_TEX(triTex);
  DELETE_TEX(diaTex);
}

void GLAxis::Revalidate() {
  Invalidate();
  dotTex = initMarker("images/marker_dot.png");
  triTex = initMarker("images/marker_tri.png");
  diaTex = initMarker("images/marker_dia.png");
}

// Initialise texture for dv markers
GLuint GLAxis::initMarker(const char *name) {

  GLuint tex = 0;

  CImg<BYTE> img(name);
  //if( img.LoadCImage(name) ) {

    // Make 32 Bit RGBA buffer
	int fWidth = img.width();
    int fHeight = img.height();
    BYTE *buff32 = (BYTE *)malloc(fWidth*fHeight*4);
	//BYTE *data = img.data();
    for(int y=0;y<fHeight;y++) {
      for(int x=0;x<fWidth;x++) {
		  buff32[x * 4 + 0 + y * 4 * fWidth] = /*data[x*3+2 + y*3*fWidth];*/ *(img.data(x, y, 0, 0));
        buff32[x*4 + 1 + y*4*fWidth] = /*data[x*3+1 + y*3*fWidth];*/ *(img.data(x, y, 0, 1));
        buff32[x*4 + 2 + y*4*fWidth] = /*data[x*3+0 + y*3*fWidth];*/ *(img.data(x, y, 0, 2));
        buff32[x*4 + 3 + y*4*fWidth] = /*data[x*3+1 + y*3*fWidth];*/ *(img.data(x, y, 0, 1));// Green as alpha
      }
    }

    glGenTextures(1,&tex);
    glBindTexture(GL_TEXTURE_2D,tex);

    glTexImage2D (
      GL_TEXTURE_2D,      // Type
      0,                  // No Mipmap
      GL_RGBA,            // Format RGBA
      fWidth,             // Width
      fHeight,            // Height
      0,                  // Border
      GL_RGBA,            // Format RGBA
      GL_UNSIGNED_BYTE,   // 8 Bit/color
      buff32              // Data
    );   

    free(buff32);
    //img.Release();

  //}

  return tex;

}

  /**
   * Sets the percent scrollback. When using {@link GLChart#addData(GLDataView , double , double ) GLChart.addData}
   * and TIME_ANNO mode for the horizontal axis this property allows to avoid a full graph repaint
   * for every new data entered.
   * @param d Scrollback percent [0..100]
   */
  void GLAxis::SetPercentScrollback(double d) {
    percentScrollback = d / 100;
  }

  /**
   * Gets the percent scrollback
   * @return scrollback percent
   */
  double GLAxis::GetPercentScrollback() {
    return percentScrollback;
  }

  /**
   * Sets the axis color.
   * @param c Axis color
   * @see getAxisColor
   */
  void GLAxis::SetAxisColor(GLColor c) {
    labelColor = c;
  }

  /**
   * Returns the axis color.
   * @return Axis color
   * @see setAxisColor
   */
  GLColor GLAxis::GetAxisColor() {
    return labelColor;
  }

  /**
   * Sets the axis label format.
   * @param l Format of values displayed on axis and in tooltips.
   * @see  AUTO_FORMAT
   * @see  SCIENTIFIC_FORMAT
   * @see  TIME_FORMAT
   * @see  DECINT_FORMAT
   * @see  HEXINT_FORMAT
   * @see  BININT_FORMAT
   * @see  SCIENTIFICINT_FORMAT
   * @see  DATE_FORMAT
   * @see  getLabelFormat
   */
  void GLAxis::SetLabelFormat(int l) {
    labelFormat = l;
  }

  /**
   * Returns the axis label format.
   * @return Axis value format
   * @see  setLabelFormat
   */
  int GLAxis::GetLabelFormat() {
    return labelFormat;
  }

  /**
   * Shows the grid.
   * @param b true to make the grid visible; false to hide it
   * @see  isGridVisible
   */
  void GLAxis::SetGridVisible(bool b) {
    gridVisible = b;
  }

  /**
   * Fit the x axis to display duration (Horizontal axis only).
   * @param b true to fit x axis false otherwise
   */
  void GLAxis::SetFitXAxisToDisplayDuration(bool b) {
    fitXAxisToDisplayDuration = b;
  }

  /**
   * Return true if the x axis fit to display duration.
   */
  bool GLAxis::IsFitXAxisToDisplayDuration() {
    return fitXAxisToDisplayDuration;
  }

  /**
   * Determines whether the axis is showing the grid
   * @return true if the grid is visible, false otherwise
   * @see  setGridVisible
   */
  bool GLAxis::IsGridVisible() {
    return gridVisible;
  }

  /**
   * Draw a second axis at the opposite side.
   * @param b true to enable the opposite axis.
   */
  void GLAxis::SetDrawOpposite(bool b) {
    drawOpposite = b;
  }

  /**
   * Determines whether the axis at the opposite side is visible
   * @return true if opposite axis is visible.
   * @see setDrawOpposite
   */
  bool GLAxis::IsDrawOpposite() {
    return drawOpposite;
  }

  /**
   * Shows the sub grid. More accurate grid displayed with a soft color.
   * @param b true to make the subgrid visible; false to hide it
   * @see  isSubGridVisible
   */
  void GLAxis::SetSubGridVisible(bool b) {
    subGridVisible = b;
  }

  /** Determines whether the axis is showing the sub grid
   *  @return true if the subgrid is visible, false otherwise
   *  @see setSubGridVisible
   */
  bool GLAxis::IsSubGridVisible() {
    return subGridVisible;
  }

  /** Sets the grid style.
   * @param s Style of the grid. Can be one of the following:
   * @see STYLE_SOLID
   * @see STYLE_DOT
   * @see STYLE_DASH
   * @see STYLE_LONG_DASH
   * @see STYLE_DASH_DOT
   * @see getGridStyle
   */
  void GLAxis::SetGridStyle(int s) {
    gridStyle = s;
  }

  /**
   * Returns the current grid style.
   * @return the current grid style
   * @see setGridStyle
   */
  int GLAxis::GetGridStyle() {
    return gridStyle;
  }

  /**
   * Set the annotation method
   * @param a Annotation for this axis
   * @see TIME_ANNO
   * @see VALUE_ANNO
   */
  void GLAxis::SetAnnotation(int a) {
    annotation = a;
  }

  /**
   * Returns the annotation method
   * @see setAnnotation
   */
  int GLAxis::GetAnnotation() {
    return annotation;
  }

  /**
   * Display or hide the axis.
   * @param b True to make the axis visible.
   */
  void GLAxis::SetVisible(bool b) {
    visible = b;
  }

  /**
   * Returns true if the axis is visble, false otherwise
   */
  bool GLAxis::IsVisible() {
    return visible;
  }

  /** Determines whether the axis is zoomed.
   * @return true if the axis is zoomed, false otherwise
   * @see zoom
   */
  bool GLAxis::IsZoomed() {
    return isZoomed;
  }

  /** Determines whether the axis is in XY mode. Use only with HORIZONTAL axis.
   *  @return true if the axis is in XY mode, false otherwise
   *  @see addDataView
   */
  bool GLAxis::IsXY() {
    return (nbView > 0);
  }

  /**
   * Sets minimum axis value. This value is ignored when using autoscale.
   * @param d Minimum value for this axis. Must be strictly positive for LOG_SCALE.
   * @see getMinimum
   */
  void GLAxis::SetMinimum(double d) {

    minimum = d;

    if (!autoScale) {
      if (scale == LOG_SCALE) {
        if (d <= 0) d = 1;
        min = log(d) / ln10;
      } else
        min = d;
    }

  }

  /**
   * Gets minimum axis value
   * @return  The minimum value for this axis.
   * @see setMinimum
   */
  double GLAxis::GetMinimum() {
    return minimum;
  }

  /**
   * Sets maximum axis value. This value is ignored when using autoscale.
   * @param d Maximum value for this axis. Must be strictly positive for LOG_SCALE.
   * @see getMaximum
   */
  void GLAxis::SetMaximum(double d) {

    maximum = d;

    if (!autoScale) {
      if (scale == LOG_SCALE) {
        if (max <= 0) max = min * 10.0;
        max = log(d) / ln10;
      } else
        max = d;
    }

  }

  /**
   * Gets maximum axis value
   * @return  The maximum value for this axis.
   * @see setMaximum
   */
  double GLAxis::GetMaximum() {
    return maximum;
  }

  /**
   * Expert usage. Get minimum axis value (according to auto scale transformation).
   * @return  The minimum value for this axis.
   */
  double GLAxis::GetMin() {
    return min;
  }

  /**
   * Expert usage. Get maximum axis value (according to auto scale transformation).
   * @return  The maximum value for this axis.
   */
  double GLAxis::GetMax() {
    return max;
  }

  /** Determines whether the axis is autoscaled.
   * @return true if the axis is autoscaled, false otherwise
   * @see setAutoScale
   */
  bool GLAxis::IsAutoScale() {
    return autoScale;
  }

  /**
   * Sets the autoscale mode for this axis.
   * @param b true if the axis is autoscaled, false otherwise
   * @see isAutoScale
   */
  void GLAxis::SetAutoScale(bool b) {
    autoScale = b;
  }

  /** Gets the scale mdoe for this axis.
   * @return scale mdoe
   * @see setScale
   */
  int GLAxis::GetScale() {
    return scale;
  }

  /** Sets scale mode
   * @param s Scale mode for this axis
   * @see LINEAR_SCALE
   * @see LOG_SCALE
   * @see getScale
   */
  void GLAxis::SetScale(int s) {

    scale = s;

    if (scale == LOG_SCALE) {
      // Check min and max
      if (minimum <= 0 || maximum <= 0) {
        minimum = 1;
        maximum = 10;
      }
    }

    if (scale == LOG_SCALE) {
      min = log(minimum) / ln10;
      max = log(maximum) / ln10;
    } else {
      min = minimum;
      max = maximum;
    }

  }

  /**
   * Sets the axis orientation and reset position.
   * @param orientation Orientation value
   * @see HORIZONTAL_DOWN
   * @see HORIZONTAL_UP
   * @see VERTICAL_LEFT
   * @see VERTICAL_RIGHT
   * @see #setPosition
   */
  void GLAxis::SetOrientation(int orientation) {
    this->orientation = orientation;
    dOrientation = orientation;
  }

  /**
   * Returns the orientation of this axis.
   * @see #setOrientation
   */
  int GLAxis::GetOrientation() {
    return orientation;
  }

  /** Zoom axis.
   * @param x1 New minimum value for this axis
   * @param x2 New maximum value for this axis
   * @see isZoomed
   * @see unzoom
   */
  void GLAxis::Zoom(int x1, int x2) {

    if (!isZoomed) lastAutoScate = autoScale;

    if (IsHorizontal()) {

      // Clip
      if (x1 < boundRect.x) x1 = boundRect.x;
      if (x2 > (boundRect.x + boundRect.width)) x2 = boundRect.x + boundRect.width;

      // Too small zoom
      if ((x2 - x1) < 10) return;

      // Compute new min and max
      double xr1 = (double) (x1 - boundRect.x) / (double) (boundRect.width);
      double xr2 = (double) (x2 - boundRect.x) / (double) (boundRect.width);
      double nmin = min + (max - min) * xr1;
      double nmax = min + (max - min) * xr2;

      // Too small zoom
      double difference = nmax - nmin;
      if (difference < 1E-13) return;

      min = nmin;
      max = nmax;

    } else {

      // Clip
      if (x1 < boundRect.y) x1 = boundRect.y;
      if (x2 > (boundRect.y + boundRect.height)) x2 = boundRect.y + boundRect.height;

      // Too small zoom
      if ((x2 - x1) < 10) return;

      // Compute new min and max
      double yr1 = (double) (boundRect.y + boundRect.height - x2) / (double) (boundRect.height);
      double yr2 = (double) (boundRect.y + boundRect.height - x1) / (double) (boundRect.height);
      double nmin = min + (max - min) * yr1;
      double nmax = min + (max - min) * yr2;

      // Too small zoom
      double difference = nmax - nmin;
      if (difference < 1E-13) return;

      min = nmin;
      max = nmax;

    }

    autoScale = false;
    isZoomed = true;

  }

  /** Unzoom the axis and restores last state.
   * @see isZoomed
   * @see unzoom
   */
  void GLAxis::Unzoom() {
    autoScale = lastAutoScate;
    if (!lastAutoScate) {
      SetMinimum(GetMinimum());
      SetMaximum(GetMaximum());
    }
    isZoomed = false;
  }

  /**
   * @deprecated Use getTickSpacing
   * @see getTickSpacing
   */
  int GLAxis::GetTick() {
    return (int)minTickStep;
  }

  /**
   * Returns the current minimum tick spacing (in pixel).
   */
  double GLAxis::GetTickSpacing() {
    return minTickStep;
  }

  /**
   * Sets the minimum tick spacing (in pixel).
   * Allows to control the number of generated labels.
   * @param spacing Minimum tick spacing
   */
  void GLAxis::SetTickSpacing(double spacing) {
    minTickStep = spacing;
  }

  /**
   * @deprecated Use setTickSpacing
   * @see setTickSpacing
   */
  void GLAxis::SetTick(int s) {
    minTickStep = s;
  }

  /**
   * Sets the tick length (in pixel).
   * @param lgth Length
   */
  void GLAxis::SetTickLength(int lgth) {
    tickLength = lgth;
    subtickLength = tickLength/2;
  }

  /**
   * Returns the tick length (in pixel).
   */
  int GLAxis::GetTickLength() {
    return tickLength;
  }

  /** Gets the axis label.
   * @return Axis name.
   * @see setName
   */
  char *GLAxis::GetName() {
    return name;
  }

  /**
   * Sets the axis label.
   * Label is displayed along or above the axis.
   * @param s Name of this axis.
   * @see getName
   */
  void GLAxis::SetName(const char *s) {
    strcpy(name,s);
  }

  /**
   * Sets the axis position
   * @param o Axis position
   * @see VERTICAL_LEFT
   * @see VERTICAL_RIGHT
   * @see VERTICAL_ORG
   * @see HORIZONTAL_DOWN
   * @see HORIZONTAL_UP
   * @see HORIZONTAL_ORG1
   * @see HORIZONTAL_ORG2
   */
  void GLAxis::SetPosition(int o) {
    if (IsHorizontal()) {
      if (o >= HORIZONTAL_DOWN && o <= HORIZONTAL_ORG2)
        orientation = o;
    } else {
      if (o >= VERTICAL_RIGHT && o <= VERTICAL_ORG)
        orientation = o;
    }
  }

  /**
   * Returns the axis position
   * @return Axis position
   * @see setPosition
   */
  int GLAxis::GetPosition() {
    return orientation;
  }

  int GLAxis::getDV(GLDataView *v) {
    bool found = false;
    int i=0;
    while(i<nbView && !found) {
      found = (dataViews[i] == v);
      if(!found) i++;
    }
    if(found)
      return i;
    else
      return -1;
  }

  /** Displays a DataView on this axis.
   * The graph switches in XY monitoring mode when adding
   * a dataView to X axis. Only one view is allowed on HORIZONTAL Axis.
   * In case of a view plotted along this horizontal axis doesn't have
   * the same number of point as this x view, points are correlated according to
   * their x values.
   * @param v The dataview to map along this axis.
   * @see removeDataView
   * @see clearDataView
   * @see getViews
   */
  void GLAxis::AddDataView(GLDataView *v) {

    if(getDV(v)>=0) return;

    if (!IsHorizontal()) {
      dataViews[nbView]=v;
      nbView++;
      v->SetAxis(this);
    } else {
      // Switch to XY mode
      // Only one view on X
      dataViews[0]=v;
      nbView = 1;
      v->SetAxis(this);
      SetAnnotation(VALUE_ANNO);
    }

  }

  /**
   * Add the given dataView at the specifed index.
   * @param index Insertion position
   * @param v DataView to add
   * @see addDataView
   */
  void GLAxis::AddDataViewAt(int index,GLDataView *v) {

    if(getDV(v)>=0) return;

    if (!IsHorizontal()) {
      for(int i=nbView;i>=index;i++)
        dataViews[i] = dataViews[i-1];
      dataViews[index]=v;
      nbView++;
      v->SetAxis(this);
    } else {
      // Switch to XY mode
      // Only one view on X
      dataViews[0]=v;
      nbView = 1;
      v->SetAxis(this);
      SetAnnotation(VALUE_ANNO);
    }

  }

  /**
   * Get the dataView of this axis at the specified index.
   * @param index DataView index
   * @return Null if index out of bounds.
   */
  GLDataView *GLAxis::GetDataView(int index) {

    if(index<0 || index>=nbView) {
      return NULL;
    }
    return dataViews[index];

  }

  /** Removes dataview from this axis
   * @param v dataView to remove from this axis.
   * @see addDataView
   * @see clearDataView
   * @see getViews
   */
  void GLAxis::RemoveDataView(GLDataView *v) {

    int idx = getDV(v);
    if( idx>=0 ) {
      for(int i=idx;i<nbView-1;i++)
        dataViews[i] = dataViews[i+1];
      nbView--;
    }
    v->SetAxis(NULL);
    if (IsHorizontal()) {
      // Restore TIME_ANNO and Liner scale
      SetAnnotation(TIME_ANNO);
      if (scale != LINEAR_SCALE) SetScale(LINEAR_SCALE);
    }

  }

  /** Removes dataview from this axis and returns true if the dataview has been found for this Axis
   * @param v dataView to remove from this axis.
   * @return true if Axis contained the dataview and false if this dataview did not belong to the axis
   * @see addDataView
   * @see clearDataView
   * @see getViews
   */
  bool GLAxis::CheckRemoveDataView(GLDataView *v)
  {

    int idx = getDV(v);
    if( idx>=0 ) {
      for(int i=idx;i<nbView-1;i++)
        dataViews[i] = dataViews[i+1];
      nbView--;
	    v->SetAxis(NULL);
	    if (IsHorizontal())
	    {
	      // Restore TIME_ANNO and Liner scale
	      SetAnnotation(TIME_ANNO);
	      if (scale != LINEAR_SCALE) SetScale(LINEAR_SCALE);
	    }
      return true;
    }
    return false;

  }

  /** Clear all dataview from this axis
   * @see removeDataView
   * @see addDataView
   * @see getViews
   */
  void GLAxis::ClearDataView() {
   // GLDataView v;
    for (int i = 0; i < nbView; i++) {
      dataViews[i]->SetAxis(NULL);
    }
    nbView=0;
  }

  /** Gets all dataViews displayed on this axis.
   * Do not modify the returned vector (Use as read only).
   * @return Vector3d of GLDataView.
   * @see addDataView
   * @see removeDataView
   * @see clearDataView
   */
  GLDataView **GLAxis::GetViews() {
    return dataViews;
  }

  /**
   * Returns the number if dataview in this axis.
   * @return DataView number.
   */
  int GLAxis::GetViewNumber() {
    return nbView;
  }

  /**
   * Invert this axis.
   * @param i true to invert the axis
   */
  void GLAxis::SetInverted(bool i) {
    inverted = i;
  }

  /**
   * Returns true if this axis is inverted.
   */
  bool GLAxis::IsInverted() {
    return inverted;
  }

  /**
   * Returns the bouding rectangle of this axis.
   * @return The bounding rectangle
   */
  GLCRectangle GLAxis::GetBoundRect() {
    return boundRect;
  }

  /** Return a scientific (exponential) representation of the double.
   * @param d double to convert
   * @return A string continaing a scientific representation of the given double.
   */
  char *GLAxis::ToScientific(double d) {

    static char ret[64];

    double a = fabs(d);
    int e = 0;
    std::string f = "%.2fe%d";

    if (a != 0) {
      if (a < 1) {
        while (a < 1) {
          a = a * 10;
          e--;
        }
      } else {
        while (a >= 10) {
          a = a / 10;
          e++;
        }
      }
    }

    if (a >= 9.999999999) {
      a = a / 10;
      e++;
    }

    if (d < 0) a = -a;

    sprintf(ret,f.c_str(),a,e);
    return ret;

  }

  char *GLAxis::ToScientificInt(double d) {

    static char ret[64];

    double a = fabs(d);
    int e = 0;
    std::string f = "%de%d";

    if (a != 0) {
      if (a < 1) {
        while (a < 1) {
          a = a * 10;
          e--;
        }
      } else {
        while (a >= 10) {
          a = a / 10;
          e++;
        }
      }
    }

    if (a >= 9.99999) {
      a = a / 10;
      e++;
    }

    if (d < 0) a = -a;

    sprintf(ret,f.c_str(),(int)(a+0.5),e);
    return ret;

  }

  /**
   * Returns a representation of the double in time format
   * @param vt number of sec since epoch
   * @return A string continaing a time representation of the given double.
   */
  char *GLAxis::FormatTimeValue(double vt) {

    static char ret[64];
    time_t timeval = (time_t)vt;
    strftime(ret,64,dateFormat,localtime(&timeval));
    return ret;

  }

  /**
   * Sets the appropriate time format for the range to display
   */
  void GLAxis::computeDateformat(int maxLab) {

    //find optimal precision
    bool found = false;
    int i = 0;
    while (i < nbTimeF && !found) {
      int n = (int) ((max - min) / timePrecs[i]);
      found = (n <= maxLab);
      if (!found) i++;
    }

    if (!found) {
      // TODO Year Linear scale
      i--;
      desiredPrec = 10 * YEAR;
      useFormat = "%Y"; // yearFormat
    } else {
      desiredPrec = timePrecs[i];
      useFormat = timeFormats[i];
    }

  }

  /**
   * Suppress last non significative zeros
   * @param n String representing a floating number
   */
  std::string GLAxis::suppressZero(const char *n) {

    static char ret[64];
    bool hasDecimal = (strrchr(n,'.') != NULL);

    if(hasDecimal) {

      strcpy(ret,n);
      int i = (int)strlen(n) - 1;
      while( n[i]=='0' ) {
        ret[i]=0;
        i--;
      }
      if(n[i]=='.') {
        // Remove unwanted decimal
        ret[i]=0;
      }

      return ret;

    } else {
      return n;
    }

  }

  /**
   * Returns a representation of the double acording to the format
   * @param vt double to convert
   * @param prec Desired precision (Pass 0 to not perform prec rounding).
   * @return A string continaing a formated representation of the given double.
   */
   std::string GLAxis::FormatValue(double vt, double prec) {

    static char ret[68];
    char format[64];
    if(isnan(vt))
      return "NaN";

    // Round value to nearest multiple of prec
    // TODO: rounding in LOG_SCALE
    if (prec != 0 && scale == LINEAR_SCALE) {

      bool isNegative = (vt < 0.0);
      if(isNegative) vt = -vt;

      // Find multiple
      double i = floor(vt/prec + 0.5);
      vt = i * prec;

      if(isNegative) vt = -vt;

    }

    switch (labelFormat) {
      case SCIENTIFIC_FORMAT:
        return ToScientific(vt);

      case SCIENTIFICINT_FORMAT:
        return ToScientificInt(vt);

      case DECINT_FORMAT:
      case HEXINT_FORMAT:
      case BININT_FORMAT:
        if (vt < 0.0) {
          sprintf(format,labelFomats[labelFormat],(int)(fabs(vt)+0.5));
          sprintf(ret,"-%s",format);
          return ret;
        } else {
          sprintf(format,labelFomats[labelFormat],(int)(fabs(vt)+0.5));
          sprintf(ret,"%s",format);
          return ret;
        }

      case TIME_FORMAT: {

        int sec = (int) (fabs(vt));
        int hh = (sec / 3600);
        int mm = (sec % 3600) / 60;
        int ss = (sec % 60);

        if (vt < 0.0) {
          sprintf(format,labelFomats[labelFormat],hh,mm,ss);
          sprintf(ret,"-%s",format);
          return ret;
        } else {
          sprintf(format,labelFomats[labelFormat],hh,mm,ss);
          sprintf(ret,"%s",format);
          return ret;
        }

        }break;

      case DATE_FORMAT: {
        time_t timeval = (time_t)fabs(vt);
        strftime(ret,64,dateFormat,localtime(&timeval));
        return ret;
        } break;

      default:

        // Auto format
        if(vt==0.0) return "0";

        if(fabs(vt)<=1.0E-4) {

          return ToScientific(vt);

        } else {

          //int nbDigit = -(int)floor(log10(prec));
          //if( nbDigit<=0 ) {
            sprintf(ret,"%g",vt);
            return suppressZero(ret);
          //} else {
          //  sprintf(format,"%%.%df",nbDigit);
          //  sprintf(ret,format,vt);
          //  return suppressZero(ret);
          //}

        }

    }

  }

  bool GLAxis::IsHorizontal() {
    return (dOrientation == HORIZONTAL_DOWN) ||
      (dOrientation == HORIZONTAL_UP);
  }

  // *****************************************************
  // AutoScaling stuff
  // Expert usage

  double GLAxis::computeHighTen(double d) {
    int p = (int)log10(d);
    return pow(10.0, p + 1);
  }

  double GLAxis::computeLowTen(double d) {
    int p = (int)log10(d);
    return pow(10.0, p);
  }

  void GLAxis::computeAutoScale() {

    int i = 0;
    double mi = 0,ma = 0;

    if (autoScale && nbView > 0) {

      GLDataView *v;
      min = MAX_VALUE;
      max = -MAX_VALUE;

      for (i = 0; i < nbView; i++) {

        v = dataViews[i];

        if (v->HasTransform()) {
          v->ComputeTransformedMinMax(&mi,&ma);
        } else {
          mi = v->GetMinimum();
          ma = v->GetMaximum();
        }

        if (scale == LOG_SCALE) {

          if (mi <= 0) mi = v->ComputePositiveMin();
          if (mi != MAX_VALUE) mi = log(mi) / ln10;

          if (ma <= 0)
            ma = -MAX_VALUE;
          else
            ma = log(ma) / ln10;
        }

        if (ma > max) max = ma;
        if (mi < min) min = mi;

      }

      // Check max and min
      if (min == MAX_VALUE && max == -MAX_VALUE) {

        // Only invalid data !!
        if (scale == LOG_SCALE) {
          min = 0;
          max = 1;
        } else {
          min = 0;
          max = 99.99;
        }

      }
      else if (zeroAlwaysVisible)
      {
        if (min < 0 && max < 0)
        {
          max = 0;
        }
        else if (min > 0 && max > 0)
        {
          min = 0;
        }
      }

      if ((max - min) < 1e-100) {
        max += 0.999;
        min -= 0.999;
      }

      double prec = computeLowTen(max - min);

      // Avoid unlabeled axis when log scale
      if( scale==LOG_SCALE && prec<1.0 )
        prec=1.0;

      if (min < 0)
        min = ((int) (min / prec) - 1) * prec;
      else
        if ( scale==LOG_SCALE) min = (int) (min / prec) * prec;
		else min*=0.95;

      if (max < 0)
        max = (int) (max / prec) * prec;
      else
        if ( scale==LOG_SCALE) max = ((int) (max / prec) + 1) * prec;
		else max*=1.05;

    } // end ( if autoScale )

  }

  /**
   * Expert usage. Sets the preferred scale for time axis (HORIZONTAL axis only)
   * @param d Duration (sec)
   */
  void GLAxis::SetAxisDuration(double d) {
    axisDuration = d;
  }

  /**
   * Expert usage. Compute X auto scale (HORIZONTAL axis only)
   * @param views All views displayed along all Y axis.
   */
  void GLAxis::ComputeXScale(GLDataView **views,int sz) {

    int i = 0;
    double mi,ma;

    if (IsHorizontal() && autoScale && sz > 0) {

      if (!IsXY()) {

        //******************************************************
        // Classic monitoring

        GLDataView *v;
        min = MAX_VALUE;
        max = -MAX_VALUE;

        // Horizontal autoScale

        for (i = 0; i < sz; i++) {

          v = views[i];
	  
          ma = v->GetMaxXValue();
          mi = v->GetMinXValue();

          if (scale == LOG_SCALE) {
            if (mi <= 0) {
              if (annotation == VALUE_ANNO) {
                mi = v->GetPositiveMinXValue();
              }
              else {
                mi = v->GetPositiveMinTime();
              }
            }
            if (mi != MAX_VALUE) mi = log(mi) / ln10;

            if (ma <= 0)
              ma = -MAX_VALUE;
            else
              ma = log(ma) / ln10;
          }

          if (ma > max) max = ma;
          if (mi < min) min = mi;

        }

        if (min == MAX_VALUE && max == -MAX_VALUE) {

          // Only empty views !

          if (scale == LOG_SCALE) {

            min = 0;
            max = 1;

          } else {

            if (annotation == TIME_ANNO) {
              min = (double)time(NULL) - HOUR;
              max = (double)time(NULL);
            } else {
              min = 0;
              max = 99.99;
            }

          }

        }

        if (annotation == TIME_ANNO) {

          if( axisDuration != MAX_VALUE && fitXAxisToDisplayDuration)
            min = max - axisDuration;

          max += (max - min) * percentScrollback;

        }

        if ((max - min) < 1e-100) {
          max += 0.999;
          min -= 0.999;
        }

      } else {

        //******************************************************
        // XY monitoring
        computeAutoScale();

      }

    }

  }

  // *****************************************************
  // Measurements stuff

  /**
   * @deprecated Use GetLabelFontDimension() instead
   */
  int GLAxis::GetFontHeight() {
    return GetLabelFontDimension();
  }

  /**
   * Expert usage.
   * @return Axis font dimension.
   */
  int GLAxis::GetLabelFontDimension() {

    if(!visible)
      return 5; // 5 pixel margin when axis not visible

    int hFont = GLToolkit::GetDialogFont()->GetTextHeight();

    if (IsHorizontal()) {

      if (strlen(name)>0) {
        if( orientation==HORIZONTAL_DOWN || orientation==HORIZONTAL_UP )
          return 2*hFont+5;
        else
          return hFont+5;
      } else {
          return hFont+5;
      }

    } else {
      if (strlen(name)>0)
        return hFont+5;
      else
        return 5;
    }

  }

  int GLAxis::GetFontOverWidth() {
    return fontOverWidth;
  }

  /**
   * Expert usage.
   * Returns axis tichkness in pixel ( shorter side )
   * @return Axis tichkness
   * @see getLength
   */
  int GLAxis::GetThickness() {
    if (visible) {
      if (!IsHorizontal())
        return csize.width;
      else
        return csize.height;
    }
    return 0;
  }

  /**
   * Expert usage.
   * Returns axis lenght in pixel ( larger side ).
   * @return Axis lenght.
   * @see getThickness
   */
  int GLAxis::getLength() {
    if (IsHorizontal())
      return csize.width;
    else
      return csize.height;
  }

  /**
   * Expert usage.
   * Computes labels and measures axis dimension.
   * @param frc Font render context
   * @param desiredWidth Desired width
   * @param desiredHeight Desired height
   */
  void GLAxis::MeasureAxis(int desiredWidth, int desiredHeight) {

    int i;
    int max_width = 10; // Minimun width
    int max_height = 0;

    computeAutoScale();

    if(autoLabeling) {
      if(!IsHorizontal())
        computeLabels(desiredHeight);
      else
        computeLabels(desiredWidth);
    } else {
      //if(!IsHorizontal())
      //  computeUserLabels(desiredHeight);
      //else
      //  computeUserLabels(desiredWidth);
    }

    for (i = 0; i < nbLabel; i++) {
      int lw = labels[i].size.width + std::abs(labels[i].offsetX);
      int lh = labels[i].size.height + std::abs(labels[i].offsetY);
      if (lw > max_width)   max_width = lw;
      if (lh > max_height)  max_height = lh;
    }

    fontOverWidth = max_width/2+1;

    if (!IsHorizontal()) {
      csize.width = max_width + GetLabelFontDimension();
      csize.height = desiredHeight;
    } else {
      csize.width = desiredWidth;
      csize.height = max_height;
    }

  }

  // ****************************************************************
  //	search nearest point stuff

  /**
   * Expert usage.
   * Transfrom given coordinates (real space) into pixel coordinates
   * @param x The x coordinates (Real space)
   * @param y The y coordinates (Real space)
   * @param xAxis The axis corresponding to x coordinates.
   * @return Point(-100,-100) when cannot transform
   */
  GLCPoint GLAxis::transform(double x, double y, GLAxis *xAxis) {

    GLCPoint ret;
    ret.x = -100;
    ret.y = -100;

    // The graph must have been measured before
    // we can transform
    if (csize.width<=0 || csize.height<=0) return ret;

    double xlength = (xAxis->GetMax() - xAxis->GetMin());
    int xOrg = boundRect.x;
    int yOrg = boundRect.y + getLength();
    double vx,vy;

    // Check validity
    if (isnan(y) || isnan(x))
      return ret;

    if (xAxis->GetScale() == LOG_SCALE) {
      if (x <= 0)
        return ret;
      else
        vx = log(x) / ln10;
    } else
      vx = x;

    if (scale == LOG_SCALE) {
      if (y <= 0)
        return ret;
      else
        vy = log(y) / ln10;

    } else
      vy = y;

    double xratio = (vx - xAxis->GetMin()) / (xlength) * (xAxis->getLength());
    double yratio = -(vy - min) / (max - min) * csize.height;

    // Saturate
    if (xratio < -32000) xratio = -32000;
    if (xratio > 32000) xratio = 32000;
    if (yratio < -32000) yratio = -32000;
    if (yratio > 32000) yratio = 32000;

    ret.x = (int) (xratio) + xOrg;
    ret.y = (int) (yratio) + yOrg;
    return ret;

  }

  //Return the square distance
  int GLAxis::distance2(int x1, int y1, int x2, int y2) {
    return (x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1);
  }

  bool GLAxis::insideRect(GLCRectangle *r,GLCPoint *p) {
    return (p->x >= r->x) && (p->x <= (r->width + r->x)) &&
           (p->y >= r->y) && (p->y <= (r->height + r->y));
  }

  /** Expert usage.
   * Search the nearest point in the dataViews in normal monitoring mode
   * @param x The x coordinates (Real space)
   * @param y The y coorsinates (Real space)
   * @param xAxis The axis corresponding to x coordinates.
   * @return A structure containing search result.
   * @see searchNearestXY
   */
  SearchInfo *GLAxis::searchNearestNormal(int x, int y, GLAxis *xAxis) {

    int idx = 0;
    int norme2;
    DataList *minP = NULL;
    GLCPoint minPt;
    int minNorme = 0x7FFFFFFFL;
    GLDataView *minDataView = NULL;
    int minPl = 0;
    int minIdx = -1;

    GLCRectangle boundRect2;
    boundRect2.x = boundRect.x - 2;
    boundRect2.y = boundRect.y - 2;
    boundRect2.width = boundRect.width + 4;
    boundRect2.height = boundRect.height + 4;

    for (int i = 0; i < nbView; i++) {

      GLDataView *v = dataViews[i];
      if (v->IsClickable()) {
        DataList *e = v->GetData();

        idx = 0;
        while (e != NULL) {

          GLCPoint p = transform(e->x, v->GetTransformedValue(e->y), xAxis);

          if (insideRect(&boundRect2,&p)) {
            norme2 = distance2(x, y, p.x, p.y);
            if (norme2 < minNorme) {

              minNorme = norme2;
              minP = e;
              minDataView = v;
              minPt = p;
              minIdx = idx;

              // Compute placement for the value info window
              if (p.x < (boundRect.x + boundRect.width / 2)) {
                if (p.y < (boundRect.y + boundRect.height / 2)) {
                  minPl = BOTTOMRIGHT;
                } else {
                  minPl = TOPRIGHT;
                }
              } else {
                if (p.y < (boundRect.y + boundRect.height / 2)) {
                  minPl = BOTTOMLEFT;
                } else {
                  minPl = TOPLEFT;
                }
              }
            }
          }

          e = e->next;
          idx++;

        }
      }
    }

    if (minNorme == 0x7FFFFFFFL)
      return new SearchInfo(); //No item found
    else
      return new SearchInfo(minPt.x, minPt.y, minDataView, this, minP, minNorme, minPl,minIdx);
  }

  /** Expert usage.
   * Search the nearest point in the dataViews in XY monitoring mode
   * @param x The x coordinates (Real space)
   * @param y The y coorsinates (Real space)
   * @param xAxis The axis corresponding to x coordinates.
   * @return A structure containing search result.
   * @see searchNearestNormal
   */
  SearchInfo *GLAxis::searchNearestXY(int x, int y, GLAxis *xAxis) {

    int norme2;
    DataList *minP = NULL;
    DataList *minXP = NULL;
    GLCPoint minPt;
    int minNorme = 0x7FFFFFFFL;
    GLDataView *minDataView = NULL;
    int minPl = 0;

    GLDataView *w = (xAxis->GetViews())[0];

    GLCRectangle boundRect2;
    boundRect2.x = boundRect.x - 2;
    boundRect2.y = boundRect.y - 2;
    boundRect2.width = boundRect.width + 4;
    boundRect2.height = boundRect.height + 4;

    for (int i = 0; i < nbView; i++) {

      GLDataView *v = dataViews[i];
      if (v->IsClickable()) {
        XYData e(v->GetData(), w->GetData());

        if (e.isValid()) e.initFirstPoint();

        while (e.isValid()) {

          GLCPoint p = transform(w->GetTransformedValue(e.d2->y), v->GetTransformedValue(e.d1->y), xAxis);

          if (insideRect(&boundRect2,&p)) {
            norme2 = distance2(x, y, p.x, p.y);
            if (norme2 < minNorme) {

              minNorme = norme2;
              minP = e.d1;
              minXP = e.d2;
              minDataView = v;
              minPt = p;

              // Compute placement for the value info window
              if (p.x < (boundRect.x + boundRect.width / 2)) {
                if (p.y < (boundRect.y + boundRect.height / 2)) {
                  minPl = BOTTOMRIGHT;
                } else {
                  minPl = TOPRIGHT;
                }
              } else {
                if (p.y < (boundRect.y + boundRect.height / 2)) {
                  minPl = BOTTOMLEFT;
                } else {
                  minPl = TOPLEFT;
                }
              }
            }
          }

          e.toNextXYPoint();

        }
      }

    }

    if (minNorme == 0x7FFFFFFFL) {
      return new SearchInfo(); //No item found
    } else {
      SearchInfo *si = new SearchInfo(minPt.x, minPt.y, minDataView, this, minP, minNorme, minPl,-1);
      si->setXValue(minXP, w);
      return si;
    }

  }

  /**
   * Search the nearest point in the dataViews.
   * @param x The x coordinates (Real space)
   * @param y The y coordinates (Real space)
   * @param xAxis The axis corresponding to x coordinates.
   * @return A structure containing search result.
   */
  SearchInfo *GLAxis::SearchNearest(int x, int y, GLAxis *xAxis) {

    //Search only in graph area
    if( x<=boundRect.x-5 ||
        x>=boundRect.x+boundRect.width+5 ||
        y<=boundRect.y-5 ||
        y>=boundRect.y+boundRect.height+5
      ) {
      return new SearchInfo();
    }

    if (xAxis->IsXY()) {
      return searchNearestXY(x, y, xAxis);
    } else {
      return searchNearestNormal(x, y, xAxis);
    }

  }

  // ****************************************************************
  // Label management

  void GLAxis::addLabel(const char *lab, int w, int h, double pos,int offX,int offY) {
    labels[nbLabel].value = strdup(lab);
    labels[nbLabel].size.width = w;
    labels[nbLabel].size.height = h;
    labels[nbLabel].pos = pos;
    labels[nbLabel].offsetX = offX;
    labels[nbLabel].offsetY = offY;
    nbLabel++;
  }

  void GLAxis::clearLabel() {
    for(int i=0;i<nbLabel;i++) free(labels[i].value);
    nbLabel = 0;
  }

  // ****************************************************************
  // Compute labels
  // Expert usage
  void GLAxis::computeLabels(double length) {

    if (max < min) {
   	  double a = min;
   	  min=max;
   	  max=a;
    }

    double sz = max - min;
    double pos;
    int w,h;
    int lgth = (int) length;
    char s[64];
    double startx;
    double prec;
    double precDelta = sz / length;
    time_t timeval;
    size_t round;

    clearLabel();

    switch (annotation) {

      case TIME_ANNO: {

        // Only for HORINZONTAL axis !
        // This has nothing to do with TIME_FORMAT
        // 10 labels maximum should be enough...
        computeDateformat(10);

        // round to multiple of prec
        round = (size_t) (min / desiredPrec);
        startx = (round + 1) * desiredPrec;

        if (inverted)
          pos = length * (1.0 - (startx - min) / sz);
        else
          pos = length * ((startx - min) / sz);

        timeval = (time_t)startx;
        strftime(s,64,useFormat.c_str(),localtime(&timeval));
        w = GLToolkit::GetDialogFont()->GetTextWidth(s);
        h = GLToolkit::GetDialogFont()->GetTextHeight();
        addLabel(s, w, h, pos);

        double minStep = (double) w * 1.3;
        if(minStep<minTickStep) minStep = minTickStep;
        double minPrec = (minStep / length) * sz;

        // Correct to avoid label overlap
        prec = desiredPrec;
        while (prec < minPrec) prec += desiredPrec;

        tickStep = length * prec/sz;
        startx += prec;
        if(inverted) tickStep = -tickStep;
        subTickStep = 0;

        // Build labels
        while (startx <= (max + precDelta)) {

          if (inverted)
            pos = (int)(length * (1.0 - (startx - min) / sz) + 0.5);
          else
            pos = (int)(length * ((startx - min) / sz) + 0.5);

          // Check limit
          if (pos > 0 && pos < lgth) {
            timeval = (time_t)startx;
            strftime(s,64,useFormat.c_str(),localtime(&timeval));
            w = GLToolkit::GetDialogFont()->GetTextWidth(s);
            h = GLToolkit::GetDialogFont()->GetTextHeight();
            addLabel(s, w, h, pos);
          }

          startx += prec;

        }
        }break;

      case VALUE_ANNO:{

        double fontAscent = (double) (GLToolkit::GetDialogFont()->GetTextHeight());
        prec = computeLowTen(max - min);
        bool extractLabel = false;

        // Anticipate label overlap

        int nbMaxLab;

        if(!IsHorizontal()) {

          // VERTICAL
          nbMaxLab = (int) (length / (2.0*fontAscent));

        } else {

          // HORIZONTAL
          // The strategy is not obvious here as we can't estimate the max label width
          // Do it with the min and the max
          double minT;
          double maxT;
          if (scale == LOG_SCALE) {
            minT = pow(10.0, min);
            maxT = pow(10.0, max);
          } else {
            minT = min;
            maxT = max;
          }

          double mW;
          mW = GLToolkit::GetDialogFont()->GetTextWidth(FormatValue(minT, prec).c_str());
          w = GLToolkit::GetDialogFont()->GetTextWidth(FormatValue(maxT, prec).c_str());
          if(w>mW) mW = w;
          mW = 1.5*mW;
          nbMaxLab = (int) (length / mW);

        }

        int n;
        int step = 0;
        int subStep = 0;

        // Overrides maxLab
        int userMaxLab = (int)(length / minTickStep + 0.5);
        if (userMaxLab<nbMaxLab) nbMaxLab = userMaxLab;

        if (nbMaxLab<1) nbMaxLab=1; // At least 1 labels !

        // Find the best precision

        if (scale == LOG_SCALE) {

          prec = 1;   // Decade
          step = -1;  // Logarithm subgrid

          if( min<0.0 ) {
            startx = (double)( (int)(min - 0.5) );
          } else {
            startx = (double)( (int)(min + 0.5) );
          }

          n = (int)((max - min) / prec + 0.5);

          while (n > nbMaxLab) {
            prec = prec * 2;
            step = 2;
            n = (int)((max - min) / prec + 0.5);
            if (n > nbMaxLab) {
              prec = prec * 5;
              step = 10;
              n = (int)((max - min) / prec + 0.5);
            }
          }

          subStep = step;

        } else {

          // Linear scale

          step = 10;
          n = (int)((max - min) / prec + 0.5);

          if (n <= nbMaxLab) {

            // Look forward
            n = (int)((max - min) / (prec / 2.0) + 0.5);

            while (n <= nbMaxLab) {
              prec = prec / 2.0;
              step = 5;
              n = (int)((max - min) / (prec / 5.0) + 0.5);
              if (n <= nbMaxLab) {
                prec = prec / 5.0;
                step = 10;
                n = (int)((max - min) / (prec / 2.0) + 0.5);
              }
            }

          } else {

            // Look backward
            while(n>nbMaxLab) {
              prec = prec * 5.0;
              step = 5;
              n = (int)((max - min) / prec + 0.5);
              if(n>nbMaxLab) {
                prec = prec * 2.0;
                step = 10;
                n = (int)((max - min) / prec + 0.5);
              }
            }

          }

          // round to multiple of prec (last not visible label)

          round = (size_t)floor(min / prec);
          startx = (double)round * prec;

          // Compute real number of label

          double sx = startx;
          int nbL = 0;
          while(sx<=(max + precDelta)) {
            if( sx >= (min - precDelta)) {
              nbL++;
            }
            sx += prec;
          }

          if(nbL<=2) {
            // Only one label
            // Go backward and extract the 2 extremity
            if(step==10) {
              step=5;
              prec=prec/2.0;
            } else {
              step=10;
              prec=prec/5.0;
            }
            extractLabel = true;
          }

          // Compute tick sapcing

          double tickSpacing = fabs(((prec / sz)*length) / (double)step);
          subStep = step;
          while(tickSpacing<10.0 && subStep>1) {
            switch (subStep) {
              case 10:
               subStep = 5;
               tickSpacing = tickSpacing * 2;
               break;
              case 5:
               subStep = 2;
               tickSpacing = tickSpacing * 2.5;
               break;
              case 2:
               // No sub tick
               subStep = 1;
               break;
            }
          }

        }

        // Compute tickStep

        tickStep = length * prec/sz;
        if(inverted) tickStep = -tickStep;
        subTickStep = subStep;

        //Adjust labels offset according to tick

        int offX = 0;
        int offY = 0;
        switch(dOrientation) {
          case VERTICAL_LEFT:
            offX = (tickLength<0)?tickLength:0;
            break;
          case VERTICAL_RIGHT:
            offX = (tickLength<0)?-tickLength:0;
            break;
          default: // HORIZONTAL_DOWN
            offY = (tickLength<0)?-tickLength:0;
            break;
        }

        //Build labels

        char lastLabelText[64];
        double lastDiff = MAX_VALUE;
        LabelInfo *lastLabel = NULL;
        strcpy(lastLabelText,"");
        while (startx <= (max + precDelta)) {

          if (inverted)
            pos = (int)(length * (1.0 - (startx - min) / sz) + 0.5);
          else
            pos = (int)(length * ((startx - min) / sz) + 0.5);

          double vt;
          if (scale == LOG_SCALE)
            vt = pow(10.0, startx);
          else
            vt = startx;

		  std::string tempValue = FormatValue(vt, prec);
          double diff = 0;
          if (labelFormat != TIME_FORMAT && labelFormat != DATE_FORMAT)
          {
            double t;
            sscanf(tempValue.c_str(),"%lf",&t);
            diff = fabs(t - vt);
          }
          if (strcmp(lastLabelText,tempValue.c_str())==0)
          {
            //avoiding label duplication
            if (diff < lastDiff)
            {
              strcpy(s,tempValue.c_str());
              if (lastLabel != NULL)
              {
                free(lastLabel->value);
                lastLabel->value = strdup("");
              }
            }
            else
            {
              strcpy(s,"");
            }
          }
          else
          {
            strcpy(s,tempValue.c_str());
          }
          lastDiff = diff;
          strcpy(lastLabelText,tempValue.c_str());

          if (startx >= (min - precDelta)) {
            w = GLToolkit::GetDialogFont()->GetTextWidth(s);
            h = GLToolkit::GetDialogFont()->GetTextHeight();
            addLabel(s,w,h,pos,offX,offY);
            lastLabel = labels + (nbLabel-1);
          }

          startx += prec;

        }

        // Extract 2 bounds when we didn't found a correct match.
        if(extractLabel) {
          if(nbLabel>2) {
            for(int i=1;i<nbLabel-1;i++) free(labels[i].value);
            labels[1] = labels[nbLabel-1];
            tickStep = labels[1].pos - labels[0].pos;
            subTickStep = nbLabel-1;
            nbLabel = 2;
          }
        }

        int iLenght = (int)(length+0.5);
        h = GLToolkit::GetDialogFont()->GetTextHeight();

        if( IsHorizontal() && nbLabel==1 && labels[0].pos==0 && !inverted ) {
          // Add the max at the end
		std::string mStr = FormatValue((scale==LOG_SCALE)?pow(10.0,max):max, prec);
          w = GLToolkit::GetDialogFont()->GetTextWidth(mStr.c_str());
          addLabel(mStr.c_str(),w,h,iLenght,0,0);
          tickStep = -1.0;
        }

        if( IsHorizontal() && nbLabel==1 && labels[0].pos==iLenght && !inverted ) {
          // Add the min at the begining
			std::string mStr = FormatValue((scale==LOG_SCALE)?pow(10.0,min):min, prec);
          w = GLToolkit::GetDialogFont()->GetTextWidth(mStr.c_str());
          addLabel(mStr.c_str(),w,h,0,0,0);
          tickStep = -1.0;
        }

        if( IsHorizontal() && nbLabel==0 && !inverted ) {
          // Add min and max
		  std::string mStr = FormatValue((scale==LOG_SCALE)?pow(10.0,max):max, prec);
          w = GLToolkit::GetDialogFont()->GetTextWidth(mStr.c_str());
          addLabel(mStr.c_str(),w,h,iLenght,0,0);
          mStr = FormatValue((scale==LOG_SCALE)?pow(10.0,min):min, prec);
          w = GLToolkit::GetDialogFont()->GetTextWidth(mStr.c_str());
          addLabel(mStr.c_str(),w,h,0,0,0);
          tickStep = -1.0;
        }

        }break;

    }

  }

  // ****************************************************************
  // Painting stuff

  /** Expert Usage.
   * Paint last point of a dataView.
   * @param g Graphics object
   * @param lp last point
   * @param p new point
   * @param v view containing the lp and p.
   */
  void GLAxis::DrawFast(GLCPoint lp, GLCPoint p, GLDataView *v) {

    if (insideRect(&boundRect,&lp)) {

      int pointX[2];
      int pointY[2];
      pointX[0] = lp.x;
      pointX[1] =  p.x;
      pointY[0] = lp.y;
      pointY[1] =  p.y;
      GLColor c = v->GetColor();
      GLToolkit::DrawPoly(v->GetLineWidth(),v->GetStyle(),c.r,c.g,c.b,2,pointX,pointY);

    }

    //Paint marker
    PaintMarker(v->GetMarkerColor(), v->GetMarker(), v->GetMarkerSize(), p.x, p.y);

  }

  void GLAxis::paintMarkerTex(GLuint mTex,int x,int y,int width,int height,int r,int g,int b) {

    if(!mTex) return;

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D,mTex);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    glColor3f((float)r/255.0f,(float)g/255.0f,(float)b/255.0f);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f,0.0f);
    glVertex2i(x,y);
    glTexCoord2f(1.0f,0.0f);
    glVertex2i(x+width,y);
    glTexCoord2f(1.0f,1.0f);
    glVertex2i(x+width,y+height);
    glTexCoord2f(0.0f,1.0f);
    glVertex2i(x,y+height);
    glEnd();

#ifdef _DEBUG
    theApp->nbPoly++;
#endif

  }

  /** Expert usage.
   * Paint a marker a the specified position
   * @param g Graphics object
   * @param mType Marker type
   * @param mSize Marker size
   * @param x x coordinates (pixel space)
   * @param y y coordinates (pixel space)
   */
  void GLAxis::PaintMarker(GLColor c,int mType, int mSize, int x, int y) {

    int mSize2 = mSize / 2;
    //int mSize21 = mSize / 2 + 1;
    int mSize21 = mSize2;

    switch (mType) {

      case MARKER_DOT:
        paintMarkerTex(dotTex,x-mSize2,y-mSize2,mSize,mSize,c.r,c.g,c.b);
        break;
      case MARKER_BOX:
        GLToolkit::DrawBox(x-mSize2,y-mSize2,mSize,mSize,c.r,c.g,c.b);
        break;
      case MARKER_TRIANGLE:
        paintMarkerTex(triTex,x-mSize2,y-mSize2,mSize,mSize,c.r,c.g,c.b);
        break;
      case MARKER_DIAMOND:
        paintMarkerTex(diaTex,x-mSize2,y-mSize2,mSize,mSize,c.r,c.g,c.b);
        break;
      case MARKER_STAR:        
        drawLine(c,0,1,x - mSize2, y + mSize2, x + mSize21, y - mSize21);
        drawLine(c,0,1,x + mSize2, y + mSize2, x - mSize21, y - mSize21);
        drawLine(c,0,1,x, y - mSize2, x, y + mSize21);
        drawLine(c,0,1,x - mSize2, y, x + mSize21, y);
        break;
      case MARKER_VERT_LINE:
        drawLine(c,0,1,x, y - mSize2, x, y + mSize21);
        break;
      case MARKER_HORIZ_LINE:
        drawLine(c,0,1,x - mSize2, y, x + mSize21, y);
        break;
      case MARKER_CROSS:
        drawLine(c,0,1,x, y - mSize2, x, y + mSize21);
        drawLine(c,0,1,x - mSize2, y, x + mSize21, y);
        break;
      case MARKER_CIRCLE:
        // TODO
        break;
      case MARKER_SQUARE:
        drawLine(c,0,1,x-mSize2,y-mSize2,x+mSize21,y-mSize2);
        drawLine(c,0,1,x+mSize21,y-mSize2,x+mSize21,y+mSize21);
        drawLine(c,0,1,x+mSize21,y+mSize21,x-mSize2,y+mSize21);
        drawLine(c,0,1,x-mSize2,y+mSize21,x-mSize2,y-mSize2);
        break;
    }

  }

  void GLAxis::paintBarBorder(int barWidth, int y0, int x, int y) {
    glBegin(GL_LINE_STRIP);
    _glVertex2i(x - barWidth / 2, y);
    _glVertex2i(x + barWidth / 2, y);
    _glVertex2i(x + barWidth / 2, y0);
    _glVertex2i(x - barWidth / 2, y0);
    _glVertex2i(x - barWidth / 2, y);
    glEnd();
  }

  void GLAxis::paintBar(int barWidth, GLColor background, int fillStyle, int y0, int x, int y) {

    if (fillStyle != FILL_STYLE_NONE) {
      // TODO pattern fillStyle
      if (y > y0) {
        GLToolkit::DrawBox(x - barWidth / 2, y0, barWidth, y-y0,background.r,background.g,background.b);
      } else {
        GLToolkit::DrawBox(x - barWidth / 2, y, barWidth, (y0 - y),background.r,background.g,background.b);
      }
    }

  }

  /** Expert usage.
   * Draw a sample line of a dataview
   * @param g Graphics object
   * @param x x coordinates (pixel space)
   * @param y y coordinates (pixel space)
   * @param v dataview
   */
  void GLAxis::DrawSampleLine(int x, int y, GLDataView *v) {

    GLColor c = v->GetColor();

    // Draw
    if( v->GetViewType()== TYPE_LINE ) {

      int pointX[2];
      int pointY[2];
      pointX[0] = x;
      pointX[1] = x+40;
      pointY[0] = y;
      pointY[1] = y;
      GLToolkit::DrawPoly(v->GetLineWidth(),v->GetStyle(),c.r,c.g,c.b,2,pointX,pointY);
      GLAxis::PaintMarker(v->GetMarkerColor(), v->GetMarker(), v->GetMarkerSize(), x + 20, y);

    } else if( v->GetViewType() == TYPE_BAR ) {

      if(v->GetFillStyle()!= FILL_STYLE_NONE) {
        GLColor fc = v->GetFillColor();
        GLToolkit::DrawBox(x+16,y-4,8,8,fc.r,fc.g,fc.b);
      }

      if( v->GetLineWidth()>0 ) {

        int pointX[5];
        int pointY[5];
        pointX[0] = x+16;pointY[0] = y-4;
        pointX[1] = x+24;pointY[1] = y-4;
        pointX[2] = x+24;pointY[2] = y+4;
        pointX[3] = x+16;pointY[3] = y+4;
        pointX[4] = x+16;pointY[4] = y-4;
        GLToolkit::DrawPoly(v->GetLineWidth(),v->GetStyle(),c.r,c.g,c.b,5,pointX,pointY);

      }

    }

  }

  void GLAxis::clip(int x,int y,int width,int height) {
    parent->GetWindow()->ClipRect(parent,x,y,width,height);
  }

  void GLAxis::drawLine(GLColor c,int dash,int lWidth,int x1,int y1,int x2,int y2) {
    int pointX[2];
    int pointY[2];
    pointX[0] = x1;
    pointX[1] = x2;
    pointY[0] = y1;
    pointY[1] = y2;
    GLToolkit::DrawPoly(lWidth,dash,c.r,c.g,c.b,2,pointX,pointY);
  }

  //   Expert usage
  //  Paint dataviews along the given axis
  //  xAxis horizonbtal axis of the graph
  //  xOrg x origin (pixel space)
  //  yOrg y origin (pixel space)
  void GLAxis::PaintDataViews(GLAxis *xAxis, int xOrg, int yOrg) {

    int k;
    bool isXY = xAxis->IsXY();
    GLDataView *vx = NULL;

    //-------- Clipping

    int xClip = xOrg + 1;
    int yClip = yOrg - getLength() + 1;
    int wClip = xAxis->getLength() - 1;
    int hClip = getLength() - 1;

    if (wClip <= 1 || hClip <= 1) return;
    clip(xClip, yClip, wClip, hClip);

    //-------- Draw dataView
    if (isXY) vx = xAxis->GetViews()[0];

    for (k = 0; k < nbView; k++) {

      GLDataView *v = dataViews[k];

      if (isXY)
        paintDataViewXY(v, vx, xAxis, xOrg-xClip, yOrg-yClip);
      else
        paintDataViewNormal(v, xAxis, xOrg-xClip, yOrg-yClip);

    } // End (for k<nbView)

  }

  // Paint dataviews along the given axis
  // Expert usage
  int GLAxis::computeBarWidth(GLDataView *v, GLAxis *xAxis) {

    int defaultWidth = 20;
    double minx = xAxis->GetMin();
    double maxx = xAxis->GetMax();
    int bw = v->GetBarWidth();
    double minI = MAX_VALUE;

    // No auto scale
    if (bw > 0)
      return bw;

    // No autoScale when horizontal axis is logarithmic
    if (xAxis->GetScale() == LOG_SCALE)
      return defaultWidth;

    if (xAxis->IsXY()) {

      GLDataView *vx = xAxis->GetViews()[0];

      // Look for the minimun interval
      DataList *d = vx->GetData();
      if (d != NULL) {
        double x = d->y;
        d = d->next;
        while (d != NULL) {
          double diff = fabs(d->y - x);
          if (diff < minI) minI = diff;
          x = d->y;
          d = d->next;
        }
      }

    } else {

      // Look for the minimun interval
      DataList *d = v->GetData();
      if (d != NULL) {
        double x = d->x;
        d = d->next;
        while (d != NULL) {
          double diff = fabs(d->x - x);
          if (diff < minI) minI = diff;
          x = d->x;
          d = d->next;
        }
      }

    }

    if (minI == MAX_VALUE)
      return defaultWidth;

    bw = (int)floor(minI / (maxx - minx) * xAxis->getLength()) - 2;

    // Make width multiple of 2 and saturate
    bw = bw / 2;
    bw = bw * 2;
    if (bw < 0) bw = 0;

    return bw;
  }

  void GLAxis::paintDataViewBar(GLDataView *v,
                                int barWidth,
                                int y0,
                                int x,
                                int y) {

    if (v->GetViewType() == TYPE_BAR) {

      paintBar(barWidth,v->GetFillColor(),v->GetFillStyle(),y0,x,y);

      // Draw bar border
      if (v->GetLineWidth() > 0)
        paintBarBorder(barWidth, y0, x, y);

    }

  }

  void GLAxis::paintDataViewPolyline(GLDataView *v,
                                     int nb,
                                     int yOrg,
                                     int *pointX,
                                     int *pointY) {

    if (nb > 1 && v->GetViewType() == TYPE_LINE) {

      // Draw surface
      if (v->GetFillStyle() != FILL_STYLE_NONE) {
        // TODO
        /*
        int[] Xs = new int[nb + 2];
        int[] Ys = new int[nb + 2];
        for (int i = 0; i < nb; i++) {
          Xs[i + 1] = pointX[i];
          Ys[i + 1] = pointY[i];
        }
        Xs[0] = Xs[1];
        Ys[0] = yOrg;
        Xs[nb + 1] = Xs[nb];
        Ys[nb + 1] = yOrg;
        if (fPattern != null) g2.setPaint(fPattern);
        g2.fillPolygon(Xs, Ys, nb + 2);
        */
      }

      if (v->GetLineWidth() > 0) {
        GLColor c = v->GetColor();
        GLToolkit::DrawPoly(v->GetLineWidth(),v->GetStyle(),c.r,c.g,c.b,nb,pointX,pointY);
      }

    }

  }

   void GLAxis::paintDataViewNormal(GLDataView *v, GLAxis *xAxis, int xOrg, int yOrg) {

    DataList *l = v->GetData();

    if (l != NULL) {

      // Get some variables

      int nbPoint = v->GetDataLength();
      int *pointX = (int *)malloc(nbPoint*sizeof(int));
      int *pointY = (int *)malloc(nbPoint*sizeof(int));

      double minx,maxx,lx;
      double miny,maxy,ly;
      double xratio;
      double yratio;
      double vt;
      double A0 = v->GetA0();
      double A1 = v->GetA1();
      double A2 = v->GetA2();
      int y0;

      minx = xAxis->GetMin();
      maxx = xAxis->GetMax();
      lx = xAxis->getLength();
      int sx = xAxis->GetScale();

      miny = min;
      maxy = max;
      ly = getLength();

      int j = 0;
      bool valid = true;

      // Compute zero vertical offset
      switch (v->GetFillMethod()) {
        case METHOD_FILL_FROM_TOP:
          y0 = yOrg - (int) ly;
          break;
        case METHOD_FILL_FROM_ZERO:
          if (scale == LOG_SCALE)
            y0 = yOrg;
          else
            y0 = (int) (miny / (maxy - miny) * ly) + yOrg;
          break;
        default:
          y0 = yOrg;
          break;
      }

      int barWidth = computeBarWidth(v, xAxis);

      while (l != NULL) {

        while (valid && l != NULL) {

          // Compute transform here for performance
          vt = A0 + A1 * l->y + A2 * l->y * l->y;
          valid = !isnan(vt) && (sx != LOG_SCALE || l->x > 1e-100)
            && (scale != LOG_SCALE || vt > 1e-100);

          if (valid) {

            if (sx == LOG_SCALE)
              xratio = (log(l->x) / ln10 - minx) / (maxx - minx) * lx;
            else
              xratio = (l->x - minx) / (maxx - minx) * lx;

            if (scale == LOG_SCALE)
              yratio = -(log(vt) / ln10 - miny) / (maxy - miny) * ly;
            else
              yratio = -(vt - miny) / (maxy - miny) * ly;

            // Saturate
            if (xratio < -32000) xratio = -32000;
            if (xratio > 32000) xratio = 32000;
            if (yratio < -32000) yratio = -32000;
            if (yratio > 32000) yratio = 32000;

            if (j < nbPoint) {
              pointX[j] = (int) (xratio+0.5) + xOrg;
              pointY[j] = (int) (yratio+0.5) + yOrg;

              // Draw marker
              if (v->GetMarker() > MARKER_NONE) {
                PaintMarker(v->GetMarkerColor(), v->GetMarker(), v->GetMarkerSize(), pointX[j], pointY[j]);
              }

              // Draw bar
              paintDataViewBar(v, barWidth, y0, pointX[j], pointY[j]);

              j++;
            }

            l = l->next;
          }

        }

        // Draw the polyline
        paintDataViewPolyline(v, j, y0, pointX, pointY);

        j = 0;
        if (!valid) {
          l = l->next;
          valid = true;
        }

      } // End (while l!=null)

      free(pointX);
      free(pointY);

    } // End (if l!=null)

  }

  // Paint dataviews along the given axis in XY mode
  // Expert usage
  void GLAxis::paintDataViewXY(GLDataView *v, GLDataView *w, GLAxis *xAxis, int xOrg, int yOrg) {

    XYData l(v->GetData(), w->GetData());

    if (l.isValid()) {

      int nbPoint = v->GetDataLength() + w->GetDataLength(); // Max number of point
      int *pointX = (int *)malloc(nbPoint*sizeof(int));
      int *pointY = (int *)malloc(nbPoint*sizeof(int));

      // Transform points

      double minx,maxx,lx;
      double miny,maxy,ly;
      double xratio;
      double yratio;
      double vtx;
      double vty;
      double A0y = v->GetA0();
      double A1y = v->GetA1();
      double A2y = v->GetA2();
      double A0x = w->GetA0();
      double A1x = w->GetA1();
      double A2x = w->GetA2();

      minx = xAxis->GetMin();
      maxx = xAxis->GetMax();
      lx = xAxis->getLength();
      int sx = xAxis->GetScale();

      miny = min;
      maxy = max;
      ly = getLength();
      int y0;

      int j = 0;
      bool valid = true;

      // Compute zero vertical offset
      switch (v->GetFillMethod()) {
        case METHOD_FILL_FROM_TOP:
          y0 = yOrg - (int) ly;
          break;
        case METHOD_FILL_FROM_ZERO:
          if (scale == LOG_SCALE)
            y0 = yOrg;
          else
            y0 = (int) (miny / (maxy - miny) * ly) + yOrg;
          break;
        default:
          y0 = yOrg;
          break;
      }

      int barWidth = computeBarWidth(v, xAxis);

      while (l.isValid()) {

        // Go to starting time position
        l.initFirstPoint();

        while (valid && l.isValid()) {

          // Compute transform here for performance
          vty = A0y + A1y * l.d1->y + A2y * l.d1->y * l.d1->y;
          vtx = A0x + A1x * l.d2->y + A2x * l.d2->y * l.d2->y;

          valid = !isnan(vtx) && !isnan(vty) &&
            (sx != LOG_SCALE || vtx > 1e-100) &&
            (scale != LOG_SCALE || vty > 1e-100);

          if (valid) {

            if (sx == LOG_SCALE)
              xratio = (log(vtx) / ln10 - minx) / (maxx - minx) * lx;
            else
              xratio = (vtx - minx) / (maxx - minx) * lx;

            if (scale == LOG_SCALE)
              yratio = -(log(vty) / ln10 - miny) / (maxy - miny) * ly;
            else
              yratio = -(vty - miny) / (maxy - miny) * ly;

            // Saturate
            if (xratio < -32000) xratio = -32000;
            if (xratio > 32000) xratio = 32000;
            if (yratio < -32000) yratio = -32000;
            if (yratio > 32000) yratio = 32000;
            if (j < nbPoint) {
              pointX[j] = (int) (xratio) + xOrg;
              pointY[j] = (int) (yratio) + yOrg;

              // Draw marker
              if (v->GetMarker() > MARKER_NONE) {
                PaintMarker(v->GetMarkerColor(), v->GetMarker(), v->GetMarkerSize(), pointX[j], pointY[j]);
              }

              // Draw bar
              paintDataViewBar(v, barWidth, y0, pointX[j], pointY[j]);

              j++;
            }
            // Go to next pos
            l.toNextXYPoint();
          }

        }

        // Draw the polyline
        paintDataViewPolyline(v, j, y0, pointX, pointY);

        j = 0;
        if (!valid) {
          // Go to next pos
          l.toNextXYPoint();
          valid = true;
        }

      } // End (while l!=null)

      free(pointX);
      free(pointY);

    } // End (if l.isvalid)

  }

  // Paint sub tick outside label limit
  // Expert usage
  void GLAxis::paintYOutTicks(GLColor c,int x0, double ys, int y0, int la, int tr,int off,bool grid) {

    int j,h;

    if (subtickVisible && tickStep>0.0) {

      if (subTickStep == -1) {

        for (j = 0; j < 8; j++) {
          h = (int)(ys + tickStep * logStep[j] + 0.5);
          if (h >= y0 && h <= (y0 + csize.height)) {
            drawLine(c,STYLE_SOLID,1,x0 + tr + off , h, x0 + tr + off + subtickLength, h);
            if (gridVisible && subGridVisible && grid) {
              drawLine(c,gridStyle,1,x0, h, x0 + la, h);
            }
          }
        }

      } else if (subTickStep > 0) {

        double r = 1.0 / (double)subTickStep;
        for (j = 0; j < subTickStep; j ++) {
          h = (int)(ys + tickStep * r * j + 0.5);
          if (h >= y0 && h <= (y0 + csize.height)) {
            drawLine(c,STYLE_SOLID,1,x0 + tr + off, h, x0 + tr + off + subtickLength, h);
            if ((j > 0) && gridVisible && subGridVisible && grid) {
              drawLine(c,gridStyle,1,x0, h, x0 + la, h);
            }
          }
        }

      }

    }

  }

  // Paint sub tick outside label limit
  // Expert usage
  void GLAxis::paintXOutTicks(GLColor c,int y0, double xs, int x0, int la,int tr,int off,bool grid) {

    int j,w;

    if (subtickVisible && tickStep>0.0) {

      if (subTickStep == -1) {

        for (j = 0; j < 8; j++) {
          w = (int)(xs + tickStep * logStep[j] + 0.5);
          if (w >= x0 && w <= (x0 + csize.width)) {
            drawLine(c,STYLE_SOLID,1,w, y0 + tr + off, w, y0 + tr + off + subtickLength);
            if (gridVisible && subGridVisible && grid) {
              drawLine(c,gridStyle,1,w, y0, w, y0 + la);
            }
          }
        }

      } else if (subTickStep > 0) {

        double r = 1.0 / (double)subTickStep;
        for (j = 0; j < subTickStep; j ++) {
          w = (int)(xs + tickStep * r * j + 0.5);
          if (w >= x0 && w <= (x0 + csize.width)) {
            drawLine(c,STYLE_SOLID,1,w, y0 + tr + off, w, y0 + tr + off + subtickLength);
            if ((j > 0) && gridVisible && subGridVisible && grid) {
              drawLine(c,gridStyle,1,w, y0, w, y0 + la);
            }
          }
        }

      }

    }

  }

  int GLAxis::getTickShift(int width) {

    // Calculate position
    int off=0;
    switch(dOrientation) {
      case VERTICAL_LEFT:
        if(orientation==VERTICAL_ORG)
          off=-width/2;
        break;
      case VERTICAL_RIGHT:
        if(orientation==VERTICAL_ORG)
          off=-width/2;
        else
          off=-width;
        break;
      case HORIZONTAL_DOWN:
      case HORIZONTAL_UP:
        if(orientation==HORIZONTAL_ORG1 || orientation==HORIZONTAL_ORG2)
          off=-width/2;
        else if (orientation==HORIZONTAL_UP)
          off=0;
        else
          off=-width;
        break;
    }

    return off;
  }

  int GLAxis::getTickShiftOpposite(int width) {

    // Calculate position
    int off=0;
    switch(dOrientation) {
      case VERTICAL_RIGHT:
        if(orientation==VERTICAL_ORG)
          off=-width/2;
        break;
      case VERTICAL_LEFT:
        if(orientation==VERTICAL_ORG)
          off=-width/2;
        else
          off=-width;
        break;
      case HORIZONTAL_DOWN:
      case HORIZONTAL_UP:
        if(orientation==HORIZONTAL_ORG1 || orientation==HORIZONTAL_ORG2)
          off=-width/2;
        else if (orientation==HORIZONTAL_UP)
          off=-width;
        else
          off=0;
        break;
    }

    return off;
  }

  // Paint Y sub tick and return tick spacing
  // Expert usage
  void GLAxis::paintYTicks(GLColor c,int i, int x0, double y, int la, int tr,int off,bool grid) {

    int j,h;

    if (subtickVisible && i < (nbLabel - 1) && tickStep>0.0) {

      // Draw ticks

      if (subTickStep == -1) {  // Logarithmic step

        for (j = 0; j < 8; j++) {
          h = (int)(y + tickStep * logStep[j] + 0.5);
          drawLine(c,STYLE_SOLID,1,x0 + tr + off, h, x0 + tr + off + subtickLength, h);
          if (gridVisible && subGridVisible && grid) {
            drawLine(c,gridStyle,1,x0, h, x0 + la, h);
          }
        }

      } else if (subTickStep > 0) {  // Linear step

        double r = 1.0 / (double)subTickStep;
        for (j = 0; j < subTickStep; j ++) {
          h = (int)(y + tickStep * r * j + 0.5);
          drawLine(c,STYLE_SOLID,1,x0 + tr + off , h, x0 + tr + off + subtickLength, h);
          if ((j > 0) && gridVisible && subGridVisible && grid) {
            drawLine(c,gridStyle,1,x0, h, x0 + la, h);
          }
        }

      }

    }

  }

  // Paint X sub tick and return tick spacing
  // Expert usage
  void GLAxis::paintXTicks(GLColor c,int i, int y0, double x, int la, int tr,int off,bool grid) {

    int j,w;

    if (subtickVisible && i < (nbLabel - 1) && tickStep>0.0) {

      if (subTickStep == -1) {  // Logarithmic step

        for (j = 0; j < 8; j++) {
          w = (int)(x + tickStep * logStep[j] + 0.5);
          drawLine(c,STYLE_SOLID,1,w, y0 + tr + off, w, y0 + tr + off + subtickLength);
          if (gridVisible && subGridVisible && grid) {
            drawLine(c,gridStyle,1,w, y0, w, y0 + la);
          }
        }

      } else if (subTickStep > 0) {  // Linear step

        double r = 1.0 / (double)subTickStep;
        for (j = 0; j < subTickStep; j++ ) {
          w = (int)(x + tickStep * r * j + 0.5);
          drawLine(c,STYLE_SOLID,1,w, y0 + tr + off, w, y0 + tr + off + subtickLength);
          if ((j > 0) && gridVisible && subGridVisible && grid) {
            drawLine(c,gridStyle,1,w, y0, w, y0 + la);
          }
        }

      }

    }

  }

  /**
   * Compute the medium color of c1,c2
   * @param c1 Color 1
   * @param c2 Color 2
   * @return Averaged color.
   */
  GLColor GLAxis::ComputeMediumColor(GLColor c1, GLColor c2) {
    GLColor r(
		c1.r + 3 * c2.r / 4,
		c1.g + 3 * c2.g / 4,
		c1.b + 3 * c2.b / 4
		);
    return r;
  }

  /** Expert usage.
   * Paint the axis and its DataView at the specified position along the given axis.
   * @param g Graphics object
   * @param frc Font render context
   * @param x0 Axis x position (pixel space)
   * @param y0 Axis y position (pixel space)
   * @param xAxis Horizontal axis of the graph
   * @param xOrg X origin for transformation (pixel space)
   * @param yOrg Y origin for transformation (pixel space)
   * @param back Background color
   * @param oppositeVisible Oposite axis is visible.
   *
   */
  void GLAxis::PaintAxis(int x0, int y0, GLAxis *xAxis, int xOrg, int yOrg, GLColor back,bool oppositeVisible) {

    int la = 0;
    int tr = 0;
    GLCPoint p0;

    // Do not paint vertical axis without data
    if (!IsHorizontal() && nbView == 0) return;

    // Do not paint when too small
    if (getLength() <= 1) {
      memset(&boundRect,0,sizeof(GLCRectangle));
      return;
    }

    la = xAxis->getLength();

    // Do not paint when out of bounds
    if(la<=0) {
      memset(&boundRect,0,sizeof(GLCRectangle));
      return;
    }

    // Update bounding rectangle

    switch (dOrientation) {

      case VERTICAL_LEFT:

        boundRect.x = x0 + GetThickness();
        boundRect.y = y0;
        boundRect.width = la;
        boundRect.height = csize.height;

        if (orientation == VERTICAL_ORG) {
          p0 = transform(0.0, 1.0, xAxis);
          if ((p0.x >= (x0 + csize.width)) && (p0.x <= (x0 + csize.width + la)))
            tr = p0.x - (x0 + csize.width);
          else
          // Do not display axe ot of bounds !!
            return;
        }
        break;

      case VERTICAL_RIGHT:

        boundRect.x = x0 - la - 1;
        boundRect.y = y0;
        boundRect.width = la;
        boundRect.height = csize.height;

        if (orientation == VERTICAL_ORG) {
          p0 = transform(0.0, 1.0, xAxis);
          if ((p0.x >= (x0 - la - 1)) && (p0.x <= x0))
            tr = p0.x - x0;
          else
          // Do not display axe ot of bounds !!
            return;
        }
        break;

      case HORIZONTAL_DOWN:
      case HORIZONTAL_UP:

        boundRect.x = x0;
        boundRect.y = y0 - la;
        boundRect.width = csize.width;
        boundRect.height = la;

        if (orientation == HORIZONTAL_ORG1 || orientation == HORIZONTAL_ORG2) {

          p0 = xAxis->transform(1.0, 0.0, this);
          if ((p0.y >= (y0 - la)) && (p0.y <= y0))
            tr = p0.y - y0;
          else
          // Do not display axe ot of bounds !!
            return;

        }

        break;

      default:
        printf("GLChart warning: Wrong axis position\n");
        break;

    }

    if(!visible)
      return;

    PaintAxisDirect(x0,y0,back,tr,xAxis->getLength());
    if(drawOpposite && oppositeVisible) {
      if(orientation==VERTICAL_ORG || orientation == HORIZONTAL_ORG1 || orientation == HORIZONTAL_ORG2)
        PaintAxisOppositeDouble(x0,y0,back,tr,xAxis->getLength());
      else
        PaintAxisOpposite(x0,y0,back,tr,xAxis->getLength());
    }

  }

  /**
   * Paint this axis.
   * @param g Graphics context
   * @param frc Font render context
   * @param x0 Axis position
   * @param y0 Axis position
   * @param back background Color (used to compute subTick color)
   * @param tr Translation from x0 to axis.
   * @param la Translation to opposite axis (used by grid).
   */
  void GLAxis::PaintAxisDirect(int x0, int y0,GLColor back,int tr,int la) {

    int i,x,y,tickOff,subTickOff,labelShift;

    GLColor subgridColor = ComputeMediumColor(labelColor, back);

    tickOff = getTickShift(tickLength);
    subTickOff = getTickShift(subtickLength);

    switch (dOrientation) {

      case VERTICAL_LEFT:

        //Draw extra sub ticks (outside labels limit)
        if (nbLabel>0 && autoLabeling) {
          LabelInfo *lis = labels;
          LabelInfo *lie = labels+ (nbLabel-1);
          paintYOutTicks(subgridColor, x0 + csize.width, (double)y0 + lis->pos - tickStep, y0, la, tr,subTickOff,true);
          paintYOutTicks(subgridColor, x0 + csize.width, (double)y0 + lie->pos, y0, la, tr,subTickOff,true);
        }

        for (i = 0; i < nbLabel; i++) {

          // Draw labels
          LabelInfo *li = labels + i;

          x = x0 + (csize.width - 4) - li->size.width;
          y = (int)(li->pos + 0.5) + y0;
          GLToolkit::GetDialogFont()->SetTextColor((float)labelColor.r/255.0f,(float)labelColor.g/255.0f,(float)labelColor.b/255.0f);
          GLToolkit::GetDialogFont()->DrawText(x + tr + li->offsetX, y + li->offsetY - li->size.height / 2,li->value,false);

          //Draw the grid
          if (gridVisible) {
            drawLine(labelColor,gridStyle,1,x0 + (csize.width + 1), y, x0 + (csize.width + 1) + la, y);
          }

          //Draw sub tick
          if(autoLabeling) {
            paintYTicks(subgridColor, i, x0 + csize.width,li->pos + (double)y0, la, tr,subTickOff,true);
          }

          //Draw tick
          drawLine(labelColor,STYLE_SOLID,1,x0 + tr + csize.width + tickOff, y, x0 + tr + csize.width + tickOff + tickLength, y);

        }

        // Draw Axe
        drawLine(labelColor,STYLE_SOLID,1,x0 + tr + csize.width, y0, x0 + tr + csize.width, y0 + csize.height);

        if (strlen(name)>0) {          
          // Draw vertical label
          int w = GLToolkit::GetDialogFont()->GetTextWidth(name);
          GLToolkit::GetDialogFont()->SetTextColor((float)labelColor.r/255.0f,(float)labelColor.g/255.0f,(float)labelColor.b/255.0f);
          GLToolkit::GetDialogFont()->DrawTextV(x0, y0+csize.height/2+w/2,name,false);
        }
        break;

      case VERTICAL_RIGHT:

        //Draw extra sub ticks (outside labels limit)
        if (nbLabel>0 && autoLabeling) {
          LabelInfo *lis = labels;
          LabelInfo *lie = labels + (nbLabel-1);
          paintYOutTicks(subgridColor, x0, (double)y0 + lis->pos - tickStep, y0, -la, tr,subTickOff,true);
          paintYOutTicks(subgridColor, x0, (double)y0 + lie->pos, y0, -la, tr,subTickOff,true);
        }

        for (i = 0; i < nbLabel; i++) {

          // Draw labels
          LabelInfo *li = labels + i;

          y = (int)(li->pos + 0.5) + y0;
          GLToolkit::GetDialogFont()->SetTextColor((float)labelColor.r/255.0f,(float)labelColor.g/255.0f,(float)labelColor.b/255.0f);
          GLToolkit::GetDialogFont()->DrawText(x0 + tr + li->offsetX + 6, y + li->offsetY - li->size.height / 2,li->value,false);

          //Draw the grid
          if (gridVisible) {
            drawLine(labelColor,gridStyle,1,x0, y, x0- la, y);
          }

          //Draw sub tick
          if(autoLabeling) {
            paintYTicks(subgridColor, i, x0, li->pos + (double)y0, -la, tr,subTickOff,true);
          }

          //Draw tick
          drawLine(labelColor,STYLE_SOLID,1,x0 + tr + tickOff , y, x0 + tr + tickOff + tickLength, y);
        }

        // Draw Axe
        drawLine(labelColor,STYLE_SOLID,1,x0 + tr, y0, x0 + tr, y0 + csize.height);

        if (strlen(name)>0) {
          // Draw vertical label
          int w = GLToolkit::GetDialogFont()->GetTextWidth(name);
          int h = GLToolkit::GetDialogFont()->GetTextHeight();
          GLToolkit::GetDialogFont()->SetTextColor((float)labelColor.r/255.0f,(float)labelColor.g/255.0f,(float)labelColor.b/255.0f);
          GLToolkit::GetDialogFont()->DrawTextV(x0+csize.width-h+2, y0+csize.height/2+w/2,name,false);
        }

        break;

      case HORIZONTAL_UP:
      case HORIZONTAL_DOWN:

        if(orientation==HORIZONTAL_UP) {
          tr = -la;
          labelShift = 1;
        } else {
          if(orientation==HORIZONTAL_ORG1 || orientation==HORIZONTAL_ORG2)
            labelShift = 1;
          else
            labelShift = 2;
        }

        //Draw extra sub ticks (outside labels limit)
        if (nbLabel>0 && autoLabeling) {
          LabelInfo *lis = labels;
          LabelInfo *lie = labels + (nbLabel-1);
          paintXOutTicks(subgridColor, y0, (double)x0 + lis->pos - tickStep,  x0, -la, tr,subTickOff,true);
          paintXOutTicks(subgridColor, y0, (double)x0 + lie->pos,  x0, -la, tr,subTickOff,true);
        }

        for (i = 0; i < nbLabel; i++) {

          // Draw labels
          LabelInfo *li = labels + i;

          x = (int)(li->pos + 0.5)+x0;
          y = y0;
          if (orientation==HORIZONTAL_UP) {
            GLToolkit::GetDialogFont()->SetTextColor((float)labelColor.r/255.0f,(float)labelColor.g/255.0f,(float)labelColor.b/255.0f);
            GLToolkit::GetDialogFont()->DrawText(x + li->offsetX - li->size.width / 2, y + tr - 2 - li->size.height,li->value,false);
          } else {
            GLToolkit::GetDialogFont()->SetTextColor((float)labelColor.r/255.0f,(float)labelColor.g/255.0f,(float)labelColor.b/255.0f);
            GLToolkit::GetDialogFont()->DrawText(x + li->offsetX - li->size.width / 2, y + tr + li->offsetY + 2,li->value,false);
          }

          //Draw sub tick
          if(autoLabeling) {
            paintXTicks(subgridColor, i, y, li->pos+(double)x0, -la, tr,subTickOff,true);
          }

          //Draw the grid
          if (gridVisible) {
            drawLine(labelColor,gridStyle,1,x, y0, x, y0 - la);
          }

          //Draw tick
          drawLine(labelColor,STYLE_SOLID,1,x, y0 + tr + tickOff, x, y0 + tr + tickOff + tickLength);

        }

        // Draw Axe
        drawLine(labelColor,STYLE_SOLID,1,x0, y0 + tr, x0 + csize.width, y0 + tr);

        if (strlen(name)>0) {
          int h = GLToolkit::GetDialogFont()->GetTextHeight();
          int w = GLToolkit::GetDialogFont()->GetTextWidth(name);
          GLToolkit::GetDialogFont()->DrawText(x0 + ((csize.width)-w) / 2,
            y0 + (labelShift-1) * h,name,false);
        }

        break;

    }

  }

  void GLAxis::PaintAxisOpposite(int x0, int y0,GLColor back,int tr,int la) {

    int i,x,y,tickOff,subTickOff,nX0;
    GLColor subgridColor = ComputeMediumColor(labelColor, back);

    tickOff = getTickShiftOpposite(tickLength);
    subTickOff = getTickShiftOpposite(subtickLength);

    switch (dOrientation) {

      case VERTICAL_RIGHT:

        nX0 = x0 - la - csize.width;

        //Draw extra sub ticks (outside labels limit)
        if (nbLabel>0 && autoLabeling) {
          LabelInfo *lis = labels;
          LabelInfo *lie = labels + (nbLabel-1);
          paintYOutTicks(subgridColor, nX0 + csize.width, (double)y0 + lis->pos - tickStep, y0, la, tr,subTickOff,false);
          paintYOutTicks(subgridColor, nX0 + csize.width, (double)y0 + lie->pos, y0, la, tr,subTickOff,false);
        }

        for (i = 0; i < nbLabel; i++) {

          LabelInfo *li = labels + i;
          x = nX0 + (csize.width - 4) - li->size.width;
          y = (int)(li->pos + 0.5) + y0;

          //Draw sub tick
          if(autoLabeling) {
            paintYTicks(subgridColor, i, nX0 + csize.width, li->pos + (double)y0, la, tr,subTickOff,false);
          }

          //Draw tick
          drawLine(labelColor,STYLE_SOLID,1,nX0 + tr + csize.width + tickOff, y, nX0 + tr + csize.width + tickOff + tickLength, y);

        }

        // Draw Axe
        drawLine(labelColor,STYLE_SOLID,1,nX0 + tr + csize.width, y0, nX0 + tr + csize.width, y0 + csize.height);
        break;

      case VERTICAL_LEFT:

        x = x0 + la + csize.width;

        //Draw extra sub ticks (outside labels limit)
        if (nbLabel>0 && autoLabeling) {
          LabelInfo *lis = labels;
          LabelInfo *lie = labels + (nbLabel-1);
          paintYOutTicks(subgridColor, x, (double)y0 + lis->pos - tickStep, y0, -la, tr,subTickOff,false);
          paintYOutTicks(subgridColor, x, (double)y0 + lie->pos, y0, -la, tr,subTickOff,false);
        }

        for (i = 0; i < nbLabel; i++) {

          LabelInfo *li = labels + i;
          y = (int)(li->pos + 0.5) + y0;

          //Draw sub tick
          if(autoLabeling) {
            paintYTicks(subgridColor, i, x, li->pos + (double)y0, -la, tr,subTickOff,false);
          }

          //Draw tick
          drawLine(labelColor,STYLE_SOLID,1,x + tr + tickOff , y, x + tr + tickOff + tickLength, y);
        }

        // Draw Axe
        drawLine(labelColor,STYLE_SOLID,1,x + tr, y0, x + tr, y0 + csize.height);
        break;

      case HORIZONTAL_UP:
      case HORIZONTAL_DOWN:

        if(orientation==HORIZONTAL_DOWN)
          tr = -la;

        //Draw extra sub ticks (outside labels limit)
        if (nbLabel>0 && autoLabeling) {
          LabelInfo *lis = labels;
          LabelInfo *lie = labels + (nbLabel-1);
          paintXOutTicks(subgridColor, y0, (double)x0 + lis->pos - tickStep, x0, -la, tr,subTickOff,false);
          paintXOutTicks(subgridColor, y0, (double)x0 + lie->pos, x0, -la, tr,subTickOff,false);
        }

        for (i = 0; i < nbLabel; i++) {

          LabelInfo *li = labels + i;
          x = (int)(li->pos + 0.5) + x0;
          y = y0;

          //Draw sub tick
          if(autoLabeling) {
            paintXTicks(subgridColor, i, y, li->pos + (double)x0, -la, tr,subTickOff,false);
          }

          //Draw tick
          drawLine(labelColor,STYLE_SOLID,1,x, y0 + tr + tickOff, x, y0 + tr + tickOff + tickLength);

        }

        // Draw Axe
        drawLine(labelColor,STYLE_SOLID,1,x0, y0 + tr, x0 + csize.width, y0 + tr);
        break;

    }

  }

  void GLAxis::PaintAxisOppositeDouble(int x0, int y0,GLColor back,int tr,int la) {

    int nX0;

    switch (dOrientation) {

      case VERTICAL_RIGHT:

        nX0 = x0 - la;

        // Draw Axe
        drawLine(labelColor,STYLE_SOLID,1,nX0, y0, nX0 , y0 + csize.height);

        // Draw Axe
        drawLine(labelColor,STYLE_SOLID,1,nX0 + la , y0, nX0 + la, y0 + csize.height);
        break;

      case VERTICAL_LEFT:

        nX0 = x0 + csize.width;

        // Draw Axe
        drawLine(labelColor,STYLE_SOLID,1,nX0, y0, nX0 , y0 + csize.height);

        // Draw Axe
        drawLine(labelColor,STYLE_SOLID,1,nX0 + la , y0, nX0 + la, y0 + csize.height);

        break;

      case HORIZONTAL_UP:
      case HORIZONTAL_DOWN:

        // Draw Axe
        drawLine(labelColor,STYLE_SOLID,1,x0, y0 - la, x0 + csize.width, y0 - la);

        // Draw Axe
        drawLine(labelColor,STYLE_SOLID,1,x0, y0, x0 + csize.width, y0);
        break;

    }

  }

  /**
   * Allaws user to know if the 0 value will always be visible in case of auto scale
   * @return a boolean value
   */
  bool GLAxis::IsZeroAlwaysVisible()
  {
    return zeroAlwaysVisible;
  }

  /**
   * Sets if 0 must always be visible in case of auto scale or not
   * @param zeroAlwaysVisible a boolean value. True for always visible, false otherwise.
   */
  void GLAxis::SetZeroAlwaysVisible(bool zeroAlwaysVisible)
  {
    this->zeroAlwaysVisible = zeroAlwaysVisible;
  }

  char *GLAxis::GetDateFormat()
  {
    return dateFormat;
  }

  /**
   * Sets date format chen chosen label format is DATE_FORMAT
   * @param dateFormat
   * @see #US_DATE_FORMAT
   * @see #FR_DATE_FORMAT
   */
  void GLAxis::SetDateFormat (const char *dateFormat)
  {
    strcpy(this->dateFormat,dateFormat);
  }

