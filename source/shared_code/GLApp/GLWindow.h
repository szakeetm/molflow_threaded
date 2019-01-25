// Copyright (c) 2011 rubicon IT GmbH
#ifndef _GLWINDOWH_
#define _GLWINDOWH_

//#include <SDL/SDL.h>
//#include <SDL/SDL_opengl.h>
#include "GLContainer.h"
//#include "GLMenu.h"
class GLMenu;
#include <string>

class GLWindow : public GLContainer {

public:

  // Construction
  GLWindow();
  ~GLWindow();

  // Window methods
  void GetBounds(int *x,int *y,int *w,int *h);
  void SetPosition(int x,int y);
  int  GetHeight();
  int  GetWidth();
  void GetClientArea(int *x,int *y,int *w,int *h);
  void SetTitle(const char *title);
  void SetTitle(std::string title);
  void SetBorder(bool b);
  void SetBackgroundColor(int r,int g,int b);
  void GetBackgroundColor(int *r,int *g,int *b);
  bool IsCtrlDown();
  bool IsShiftDown();
  bool IsAltDown();
  bool IsSpaceDown();
  bool IsCapsLockOn();
  bool IsTabDown();
  //int  GetModState();
  int  GetX(GLComponent *src,SDL_Event *evt);
  int  GetY(GLComponent *src,SDL_Event *evt);
  int  GetScreenX(GLComponent *src);
  int  GetScreenY(GLComponent *src);
  void DoModal();
  void SetVisible(bool visible);
  bool IsVisible();
  void SetResizable(bool sizable);
  void SetIconfiable(bool iconifiable);
  void SetMinimumSize(int width,int height);
  void Iconify(bool iconify);
  void Maximise(bool max);
  bool IsIconic();
  bool IsMaximized();
  void SetAnimatedFocus(bool animate);

  // Expert usage
  void Clip(GLComponent *src,int lMargin,int uMargin,int rMargin,int bMargin);
  void ClipRect(GLComponent *src,int x,int y,int width,int height);
  void ClipToWindow();
  void ClipWindowExtent();
  bool IsMoving();
  bool IsInComp(GLComponent *src,int mx,int my);
  void SetMaster(bool master);
  bool IsDragging();
  int  GetIconWidth();
  void PaintTitle(int width,int height);
  void PaintMenuBar();
  void UpdateOnResize();

  // Menu management
  void SetMenuBar(GLComponent *bar,int hBar=20);
  void AddMenu(GLMenu *menu);
  void RemoveMenu(GLMenu *menu);
  void CloseMenu();

  //Implementation
  virtual void ProcessMessage(GLComponent *src,int message);
  virtual void ManageEvent(SDL_Event *evt);
  virtual void ManageMenu(SDL_Event *evt);
  virtual void Paint();
  virtual void PaintMenu();
  virtual void SetBounds(int x,int y,int w,int h);
  virtual void CancelDrag(SDL_Event *evt);
  virtual void DestroyComponents() {};

  

protected:

  // Coordinates (absolute)
  int  width;
  int  height;
  int  posX;
  int  posY;

private:

  int  GetUpMargin();
  bool IsInWindow(int mx,int my);
  bool IsInSysButton(SDL_Event *evt,int witch);
  void UpdateSize(int newWidht,int newHeight,int cursor);

  int  draggMode;
  int  mXOrg;
  int  mYOrg;
  char title[128];
  char iconTitle[64];
  int  closeState;
  int  maxState;
  int  iconState;
  bool iconifiable;
  bool iconified;
  bool maximized;
  bool border;
  bool animateFocus;
  int  rBack;
  int  gBack;
  int  bBack;
  bool isMaster;
  GLComponent *menuBar;
  bool visible;
  bool isResizable;
  int  minWidth;
  int  minHeight;
  int  orgWidth;
  int  orgHeight;
  GLContainer *menus;
  bool isModal;
  int  iconWidth;
  int  posXSave;
  int  posYSave;
  int  widthSave;
  int  heightSave;
  size_t lastClick;

};

#endif /* _GLWINDOWH_ */