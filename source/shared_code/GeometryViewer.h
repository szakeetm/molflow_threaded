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
#ifndef _GEOMETRYVIEWERH_
#define _GEOMETRYVIEWERH_

#include "GLApp/GLComponent.h"
#include "GLApp/GLTypes.h"
#include "Vector.h"
#include <vector>

class Worker;
class GLButton;
class GLLabel;
class GLCombo;
class GLOverlayLabel;

#define DRAGG_NONE   0
#define DRAGG_SELECT 1
#define DRAGG_ROTATE 2
#define DRAGG_ZOOM   3
#define DRAGG_MOVE   4
#define DRAGG_SELECTVERTEX 5
#ifdef  SYNRAD
#define DRAGG_SELECTTRAJ 6
#endif

#define MODE_SELECT  0
#define MODE_ZOOM    1
#define MODE_MOVE    2
#define MODE_SELECTVERTEX 3
#ifdef  SYNRAD
#define MODE_SELECTTRAJ 4
#endif

#define SHOW_FRONTANDBACK 0
#define SHOW_FRONT        1
#define SHOW_BACK         2

#define PERSPECTIVE_PROJ  0
#define ORTHOGRAPHIC_PROJ 1

#define XYZ_NONE          0
#define XYZ_TOP           1
#define XYZ_SIDE          2
#define XYZ_FRONT         3

#define MSG_GEOMVIEWER_MAXIMISE MSG_USER + 1
#define MSG_GEOMVIEWER_SELECT   MSG_USER + 2

#define FOV_ANGLE 45.0

// Definition of a view. Note: all basis are left handed

typedef struct {

  char    *name;       // View name

  int      projMode;   // Projection type
  double   camAngleOx; // Spheric coordinates. Right-hand rotation (in left-hand coord.sys)
  double   camAngleOy; // Spheric coordinates Left-hand rotation (in left-hand c.sys)

  double   camAngleOz;

  double   camDist;    // Camera distance (or zoom in orthographic)

  double   lightAngleOx; //Light direction
  double   lightAngleOy; //Light direction

  Vector3d camOffset;  // Camera target offset
  int      performXY;  // Draw x,y,z coordinates when aligned with axis and orthographic

  double   vLeft;      // Viewport in 2D proj space (used for orthographic autoscaling)
  double   vRight;     // Viewport in 2D proj space (used for orthographic autoscaling)
  double   vTop;       // Viewport in 2D proj space (used for orthographic autoscaling)
  double   vBottom;    // Viewport in 2D proj space (used for orthographic autoscaling)

} AVIEW;

typedef struct {

  std::string    name;       // Selection name
  std::vector<size_t> selection; // List of facets

} SelectionGroup;

typedef struct {
	int requested; //0=no request, 1=waiting for area selection, 2=take screenshot on next viewer paint
	std::string fileName;
	int x, y, w, h; //Screenshotarea
} ScreenshotStatus;

class GeometryViewer : public GLComponent {

public:

  // Construction
  GeometryViewer(int id);

  void UpdateLabelColors();

  // Component method
  void ToOrigo();
  void SetWorker(Worker *s);
  void SetProjection(int mode);
  void ToTopView();
  void ToSideView();
  void ToFrontView();
  bool SelectionChanged();
  bool IsDragging();
  AVIEW GetCurrentView();
  void  SetCurrentView(AVIEW v);
  bool IsSelected();
  void SetSelected(bool s);

  // Implementation
  void Paint();
  void ManageEvent(SDL_Event *evt);
  void SetBounds(int x,int y,int width,int height);
  void ProcessMessage(GLComponent *src,int message);
  void SetFocus(bool focus);

  void SelectCoplanar(double tolerance); //launcher function to get viewport parameters
  void UpdateMatrix();
  void RequestScreenshot(std::string fileName, int x,int y,int w,int h);

  // Flag view
  bool showIndex;
  bool showVertex;
  bool showNormal;
  bool showRule;
  bool showUV;
  bool showLeak;
  bool showHit;
  bool showLine;
  bool showVolume;
  bool showTexture;
  int  showBack;
  bool showFilter;
 // bool showColormap;
  bool showTP;
  bool showHidden;
  bool showHiddenVertex;
  bool showMesh;
  bool bigDots;
  bool showDir;
  bool autoScaleOn;
  int  hideLot;

  #ifdef  MOLFLOW
  bool showTime;
  #endif

  #ifdef  SYNRAD
  bool shadeLines;
  size_t dispNumTraj;  // displayed number of trajectory points
  #endif
  
  size_t dispNumHits; // displayed number of lines and hits
  size_t dispNumLeaks; // displayed number of leaks
  double transStep;  // translation step
  double angleStep;  // angle step
  
  GLLabel       *facetSearchState;

private:

  double ToDeg(double radians);
  void DrawIndex();
  void DrawRule();
  void DrawNormal();
  void DrawUV();
  void DrawLeak();
  void DrawLinesAndHits();
  void Zoom();
  void UpdateMouseCursor(int mode);
  void TranslateScale(double diff);
  void PaintCompAndBorder();
  void PaintSelectedVertices(bool hiddenVertex);
  void AutoScale(bool reUpdateMouseCursor=true);
  void ComputeBB(/*bool getAll*/);
  void UpdateLight();
  void Screenshot();

  //void DrawBB();
  //void DrawBB(AABBNODE *node);

  Worker *work;

  // Toolbar
  GLLabel       *toolBack;
  GLButton      *frontBtn;
  GLButton      *topBtn;
  GLButton      *sideBtn;
  GLCombo       *projCombo;
  GLButton      *zoomBtn;
  GLButton      *autoBtn;
  GLButton      *selBtn;
  GLButton      *selVxBtn;
  GLButton      *sysBtn;
  GLButton      *handBtn;
  GLLabel       *coordLab;

  
  GLLabel       *capsLockLabel;
  GLLabel       *hideLotlabel;
  GLLabel		*screenshotLabel;
  GLLabel		*selectLabel;
  GLLabel		*rotateLabel;
  GLLabel		*panLabel;
  GLLabel		*tabLabel;
  GLLabel		*nonPlanarLabel;

  #ifdef MOLFLOW
  GLOverlayLabel *timeLabel;
  #endif
  
  #ifdef SYNRAD
  GLButton      *selTrajBtn;
  #endif
  
  // Viewer mode
  int      draggMode;
  int      mode;
  bool     selected;

  // View parameters
  AVIEW    view;

  // Camera<->mouse motions
  int      mXOrg;
  int      mYOrg;    
  double   camDistInc;

  // Transformed BB
  double   xMin;
  double   xMax;
  double   yMin;
  double   yMax;
  double   zNear;
  double   zFar;

  Vector3d camDir;     // Camera basis (PERSPECTIVE_PROJ)
  Vector3d camLeft;    // Camera basis (PERSPECTIVE_PROJ)
  Vector3d camUp;      // Camera basis (PERSPECTIVE_PROJ)

  double   vectorLength;
  double   arrowLength;

  // Rectangle selection
  int      selX1;
  int      selY1;
  int      selX2;
  int      selY2;

  // Selection change
  bool selectionChange;
  ScreenshotStatus screenshotStatus;

  // SDL/OpenGL stuff
  GLfloat matView[16];
  GLfloat matProj[16];
  GLMATERIAL greenMaterial;
  GLMATERIAL blueMaterial;

  //Debug
  //GLLabel* debugLabel;
};

#endif /* _GEOMETRYVIEWERH_ */
