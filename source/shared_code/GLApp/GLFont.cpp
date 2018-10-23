// Copyright (c) 2011 rubicon IT GmbH
#include "GLFont.h"
#include "GLToolkit.h"
#include "GLApp.h"
#define cimg_use_png 1
#include <CImg/CImg.h>
using namespace cimg_library;
//#include <malloc.h>
#include <stdio.h>
#include <cstring> //strcpy, etc.

extern GLApplication *theApp;

GLFont2D::GLFont2D() {
  strcpy(fileName,"images/font.png");
  cHeight = 15;
  cWidth  = 9;
  isVariable = false;
}

GLFont2D::GLFont2D(const char *imgFileName) {
  strcpy(fileName,imgFileName);
  cHeight = 15;
  cWidth  = 9;
  isVariable = false;
}

int GLFont2D::RestoreDeviceObjects(int scrWidth,int scrHeight) {

  // Load the image
 // CImage img;
  //if( !img.LoadCImage(fileName) ) {

	try {
		CImg<BYTE> img(fileName);
		// Make 32 Bit RGBA buffer
		fWidth = img.width();
		fHeight = img.height();
		BYTE *buff32 = (BYTE *)malloc(fWidth*fHeight * 4);
		//BYTE *data = img.data();
		for (int y = 0; y < fHeight; y++) {
			for (int x = 0; x < fWidth; x++) {
				buff32[x * 4 + 0 + y * 4 * fWidth] = /*data[x * 3 + 2 + y * 3 * fWidth]*/ *(img.data(x, y, 0, 0));
				buff32[x * 4 + 1 + y * 4 * fWidth] = /*data[x * 3 + 2 + y * 3 * fWidth]*/ *(img.data(x, y, 0, 1));
				buff32[x * 4 + 2 + y * 4 * fWidth] = /*data[x * 3 + 2 + y * 3 * fWidth]*/ *(img.data(x, y, 0, 2));
				buff32[x * 4 + 3 + y * 4 * fWidth] = /*data[x * 3 + 2 + y * 3 * fWidth]*/ *(img.data(x, y, 0, 1)); // Green as alpha
			}
		}

		if (isVariable) {

			// Compute width for each char
			for (int i = 0; i < 256; i++) {
				cVarWidth[i] = 0;
				int xO = ((i % 16) * 16 + 1);
				int yO = ((i / 16) * 16);

				//scan columns
				bool black = true;
				while (black && xO < img.width()) {
					for (int j = 0; j < cHeight && black; j++)
						black = (/*data[(xO) * 3 + (yO + j) * 3 * fWidth]*/ *(img.data(xO,yO+j)) == 0);
					cVarWidth[i]++;
					xO++;
				}

				bool white = true;
				while (white && xO < img.width()) {
					black = true;
					for (int j = 0; j < cHeight && black; j++)
						black = (/*data[(xO) * 3 + (yO + j) * 3 * fWidth]*/ *(img.data(xO, yO+j)) == 0);
					white = !black;
					if (white) cVarWidth[i]++;
					xO++;
				}

				if (cVarWidth[i] > cWidth) cVarWidth[i] = cWidth;

				//Comprime space char when variable width font
				if (i == 32) cVarWidth[i] = cWidth / 3;

			}
		}

		glGenTextures(1, &texId);
		glBindTexture(GL_TEXTURE_2D, texId);

		glTexImage2D(
			GL_TEXTURE_2D,      // Type
			0,                  // No Mipmap
			4,                  // Format RGBA
			fWidth,             // Width
			fHeight,            // Height
			0,                  // Border
			GL_RGBA,            // Format RGBA
			GL_UNSIGNED_BYTE,   // 8 Bit/color
			buff32              // Data
		);

		free(buff32);
		//img.Release();
	}
	catch (...) {
		char tmp[600];
		sprintf(tmp, "Failed to load \"%s\"", fileName);
		GLToolkit::Log(tmp);
		return 0;
	}
  //}



  GLenum glError = glGetError();
  if( glError != GL_NO_ERROR )
  {
    char tmp[600];
    sprintf(tmp,"Failed to create GLFont2D \"%s\"",fileName);
    GLToolkit::Log(tmp);
    GLToolkit::printGlError(glError);
    return 0;
  }

  // Compute othographic matrix (for Transfomed Lit vertex)
  glMatrixMode( GL_PROJECTION );
  glLoadIdentity();
  glOrtho( 0, scrWidth, scrHeight, 0, -1, 1 );
  glGetFloatv( GL_PROJECTION_MATRIX , pMatrix );

  
  rC = 1.0f;
  gC = 1.0f;
  bC = 1.0f;

  return 1;

}

void GLFont2D::ChangeViewport(GLVIEWPORT *g) {
  GLfloat oldProj[16];
  glMatrixMode( GL_PROJECTION );
  glGetFloatv( GL_PROJECTION_MATRIX , oldProj );
  glLoadIdentity();
  glOrtho( g->x, g->x+g->width, g->y+g->height, g->y, -1, 1 );
  glGetFloatv( GL_PROJECTION_MATRIX , pMatrix );
  glLoadMatrixf(oldProj);
}

void GLFont2D::SetVariableWidth(bool variable) {
  isVariable = variable;
}

void GLFont2D::SetTextSize(int width,int height) {
  cHeight = height;
  cWidth  = width;
}

int GLFont2D::GetTextWidth(const char *text) {
  int lgth = (int)strlen(text);
  int w = 0;

  if( isVariable ) {
    for(int i=0;i<lgth;i++)
      w+=cVarWidth[ (unsigned char)text[i] ];
  } else {
    w = cWidth * lgth;
  }
  return w;
}

int GLFont2D::GetTextHeight() {
  return cHeight;
}

void GLFont2D::InvalidateDeviceObjects() {

  if(texId) glDeleteTextures(1, &texId);
  texId = 0;

}

void GLFont2D::SetTextColor(const float &r,const float &g,const float &b) {
  rC = r;
  gC = g;
  bC = b;
}

void GLFont2D::DrawLargeText(int cx,int cy,const char *text,float sizeFactor,bool loadMatrix) {

  int lgth = (int)strlen(text);
  if( lgth==0 ) return;
  int x = cx;
  int y = cy+1;

  glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D,texId);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glColor3f(rC,gC,bC);

  if( loadMatrix ) {
    glMatrixMode( GL_PROJECTION );
    glLoadMatrixf(pMatrix);
    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();
  }

  int xcPos=x;
  float cH   = (float)cHeight / (float)fWidth;
  //float factor=1.1f;
	//glScalef(factor,factor,factor);
  glBegin(GL_QUADS);
  for(int i=0;i<lgth;i++ ) {

    unsigned char  c = (unsigned char)text[i];
    float xPos = (float)((c % 16) * 16 + 1)/ (float)fWidth;
    float yPos = (float)((c / 16) * 16 )/ (float)fHeight;

    if(!isVariable) {

      float cW   = (float)cWidth / (float)fWidth;
      glTexCoord2f(xPos   ,yPos   );glVertex2i(xcPos       ,y   );
      glTexCoord2f(xPos+cW,yPos   );glVertex2i(xcPos+cWidth,y   );
      glTexCoord2f(xPos+cW,yPos+cH);glVertex2i(xcPos+cWidth,y+cHeight);
      glTexCoord2f(xPos   ,yPos+cH);glVertex2i(xcPos       ,y+cHeight);
      xcPos += cWidth;

    } else {
      float cW   = (float)cVarWidth[c] / (float)fWidth;
      glTexCoord2f(xPos   ,yPos   );glVertex2f((float)xcPos             , (float)y   );
      glTexCoord2f(xPos+cW,yPos   );glVertex2f((float)xcPos+cVarWidth[c]*sizeFactor, (float)y   );
      glTexCoord2f(xPos+cW,yPos+cH);glVertex2f((float)xcPos+cVarWidth[c]*sizeFactor, (float)y+cHeight*sizeFactor);
      glTexCoord2f(xPos   ,yPos+cH);glVertex2f((float)xcPos             , (float)y+cHeight*sizeFactor);
      xcPos += (int)(cVarWidth[c]*sizeFactor);

    }

  }
  
  glEnd();
  //glScalef(1.0f/factor,1.0f/factor,1.0f/factor);
#ifdef _DEBUG
  theApp->nbPoly+=lgth;
#endif

}

void GLFont2D::DrawText(const int &cx,const int &cy,const char *text,const bool &loadMatrix) {

  int lgth = (int)strlen(text);
  if( lgth==0 ) return;
  int x = cx;
  int y = cy+1;

  glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D,texId);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glColor3f(rC,gC,bC);

  if( loadMatrix ) {
    glMatrixMode( GL_PROJECTION );
    glLoadMatrixf(pMatrix);
    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();
  }

  int xcPos=x;
  float cH   = (float)cHeight / (float)fWidth;
  glBegin(GL_QUADS);
  for(int i=0;i<lgth;i++ ) {

    unsigned char  c = (unsigned char)text[i];
    float xPos = (float)((c % 16) * 16 + 1)/ (float)fWidth;
    float yPos = (float)((c / 16) * 16 )/ (float)fHeight;

    if(!isVariable) {

      float cW   = (float)cWidth / (float)fWidth;
      glTexCoord2f(xPos   ,yPos   );glVertex2i(xcPos       ,y   );
      glTexCoord2f(xPos+cW,yPos   );glVertex2i(xcPos+cWidth,y   );
      glTexCoord2f(xPos+cW,yPos+cH);glVertex2i(xcPos+cWidth,y+cHeight);
      glTexCoord2f(xPos   ,yPos+cH);glVertex2i(xcPos       ,y+cHeight);
      xcPos += cWidth;

    } else {

      float cW   = (float)cVarWidth[c] / (float)fWidth;
      glTexCoord2f(xPos   ,yPos   );glVertex2i(xcPos             ,y   );
      glTexCoord2f(xPos+cW,yPos   );glVertex2i(xcPos+cVarWidth[c],y   );
      glTexCoord2f(xPos+cW,yPos+cH);glVertex2i(xcPos+cVarWidth[c],y+cHeight);
      glTexCoord2f(xPos   ,yPos+cH);glVertex2i(xcPos             ,y+cHeight);
      xcPos += cVarWidth[c];

    }

  }
  glEnd();

#ifdef _DEBUG
  theApp->nbPoly+=lgth;
#endif

}

void GLFont2D::DrawTextFast(int cx,int cy,const char *text) {

  int lgth = (int)strlen(text);
  if( lgth==0 ) return;
  int x = cx;
  int y = cy+1;

  glBindTexture(GL_TEXTURE_2D,texId);
  glHint(GL_PERSPECTIVE_CORRECTION_HINT,GL_FASTEST);
  glColor3f(rC,gC,bC);

  int xcPos=x;
  float cH   = (float)cHeight / (float)fWidth;
  glBegin(GL_QUADS);
  for(int i=0;i<lgth;i++ ) {

    unsigned char  c = (unsigned char)text[i];
    float xPos = (float)((c % 16) * 16 + 1)/ (float)fWidth;
    float yPos = (float)((c / 16) * 16 )/ (float)fHeight;

    if(!isVariable) {

      float cW   = (float)cWidth / (float)fWidth;
      glTexCoord2f(xPos   ,yPos   );glVertex2i(xcPos       ,y   );
      glTexCoord2f(xPos+cW,yPos   );glVertex2i(xcPos+cWidth,y   );
      glTexCoord2f(xPos+cW,yPos+cH);glVertex2i(xcPos+cWidth,y+cHeight);
      glTexCoord2f(xPos   ,yPos+cH);glVertex2i(xcPos       ,y+cHeight);
      xcPos += cWidth;

    } else {

      float cW   = (float)cVarWidth[c] / (float)fWidth;
      glTexCoord2f(xPos   ,yPos   );glVertex2i(xcPos             ,y   );
      glTexCoord2f(xPos+cW,yPos   );glVertex2i(xcPos+cVarWidth[c],y   );
      glTexCoord2f(xPos+cW,yPos+cH);glVertex2i(xcPos+cVarWidth[c],y+cHeight);
      glTexCoord2f(xPos   ,yPos+cH);glVertex2i(xcPos             ,y+cHeight);
      xcPos += cVarWidth[c];

    }

  }
  glEnd();

#ifdef _DEBUG
  theApp->nbPoly+=lgth;
#endif

}

void GLFont2D::DrawTextV(int x,int y,char *text,bool loadMatrix) {

  int lgth = (int)strlen(text);
  if( lgth==0 ) return;

  glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D,texId);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glHint(GL_PERSPECTIVE_CORRECTION_HINT,GL_FASTEST);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_BLEND);
  glColor3f(rC,gC,bC);
  if( loadMatrix ) {
    glMatrixMode( GL_PROJECTION );
    glLoadMatrixf(pMatrix);
    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();
  }

  int ycPos=y;
  for(int i=0;i<lgth;i++ ) {

    char  c = text[i];
    float xPos = (float)((c % 16) * 16 + 1)/ (float)fWidth;
    float yPos = (float)((c / 16) * 16 )/ (float)fHeight;

    if(!isVariable) {

      float cW   = (float)cWidth / (float)fWidth;
      float cH   = (float)cHeight / (float)fWidth;
      glBegin(GL_QUADS);
      glTexCoord2f(xPos   ,yPos   );glVertex2i(x        ,ycPos       );
      glTexCoord2f(xPos+cW,yPos   );glVertex2i(x        ,ycPos-cWidth);
      glTexCoord2f(xPos+cW,yPos+cH);glVertex2i(x+cHeight,ycPos-cWidth);
      glTexCoord2f(xPos   ,yPos+cH);glVertex2i(x+cHeight,ycPos       );
      glEnd();
      ycPos += cWidth;

    } else {

      float cW   = (float)cVarWidth[c] / (float)fWidth;
      float cH   = (float)cHeight / (float)fWidth;
      glBegin(GL_QUADS);
      glTexCoord2f(xPos   ,yPos   );glVertex2i(x        ,ycPos             );
      glTexCoord2f(xPos+cW,yPos   );glVertex2i(x        ,ycPos-cVarWidth[c]);
      glTexCoord2f(xPos+cW,yPos+cH);glVertex2i(x+cHeight,ycPos-cVarWidth[c]);
      glTexCoord2f(xPos   ,yPos+cH);glVertex2i(x+cHeight,ycPos             );
      glEnd();
      ycPos -= cVarWidth[c];

    }

  }

#ifdef _DEBUG
  theApp->nbPoly+=lgth;
#endif

}
