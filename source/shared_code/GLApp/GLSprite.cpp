// Copyright (c) 2011 rubicon IT GmbH
#include "GLSprite.h"
#include "GLToolkit.h"
#include "GLApp.h"
#define cimg_use_png 1
#include <CImg/CImg.h>
using namespace cimg_library;
//#include <malloc.h>
#include <stdio.h>
#include <cstring>

extern GLApplication *theApp;

Sprite2D::Sprite2D() {
  rC = 1.0f;
  gC = 1.0f;
  bC = 1.0f;
}

void Sprite2D::SetSpriteMapping(const float &mx1,const float &my1,const float &mx2,const float &my2) {

  this->mx1 = mx1;
  this->my1 = my1;
  this->mx2 = mx2;
  this->my2 = my2;

}

void Sprite2D::UpdateSprite(const int &x1,const int &y1,const int &x2,const int &y2,
							const float &mx1,const float &my1,const float &mx2,const float &my2) {

  this->x1 = x1;
  this->y1 = y1;
  this->x2 = x2;
  this->y2 = y2;
  this->mx1 = mx1;
  this->my1 = my1;
  this->mx2 = mx2;
  this->my2 = my2;

}

void Sprite2D::UpdateSprite(const int &x1,const int &y1,const int &x2,const int &y2) {

  this->x1 = x1;
  this->y1 = y1;
  this->x2 = x2;
  this->y2 = y2;

}

void Sprite2D::SetColor(const float &r,const float &g,const float &b) {
  rC = r;
  gC = g;
  bC = b;
}

int Sprite2D::RestoreDeviceObjects(const char *diffName, const char *alphaName,int scrWidth,int scrHeight) {

  GLint  bpp;
  GLenum format;
  BYTE *buff32;
  char tmp[512];

  CImg<BYTE> img, imga;
  
  try { img.load(diffName); }
  catch (...) {
    sprintf(tmp,"Failed to load \"%s\"",diffName);
    GLToolkit::Log(tmp); //Commented out for File open dialog
    return 0;
  }
  

  hasAlpha = strcmp(alphaName,"none")!=0;
  if( hasAlpha ) {
	  try {
		  imga.load(alphaName);
	  } catch (...) {
      sprintf(tmp,"Failed to load \"%s\"",alphaName);
      GLToolkit::Log(tmp);
      return 0;
    }
  }

  // Make 32 Bit RGB/RGBA buffer
  int fWidth  = img.width();
  int fHeight = img.height();

  if( hasAlpha ) {

    format = GL_RGBA;
    bpp    = 4;
    buff32 = (BYTE *)malloc(fWidth*fHeight*4);
    //BYTE *data   = img.data();
    //BYTE *adata   = imga.data();

    for(int y=0;y<fHeight;y++) {
      for(int x=0;x<fWidth;x++) {
		  buff32[x * 4 + 0 + y * 4 * fWidth] = /*data[x*3+2 + y*3*fWidth];*/ *(img.data(x, y, 0, 0));
        buff32[x*4 + 1 + y*4*fWidth] = /*data[x*3+1 + y*3*fWidth];*/ *(img.data(x, y, 0, 1));
        buff32[x*4 + 2 + y*4*fWidth] =/* data[x*3+0 + y*3*fWidth];*/ *(img.data(x, y, 0, 2));
		buff32[x * 4 + 3 + y * 4 * fWidth] = /*adata[x*3+1 + y*3*fWidth];*/ *(imga.data(x, y, 0, 1));
      }
    }

  } else {

    format = GL_RGB;
    bpp    = 3;
    buff32 = (BYTE *)malloc(fWidth*fHeight*3);
    //BYTE *data   = img.data();

    for(int y=0;y<fHeight;y++) {
      for(int x=0;x<fWidth;x++) {
        buff32[x*3 + 0 + y*3*fWidth] = *(img.data(x, y, 0, 0));
        buff32[x*3 + 1 + y*3*fWidth] = *(img.data(x, y, 0, 1));
        buff32[x*3 + 2 + y*3*fWidth] = *(img.data(x, y, 0, 2));
      }
    }

  }

  glGenTextures(1,&texId);
  glBindTexture(GL_TEXTURE_2D,texId);

  glTexImage2D (
    GL_TEXTURE_2D,       // Type
    0,                   // No Mipmap
    bpp,                // Byte per pixel
    fWidth,             // Width
    fHeight,            // Height
    0,                   // Border
    format,             // Format RGB/RGBA
    GL_UNSIGNED_BYTE,   // 8 Bit/color
    buff32              // Data
  );   

  free(buff32);
  //img.Release();
  //imga.Release();

  GLenum glError = glGetError();
  if( glError != GL_NO_ERROR )
  {
    sprintf(tmp,"Failed to create Sprite2D \"%s\"",diffName);
    GLToolkit::Log(tmp);
    GLToolkit::printGlError(glError);
    return 0;    
  }

  // Compute othographic matrix (for Transfomed Lit vertex)
  glMatrixMode( GL_PROJECTION );
  glLoadIdentity();
  glOrtho( 0, scrWidth, scrHeight, 0, -1, 1 );
  glGetFloatv( GL_PROJECTION_MATRIX , pMatrix );

  return 1;

}

void Sprite2D::InvalidateDeviceObjects() {

  if(texId) glDeleteTextures(1, &texId);
  texId = 0;

}

void Sprite2D::Render(const bool &doLinear) {

  glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D,texId);
  if( doLinear ) {
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  } else {
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  }
  if( hasAlpha ) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  } else {
    glDisable(GL_BLEND);
  }
  glColor3f(rC,gC,bC);
  /*
  if(loadMatrix) {
    glMatrixMode( GL_PROJECTION );
    glLoadMatrixf(pMatrix);
    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();
  }
  */
  glBegin(GL_QUADS);
  glTexCoord2f(mx1,my1);glVertex2i(x1,y1);
  glTexCoord2f(mx2,my1);glVertex2i(x2,y1);
  glTexCoord2f(mx2,my2);glVertex2i(x2,y2);
  glTexCoord2f(mx1,my2);glVertex2i(x1,y2);
  glEnd();

#ifdef _DEBUG
  theApp->nbPoly++;
#endif

}

void Sprite2D::Render90(bool doLinear) {

  glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D,texId);
  if( doLinear ) {
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  } else {
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  }
  if( hasAlpha ) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  } else {
    glDisable(GL_BLEND);
  }
  glColor3f(rC,gC,bC);
  /*
  if(loadMatrix) {
    glMatrixMode( GL_PROJECTION );
    glLoadMatrixf(pMatrix);
    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();
  }
  */
  glBegin(GL_QUADS);
  glTexCoord2f(mx1,my1);glVertex2i(x1,y1);
  glTexCoord2f(mx1,my2);glVertex2i(x2,y1);
  glTexCoord2f(mx2,my2);glVertex2i(x2,y2);
  glTexCoord2f(mx2,my1);glVertex2i(x1,y2);
  glEnd();

#ifdef _DEBUG
  theApp->nbPoly++;
#endif

}
