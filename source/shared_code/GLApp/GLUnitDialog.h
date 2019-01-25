// Copyright (c) 2011 rubicon IT GmbH
#ifndef _GLUNITDIALOGH_
#define _GLUNITDIALGOH_

//#include <SDL2/SDL_opengl.h>
#include "GLWindow.h"

// Buttons
#define GLDLG_MM		1
#define GLDLG_CM		2
#define GLDLG_M			4
#define GLDLG_INCH		8
#define GLDLG_FOOT		16
#define GLDLG_CANCEL_U  32

// Icons
#define GLDLG_ICONNONE    0
#define GLDLG_ICONERROR   1
#define GLDLG_ICONWARNING 2
#define GLDLG_ICONINFO    3

class GLUnitDialog : private GLWindow {

public:
  // Display a modal dialog and return the code of the pressed button
  static int Display(const char *message, const char *title=NULL,int mode=GLDLG_MM|GLDLG_CM|GLDLG_M|GLDLG_INCH|GLDLG_FOOT|GLDLG_CANCEL_U,int icon=GLDLG_ICONNONE);

  int  rCode;

private:
  GLUnitDialog(const char *message, const char *title,int mode,int icon);
  void ProcessMessage(GLComponent *src,int message);

};

#endif /* _GLMESSAGEBOXH_ */
