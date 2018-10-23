// Copyright (c) 2011 rubicon IT GmbH
#ifndef _GLCOMPONENTH_
#define _GLCOMPONENTH_

#include "GLContainer.h"

#define BORDER_NONE      0
#define BORDER_BEVEL_IN  1
#define BORDER_BEVEL_OUT 2
#define BORDER_ETCHED    3

class GLComponent: public GLContainer {

public:

  // Construction
  GLComponent(int compId);

  // Component methods
  void SetPosition(int x,int y);
  void SetSize(int width, int height);
  void GetBounds(int *x,int *y, int *w, int *h);
  void SetBorder(int border);
  void SetOpaque(bool opaque);
  void SetBackgroundColor(int r,int g,int b);
  void GetBackgroundColor(int *r,int *g,int *b);
  int  GetId();
  bool HasFocus();
  void SetFocusable(bool acceptFocus);
  bool IsFocusable();
  void SetEnabled(bool enable);
  bool IsEnabled();
  void SetVisible(bool visible);
  bool IsVisible();
  GLContainer *GetParent();
  void SetCursor(int cursor);
  int GetCursor();
  int GetWidth();
  int GetHeight();

  // Implementation
  virtual void Paint();
  virtual void SetFocus(bool focus);
  virtual void SetBounds(int x,int y, int width, int height);
  virtual void SetParent(GLContainer *parent);
  virtual void DestroyComponents() {};

protected:

	int  width;
	int  height;
  int  posX;
  int  posY;
  bool opaque;
  int  border;
  int  rBack;
  int  gBack;
  int  bBack;
  int  id;
  int  cursor;
  bool focus;
  bool enabled;
  bool visible;
  bool focusable;
  GLContainer *parent;

};

#endif /* _GLCOMPONENTH_ */