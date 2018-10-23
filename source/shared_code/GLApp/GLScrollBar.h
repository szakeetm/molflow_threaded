// Copyright (c) 2011 rubicon IT GmbH
#ifndef _GLSCROOLBARH_
#define _GLSCROOLBARH_

#include "GLComponent.h"

#define SB_VERTICAL   1
#define SB_HORIZONTAL 2

class GLScrollBar : public GLComponent {

public:

  // Construction
  GLScrollBar(int compId);

  // Component methods
  void SetRange(int max,int page,int wheel_delta);
  void SetPosition(int nPos);
  void SetPositionMax();
  int  GetPosition();
  void SetOrientation(int orientation);

  // Implementation
  void Paint();
  void ManageEvent(SDL_Event *evt);

private:

  void     SetPos(int nPos);
  void     Measure();

  int      m_Pos;
  int      m_Max;
  int      m_Page;
  int      m_Wheel_Delta;
  int      m_Drag;
  int      lastX;
  int      lastY;
  int      orientation;
  int      ws;
  int      ss;
  int      d1,d2;

};

#endif /* _GLSCROOLBARH_ */