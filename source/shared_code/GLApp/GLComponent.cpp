// Copyright (c) 2011 rubicon IT GmbH
#include "GLWindow.h"
#include "GLComponent.h"
#include "GLToolkit.h"

GLComponent::GLComponent(int compId):GLContainer() {

  width=0;
  height=0;
  posX=0;
  posY=0;
  opaque=true;
  border=BORDER_NONE;
  rBack = 212;
  gBack = 208;
  bBack = 200;
  id = compId;
  parent = NULL;
  focus = false;
  enabled = true;
  visible = true;
  focusable = true;
  cursor = CURSOR_DEFAULT;

}

int GLComponent::GetWidth() {
  return width;
}

int GLComponent::GetHeight() {
  return height;
}

void GLComponent::SetParent(GLContainer *parent) {

  this->parent =parent;
  SetWindow(parent->GetWindow());

}

GLContainer *GLComponent::GetParent() {
  return parent;
}

void GLComponent::SetCursor(int cursor) {
  this->cursor = cursor;
}

int GLComponent::GetCursor() {
  if( enabled )
    return cursor;
  else
    return CURSOR_DEFAULT;
}

void GLComponent::SetFocusable(bool acceptFocus) {
  focusable = acceptFocus;
}

bool GLComponent::IsFocusable() {
  return focusable;
}

void GLComponent::SetPosition(int x,int y) {
  posX = x;
  posY = y;
}

void GLComponent::SetBorder(int border) {
  this->border = border;
}

void GLComponent::SetOpaque(bool opaque) {
  this->opaque = opaque;
}

void GLComponent::SetSize(int width, int height) {
  this->width = width;
  this->height = height;
}

void GLComponent::SetBounds(int x,int y, int width, int height) {
  SetSize(width,height);
  SetPosition(x,y);
}

void GLComponent::GetBackgroundColor(int *r,int *g,int *b) {
  *r = rBack;
  *g = gBack;
  *b = bBack;
}

void GLComponent::SetBackgroundColor(int r,int g,int b) {
  rBack = r;
  gBack = g;
  bBack = b;
}

int GLComponent::GetId() {
  return id;
}

void GLComponent::SetFocus(bool focus) {
  this->focus = focus;
}

bool GLComponent::HasFocus() {
  return focus;
}

void GLComponent::SetEnabled(bool enable) {
  enabled = enable;
}

bool GLComponent::IsEnabled() {
  return enabled;
}

void GLComponent::SetVisible(bool visible) {
  this->visible = visible;
}

bool GLComponent::IsVisible() {
  return visible;
}

void GLComponent::Paint() {
	
  // Paint background
  if(parent && width>0 && height>0 && opaque) {
    switch(border) {
      case BORDER_NONE:
        GLToolkit::DrawBox(posX,posY,width,height,rBack,gBack,bBack);
        break;
      case BORDER_BEVEL_IN:
        GLToolkit::DrawBox(posX,posY,width,height,rBack,gBack,bBack,true,true);
        break;
      case BORDER_BEVEL_OUT:
        GLToolkit::DrawBox(posX,posY,width,height,rBack,gBack,bBack,true,false);
        break;
      case BORDER_ETCHED:
        GLToolkit::DrawBox(posX,posY,width,height,rBack,gBack,bBack,false,false,true);
        break;
    }
  }
}

void GLComponent::GetBounds(int *x,int *y, int *w, int *h) {
  *x = posX;
  *y = posY;
  *h = height;
  *w = width;
}
