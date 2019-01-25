// Copyright (c) 2011 rubicon IT GmbH
#ifndef _GLSAVEDIALOGH_
#define _GLSAVEDIALGOH_

#include <SDL2/SDL_opengl.h>
#include "GLWindow.h"

// Buttons
#define GLDLG_SAVE		  0x0001
#define GLDLG_DISCARD	  0x0002
#define GLDLG_CANCEL_S    0x0004

// Icons
#define GLDLG_ICONNONE    0
#define GLDLG_ICONERROR   1
#define GLDLG_ICONWARNING 2
#define GLDLG_ICONINFO    3

class GLSaveDialog : private GLWindow {

public:
  // Display a modal dialog and return the code of the pressed button
  static int Display(const char *message, const char *title=NULL,int mode=GLDLG_SAVE|GLDLG_DISCARD|GLDLG_CANCEL_S,int icon=GLDLG_ICONNONE);

  int  rCode;

private:
  GLSaveDialog(const char *message, const char *title,int mode,int icon);
  void ProcessMessage(GLComponent *src,int message);

};

#endif /* _GLSAVEDIALOGH_ */
