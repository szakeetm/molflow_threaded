// Copyright (c) 2011 rubicon IT GmbH
#ifndef _GLWINDOWMANAGERH_
#define _GLWINDOWMANAGERH_

#include <SDL2/SDL.h>
//#include <SDL_opengl.h>
//#include "GLTypes.h"
//#include "GLFont.h"

#define NO_MODIFIER    0
#define ALT_MODIFIER   1
#define CTRL_MODIFIER  2
#define SHIFT_MODIFIER 4
#define CAPSLOCK_MODIFIER 8
#define SPACE_MODIFIER 16
#define TAB_MODIFIER 32

class GLWindow;
class GLComponent;
class GLApplication;

class GLWindowManager {

public:

  // Window functions
  static void      Repaint();
  static void      RepaintNoSwap();
  static void      RepaintRange(int w0,int w1);
  static void      FullRepaint();
  static GLWindow* GetWindow(int idx);
  static int       GetNbWindow();
  static GLWindow *GetTopLevelWindow();
  static void      BringToFront(GLWindow *wnd);
  static void      SetDefault();

  // Key info
  //static bool      IsCtrlDown();
  //static bool      IsShiftDown();
  //static bool      IsAltDown();
  //static bool      IsCapsLockOn();
  //static bool      IsSpaceDown();
  //static bool		IsTabDown();
  //static int	GetModState();

  // Registering
  static void RegisterWindow(GLWindow *wnd);
  static void UnRegisterWindow(GLWindow *wnd);
  static void RegisterKeyboardShortcut(GLComponent *src,int keyCode,int modifier,int accId);

  // Processing
  static bool RestoreDeviceObjects(int width,int height);
  static bool ManageEvent(SDL_Event *evt);
  static bool SearchKeyboardShortcut(SDL_Event *evt,bool processAcc);
  static void RestoreDeviceObjects();
  static void InvalidateDeviceObjects();
  static void Resize();

  // Utils functions
  static void AnimateFocus(GLWindow *src);
  static void AnimateIconify(GLWindow *src);
  static void AnimateDeIconify(GLWindow *src);
  static void AnimateMaximize(GLWindow *src,int fsX,int fsY,int fsWidth,int fsHeight);
  static void RemoveAccFromStr(char *txt,char *acc=NULL,int *pos=NULL,int *width=NULL);
  static char *GetAccStr(int keyCode,int keyModifier);
  static void DrawStats();
  static void NoClip();

};

#endif /* _GLWINDOWMANAGERH_ */