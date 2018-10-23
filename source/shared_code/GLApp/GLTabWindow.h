// Copyright (c) 2011 rubicon IT GmbH
#pragma once

//#include "GLApp.h"
#include "GLWindow.h"
class GLComponent;
class TabbedBar;

typedef struct {

  char         *name;
  int           width;
  GLComponent **comp;
  int           nbComp;
  bool          selected;

} APANEL;

class GLTabWindow : public GLWindow {

public:

  // Construction
  GLTabWindow();
  ~GLTabWindow();

  // Add/Remove components to this windows
  void Add(int panel,GLComponent *comp);
  void SetPanelNumber(int numP);
  void SetPanelName(int idx,const char *name);
  void Clear();
  void Update();
  void SetTextColor(int r,int g,int b);

  //Overrides
  void SetBounds(int x,int y,int w,int h);
  void ProcessMessage(GLComponent *src,int message);

private:

  void showHide();

  APANEL *panels;
  int nbPanel;
  TabbedBar *bar;

};

