// Copyright (c) 2011 rubicon IT GmbH
#ifndef _GLMESSAGEBOXH_
#define _GLMESSAGEBOXH_

//#include <SDL_opengl.h>
#include "GLWindow.h"
#include <vector>

// Buttons
#define GLDLG_OK          0x0001
#define GLDLG_CANCEL      0x0002

// Icons
#define GLDLG_ICONNONE    0
#define GLDLG_ICONERROR   1
#define GLDLG_ICONWARNING 2
#define GLDLG_ICONINFO    3
#define GLDGL_ICONDEAD    4

class GLMessageBox : private GLWindow {

public:
  // Display a modal dialog and return the code of the pressed button
  static int Display(const char *message, const char *title=NULL,int mode=GLDLG_OK,int icon=GLDLG_ICONNONE);
  static int Display(const std::string & message, const std::string & title, const std::vector<std::string>& buttonList, int icon);

  int  rCode;

private:
	GLMessageBox(const std::string & message, const std::string & title, const std::vector<std::string>& buttonList, int icon);
	//GLMessageBox(const char *message,char *title,int mode,int icon);
  void ProcessMessage(GLComponent *src,int message);

};

#endif /* _GLMESSAGEBOXH_ */
