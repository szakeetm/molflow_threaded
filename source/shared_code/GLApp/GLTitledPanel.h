// Copyright (c) 2011 rubicon IT GmbH
#ifndef _GLTITLEDPANELH_
#define _GLTITLEDPANELH_

#include "GLComponent.h"

class GLTitledPanel : public GLComponent {

public:

  // Construction
  GLTitledPanel(const char *title);

  // Component method
  void SetTitle(const char *title);
  void SetTextColor(int r,int g,int b);
  void SetBold(bool b);
  void SetClosable(bool c);
  void Close();
  void Open();
  bool IsClosed();
  void SetCompBounds(GLComponent *src,int x,int y,int width,int height);

  // Implementation
  virtual void Paint();
  virtual void ManageEvent(SDL_Event *evt);
  virtual void ProcessMessage(GLComponent *src,int message);
  virtual void SetBounds(int x,int y,int width,int height);

private:

  char  title[256];
  float rText;
  float gText;
  float bText;
  bool  isBold;
  bool  closeAble;
  bool  closeState; //make collapse/expand button grey while pressed
  bool  closed;
  int   wOrg;
  int   hOrg;
  int   txtWidth;

};

#endif /* _GLTITLEDPANELH_ */