// Copyright (c) 2011 rubicon IT GmbH
#ifndef _GLPROGESSH_
#define _GLPROGESSH_

//#include <SDL_opengl.h>
#include "GLWindow.h"
#include <string>

class GLLabel;

class GLProgress : public GLWindow {

public:

  GLProgress(const char *message,const char *title);

  // Update progress bar (0 to 1)
  void SetProgress(double value);
  double GetProgress();
  void SetMessage(const char *msg);
  void SetMessage(std::string msg);

private:

  GLLabel   *scroll;
  GLLabel   *scrollText;
  GLLabel	*label;
  int        progress;
  int        xP,yP,wP,hP;
  Uint32     lastUpd;

  void ProcessMessage(GLComponent *src,int message);

};

#endif /* _GLPROGESSH_ */
