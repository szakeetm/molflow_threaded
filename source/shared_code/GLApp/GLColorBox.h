// Copyright (c) 2011 rubicon IT GmbH

//#include <SDL2/SDL_opengl.h>

#ifndef _GLCOLORBOXH_
#define _GLCOLORBOXH_

class GLColorBox : private GLWindow {

public:
  // Display a modal dialog and return 1 on ok, 0 on cancel. r,g and b 
  // contains the choosen color
  static int Display(const char *title,int *r,int *g,int *b);

  int  rCode;
  int  curR;
  int  curG;
  int  curB;

  // Implementation
  void InvalidateDeviceObjects();
  void RestoreDeviceObjects();

private:

  GLTitledPanel *swPanel;
  GLTitledPanel *hsvPanel;
  GLTextField   *rText;
  GLTextField   *gText;
  GLTextField   *bText;
  GLLabel       *oldColor;
  GLLabel       *newColor;
  GLLabel       *swBox;
  GLLabel       *hsBox;
  GLLabel       *vBox;
  GLuint         hsvTex;
  GLuint         sliderTex;
  float          curH;
  float          curS;
  float          curV;
  bool           draggV;

  GLColorBox(const char *title,int *r,int *g,int *b);
  ~GLColorBox();
  void ProcessMessage(GLComponent *src,int message);
  void Paint();
  void ManageEvent(SDL_Event *evt);

  void rgb_to_hsv( int ri,int gi,int bi, float *h,float *s,float *v);
  size_t hsv_to_rgb( float h,float s,float v,bool swap=false );
  float get_red( size_t c );
  float get_green( size_t c );
  float get_blue( size_t c );
  int   get_redi( size_t c );
  int   get_greeni( size_t c );
  int   get_bluei( size_t c );
  void  paintBox(int x,int y,int w,int h);
  void  updateColor(int r,int g,int b);

};

#endif /* _GLCOLORBOXH_ */
