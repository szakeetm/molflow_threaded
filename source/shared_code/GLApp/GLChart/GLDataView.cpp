// Copyright (c) 2011 rubicon IT GmbH
#include "GLChart.h"
#include "../GLTypes.h"
#include "../MathTools.h" //PI
//#include <malloc.h>
#include <float.h>
#include <math.h>
#include <cstring> //strcpy, etc.

GLDataView::GLDataView() {

  labelColor.r = 0;
  labelColor.g = 0;
  labelColor.b = 0;
  theData = NULL;
  theFilteredData = NULL;
  theDataEnd = NULL;
  dataLength = 0;
  strcpy(name,"");
  strcpy(unit,"");
  lineColor.r = 255;
  lineColor.g = 0;
  lineColor.b = 0;
  fillColor.r = 180;
  fillColor.g = 180;
  fillColor.b = 180;
  markerColor = lineColor;
  min = MAX_VALUE;
  max = -MAX_VALUE;
  minXValue = MAX_VALUE;
  maxXValue = -MAX_VALUE;
  markerType = MARKER_NONE;
  lineStyle = STYLE_SOLID;
  type = TYPE_LINE;
  fillStyle = FILL_STYLE_NONE;
  fillMethod = METHOD_FILL_FROM_BOTTOM;
  lineWidth = 1;
  barWidth = 10;
  markerSize = 6;
  A0 = 0;
  A1 = 1;
  A2 = 0;
  parentAxis = NULL;
  clickable=true;
  labelVisible=true;
  userFormat = NULL;
  interpMethod = INTERPOLATE_NONE;
  interpStep = 10;
  interpTension = 0.0;
  interpBias = 0.0;
  smoothMethod = SMOOTH_NONE;
  smoothCoefs = NULL;
  smoothNeighbor = 3;
  smoothSigma = 0.5;
  smoothExtrapolation = SMOOTH_EXT_LINEAR;
  mathFunction = MATH_NONE;

}

void GLDataView::FreeList(DataList *l) {
	if(l) {
		FreeList(l->next);
		free(l);
  } 
}

GLDataView::~GLDataView() {

  FreeList(theFilteredData);
  FreeList(theData);
  SAFE_FREE(smoothCoefs);

}

/* ----------------------------- Global config --------------------------------- */

/**
 * Sets the graph type (Line or BarGraph).
 * @param s Type of graph
 * @see TYPE_LINE
 * @see TYPE_BAR
 */
void GLDataView::SetViewType(int s) {
  type = s;
}

/**
 * Gets view type.
 * @return View type
 * @see setViewType
 */
int GLDataView::GetViewType() {
  return type;
}

/**
 * Sets the filling style of this view.
 * @param b Filling style
 * @see FILL_STYLE_NONE
 * @see FILL_STYLE_SOLID
 * @see FILL_STYLE_LARGE_RIGHT_HATCH
 * @see FILL_STYLE_LARGE_LEFT_HATCH
 * @see FILL_STYLE_LARGE_CROSS_HATCH
 * @see FILL_STYLE_SMALL_RIGHT_HATCH
 * @see FILL_STYLE_SMALL_LEFT_HATCH
 * @see FILL_STYLE_SMALL_CROSS_HATCH
 * @see FILL_STYLE_DOT_PATTERN_1
 * @see FILL_STYLE_DOT_PATTERN_2
 * @see FILL_STYLE_DOT_PATTERN_3
 */

void GLDataView::SetFillStyle(int b) {
  fillStyle = b;
}

/**
 * Gets the current filling style.
 * @return Filling style
 * @see setFillStyle
 */
int GLDataView::GetFillStyle() {
  return fillStyle;
}

/**
 * Sets the filling method for this view.
 * @param m Filling method
 * @see METHOD_FILL_FROM_TOP
 * @see METHOD_FILL_FROM_ZERO
 * @see METHOD_FILL_FROM_BOTTOM
 */

void GLDataView::SetFillMethod(int m) {
  fillMethod = m;
}

/**
 * Gets the current filling style.
 * @return Filling method
 * @see setFillMethod
 */
int GLDataView::GetFillMethod() {
  return fillMethod;
}

/**
 * Sets the filling color of this dataView.
 * @param c Filling color
 * @see getFillColor
 */
void GLDataView::SetFillColor(GLColor c) {
  fillColor = c;
}

/**
 * Gets the filling color.
 * @return Filling color
 * @see setFillColor
 */
GLColor GLDataView::GetFillColor() {
  return fillColor;
}

/* ----------------------------- Line config --------------------------------- */

/**
 * Sets the color of the curve.
 * @param c Curve color
 * @see getColor
 */
void GLDataView::SetColor(GLColor c) {
  lineColor = c;
}

/**
 * Gets the curve color.
 * @return Curve color
 * @see setColor
 */
GLColor GLDataView::GetColor() {
  return lineColor;
}

/**
 * Provided for backward compatibility.
 * @see setFillStyle
 * @return true if the view is filled, false otherwise
 */
bool GLDataView::IsFill() {
  return fillStyle!=FILL_STYLE_NONE;
}

/**
 * Provided for backward compatibility.
 * @param b true if the view is filled, false otherwise
 * @see setFillStyle
 */
void GLDataView::SetFill(bool b) {
  if( !b ) {
    SetFillStyle(FILL_STYLE_NONE);
  } else {
    SetFillStyle(FILL_STYLE_SOLID);
  }
}

/**
 * Sets this view clickable or not. When set to false, this view
 * is excluded from the search that occurs when the user click on the
 * chart. Default is true.
 * @param b Clickable state
 */
void GLDataView::SetClickable(bool b) {
  clickable=b;
}

/**
 * Returns the clickable state.
 * @see setClickable
 */
bool GLDataView::IsClickable() {
  return clickable;
}

/**
 * Display the label of this view when true.
 * This has effects only if the parent chart has visible labels.
 * @param b visible state
 * @see JLChart#setLabelVisible
 */
void GLDataView::SetLabelVisible(bool b) {
  labelVisible=b;
}

/** Returns true when the label is visible.
 * @see setLabelVisible
 */
bool GLDataView::IsLabelVisible() {
  return labelVisible;
}

GLColor GLDataView::GetLabelColor () {
  return labelColor;
}

void GLDataView::SetLabelColor (GLColor labelColor) {
  this->labelColor = labelColor;
}

/* ----------------------------- Interpolation config --------------------------------- */

/**
 * Set an interpolation on this dataview using the specified method.
 * (Cubic interpolation requires constant x interval)
 * @param method Interpolation method
 * @see #INTERPOLATE_NONE
 * @see #INTERPOLATE_LINEAR
 * @see #INTERPOLATE_COSINE
 * @see #INTERPOLATE_CUBIC
 * @see #INTERPOLATE_HERMITE
 */
void GLDataView::SetInterpolationMethod(int method) {
  interpMethod = method;
  CommitChange();
}

/**
 * Return current interpolation mode.
 * @see #setInterpolationMethod
 */
int GLDataView::GetInterpolationMethod() {
  return interpMethod;
}

/**
 * Sets the interpolation step
 * @param step Interpolation step (must be >=2)
 * @see #setInterpolationMethod
 */
void GLDataView::SetInterpolationStep(int step) {
  if(step<2) step=2;
  interpStep = step;
  updateFilters();
}

/**
 * Returns the interpolation step.
 * @see #setInterpolationStep
 */
int GLDataView::GetInterpolationStep() {
  return interpStep;
}

/**
 * Set the Hermite interpolation tension coefficient
 * @param tension Hermite interpolation tension coefficient (1=>high, 0=>normal, -1=>low)
 */
void GLDataView::SetHermiteTension(double tension) {
  if(tension<-1.0) tension=-1.0;
  if(tension>1.0) tension=1.0;
  interpTension = tension;
  updateFilters();
}

/**
 * Get the Hermite interpolation tension coefficient
 * @see #setHermiteTension
 */
double GLDataView::GetHermiteTension() {
  return interpTension;
}

/**
 * Set the Hermite interpolation bias coefficient.
 * 0 for no bias, positive value towards first segment, negative value towards the others.
 * @param bias Hermite interpolation bias coefficient
 */
void GLDataView::SetHermiteBias(double bias) {
  interpBias = bias;
  updateFilters();
}

/**
 * Set the Hermite interpolation bias coefficient.
 */
double GLDataView::GetHermiteBias() {
  return interpBias;
}

/* ----------------------------- Smoothing config --------------------------- */

/**
 * Sets the smoothing method (Convolution product). Requires constant x intervals.
 * @param method Smoothing filer type
 * @see #SMOOTH_NONE
 * @see #SMOOTH_FLAT
 * @see #SMOOTH_TRIANGULAR
 * @see #SMOOTH_GAUSSIAN
 */
void GLDataView::SetSmoothingMethod(int method) {
  smoothMethod = method;
  updateSmoothCoefs();
  CommitChange();
}

/**
 * Return the smoothing method.
 * @see #setSmoothingMethod
 */
int GLDataView::GetSmoothingMethod() {
  return smoothMethod;
}

/**
 * Sets number of neighbors for smoothing
 * @param n Number of neighbors (Must be >=2)
 */
void GLDataView::SetSmoothingNeighbors(int n) {
  smoothNeighbor = (n/2)*2+1;
  if(smoothNeighbor<3) smoothNeighbor=3;
  updateSmoothCoefs();
  updateFilters();
}

/**
 * Sets number of neighbors for smoothing
 * @see #setSmoothingNeighbors
 */
int GLDataView::GetSmoothingNeighbors() {
  return smoothNeighbor-1;
}

/**
 * Sets the standard deviation of the gaussian (Smoothing filter).
 * @param sigma Standard deviation
 * @see #setSmoothingMethod
 */
void GLDataView::SetSmoothingGaussSigma(double sigma) {
  smoothSigma = sigma;
  updateSmoothCoefs();
  CommitChange();
}

/**
 * Return the standard deviation of the gaussian (Smoothing filter).
 * @see #setSmoothingMethod
 */
double GLDataView::GetSmoothingGaussSigma() {
  return smoothSigma;
}

/**
 * Sets the extrapolation method used in smoothing operation
 * @param extMode Extrapolation mode
 * @see #SMOOTH_EXT_NONE
 * @see #SMOOTH_EXT_FLAT
 * @see #SMOOTH_EXT_LINEAR
 */
void GLDataView::SetSmoothingExtrapolation(int extMode) {
  smoothExtrapolation = extMode;
  updateFilters();
}

/**
 * Returns the extrapolation method used in smoothing operation.
 * @see #setSmoothingExtrapolation
 */
int GLDataView::GetSmoothingExtrapolation() {
  return smoothExtrapolation;
}

/* ----------------------------- Math config -------------------------------- */

/**
 * Sets a mathematical function
 * @param function Function
 * @see #MATH_NONE
 * @see #MATH_DERIVATIVE
 * @see #MATH_INTEGRAL
 * @see #MATH_FFT_MODULUS
 * @see #MATH_FFT_PHASE
 */
void GLDataView::SetMathFunction(int function) {
  mathFunction = function;
  updateFilters();
  if(function==MATH_NONE)
    computeDataBounds();
}

/**
 * Returns the current math function.
 * @see #setMathFunction
 */
int GLDataView::GetMathFunction() {
  return mathFunction;
}

/* ----------------------------- BAR config --------------------------------- */

/**
 * Sets the width of the bar in pixel (Bar graph mode).
 * Pass 0 to have bar auto scaling.
 * @param w Bar width (pixel)
 * @see getBarWidth
 */
void GLDataView::SetBarWidth(int w) {
  barWidth = w;
}

/**
 * Gets the bar width.
 * @return Bar width (pixel)
 * @see setBarWidth
 */
int GLDataView::GetBarWidth() {
  return barWidth;
}

/**
 * Sets the marker color.
 * @param c Marker color
 * @see getMarkerColor
 */
void GLDataView::SetMarkerColor(GLColor c) {
  markerColor = c;
}

/**
 * Gets the marker color.
 * @return Marker color
 * @see setMarkerColor
 */
GLColor GLDataView::GetMarkerColor() {
  return markerColor;
}

/**
 * Set the plot line style.
 * @param c Line style
 * @see STYLE_SOLID
 * @see STYLE_DOT
 * @see STYLE_DASH
 * @see STYLE_LONG_DASH
 * @see STYLE_DASH_DOT
 * @see getStyle
 */
void GLDataView::SetStyle(int c) {
  lineStyle = c;
}

/**
 * Gets the marker size.
 * @return Marker size (pixel)
 * @see setMarkerSize
 */
int GLDataView::GetMarkerSize() {
  return markerSize;
}

/**
 * Sets the marker size (pixel).
 * @param c Marker size (pixel)
 * @see getMarkerSize
 */
void GLDataView::SetMarkerSize(int c) {
  markerSize = c;
}

/**
 * Gets the line style.
 * @return Line style
 * @see setStyle
 */
int GLDataView::GetStyle() {
  return lineStyle;
}

/**
 * Gets the line width.
 * @return Line width
 * @see setLineWidth
 */
int GLDataView::GetLineWidth() {
  return lineWidth;
}

/**
 * Sets the line width (pixel).
 * @param c Line width (pixel)
 * @see getLineWidth
 */
void GLDataView::SetLineWidth(int c) {
  lineWidth = c;
}

/**
 * Sets the view name.
 * @param s Name of this view
 * @see getName
 */
void GLDataView::SetName(const char *s) {
  strcpy(name,s);
}

/**
 * Gets the view name.
 * @return Dataview name
 * @see setName
 */
char *GLDataView::GetName() {
  return name;
}

/**
 * Set the dataView unit. (Used only for display)
 * @param s Dataview unit.
 * @see getUnit
 */
void GLDataView::SetUnit(const char *s) {
  strcpy(unit,s);
}

/**
 * Gets the dataView unit.
 * @return Dataview unit
 * @see setUnit
 */
char *GLDataView::GetUnit() {
  return unit;
}

/**
 * Gets the extended name. (including transform description when used)
 * @return Extended name of this view.
 */
char *GLDataView::GetExtendedName() {

  static char ret[256];
  std::string t;
  char tmp[128];

  if (HasTransform()) {

    sprintf(ret,"%s [",name);

    if (A0 != 0.0) {
      sprintf(tmp,"%g",A0);
      strcat(ret,tmp);
      t = " + ";
    }
    if (A1 != 0.0) {
      sprintf(tmp,"%g",A1);
      strcat(ret,t.c_str());
      strcat(ret,tmp);
      strcat(ret,"*y");
      t = " + ";
    }
    if (A2 != 0.0) {
      sprintf(tmp,"%g",A1);
      strcat(ret,t.c_str());
      strcat(ret,tmp);
      strcat(ret,"*y^2");
    }
    strcat(ret,"]");
    return ret;

  } else {
    return name;
  }

}

/**
 * Gets the marker type.
 * @return Marker type
 * @see setMarker
 */
int GLDataView::GetMarker() {
  return markerType;
}

/**
 * Sets the marker type.
 * @param m Marker type
 * @see MARKER_NONE
 * @see MARKER_DOT
 * @see MARKER_BOX
 * @see MARKER_TRIANGLE
 * @see MARKER_DIAMOND
 * @see MARKER_STAR
 * @see MARKER_VERT_LINE
 * @see MARKER_HORIZ_LINE
 * @see MARKER_CROSS
 * @see MARKER_CIRCLE
 * @see MARKER_SQUARE
 */
void GLDataView::SetMarker(int m) {
  markerType = m;
}

/**
 * Gets the A0 transformation coeficient.
 * @return A0 value
 * @see setA0
 */
double GLDataView::GetA0() {
  return A0;
}

/**
 * Gets the A1 transformation coeficient.
 * @return A1 value
 * @see setA1
 */
double GLDataView::GetA1() {
  return A1;
}

/**
 * Gets the A2 transformation coeficient.
 * @return A2 value
 * @see setA2
 */
double GLDataView::GetA2() {
  return A2;
}

/**
 * Set A0 transformation coeficient. The transformation computes
 * new value = A0 + A1*v + A2*v*v.
 * Transformation is disabled when A0=A2=0 and A1=1.
 * @param d A0 value
 */
void GLDataView::SetA0(double d) {
  A0 = d;
}

/**
 * Set A1 transformation coeficient. The transformation computes
 * new value = A0 + A1*v + A2*v*v.
 * Transformation is disabled when A0=A2=0 and A1=1.
 * @param d A1 value
 */
void GLDataView::SetA1(double d) {
  A1 = d;
}

/**
 * Set A2 transformation coeficient. The transformation computes
 * new value = A0 + A1*v + A2*v*v.
 * Transformation is disabled when A0=A2=0 and A1=1.
 * @param d A2 value
 */
void GLDataView::SetA2(double d) {
  A2 = d;
}

/**
 * Determines wether this views has a transformation.
 * @return false when A0=A2=0 and A1=1, true otherwise
 */
bool GLDataView::HasTransform() {
  return !(A0 == 0 && A1 == 1 && A2 == 0);
}

/**
 * Determines wether this view is affected by a transform.
 * @return false when no filtering true otherwise.
 * @see #setInterpolationMethod
 * @see #setSmoothingMethod
 * @see #setMathFunction
 */
bool GLDataView::HasFilter() {
  return (interpMethod!=INTERPOLATE_NONE) || (smoothMethod!=SMOOTH_NONE) || (mathFunction!=MATH_NONE);
}

/** Expert usage.
 * Sets the parent axis.
 * ( Used by JLAxis.addDataView() )
 * @param a Parent axis
 */
void GLDataView::SetAxis(GLAxis *a) {
  parentAxis = a;
}

/** Expert usage.
 * Gets the parent axis.
 * @return Parent axis
 */
GLAxis *GLDataView::GetAxis() {
  return parentAxis;
}

/** Expert usage.
 * Gets the minimum (Y axis).
 * @return Minimum value
 */
double GLDataView::GetMinimum() {
  return min;
}

/** Expert usage.
 * Gets the maximum (Y axis).
 * @return Maximun value
 */
double GLDataView::GetMaximum() {
  return max;
}

/** Expert usage.
 * Gets the minimun on X axis (with TIME_ANNO).
 * @return Minimum time
 */
double GLDataView::GetMinTime() {
  if (HasFilter()) {
    return minXValue;
  } else {
    if (theData != NULL)
      return theData->x;
    else
      return MAX_VALUE;
  }
}

/** Expert usage.
 * Gets the minimum on X axis.
 * @return Minimum x value
 */
double GLDataView::GetMinXValue() {
  if (IsXDataSorted()) {
    return GetMinTime();
  }
  return minXValue;
}

/** Expert usage.
 * Get the positive minimum on X axis.
 * @return Minimum value strictly positive
 */
double GLDataView::GetPositiveMinXValue() {
  double mi = MAX_VALUE;
  DataList *e = theData;
  if(HasFilter()) e = theFilteredData;
  while (e != NULL) {
    if (e->x > 0 && e->x < mi) mi = e->x;
    e = e->next;
  }
  return mi;
}

/** Expert usage.
 * Get the positive minimum on X axis.
 * @return Minimum value strictly positive
 */
double GLDataView::GetPositiveMinTime() {
  DataList *e = theData;
  if(HasFilter()) e = theFilteredData;
  bool found = false;
  while (e != NULL && !found) {
    found = (e->x > 0);
    if (!found) e = e->next;
  }

  if (e != NULL)
    return e->x;
  else
    return MAX_VALUE;
}

/** Expert usage.
 * Get the maximum on X axis
 * @return Maximum x value
 */
double GLDataView::GetMaxXValue() {
  if (IsXDataSorted()) {
    return GetMaxTime();
  }
  return maxXValue;
}

/** Expert usage.
 * Get the maxinmun on X axis (with TIME_ANNO)
 * @return Maximum value
 */
double GLDataView::GetMaxTime() {
  if (HasFilter()) {
    return maxXValue;
  } else {
    if (theDataEnd != NULL)
      return theDataEnd->x;
    else
      return -MAX_VALUE;
  }
}

/**
 * Gets the number of data in this view.
 * @return Data length
 */
int GLDataView::GetDataLength() {
  if(HasFilter())
    return filteredDataLength;
  else
    return dataLength;
}

/** Return a handle on the data.
 * If you modify data, call commitChange() after the update.
 * Expert usage.
 * @return A handle to the data.
 * @see commitChange()
 */
DataList *GLDataView::GetData() {
  if(HasFilter())
    return theFilteredData;
  else
    return theData;
}

/**
 * Commit change when some data has been in modified in the DataList (via getData())
 * @see getData()
 */
void GLDataView::CommitChange() {
  if(HasFilter()) updateFilters();
  else            computeDataBounds();
}

/**
 * Add datum to the dataview. If you call this routine directly the graph will be updated only after a repaint
 * and your data won't be garbaged (if a display duration is specified). You should use JLChart.addData instead.
 * @param x x coordinates (real space)
 * @param y y coordinates (real space)
 * @see JLChart#addData
 * @see JLChart#setDisplayDuration
 */
void GLDataView::Add(double x, double y) {
  Add(x,y,true);
}

/**
 * Add datum to the dataview. If you call this routine directly the graph will be updated only after a repaint
 * and your data won't be garbaged (if a display duration is specified). You should use JLChart.addData instead.
 * @param x x coordinates (real space)
 * @param y y coordinates (real space)
 * @param updateFilter update filter flag.
 * @see JLChart#addData
 * @see JLChart#setDisplayDuration
 */
void GLDataView::Add(double x, double y,bool updateFilter) {

	DataList *newData = (DataList *)malloc(sizeof(DataList));
	assert(newData);
	newData->x = x;
	newData->y = y;
	newData->next = NULL;

	if (theData == NULL) {
		theData = newData;
		theDataEnd = theData;
	} else {
		theDataEnd->next = newData;
		theDataEnd = theDataEnd->next;
	}

	if (y < min) min = y;
	if (y > max) max = y;

	if (x < minXValue) minXValue = x;
	if (x > maxXValue) maxXValue = x;

	dataLength++;
	if(updateFilter) updateFilters();

}

/**
 * Set data of this dataview.
 * @param x x values
 * @param y y values
 * @see add
 */
void GLDataView::SetData(double *x,double *y,int nbData) {

  Reset();
  for(int i=0;i<nbData;i++)
    Add(x[i],y[i],false);
  updateFilters();

}

/**
 * Garbage old data according to time.
 * @param garbageLimit Limit time (in millisec)
 * @return Number of deleted point.
 */
int GLDataView::GarbagePointTime(double garbageLimit) {

  bool found = false;
  int nbr = 0;
  DataList *old;

  // Garbage old data

  if (theData != NULL) {

    while (theData != NULL && !found) {

      // Keep 3 seconds more to avoid complete curve
      found = (theData->x > (maxXValue - garbageLimit - 3000.0));

      if (!found) {
        // Remove first element
        old = theData;
        theData = theData->next;
        free(old);
        dataLength--;
        nbr++;
      }

    }

  }

  if ( theData==NULL ) {
    // All points has been removed
    Reset();
  } else {
    if( HasFilter() ) {
      updateFilters();
    } else {
      computeDataBounds();
    }
  }
  return nbr;

}

/**
 * Garbage old data according to data length.
 * It will remove the (dataLength-garbageLimit) first points.
 * @param garbageLimit Index limit
 */
void GLDataView::GarbagePointLimit(int garbageLimit) {

  DataList *old;

  // Garbage old data
  int nb = dataLength - garbageLimit;
  for (int i = 0; i < nb; i++) {
    old = theData;
    theData = theData->next;
    free(old);
    dataLength--;
  }

  if (HasFilter()) {
    updateFilters();
  } else {
    computeDataBounds();
  }

}

//Compute min
void GLDataView::computeMin() {
  min = MAX_VALUE;
  DataList *e = theData;
  if(HasFilter()) e = theFilteredData;
  while (e != NULL) {
    if (e->y < min) min = e->y;
    e = e->next;
  }
}

//Compute max
void GLDataView::computeMax() {
  max = -MAX_VALUE;
  DataList *e = theData;
  if(HasFilter()) e = theFilteredData;
  while (e != NULL) {
    if (e->y > max) max = e->y;
    e = e->next;
  }
}

//Compute minXValue
void GLDataView::computeMinXValue() {
  minXValue = MAX_VALUE;
  DataList *e = theData;
  if(HasFilter()) e = theFilteredData;
  while (e != NULL) {
    if (e->x < minXValue) minXValue = e->x;
    e = e->next;
  }
}

//Compute maxXValue
void GLDataView::computeMaxXValue() {
  maxXValue = -MAX_VALUE;
  DataList *e = theData;
  if(HasFilter()) e = theFilteredData;
  while (e != NULL) {
    if (e->x > maxXValue) maxXValue = e->x;
    e = e->next;
  }
}

/**
 * Computes and stores min and max on x and y
 */
void GLDataView::computeDataBounds() {

  minXValue = MAX_VALUE;
  maxXValue = -MAX_VALUE;
  min = MAX_VALUE;
  max = -MAX_VALUE;
  DataList *e = theData;
  if(HasFilter()) e = theFilteredData;
  while (e != NULL) {
    if (e->x < minXValue) minXValue = e->x;
    if (e->x > maxXValue) maxXValue = e->x;
    if (e->y < min) min = e->y;
    if (e->y > max) max = e->y;
    e = e->next;
  }

}

/** Expert usage.
 * Compute transformed min and max.
 * @return Transformed min and max
 */
void GLDataView::ComputeTransformedMinMax(double *tmin,double *tmax) {

  double mi = MAX_VALUE;
  double ma = -MAX_VALUE;

  DataList *e = theData;
  if(HasFilter()) e = theFilteredData;

  while (e != NULL) {
    double v = A0 + A1 * e->y + A2 * e->y * e->y;
    if (v < mi) mi = v;
    if (v > ma) ma = v;
    e = e->next;
  }

  if (mi == MAX_VALUE) mi = 0;
  if (ma == -MAX_VALUE) ma = 99;

  *tmin = mi;
  *tmax = ma;

}

/**
 * Compute minimun of positive value.
 * @return Double.MAX_VALUE when no positive value are found
 */
double GLDataView::ComputePositiveMin() {

  double mi = MAX_VALUE;
  DataList *e = theData;
  if(HasFilter()) e = theFilteredData;
  while (e != NULL) {
    if (e->y > 0 && e->y < mi) mi = e->y;
    e = e->next;
  }
  return mi;

}

/**
 *  Compute transformed value of x.
 * @param x Value to transform
 * @return transformed value (through A0,A1,A2 transformation)
 */
double GLDataView::GetTransformedValue(double x) {
  return A0 + A1 * x + A2 * x * x;
}

/**
 * Get last value.
 * @return Last value
 */
DataList *GLDataView::GetLastValue() {
  if(HasFilter())
    return theDataEnd;
  else
    return theFilteredDataEnd;
}

/**
 * Clear all data in this view.
 */
void GLDataView::Reset() {
  FreeList(theData);
  theData = NULL;
  theDataEnd = NULL;
  dataLength = 0;
  updateFilters();
  computeDataBounds();
}

/**
 * Returns Y value according to index.
 * @param idx Value index
 * @return Y value
 */
double GLDataView::GetYValueByIndex(int idx)
{
   if (idx<0 || idx>=GetDataLength() )
   {
     return NaN;
   }

   int i=0;
   DataList *e = theData;

   while (e != NULL && i<idx)
   {
     e=e->next;
     i++;
   }

   if( e!=NULL )
   {
     return e->y;
   }
   else
   {
     return NaN;
   }

}

/**
 * Returns X value according to index.
 * @param idx Value index
 * @return X value
 */
double GLDataView::GetXValueByIndex(int idx) {

  if (idx<0 || idx>=GetDataLength() )
  {
    return NaN;
  }

  int i=0;
  DataList *e = theData;

  while (e != NULL && i<idx)
  {
    e=e->next;
    i++;
  }

  if( e!=NULL )
  {
    return e->x;
  }
  else
  {
    return NaN;
  }

}

/**
 * Sets the format property for this dataview (C format).
 * By default, it uses the Axis format.
 * @param format Format (C style)
 */
void GLDataView::SetUserFormat(const char *format) {
  // TODO
	strcpy(userFormat, format);
}

/**
 * Returns the current user format (null when none).
 */
char *GLDataView::GetUserFormat() {
  return userFormat;
}

/**
 * Format the given value according the userFormat or
 * to the Axis format.
 * @param v Value to be formated
 */
std::string GLDataView::FormatValue(double v) {

  static char ret[64];

  if(isnan(v)) {
    return "NaN";
  }

  if(userFormat != NULL) {

    sprintf(ret,userFormat,v);
    return ret;

  } else if (parentAxis==NULL) {

    sprintf(ret,"%f",v);
    return ret;

  } else {

    return parentAxis->FormatValue(v,0);

  }

}

/**
 * Returns whether data is supposed to be sorted on x or not
 * @return a boolean value
 */
bool GLDataView::IsXDataSorted () {
  // return xDataSorted;
  return true; // TODO
}

/**
 * Set whether data is supposed to be sorted on x or not.
 * <code>false</code> by default
 * @param dataSorted a boolean value
 */
void GLDataView::SetXDataSorted (bool dataSorted) {
  //if (xDataSorted && !dataSorted) computeDataBounds();
  //xDataSorted = dataSorted;
  //TODO
}

// Interpolation/Filtering stuff

void GLDataView::addPts(Point2D *p,double x,double y) {

  if( (p->length % 50)==0 ) {
    APoint2D *newPts = (APoint2D *)malloc((p->length+50) * sizeof(APoint2D));
    if(p->length) memcpy(newPts,p->pts,p->length*sizeof(APoint2D));
    free(p->pts);
    p->pts = newPts;
  }
  p->pts[ p->length ].x = x;
  p->pts[ p->length ].y = y;
  p->length++;

}

void GLDataView::addPts(Point2D *p,APoint2D pt) {
  addPts(p,pt.x,pt.y);
}

/**
 * Returns an array of points.
 * @param nbExtra Number of extrapolated point
 * @param interpNan Interpolate NaN values when true, remove them otherwise
 */
Point2D GLDataView::getSource(DataList *src,int nbExtra,bool interpNan) {

  DataList *f = src;

  // Interpolate NaN (When possible)
  Point2D p;
  p.length = 0;
  p.pts = NULL;
  for(int i=0;i<nbExtra;i++) addPts(&p,0.0,0.0);
  APoint2D lastValue;
  APoint2D nextValue;
  lastValue.x = NaN;
  lastValue.y = NaN;
  nextValue.x = NaN;
  nextValue.y = NaN;

  while (f != NULL) {
    if (!isnan(f->y)) {
      addPts(&p,f->x, f->y);
    } else {
      if (interpNan) {
        if (!isnan(lastValue.y)) {
          if (f->next != NULL && !isnan(f->next->y)) {
            // Linear interpolation possible
            nextValue.x = f->next->x;
            nextValue.y = f->next->y;
            // Interpolate also x value, work around client side timestamps error.
            addPts(&p,LinearInterpolate(lastValue, nextValue, 0.5));
          } else {
            // Duplicate last value
            addPts(&p,f->x, lastValue.y);
          }
        }
      }
    }
    lastValue.x = f->x;
    lastValue.y = f->y;
    f = f->next;
  }

  for(int i=0;i<nbExtra;i++) addPts(&p,0.0,0.0);
  return p;

}

/* Mathematical operation (integral,derivative,fft) */
void GLDataView::mathop() {

  if(mathFunction==MATH_NONE)
    return;

  // Get source data
  Point2D source;
  if(interpMethod==INTERPOLATE_NONE && smoothMethod==SMOOTH_NONE)
    source = getSource(theData,0,false);
  else
    source = getSource(theFilteredData,0,false);
  // Reset filteredData
  FreeList(theFilteredData);
  theFilteredData = NULL;
  theFilteredDataEnd = NULL;
  filteredDataLength = 0;
  if( source.length==0 ) return;

  double sum = 0.0;
  switch(mathFunction) {
    case MATH_DERIVATIVE:
      for(int i=0;i<source.length-1;i++) {
        double d = (source.pts[i+1].y - source.pts[i].y) / (source.pts[i+1].x - source.pts[i].x);
        addInt(source.pts[i].x, d);
      }
      break;
    case MATH_INTEGRAL:
      addInt(source.pts[0].x, sum);
      for(int i=0;i<source.length-1;i++) {
        sum += ((source.pts[i+1].y + source.pts[i].y)/2.0) * (source.pts[i+1].x - source.pts[i].x);
        addInt(source.pts[i+1].x, sum);
      }
      break;
    case MATH_FFT_MODULUS:
      doFFT(source,0);
      break;
    case MATH_FFT_PHASE:
      doFFT(source,1);
      break;
  }

  SAFE_FREE(source.pts);

}

/* Reverse bit of idx on p bits */
int  GLDataView::reverse(int idx,int p) {
  int i, rev;
  for (i = rev = 0; i < p; i++) {
     rev = (rev << 1) | (idx & 1);
     idx >>= 1;
  }
  return rev;
}

/**
 * Performs FFT of the input signal (Requires constant x intervals)
 * @param in Input signal
 * @param mode 0=>modulus 1=>argument
 */
void GLDataView::doFFT(Point2D in,int mode) {

    int nbSample = in.length;
    if(nbSample<2) return;
    int p = 0;
    int i,idx;
    // Get the power of 2 above
    if((nbSample & (nbSample-1))==0) p = -1; // already a power of 2
    else                             p = 0;
    while(nbSample!=0) { nbSample = nbSample >> 1; p++; }
    nbSample = 1 << p;

    // Create initial array
    double* real = (double *)malloc(nbSample*sizeof(double));
    double* imag = (double *)malloc(nbSample*sizeof(double));
    for(i=0;i<in.length;i++) {
      idx = reverse(i,p);
      real[idx] = in.pts[i].y;
      imag[idx] = 0.0;
    }
    for(;i<nbSample;i++) {
      idx = reverse(i,p);
      real[idx] = 0.0;
      imag[idx] = 0.0;
    }

    // Do the FFT
    int blockEnd = 1;
    for (int blockSize = 2; blockSize <= nbSample; blockSize <<= 1) {

       double deltaAngle = 2.0 * PI / (double) blockSize;

       double sm2 = sin(-2.0 * deltaAngle);
       double sm1 = sin(-deltaAngle);
       double cm2 = cos(-2.0 * deltaAngle);
       double cm1 = cos(-deltaAngle);
       double w = 2.0 * cm1;
       double ar0, ar1, ar2, ai0, ai1, ai2;

       for (i = 0; i < nbSample; i += blockSize) {
          ar2 = cm2;
          ar1 = cm1;

          ai2 = sm2;
          ai1 = sm1;

          for (int j = i, n = 0; n < blockEnd; j++, n++) {
             ar0 = w * ar1 - ar2;
             ar2 = ar1;
             ar1 = ar0;

             ai0 = w * ai1 - ai2;
             ai2 = ai1;
             ai1 = ai0;

             int k = j + blockEnd;
             double tr = ar0 * real[k] - ai0 * imag[k];
             double ti = ar0 * imag[k] + ai0 * real[k];

             real[k] = real[j] - tr;
             imag[k] = imag[j] - ti;

             real[j] += tr;
             imag[j] += ti;
          }
       }

       blockEnd = blockSize;
    }

    // Create modulus of arguments results
    switch(mode) {
      case 0: // Modulus
        for(i=0;i<nbSample/2;i++) {
          double n = sqrt( real[i]*real[i] + imag[i]*imag[i] );
          addInt((double)i,n);
        }
        break;
      case 1: // Arguments (in radians)
        for(i=0;i<nbSample;i++) {
          double n = sqrt( real[i]*real[i] + imag[i]*imag[i] );
          double arg = asin( imag[i]/n );
          addInt((double)i,arg);
        }
        break;
    }

    free(real);
    free(imag);

}

/**
 * Update filter calulation. Call this function if you're using
 * the add(x,y,false) mehtod.
 */
void GLDataView::updateFilters() {
  FreeList(theFilteredData);
  theFilteredData = NULL;
  theFilteredDataEnd = NULL;
  filteredDataLength = 0;
  if(HasFilter()) {
    interpolate();
    convolution();
    mathop();
    computeDataBounds();
  }
}

/** Smoothing filter shapes */
void GLDataView::updateSmoothCoefs() {

    SAFE_FREE(smoothCoefs);
    if(smoothMethod==SMOOTH_NONE) {
      return;
    }

    smoothCoefs = (double *)malloc(smoothNeighbor*sizeof(double));
    int    nb = smoothNeighbor/2;
    double l;

    switch(smoothMethod) {

      case SMOOTH_FLAT:
        // Flat shape
        for(int i=0;i<smoothNeighbor;i++) smoothCoefs[i]=1.0;
        break;

      case SMOOTH_TRIANGULAR:
        // Triangular shape
        l  = 1.0 / ((double)nb + 1.0);
        for(int i=0;i<nb;i++)
          smoothCoefs[i]=  (double)(i-nb)*l + 1.0;
        for(int i=1;i<=nb;i++)
          smoothCoefs[i+nb]=  (double)(-i)*l + 1.0;
        smoothCoefs[nb]=1.0;
        break;

      case SMOOTH_GAUSSIAN:
        // Gaussian shape
        double A = 1.0 / (2.0 * sqrt(PI) * smoothSigma);
        double B = 1.0 / (2.0 * smoothSigma * smoothSigma);
        for(int i=0;i<smoothNeighbor;i++) {
          double x = (double)(i-nb);
          smoothCoefs[i]= A*exp(-x*x*B);
        }
        break;

    }

    // Normalize coef
    double sum = 0.0;
    for(int i=0;i<smoothNeighbor;i++) sum+=smoothCoefs[i];
    for(int i=0;i<smoothNeighbor;i++) smoothCoefs[i]=smoothCoefs[i]/sum;

}

/** Convolution product */
void GLDataView::convolution() {

  if(smoothMethod==SMOOTH_NONE)
    return;

  int nbG = smoothNeighbor/2;
  int nbF;   // number of smoothed points
  int nbE;   // number of exptrapoled points
  int start; // start index (input signal)
  int end;   // end index (input signal)
  int i;
  double sum;

  // Number of added extrapolated points
  switch(smoothExtrapolation) {
    case SMOOTH_EXT_NONE:
      nbE   = 0;
      break;
    default:
      nbE = nbG;
      break;
  }

  // Get source data
  Point2D source;
  if(interpMethod==INTERPOLATE_NONE)
    source = getSource(theData,nbE,true);
  else
    source = getSource(theFilteredData,nbE,true);

  nbF   = source.length-2*nbG; // number of smoothed points
  start = nbG;                 // start index
  end   = nbF + nbG - 1;       // end index

  // Too much neighbor or too much NaN
  if(nbF==nbE || start>end)
    return;

  // Extrapolation on boundaries
  // x values are there for testing purpose only.
  // They do not impact on calculation.
  switch(smoothExtrapolation) {

    // Linearly extrpolate points based on the
    // average direction of the 3 first (or last) vectors.
    // TODO: Make this configurable
    case SMOOTH_EXT_LINEAR: {

      int maxPts = 3;
      APoint2D vect;

      // Fisrt points
      int nb = 0;
      vect.x =  0.0; // Sum of vectors
      vect.y =  0.0; // Sum of vectors
      for(i=start;i<maxPts+start && i<nbF+start-1;i++) {
        vect.x += (source.pts[i].x - source.pts[i+1].x);
        vect.y += (source.pts[i].y - source.pts[i+1].y);
        nb++;
      }
      if(nb==0) {
        // Revert to flat
        vect.x= source.pts[start].x - (double)nbE;
        vect.y= source.pts[start].y;
      } else {
        vect.x= (vect.x / (double)nb) * (double)nbE + source.pts[start].x;
        vect.y= (vect.y / (double)nb) * (double)nbE + source.pts[start].y;
      }
      for(i=0;i<nbE;i++)
        source.pts[i] = LinearInterpolate(vect,source.pts[start],(double)i/(double)nbE);

      // Last points
      nb = 0;
      vect.x =  0.0; // Sum of vectors
      vect.y =  0.0; // Sum of vectors
      for(i=end-maxPts;i<end;i++) {
        if(i>=start) {
          vect.x += (source.pts[i+1].x - source.pts[i].x);
          vect.y += (source.pts[i+1].y - source.pts[i].y);
          nb++;
        }
      }
      if(nb==0) {
        // Revert to flat
        vect.x=  source.pts[end].x + (double)nbE;
        vect.y=  source.pts[end].y;
      } else {
        vect.x= (vect.x / (double)nb) * (double)nbE + source.pts[end].x;
        vect.y= (vect.y / (double)nb) * (double)nbE + source.pts[end].y;
      }
      for(i=1;i<=nbE;i++)
        source.pts[end+i] = LinearInterpolate(source.pts[end],vect,(double)i/(double)nbE);

    } break;

    // Duplicate start and end values
    case SMOOTH_EXT_FLAT:
      for(i=0;i<nbE;i++) {
        source.pts[i].x = source.pts[start].x-nbE+i;
        source.pts[i].y = source.pts[start].y;
        source.pts[i+end+1].x = source.pts[end].x+i+1.0;
        source.pts[i+end+1].y = source.pts[end].y;
      }
      break;
  }

  // Reset filteredData
  FreeList(theFilteredData);
  theFilteredData = NULL;
  theFilteredDataEnd = NULL;
  filteredDataLength = 0;

  for (i = start; i <= end; i++) {
     // Convolution product
     sum = 0.0;
     for (int j = 0; j < smoothNeighbor; j++)
       sum += smoothCoefs[j] * source.pts[i + j - nbG].y;
     addInt(source.pts[i].x, sum);
  }

  SAFE_FREE(source.pts);

}

void GLDataView::addInt(APoint2D p) {
  addInt(p.x, p.y);
}

void GLDataView::addInt(double x,double y) {

  DataList *newData = (DataList *)malloc(sizeof(DataList));
  newData->x = x;
  newData->y = y;
  newData->next = NULL;

  if (theFilteredData == NULL) {
    theFilteredData = newData;
  } else {
    theFilteredDataEnd->next = newData;
  }
  theFilteredDataEnd = newData;

  filteredDataLength++;

}

Point2D GLDataView::getSegments(DataList *l) {

  int nb = 0;
  DataList *head = l;
  while(l!=NULL && !isnan(l->y)) {nb++;l=l->next;}
  l=head;
  Point2D ret;
  ret.length = 0;
  ret.pts = NULL;
  for(int i=0;i<nb;i++) {
    addPts(&ret,l->x,l->y);
    l=l->next;
  }
  return ret;

}

/**
 * Linear interpolation
 * @param p1 Start point (t=0)
 * @param p2 End point (t=1)
 * @param t 0=>p1 to 1=>p2
 */
APoint2D GLDataView::LinearInterpolate(APoint2D p1,APoint2D p2,double t)
{
  APoint2D p;
  p.x = p1.x + (p2.x - p1.x)*t;
  p.y = p1.y + (p2.y - p1.y)*t;
  return p;
}

/**
 * Cosine interpolation
 * @param p1 Start point (t=0)
 * @param p2 End point (t=1)
 * @param t 0=>p1 to 1=>p2
 */
APoint2D GLDataView::CosineInterpolate(APoint2D p1,APoint2D p2,double t)
{
  APoint2D p;
  double t2;
  t2 = (1.0-cos(t*PI))/2.0;
  p.x = p1.x + (p2.x - p1.x)*t;
  p.y =  p1.y*(1.0-t2)+p2.y*t2;
  return p;
}

/**
 * Cubic interpolation (1D cubic interpolation, requires constant x intervals)
 * @param p0 neighbour point
 * @param p1 Start point (t=0)
 * @param p2 End point (t=1)
 * @param p3 neighbour point
 * @param t 0=>p1 to 1=>p2
 */
APoint2D GLDataView::CubicInterpolate(APoint2D p0,APoint2D p1,APoint2D p2,APoint2D p3,double t) {
  APoint2D p;
  double t2 = t*t;
  double t3 = t2*t;

  double ay3 = p3.y - p2.y - p0.y + p1.y;
  double ay2 = p0.y - p1.y - ay3;
  double ay1 = p2.y - p0.y;
  double ay0 = p1.y;

  p.x = p1.x + (p2.x - p1.x)*t;
  p.y = ay3*t3+ay2*t2+ay1*t+ay0;
  return p;
}

/**
 * Hermite interpolation.
 * @param p0 neighbour point
 * @param p1 Start point (t=0)
 * @param p2 End point (t=1)
 * @param p3 neighbour point
 * @param mu 0=>p1 to 1=>p2
 * @param tension (1=>high, 0=>normal, -1=>low)
 * @param bias (0 for no bias, positive value towards first segment, negative value towards the others)
 */
APoint2D GLDataView::HermiteInterpolate(APoint2D p0, APoint2D p1, APoint2D p2, APoint2D p3,
                                        double mu, double tension, double bias) {
  double ym0, ym1, xm0, xm1, mu2, mu3;
  double a0, a1, a2, a3;
  double t = (1.0 - tension) / 2.0;
  APoint2D p;
  mu2 = mu * mu;
  mu3 = mu2 * mu;

  a0 = 2.0 * mu3 - 3.0 * mu2 + 1.0;
  a1 = mu3 - 2.0 * mu2 + mu;
  a2 = mu3 - mu2;
  a3 = -2.0 * mu3 + 3.0 * mu2;

  xm0 = (p1.x - p0.x) * (1.0 + bias)  * t;
  xm0 += (p2.x - p1.x) * (1.0 - bias) * t;
  xm1 = (p2.x - p1.x) * (1.0 + bias)  * t;
  xm1 += (p3.x - p2.x) * (1.0 - bias) * t;

  ym0 = (p1.y - p0.y) * (1.0 + bias)  * t;
  ym0 += (p2.y - p1.y) * (1.0 - bias) * t;
  ym1 = (p2.y - p1.y) * (1.0 + bias)  * t;
  ym1 += (p3.y - p2.y) * (1.0 - bias) * t;

  p.x = a0 * p1.x + a1 * xm0 + a2 * xm1 + a3 * p2.x;
  p.y = a0 * p1.y + a1 * ym0 + a2 * ym1 + a3 * p2.y;
  return p;

}

/**
 * Interpolate the Dataview
 */
void GLDataView::interpolate() {

  if(interpMethod==INTERPOLATE_NONE)
    return;

  double dt = 1.0/(double)interpStep;
  double t;
  int i;
  APoint2D fP;
  APoint2D lP;

  DataList *l = theData;

  while( l!=NULL ) {

    // Get continuous segments
    Point2D pts = getSegments(l);
    int nbPts = pts.length;
    int method = interpMethod;
    if(nbPts==0) {
      // NaN found
      method = INTERPOLATE_NONE;        // No interpolation possible
      addInt(l->x, l->y);
      l = l->next;
    } else if(nbPts==1) {
      method = INTERPOLATE_NONE;        // No interpolation possible
      addInt(pts.pts[0]);
    } else if(nbPts==2) {
      method = INTERPOLATE_LINEAR; // Fallback to linear when less than 3 points
    }

    switch( method ) {

      
      case INTERPOLATE_LINEAR:

        for(i=0;i<nbPts-2;i++) {
          for(int j=0;j<interpStep;j++) {
            t = (double)j * dt;
            addInt(LinearInterpolate(pts.pts[i],pts.pts[i+1],t));
          }
        }
        for(int j=0;j<=interpStep;j++) {
          t = (double)j * dt;
          addInt(LinearInterpolate(pts.pts[i],pts.pts[i+1],t));
        }
        break;

      
      case INTERPOLATE_COSINE:

        for(i=0;i<nbPts-2;i++) {
          for(int j=0;j<interpStep;j++) {
            t = (double)j * dt;
            addInt(CosineInterpolate(pts.pts[i],pts.pts[i+1],t));
          }
        }
        for(int j=0;j<=interpStep;j++) {
          t = (double)j * dt;
          addInt(LinearInterpolate(pts.pts[i],pts.pts[i+1],t));
        }
        break;

      
      case INTERPOLATE_CUBIC:

        // First segment (extend tangent)
        fP.x = 2.0 * pts.pts[0].x - pts.pts[1].x;
        fP.y = 2.0 * pts.pts[0].y - pts.pts[1].y;
        for (int j = 0; j < interpStep; j++) {
          t = (double) j * dt;
          addInt(CubicInterpolate(fP, pts.pts[0], pts.pts[1], pts.pts[2], t));
        }

        // Middle segments
        for (i = 1; i < nbPts - 2; i++) {
          for (int j = 0; j < interpStep; j++) {
            t = (double) j * dt;
            addInt(CubicInterpolate(pts.pts[i - 1], pts.pts[i], pts.pts[i + 1], pts.pts[i + 2], t));
          }
        }

        // Last segment (extend tangent)
        lP.x = 2.0 * pts.pts[i + 1].x - pts.pts[i].x;
        lP.y = 2.0 * pts.pts[i + 1].y - pts.pts[i].y;
        for (int j = 0; j <= interpStep; j++) {
          t = (double) j * dt;
          addInt(CubicInterpolate(pts.pts[i - 1], pts.pts[i], pts.pts[i + 1], lP, t));
        }
        break;

      
      case INTERPOLATE_HERMITE:

        // First segment (extend tangent)
        fP.x = 2.0 * pts.pts[0].x - pts.pts[1].x;
        fP.y = 2.0 * pts.pts[0].y - pts.pts[1].y;
        for (int j = 0; j < interpStep; j++) {
          t = (double) j * dt;
          addInt(HermiteInterpolate(fP, pts.pts[0], pts.pts[1], pts.pts[2], t, interpTension, interpBias));
        }

        // Middle segments
        for (i = 1; i < nbPts - 2; i++) {
          for (int j = 0; j < interpStep; j++) {
            t = (double) j * dt;
            addInt(HermiteInterpolate(pts.pts[i - 1], pts.pts[i], pts.pts[i + 1], pts.pts[i + 2], t, interpTension, interpBias));
          }
        }

        // Last segment (extend tangent)
        lP.x = 2.0 * pts.pts[i + 1].x - pts.pts[i].x;
        lP.y = 2.0 * pts.pts[i + 1].y - pts.pts[i].y;
        for (int j = 0; j <= interpStep; j++) {
          t = (double) j * dt;
          addInt(HermiteInterpolate(pts.pts[i - 1], pts.pts[i], pts.pts[i + 1], lP, t, interpTension, interpBias));
        }
        break;

    }

    SAFE_FREE(pts.pts);
    // Fetch
    for(i=0;i<nbPts;i++) l=l->next;

  }

}
