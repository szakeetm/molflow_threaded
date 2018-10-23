// Copyright (c) 2011 rubicon IT GmbH
#ifndef _GLFONT2DH_
#define _GLFONT2DH_

#include <SDL_opengl.h>
#include "GLTypes.h"

class GLFont2D {

public:

  // Default constructor
  GLFont2D();

  // Construct a font using the specified bitmap
  GLFont2D(const char *imgFileName);
  
  // Initialise the font
  // return 1 when success, 0 otherwise
  int RestoreDeviceObjects(int srcWidth,int scrHeight);
  
  // Draw a 2D text (in viewport coordinates)
  void DrawText(const int &x,const int &y,const char *text,const bool &loadMatrix=true);
  void DrawLargeText(int x,int y,const char *text,float sizeFactor,bool loadMatrix=true);
  void DrawTextFast(int cx,int cy,const char *text);
  void DrawTextV(int x,int y,char *text,bool loadMatrix=true);

  // Release any allocated resource
  void InvalidateDeviceObjects();

  // Set text color
  void SetTextColor(const float &r,const float &g,const float &b);

  // Set default character size (Default 9,15)
  void SetTextSize(int width,int height);

  // Set variable font width (Must be called before RestoreDeviceObject)
  void SetVariableWidth(bool variable);

  // Get string size
  int GetTextWidth(const char *text);
  int GetTextHeight();

  // Adapat orthographic projection on viewport change
  void ChangeViewport(GLVIEWPORT *g);

private:

  char    fileName[512];
  GLuint  texId;
  int     fWidth;
  int     fHeight;
  int     cWidth;
  int     cHeight;
  float   rC;
  float   gC;
  float   bC;
  GLfloat pMatrix[16];
  int     cVarWidth[256];
  bool    isVariable;

};

#endif /* _GLFONT2DH_ */
