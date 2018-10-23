// Copyright (c) 2011 rubicon IT GmbH
#ifndef _GLBUTTONH_
#define _GLBUTTONH_

#include "GLComponent.h"
//#include "GLSprite.h"
class Sprite2D;
class GLFont2D;

class GLButton : public GLComponent {

public:

  // Construction
  GLButton(int compId,const char *text);

  // Components method
  void SetText(const char *text);
  void SetIcon(const char *fileName);
  void SetDisabledIcon(const char *fileName);
  void SetToggle(bool toggle);
  bool GetState();
  void SetState(bool checked);

  // Implementation
  void Paint();
  void ManageEvent(SDL_Event *evt);
  void SetBounds(int x,int y,int width,int height);
  void InvalidateDeviceObjects();
  void RestoreDeviceObjects();
  void SetFontColor(float red, float green, float blue);
  void SetFontColor(int red, int green, int blue);
private:

  char text[256];
  Sprite2D *icon;
  Sprite2D *iconD;
  int  state; // 0=>Released 1=>Pressed
  char iconName[256];
  char iconNameDisa[256];
  bool toggle;
  bool toggleState;
  GLFont2D *font;
  float r, g, b;
};

#endif /* _GLBUTTONH_ */