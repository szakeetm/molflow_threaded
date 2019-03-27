// Copyright (c) 2011 rubicon IT GmbH
#ifndef _GLCHARTCONSTH_
#define _GLCHARTCONSTH_

#define MAX_VIEWS 50

#define MAX_VALUE 1e100
extern double NaN;

/** Place label at the bottom of the chart */
#define LABEL_DOWN 0
/** Place label at the top of the chart */
#define LABEL_UP 1
/** Place label at the right of the chart */
#define LABEL_RIGHT 2
/** Place label at the left of the chart */
#define LABEL_LEFT 3
/** Place label at the bottom of the chart and try to arrange them in rows */
#define LABEL_ROW 4

/* Chart properties menu item */
#define MENU_CHARTPROP 1
#define MENU_SAVETXT   2
//#define MENU_LOADTXT   3
#define MENU_COPYALL   4

/* Data view properties menu */
#define MENU_DVPROPY1  100
#define MENU_DVPROPY2  200

/** Value tooltip TOPLEFT placement */
#define TOPLEFT     0
/** Value tooltip TOPRIGHT placement */
#define TOPRIGHT    1
/** Value tooltip BOTTOMLEFT placement */
#define BOTTOMLEFT  2
/** Value tooltip BOTTOMRIGHT placement */
#define BOTTOMRIGHT 3

/** Horizontal axis at bottom of the chart */
#define HORIZONTAL_DOWN 1
/** Horizontal axis at top of the chart */
#define HORIZONTAL_UP 2
/** Horizontal axis at 0 position (on Y1) */
#define HORIZONTAL_ORG1 3
/** Horizontal axis at 0 position (on Y2) */
#define HORIZONTAL_ORG2 4
/** Vertical right axis */
#define VERTICAL_RIGHT 5
/** Vertical left axis */
#define VERTICAL_LEFT 6
/** Vertical axis at X=0 */
#define VERTICAL_ORG 7

/** Draw time annotation for x axis. */
#define TIME_ANNO 1
/** Draw formated annotation  */
#define VALUE_ANNO 2

/** Use linear scale for this axis  */
#define LINEAR_SCALE 0
/** Use logarithmic scale for this axis  */
#define LOG_SCALE 1

/** Use default compiler format to display double */
#define AUTO_FORMAT 0
/** Display value using exponential representation (x.xxEyy) */
#define SCIENTIFIC_FORMAT 1
/** Display number of second as HH:MM:SS */
#define TIME_FORMAT 2
/** Display integer using decimal format */
#define DECINT_FORMAT 3
/** Display integer using haxadecimal format */
#define HEXINT_FORMAT 4
/** Display integer using binary format */
#define BININT_FORMAT 5
/** Display value using exponential representation (xEyy) */
#define SCIENTIFICINT_FORMAT 6
/** Display value as date */
#define DATE_FORMAT 7

/** US date format to format labels as dates */
static const char *US_DATE_FORMAT = "%Y/%m/%d %H:%M:%S";
/** French date format to format labels as dates */
static const char *FR_DATE_FORMAT = "%d/%m/%Y %H:%M:%S";

/** Time constant (axis time labeling) */

#define YEAR  31536000.0
#define MONTH 2592000.0
#define DAY   86400.0
#define HOUR  3600.0
#define MINU  60.0
#define SECO  1.0

/** No marker displayed */
#define MARKER_NONE 0
/** Display a dot for each point of the view */
#define MARKER_DOT 1
/** Display a box for each point of the view */
#define MARKER_BOX 2
/** Display a triangle for each point of the view */
#define MARKER_TRIANGLE 3
/** Display a diamond for each point of the view */
#define MARKER_DIAMOND 4
/** Display a start for each point of the view */
#define MARKER_STAR 5
/** Display a vertical line for each point of the view */
#define MARKER_VERT_LINE 6
/** Display an horizontal line for each point of the view */
#define MARKER_HORIZ_LINE 7
/** Display a cross for each point of the view */
#define MARKER_CROSS 8
/** Display a circle for each point of the view */
#define MARKER_CIRCLE 9
/** Display a square for each point of the view */
#define MARKER_SQUARE 10

/** Solid line style */
#define STYLE_SOLID 0
/** Dot line style */
#define STYLE_DOT 1
/** Dash line style */
#define STYLE_DASH 2
/** Long Dash line style */
#define STYLE_LONG_DASH 3
/** Dash + Dot line style */
#define STYLE_DASH_DOT 4

/** Line style */
#define TYPE_LINE 0

/** BarGraph style */
#define TYPE_BAR  1

/** Fill curve and bar from the top of the graph */
#define METHOD_FILL_FROM_TOP 0

/** Fill curve and bar from zero position (on Yaxis) */
#define METHOD_FILL_FROM_ZERO 1

/** Fill curve and bar from the bottom of the graph */
#define METHOD_FILL_FROM_BOTTOM 2

/** No filling */
#define FILL_STYLE_NONE 0
/** Solid fill style */
#define FILL_STYLE_SOLID 1
/** Hatch fill style */
#define FILL_STYLE_LARGE_RIGHT_HATCH 2
/** Hatch fill style */
#define FILL_STYLE_LARGE_LEFT_HATCH 3
/** Hatch fill style */
#define FILL_STYLE_LARGE_CROSS_HATCH 4
/** Hatch fill style */
#define FILL_STYLE_SMALL_RIGHT_HATCH 5
/** Hatch fill style */
#define FILL_STYLE_SMALL_LEFT_HATCH 6
/** Hatch fill style */
#define FILL_STYLE_SMALL_CROSS_HATCH 7
/** Hatch fill style */
#define FILL_STYLE_DOT_PATTERN_1 8
/** Hatch fill style */
#define FILL_STYLE_DOT_PATTERN_2 9
/** Hatch fill style */
#define FILL_STYLE_DOT_PATTERN_3 10

/** No interpolation */
#define INTERPOLATE_NONE 0
/** Linear interpolation method */
#define INTERPOLATE_LINEAR 1
/** Cosine interpolation method */
#define INTERPOLATE_COSINE 2
/* Cubic interpolation method (Require constant x interval) */
#define INTERPOLATE_CUBIC 3
/* Hermite interpolation method */
#define INTERPOLATE_HERMITE 4

/** No smoothing */
#define SMOOTH_NONE 0
/** Flat smoothing (Flat shape) */
#define SMOOTH_FLAT 1
/** Linear smoothing (Triangular shape) */
#define SMOOTH_TRIANGULAR 2
/** Gaussian smoothing (Gaussian shape) */
#define SMOOTH_GAUSSIAN 3

/** No smoothing extrapolation */
#define SMOOTH_EXT_NONE 0
/** flat smoothing extrapolation (duplicate last and end value) */
#define SMOOTH_EXT_FLAT 1
/** Linear smoothing extrapolation (linear extrapolation) */
#define SMOOTH_EXT_LINEAR 2

/** No mathematical operation */
#define MATH_NONE 0
/** Derivative operation */
#define MATH_DERIVATIVE 1
/** Integral operation */
#define MATH_INTEGRAL 2
/** FFT (modulus) operation */
#define MATH_FFT_MODULUS 3
/** FFT (phase) operation */
#define MATH_FFT_PHASE 4

/** Axis settings dialog */
#define Y1_TYPE 1
#define Y2_TYPE 2
#define X_TYPE  3

/* structure defs */

typedef struct {

  int x;
  int y;
  int width;
  int height;

} GLCRectangle;

typedef struct {

  int width;
  int height;

} GLCDimension;

typedef struct {

  int x;
  int y;

} GLCPoint;

class GLColor {
public:
	GLColor() {}
	GLColor(int red, int green, int blue) {
		r = red;
		g = green;
		b = blue;
	}
	bool operator==(const GLColor& col) {
		return r == col.r && g == col.g && b == col.b;
	}
  int r;
  int g;
  int b;

};

#endif /* _GLCHARTCONSTH_ */