// Copyright (c) 2011 rubicon IT GmbH
#ifndef _GLINPUTBOXH_
#define _GLINPUTBOXH_

//#include <SDL2/SDL_opengl.h>
#include "GLWindow.h"

class GLTextField;

class GLInputBox : private GLWindow {

public:
  // Display a modal dialog and return the entered string (NULL on cancel)
  static char *GetInput(const char *initMessage=NULL,const char *label=NULL,const char *title=NULL);

  char *rValue;
 
private:

  GLTextField *text;

  GLInputBox(const char *message,const char *label,const char *title);
  void ProcessMessage(GLComponent *src,int message);
  void ManageEvent(SDL_Event *evt); //Catch enter and esc
};

#endif /* _GLINPUTBOXH_ */
