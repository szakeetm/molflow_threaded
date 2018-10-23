// Copyright (c) 2011 rubicon IT GmbH
#ifndef _GLTOGGLEH_
#define _GLTOGGLEH_

#include "GLComponent.h"
#include <string>

class GLToggle : public GLComponent {

public:

  // Construction
  GLToggle(int compId,const char *text);

  // Component method
  int  GetState();
  void SetText(std::string txt);
  void SetState(int setState);
  void SetTextColor(int r,int g,int b);
  void SetEnabled(bool enable); //override GLComponent for text color change
  void AllowMixedState(bool setAllow);

  // Implementation
  void Paint();
  void ManageEvent(SDL_Event *evt);

private:

  char text[512];
  int  state; // 0=>Unchekced 1=>Checked 2=>Multiple
  float rText;
  float gText;
  float bText;
  bool allowMixedState; //Allow "multiple" state

};

#endif /* _GLTOGGLEH_ */