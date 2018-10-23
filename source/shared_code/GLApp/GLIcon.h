// Copyright (c) 2011 rubicon IT GmbH
#ifndef _GLICONH_
#define _GLICONH_

#include "GLComponent.h"
#include "GLSprite.h"

class GLIcon : public GLComponent {

public:

  // Construction
  GLIcon(const char *iconName);

  // Implementation
  void Paint();
  void InvalidateDeviceObjects();
  void RestoreDeviceObjects();

private:

  Sprite2D *icon;
  char name[512];

};

#endif /* _GLICONH_ */