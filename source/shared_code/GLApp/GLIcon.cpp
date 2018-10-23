// Copyright (c) 2011 rubicon IT GmbH
#include "GLWindow.h"
#include "GLIcon.h"
#include "GLToolkit.h"
#include <cstring> //strcpy, etc.

GLIcon::GLIcon(const char *name):GLComponent(0) {
  strcpy(this->name,name);
  icon = NULL;
}

void GLIcon::Paint() {

  if(!parent) return;

  if(icon) {
    icon->UpdateSprite(posX,posY,posX+width,posY+height);
    icon->Render(false);
  }

}

void GLIcon::InvalidateDeviceObjects() {
  icon->InvalidateDeviceObjects();
  SAFE_DELETE(icon);
}

void GLIcon::RestoreDeviceObjects() {

  GLint viewport[4];
  glGetIntegerv(GL_VIEWPORT,viewport);
  icon = new Sprite2D();
  icon->RestoreDeviceObjects(name,"none",viewport[2],viewport[3]);
  icon->SetSpriteMapping(0.0f,0.0f,1.0f,1.0f);

}
