// Copyright (c) 2011 rubicon IT GmbH
#ifndef _SPRITE2DH_
#define _SPRITE2DH_

#include <SDL/SDL_opengl.h>

class Sprite2D {

public:

  // Default constructor
  Sprite2D();
  
  // Initialise the font
  // return 1 when success, 0 otherwise
  int RestoreDeviceObjects(const char *diffName, const char *alphaName,int srcWidth,int scrHeight);

  // Update sprite mapping and coordinates
  void UpdateSprite(const int &x1,const int &y1,const int &x2,const int &y2);
  void UpdateSprite(const int &x1,const int &y1,const int &x2,
	  const int &y2,const float &mx1,const float &my1,const float &mx2,const float &my2);
  void SetSpriteMapping(const float &mx1,const float &my1,const float &mx2,const float &my2);
  void SetColor(const float &r,const float &g,const float &b);
  
  // Draw a 2D sprite (in screen coordinates)
  void Render(const bool &doLinear);
  void Render90(bool doLinear);

  // Release any allocated resource
  void InvalidateDeviceObjects();

private:

  GLuint  texId;
  int x1;
  int y1;
  int x2;
  int y2;
  float mx1;
  float my1;
  float mx2;
  float my2;
  int hasAlpha;
  float rC;
  float gC;
  float bC;

  GLfloat pMatrix[16];

};

#endif /* _SPRITE2DH_ */
