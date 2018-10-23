// Copyright (c) 2011 rubicon IT GmbH
#include "GLToolkit.h"
#include "MathTools.h" //Min max saturate
#include "GLFont.h"
#include "GLApp.h"
#include "GLWindow.h"
#include "GLMatrix.h"
#include "GLSprite.h"
//#include "File.h"
#include <math.h>
//#include <malloc.h>
#define cimg_use_png 1
#include <CImg/CImg.h>
using namespace cimg_library;
#include <cstring> //strcpy, etc.
#ifdef MOLFLOW
#include "MolFlow.h"
#endif

#ifdef SYNRAD
#include "SynRad.h"
#endif

static GLFont2D *dlgFont  = NULL;
static GLFont2D *dlgFontB = NULL;
static int scrWidth;
static int scrHeight;

// dashed line
static GLushort dashDotPattern  = 0xE4E4;
static GLushort dotPattern      = 0x8888;
static GLushort dashPattern     = 0xF0F0;
static GLushort longDashPattern = 0xFF00;

// Cursors
static int currentCursor = -1;
static SDL_Cursor *defCursor = NULL;
static SDL_Cursor *busyCursor = NULL;
static SDL_Cursor *sizeCursor = NULL;
static SDL_Cursor *sizeHCursor = NULL;
static SDL_Cursor *sizeHSCursor = NULL;
static SDL_Cursor *sizeVCursor = NULL;
static SDL_Cursor *zoomCursor = NULL;
static SDL_Cursor *zoom2Cursor = NULL;
static SDL_Cursor *textCursor = NULL;
static SDL_Cursor *selAddCursor = NULL;
static SDL_Cursor *selDelCursor = NULL;
static SDL_Cursor *handCursor = NULL;
static SDL_Cursor *rotateCursor = NULL;
static SDL_Cursor *vertexCursor = NULL;
static SDL_Cursor *trajCursor = NULL;
static SDL_Cursor *vertexAddCursor = NULL;
static SDL_Cursor *vertexClrCursor = NULL;

// Texture for GUI components
static Sprite2D *compTex = NULL;

// Error log
#define MAX_LOG 256

static int nbLog=0;
static int logLength=0;
static char *logs[MAX_LOG];

extern GLApplication *theApp;

GLFont2D *GLToolkit::GetDialogFont() {
  return dlgFont;
}

GLFont2D *GLToolkit::GetDialogFontBold() {
  return dlgFontB;
}

void GLToolkit::InvalidateDeviceObjects() {

  if( dlgFont ) {
    dlgFont->InvalidateDeviceObjects();
    SAFE_DELETE(dlgFont);
  }
  if( dlgFontB ) {
    dlgFontB->InvalidateDeviceObjects();
    SAFE_DELETE(dlgFontB);
  }
  if( compTex ) {
    compTex->InvalidateDeviceObjects();
    SAFE_DELETE(compTex);
  }

}

void GLToolkit::CopyTextToClipboard(const std::string & text)
{
	SDL_SetClipboardText(text.c_str());
	/*
#ifdef _WIN32

	if (!OpenClipboard(NULL))
		return;

	EmptyClipboard();

	HGLOBAL hText = NULL;
	char   *lpszText;

	if (!(hText = GlobalAlloc(GMEM_ZEROINIT | GMEM_MOVEABLE, text.length() + 1))) {
		CloseClipboard();
		return;
	}
	if (!(lpszText = (char *)GlobalLock(hText))) {
		CloseClipboard();
		GlobalFree(hText);
		return;
	}

	strcpy(lpszText, text.c_str());
	SetClipboardData(CF_TEXT, hText);
	GlobalUnlock(hText);
	CloseClipboard();
	GlobalFree(hText);

#endif
*/
}

std::string GLToolkit::GetOSName() {
#ifdef _WIN32
	//define something for Windows (32-bit and 64-bit, this part is common)
#ifdef _WIN64
   //define something for Windows (64-bit only)
	return "Win64";
#else
   //define something for Windows (32-bit only)
	return "Win32";
#endif
#elif __APPLE__
#include "TargetConditionals.h"
#if TARGET_IPHONE_SIMULATOR
	// iOS Simulator
	return "iOs_sim";
#elif TARGET_OS_IPHONE
	// iOS device
	return "iOs";
#elif TARGET_OS_MAC
	// Other kinds of Mac OS
	return "Mac";
#else
	return "Apple_unknown";
#endif
#elif __linux__
	// linux
	return "Linux";
#elif __unix__ // all unices not caught above
	// Unix
	return "Unix";
#elif defined(_POSIX_VERSION)
	// POSIX
	return "Posix";
#else
	return "Unknown";
#endif
}

int GLToolkit::GetCursor() {
  return currentCursor;
}

void GLToolkit::SetCursor(int cursor) {

  if( cursor==currentCursor ) 
    return;

  currentCursor = cursor;
  switch(currentCursor) {
    case CURSOR_DEFAULT:
      SDL_SetCursor(defCursor);
      break;
	case CURSOR_BUSY:
	  SDL_SetCursor(busyCursor);
	  break;
    case CURSOR_SIZE:
      SDL_SetCursor(sizeCursor);
      break;
    case CURSOR_SIZEH:
      SDL_SetCursor(sizeHCursor);
      break;
    case CURSOR_SIZEHS:
      SDL_SetCursor(sizeHSCursor);
      break;
    case CURSOR_SIZEV:
      SDL_SetCursor(sizeVCursor);
      break;
    case CURSOR_ZOOM:
      SDL_SetCursor(zoomCursor);
      break;
    case CURSOR_ZOOM2:
      SDL_SetCursor(zoom2Cursor);
      break;
    case CURSOR_TEXT:
      SDL_SetCursor(textCursor);
      break;
    case CURSOR_SELADD:
      SDL_SetCursor(selAddCursor);
      break;
    case CURSOR_SELDEL:
      SDL_SetCursor(selDelCursor);
      break;
    case CURSOR_HAND:
      SDL_SetCursor(handCursor);
      break;
    case CURSOR_ROTATE:
      SDL_SetCursor(rotateCursor);
      break;
	case CURSOR_VERTEX:
      SDL_SetCursor(vertexCursor);
      break;
	case CURSOR_VERTEX_ADD:
      SDL_SetCursor(vertexAddCursor);
      break;
	case CURSOR_VERTEX_CLR:
      SDL_SetCursor(vertexClrCursor);
      break;
	case CURSOR_TRAJ:
      SDL_SetCursor(trajCursor);
      break;
  }

}

SDL_Cursor *InitCursor(const char *pngName,const int &tx,const int &ty) {

  int i, row, col;
  Uint8 data[4*32];
  Uint8 mask[4*32];
  memset(data,0,4*32);
  memset(mask,0,4*32);

  try{
  CImg<Uint8> img(pngName);
  //if( img.LoadCImage(pngName) ) {

    i = -1;
   // BYTE *image = img.data();
    for ( row=0; row<32; ++row ) {
      for ( col=0; col<32; ++col ) {
        if ( col % 8 ) {
          data[i] <<= 1;
          mask[i] <<= 1;
        } else {
          ++i;
        }
		
        switch (*(img.data(col,row))) {
          case 0:
            data[i] |= 0x01;
            mask[i] |= 0x01;
            break;
          case 0xFF:
            mask[i] |= 0x01;
            break;
        }
      }
    }
   // img.Release();
    return SDL_CreateCursor(data,mask,32,32,tx,ty);

  } catch (...) {
    return NULL;
  }

}

void GLToolkit::SetIcon32x32(const char *pngName) {

  //CImage img;
  //if( img.LoadCImage(pngName) ) {
	CImg<BYTE> img(pngName);
	SDL_Surface *s = SDL_CreateRGBSurface(SDL_SWSURFACE,32,32,24,0,0,0,0);
    SDL_LockSurface(s);
	for (unsigned int x = 0; x < 32; x++) {
		for (unsigned int y = 0; y < 32; y++) {
			((BYTE*)(s->pixels))[x*3 + y*3 * 32 + 0] = *(img.data(x, y, 0, 2));
			((BYTE*)(s->pixels))[x*3 + y*3 * 32 + 1] = *(img.data(x, y, 0, 1));
			((BYTE*)(s->pixels))[x*3 + y*3 * 32 + 2] = *(img.data(x, y, 0, 0));
		}
	}
    //memcpy(s->pixels , img.data() , 3*32*32);
	//memset(s->pixels, 0, 3 * 32 * 32);
    SDL_UnlockSurface(s);
    SDL_SetWindowIcon(theApp->mainScreen, s);
    //img.Release();
  //}

}

bool GLToolkit::RestoreDeviceObjects(const int &width,const int &height) {

  scrWidth = width;
  scrHeight = height;

  dlgFont = new GLFont2D("images/fnt_sansserif_8.png");
  dlgFont->SetTextSize(12,14);
  dlgFont->SetVariableWidth(true);
  if( !dlgFont->RestoreDeviceObjects(scrWidth,scrHeight) )
    return false;

  dlgFontB = new GLFont2D("images/fnt_sansserif_8b.png");
  dlgFontB->SetTextSize(12,14);
  dlgFontB->SetVariableWidth(true);
  if( !dlgFontB->RestoreDeviceObjects(scrWidth,scrHeight) )
    return false;

  if(!defCursor) defCursor = InitCursor("images/cursor_default.png",0,0);
  if(!busyCursor) busyCursor = InitCursor("images/cursor_busy.png",0,0);
  if(!sizeCursor) sizeCursor = InitCursor("images/cursor_resize.png",5,5);
  if(!sizeHCursor) sizeHCursor = InitCursor("images/cursor_resizeh.png",10,4);
  if(!sizeHSCursor) sizeHSCursor = InitCursor("images/cursor_resizehs.png",6,5);
  if(!sizeVCursor) sizeVCursor = InitCursor("images/cursor_resizev.png",4,10);
  if(!zoomCursor) zoomCursor = InitCursor("images/cursor_zoom.png",9,9);
  if(!zoom2Cursor) zoom2Cursor = InitCursor("images/cursor_zoom2.png",9,9);
  if(!textCursor) textCursor = InitCursor("images/cursor_text.png",3,7);
  if(!selAddCursor) selAddCursor = InitCursor("images/cursor_selp.png",0,0);
  if(!selDelCursor) selDelCursor = InitCursor("images/cursor_selm.png",0,0);
  if(!handCursor) handCursor = InitCursor("images/cursor_hand.png",9,9);
  if(!rotateCursor) rotateCursor = InitCursor("images/cursor_rotate.png",9,9);
  if(!vertexCursor) vertexCursor = InitCursor("images/cursor_vertex.png",0,0);
  if(!trajCursor) trajCursor = InitCursor("images/cursor_traj.png",0,0);
  if(!vertexAddCursor) vertexAddCursor = InitCursor("images/cursor_vertexp.png",0,0);
  if(!vertexClrCursor) vertexClrCursor = InitCursor("images/cursor_vertexm.png",0,0);

  compTex = new Sprite2D();
  if( !compTex->RestoreDeviceObjects("images/gui_background.png","images/gui_backgrounda.png",256,256) )
    return false;

  SetCursor(CURSOR_DEFAULT);
  return true;

}

void GLToolkit::Log(const char *message) {

  if( nbLog < MAX_LOG ) {
    logs[nbLog] = strdup(message);
    logLength += (int)strlen(logs[nbLog])+2;
    nbLog++;
  }

}

char *GLToolkit::GetLogs() {

  if( logLength ) {
    char *ret = (char *)malloc(logLength+512);
    strcpy(ret,"");
    for(int i=0;i<nbLog;i++) {
      strcat(ret,logs[i]);
      strcat(ret,"\n");
    }
    return ret;
  }

  return NULL;

}

void GLToolkit::ClearLogs() {

  for(int i=0;i<nbLog;i++)
    SAFE_FREE(logs[i]);
  logLength = 0;
  nbLog=0;

}

void GLToolkit::GetScreenSize(int *width,int *height) {
  *width = scrWidth;
  *height = scrHeight;
}

void GLToolkit::SetViewport(const GLVIEWPORT &v) {
  GLToolkit::SetViewport(v.x,v.y,v.width,v.height);
}

void GLToolkit::SetViewport(const int &x,const int &y,const int &width,const int &height) {

  int vy = scrHeight - (y+height);
  int vx = x;
  glViewport(vx,vy,width,height);

}

#define TW 256.0f

void GLToolkit::DrawButtonBack(const int &x,const int &y,const int &width,const int &height,const int &state) {

  int w1 = width / 2;
  int w2 = width - w1;

  float fw1 = (float)w1/TW;
  float fw2 = (float)w2/TW;
  float ft  = 21.0f / TW;

  if( !state ) compTex->SetColor(1.0f,1.0f,1.0f);
  else         compTex->SetColor(0.8f,0.8f,0.8f);

  // Left part
  compTex->UpdateSprite(x,y,x+w1,y+height);
  compTex->SetSpriteMapping(0.0f,0.0f,fw1,ft);
  compTex->Render(false);

  // Right part
  compTex->UpdateSprite(x+w1,y,x+width,y+height);
  compTex->SetSpriteMapping(1.0f-fw2,0.0f,1.0f,ft);
  compTex->Render(false);

}

void GLToolkit::DrawTinyButton(int x,int y,int state) {

  if( !state ) compTex->SetColor(1.0f,1.0f,1.0f);
  else         compTex->SetColor(0.8f,0.8f,0.8f);

  compTex->UpdateSprite(x,y,x+12,y+12);
  compTex->SetSpriteMapping(0.0f,95.0f/TW,12.0f/TW,107.0f/TW);
  compTex->Render(false);

}

void GLToolkit::DrawSmallButton(int x,int y,int state) {

  if( !state ) compTex->SetColor(1.0f,1.0f,1.0f);
  else         compTex->SetColor(0.8f,0.8f,0.8f);

  compTex->UpdateSprite(x,y,x+15,y+17);
  compTex->SetSpriteMapping(0.0f,24.0f/TW,15.0f/TW,41.0f/TW);
  compTex->Render(false);

}

void GLToolkit::DrawToggle(int x,int y) {

  compTex->SetColor(1.0f,1.0f,1.0f);
  compTex->UpdateSprite(x,y,x+12,y+12);
  compTex->SetSpriteMapping(17.0f/TW,24.0f/TW,29.0f/TW,36.0f/TW);
  compTex->Render(false);

}

void GLToolkit::DrawBar(int x,int y,int width,int height) {

  compTex->SetColor(1.0f,1.0f,1.0f);
  compTex->UpdateSprite(x,y,x+width,y+height);
  compTex->SetSpriteMapping(0.0f,44.0f/TW,1.0f,51.0f/TW);
  compTex->Render(false);

}

void GLToolkit::DrawTextBack(const int &x,const int &y,const int &width,const int &height,
							 const int &rBack, const int &gBack, const int &bBack) {

  int w1 = width / 2;
  int w2 = width - w1;

  float fw1 = (float)w1/TW;
  float fw2 = (float)w2/TW;
  float ft  = 21.0f / TW;

  compTex->SetColor(rBack/240.0f,gBack/240.0f,bBack/240.0f);

  // Left part
  compTex->UpdateSprite(x,y,x+w1,y+height);
  compTex->SetSpriteMapping(0.0f,54.0f/TW,fw1,75.0f/TW);
  compTex->Render(false);

  // Right part
  compTex->UpdateSprite(x+w1,y,x+width,y+height);
  compTex->SetSpriteMapping(1.0f-fw2,54.0f/TW,1.0f,75.0f/TW);
  compTex->Render(false);

}

void GLToolkit::DrawVGradientBox(int x,int y,int width,int height,bool shadow,bool iShadow,bool isEtched) {

  compTex->SetColor(1.0f,1.0f,1.0f);
  compTex->UpdateSprite(x,y,x+width,y+height);
  compTex->SetSpriteMapping(0.1f,3.0f/TW,0.5f,17.0f/TW);
  compTex->Render(true);

  glDisable(GL_TEXTURE_2D);
  glDisable(GL_BLEND);
  glLineWidth(1.0f);
  DrawBorder(x,y,width,height,shadow,iShadow,isEtched);

}

void GLToolkit::DrawHGradientBox(int x,int y,int width,int height,bool shadow,bool iShadow,bool isEtched) {

  compTex->SetColor(1.0f,1.0f,1.0f);
  compTex->UpdateSprite(x,y,x+width,y+height);
  compTex->SetSpriteMapping(32.0f/TW,25.0f/TW,45.0f/TW,35.0f/TW);
  compTex->Render(true);

  glDisable(GL_TEXTURE_2D);
  glDisable(GL_BLEND);
  glLineWidth(1.0f);
  DrawBorder(x,y,width,height,shadow,iShadow,isEtched);

}

void GLToolkit::DrawHScroll(int x,int y,int width,int height,int state) {

  int w1 = width / 2;
  int w2 = width - w1;
  if( w1>200 ) {
    w1 = 200;
    w2 = 200;
  }
  int left = width-(w1+w2);
  int nbStep = left/200 + ((left%200)?1:0);

  float fw1 = (float)w1/TW;
  float fw2 = (float)w2/TW;
  float ftu  = 109.0f / TW;
  float ftd  = 121.0f / TW;

  switch(state) {

    case 0:
     compTex->SetColor(1.0f,1.0f,1.0f);
     break;
    case 1:
     compTex->SetColor(0.9f,0.9f,0.9f);
     break;
    case 2:
     ftu  = 123.0f / TW;
     ftd  = 135.0f / TW;
     compTex->SetColor(1.0f,1.0f,1.0f);
     break;

  }

  // First part
  compTex->UpdateSprite(x,y,x+w1,y+height);
  compTex->SetSpriteMapping(0.0f,ftu,fw1,ftd);
  compTex->Render(false);

  // Middle parts
  compTex->SetSpriteMapping(0.1f,ftu,0.9f,ftd);
  for(int i=0;i<left;i+=200) {
    int sleft = Min(left-i,200);
    compTex->UpdateSprite(x+w1+i,y,x+w1+i+sleft,y+height);
    compTex->Render(false);
  }

  // Last part
  compTex->UpdateSprite(x+w1+left,y,x+width,y+height);
  compTex->SetSpriteMapping(1.0f-fw2,ftu,1.0f,ftd);
  compTex->Render(false);

}

void GLToolkit::DrawVScroll(int x,int y,int width,int height,int state) {

  int h1 = height / 2;
  int h2 = height - h1;
  if( h1>200 ) {
    h1 = 200;
    h2 = 200;
  }
  int left = height-(h1+h2);
  int nbStep = left/200 + ((left%200)?1:0);

  float fw1 = (float)h1/TW;
  float fw2 = (float)h2/TW;
  float ftu  = 109.0f / TW;
  float ftd  = 121.0f / TW;

  switch(state) {

    case 0:
     compTex->SetColor(1.0f,1.0f,1.0f);
     break;
    case 1:
     compTex->SetColor(0.9f,0.9f,0.9f);
     break;
    case 2:
     ftu  = 123.0f / TW;
     ftd  = 135.0f / TW;
     compTex->SetColor(1.0f,1.0f,1.0f);
     break;

  }

  // First part
  compTex->UpdateSprite(x,y,x+width,y+h1);
  compTex->SetSpriteMapping(0.0f,ftu,fw1,ftd);
  compTex->Render90(false);

  // Middle parts
  compTex->SetSpriteMapping(0.1f,ftu,0.9f,ftd);
  for(int i=0;i<left;i+=200) {
    int sleft = Min(left-i,200);
    compTex->UpdateSprite(x,y+h1+i,x+width,y+h1+i+sleft);
    compTex->Render90(false);
  }

  // Last part
  compTex->UpdateSprite(x,y+h1+left,x+width,y+height);
  compTex->SetSpriteMapping(1.0f-fw2,ftu,1.0f,ftd);
  compTex->Render90(false);

}

void GLToolkit::DrawHIGradientBox(int x,int y,int width,int height,bool shadow,bool iShadow,bool isEtched) {

  compTex->SetColor(1.0f,1.0f,1.0f);
  compTex->UpdateSprite(x,y,x+width,y+height);
  compTex->SetSpriteMapping(49.0f/TW,25.0f/TW,62.0f/TW,35.0f/TW);
  compTex->Render(true);

  glDisable(GL_TEXTURE_2D);
  glDisable(GL_BLEND);
  glLineWidth(1.0f);
  DrawBorder(x,y,width,height,shadow,iShadow,isEtched);

}

void GLToolkit::Draw16x16(int x,int y,int xt,int yt) {

  compTex->SetColor(1.0f,1.0f,1.0f);
  compTex->UpdateSprite(x,y,x+16,y+16);
  compTex->SetSpriteMapping((float)xt/TW,(float)yt/TW,(float)(xt+16)/TW,(float)(yt+16)/TW);
  compTex->Render(false);

}

void GLToolkit::DrawLumBitmap(int x,int y,int width,int height,BYTE *buffer) {

  glDisable(GL_TEXTURE_2D);
  glEnable(GL_BLEND);
  glRasterPos2i(x-1,y+1);
  glDrawPixels(width,height,GL_LUMINANCE_ALPHA,GL_UNSIGNED_BYTE,buffer);

}

void GLToolkit::DrawBorder(const int &x,const int &y,const int &width,
						   const int &height,const bool &shadow,const bool &iShadow,const bool &isEtched) {

  float rL = 1.0f;
  float gL = 1.0f;
  float bL = 1.0f;
  float rD = 0.5f;
  float gD = 0.5f;
  float bD = 0.5f;

  if( shadow ) {
  
    // Shadow colors
    /* 
    float rL = (float)r / 100.0f;
    float gL = (float)g / 100.0f;
    float bL = (float)b / 100.0f;
    Saturate(rL,0.0f,1.0f);
    Saturate(gL,0.0f,1.0f);
    Saturate(bL,0.0f,1.0f);

    float rD = (float)r / 500.0f;
    float gD = (float)g / 500.0f;
    float bD = (float)b / 500.0f;
    Saturate(rD,0.0f,1.0f);
    Saturate(gD,0.0f,1.0f);
    Saturate(bD,0.0f,1.0f);
    */

    if(!iShadow) glColor3f(rL,gL,bL);
    else         glColor3f(rD,gD,bD);
    glBegin(GL_LINES);
    _glVertex2i(x,y);
    _glVertex2i(x+width,y);
    _glVertex2i(x,y);
    _glVertex2i(x,y+height);
    glEnd();

    if(iShadow) glColor3f(rL,gL,bL);
    else        glColor3f(rD,gD,bD);
    glBegin(GL_LINES);
    _glVertex2i(x+width,y+height);
    _glVertex2i(x-1    ,y+height);
    _glVertex2i(x+width,y+height);
    _glVertex2i(x+width,y);
    glEnd();

    #ifdef _DEBUG
      theApp->nbLine+=4;
    #endif

  } else if( isEtched ) {

    // Etched colors
    /*
    float rL = (float)r / 100.0f;
    float gL = (float)g / 100.0f;
    float bL = (float)b / 100.0f;
    Saturate(rL,0.0f,1.0f);
    Saturate(gL,0.0f,1.0f);
    Saturate(bL,0.0f,1.0f);

    float rD = (float)r / 500.0f;
    float gD = (float)g / 500.0f;
    float bD = (float)b / 500.0f;
    Saturate(rD,0.0f,1.0f);
    Saturate(gD,0.0f,1.0f);
    Saturate(bD,0.0f,1.0f);
    */

    glColor3f(rD,gD,bD);

    glBegin(GL_LINES);
    _glVertex2i(x,y);
    _glVertex2i(x+width,y);
    _glVertex2i(x,y);
    _glVertex2i(x,y+height);
    _glVertex2i(x+width-1,y+height-1);
    _glVertex2i(x+width-1,y);
    _glVertex2i(x+width-1,y+height-1);
    _glVertex2i(x,y+height-1);
    glEnd();

    glColor3f(rL,gL,bL);

    glBegin(GL_LINES);
    _glVertex2i(x+1,y+1);
    _glVertex2i(x+width,y+1);
    _glVertex2i(x+1,y+1);
    _glVertex2i(x+1,y+height);
    _glVertex2i(x+width,y+height);
    _glVertex2i(x+width,y);
    _glVertex2i(x+width,y+height);
    _glVertex2i(x,y+height);
    glEnd();

    #ifdef _DEBUG
      theApp->nbLine+=4;
    #endif

  }

}

void GLToolkit::DrawBox(const int &x,const int &y,const int &width,const int &height,
	  const int &r,const int &g,const int &b,const bool &shadow,const bool &iShadow,
	  const bool &isEtched) {

  float rN = (float)r / 255.0f;
  float gN = (float)g / 255.0f;
  float bN = (float)b / 255.0f;
  Saturate(rN,0.0f,1.0f);
  Saturate(gN,0.0f,1.0f);
  Saturate(bN,0.0f,1.0f);

  glDisable(GL_TEXTURE_2D);
  glDisable(GL_BLEND);
  glLineWidth(1.0f);
  glColor3f(rN,gN,bN);

  glBegin(GL_QUADS);
  
  glVertex2i(x,y);
  glVertex2i(x+width,y);
  
  glVertex2i(x+width,y+height);
  glVertex2i(x,y+height);
  glEnd();

#ifdef _DEBUG
  theApp->nbPoly++;
#endif

  DrawBorder(x,y,width,height,shadow,iShadow,isEtched);

}

void GLToolkit::DrawPoly(int lineWidth,int dashStyle,int r,int g,int b,
                         int nbPoint,int *pointX,int *pointY) {

  // Draw unclosed polygon
  if( lineWidth==0 ) return;

  glDisable(GL_TEXTURE_2D);
  glDisable(GL_CULL_FACE);
  glDisable(GL_LIGHTING);
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_BLEND);
  glLineWidth((float)lineWidth);

  float rN = (float)r / 255.0f;
  float gN = (float)g / 255.0f;
  float bN = (float)b / 255.0f;
  Saturate(rN,0.0f,1.0f);
  Saturate(gN,0.0f,1.0f);
  Saturate(bN,0.0f,1.0f);

  if( dashStyle==DASHSTYLE_NONE ) {
    glDisable(GL_LINE_STIPPLE);
  } else {
    glEnable(GL_LINE_STIPPLE);
    switch(dashStyle) {
      case DASHSTYLE_DOT:
        glLineStipple(1,dotPattern);
        break;
      case DASHSTYLE_DASH:
        glLineStipple(1,dashPattern);
        break;
      case DASHSTYLE_LONG_DASH:
        glLineStipple(1,longDashPattern);
        break;
      case DASHSTYLE_DASH_DOT:
        glLineStipple(1,dashDotPattern);
        break;
    }
  }
  glColor3f(rN,gN,bN);

  glBegin(GL_LINE_STRIP);
  for(int i=0;i<nbPoint;i++)
    _glVertex2i(pointX[i],pointY[i]);
  glEnd();

  glDisable(GL_LINE_STIPPLE);

}

std::optional<std::tuple<int, int>> GLToolkit::Get2DScreenCoord(const Vector3d& p){

  GLfloat mProj[16];
  GLfloat mView[16];
  GLVIEWPORT g;

  // Compute location on screen
  glGetFloatv(GL_PROJECTION_MATRIX , mProj);
  glGetFloatv(GL_MODELVIEW_MATRIX , mView);
  glGetIntegerv(GL_VIEWPORT,(GLint *)&g);

  GLMatrix proj; proj.LoadGL(mProj);
  GLMatrix view; view.LoadGL(mView);
  GLMatrix m; m.Multiply(&proj,&view);

  float rx,ry,rz,rw;
  m.TransfomVec((float)p.x, (float)p.y, (float)p.z,1.0f,&rx,&ry,&rz,&rw);
  if(rw<=0.0f) return std::nullopt;

  return std::make_tuple( (int)(((rx / rw) + 1.0f)  * (float)g.width / 2.0f),
  (int)(((-ry / rw) + 1.0f) * (float)g.height / 2.0f) );

}

float GLToolkit::GetVisibility(double x,double y,double z,double nx,double ny,double nz) {

  GLfloat mView[16];
  float rx,ry,rz,rw;
  float ntx,nty,ntz,ntw;

  glGetFloatv( GL_MODELVIEW_MATRIX , mView );
  GLMatrix view; view.LoadGL(mView);
  view.TransfomVec((float)x,(float)y,(float)z,1.0f,&rx,&ry,&rz,&rw);
  view.TransfomVec((float)nx,(float)ny,(float)nz,0.0f,&ntx,&nty,&ntz,&ntw);
  return rx*ntx + ry*nty + rz*ntz;

}

float GLToolkit::GetCamDistance(GLfloat *mView,double x,double y,double z) {

  float rx,ry,rz,rw;
  GLMatrix view; view.LoadGL(mView);
  view.TransfomVec((float)x,(float)y,(float)z,1.0f,&rx,&ry,&rz,&rw);
  return sqrtf(rx*rx + ry*ry + rz*rz);

}

static GLMatrix   dsm;
static GLfloat    dsmProj[16];
static GLfloat    dsmView[16];
static GLVIEWPORT dsg;

void GLToolkit::DrawStringInit() {

  glGetFloatv( GL_PROJECTION_MATRIX , dsmProj );
  glGetFloatv( GL_MODELVIEW_MATRIX , dsmView );
  glGetIntegerv(GL_VIEWPORT,(GLint *)&dsg);

  GLMatrix proj; proj.LoadGL(dsmProj);
  GLMatrix view; view.LoadGL(dsmView);
  dsm.Multiply(&proj,&view);

  glMatrixMode( GL_PROJECTION );
  glLoadIdentity();
  glOrtho(dsg.x,dsg.x+dsg.width,dsg.y+dsg.height,dsg.y,-1.0,1.0);
  glMatrixMode( GL_MODELVIEW );
  glLoadIdentity();

  glEnable(GL_TEXTURE_2D);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

}

void GLToolkit::DrawString(float x,float y,float z,const char *str,GLFont2D *fnt,int offx,int offy) {

  // Compute location on screen
  float rx,ry,rz,rw;
  dsm.TransfomVec(x,y,z,1.0f,&rx,&ry,&rz,&rw);
  if(rw<0.0f) return;
  int xe = dsg.x +(int)(((rx / rw) + 1.0f) * (float)dsg.width/2.0f);
  int ye = dsg.y +(int)(((-ry / rw) + 1.0f) * (float)dsg.height/2.0f);

  fnt->DrawTextFast(xe+offx,ye+offy,str);

}

void GLToolkit::DrawStringRestore() {

  // restore transform matrix
  glMatrixMode( GL_PROJECTION );
  glLoadMatrixf(dsmProj);
  glMatrixMode( GL_MODELVIEW );
  glLoadMatrixf(dsmView);

}

void GLToolkit::DrawRule(double length,bool invertX,bool invertY,bool invertZ,double n) {

  DrawVector(0.0,0.0,0.0,(invertX)?-length:length,0.0,0.0,n);
  DrawVector(0.0,0.0,0.0,0.0,(invertY)?-length:length,0.0,n);
  DrawVector(0.0,0.0,0.0,0.0,0.0,(invertZ)?-length:length,n);
  glPointSize(4.0f);
  glBegin(GL_POINTS);
  glVertex3d(0.0,0.0,0.0);
  glEnd();

}

void GLToolkit::DrawVector(double x1,double y1,double z1,double x2,double y2,double z2,double nr) {

  double n = sqrt( (x2-x1)*(x2-x1) + (y2-y1)*(y2-y1) + (z2-z1)*(z2-z1) )*nr;
  double nx;
  double ny;
  double nz;

  // Choose a normal vector
  if( std::abs(x2-x1) > 1e-3 ) {
    // Oy
    nx = (z2-z1)/n;
    ny = (y2-y1)/n;
    nz = (x1-x2)/n;
  } else if( std::abs(y2-y1) > 1e-3 ) {
    // Oz
    nx = (y2-y1)/n;
    ny = (x1-x2)/n;
    nz = (z2-z1)/n;
  } else {
    // Ox
    nx = (x2-x1)/n;
    ny = (z2-z1)/n;
    nz = (y1-y2)/n;
  }

  glDisable(GL_TEXTURE_2D);
  glDisable(GL_LIGHTING);
  glDisable(GL_BLEND);

  glDisable(GL_CULL_FACE);

 glBegin(GL_LINES);
  glVertex3d(x1,y1,z1);
  glVertex3d(x2,y2,z2);

  if( n>3.0 ) {
    glVertex3d(x2,y2,z2);
    glVertex3d(x2+nx-(x2-x1)/n, y2+ny-(y2-y1)/n, z2+nz-(z2-z1)/n);

    glVertex3d(x2,y2,z2);
    glVertex3d(x2-nx-(x2-x1)/n, y2-ny-(y2-y1)/n ,z2-nz-(z2-z1)/n);

    glVertex3d(x2+nx-(x2-x1)/n, y2+ny-(y2-y1)/n, z2+nz-(z2-z1)/n);
    glVertex3d(x2-nx-(x2-x1)/n, y2-ny-(y2-y1)/n ,z2-nz-(z2-z1)/n);
  }

  glEnd();

}

void GLToolkit::PerspectiveLH(double fovy,double aspect,double zNear,double zFar) {

  // Create and load a left handed proj matrix
  double fov = (fovy/360.0) * (2.0*PI);
  double f = cos(fov/2.0) / sin(fov/2.0);

  GLfloat mProj[16];

  mProj[0] = (float)(f/aspect); mProj[1] = 0.0f; mProj[2] = 0.0f; mProj[3] = 0.0f;
  mProj[4] = 0.0f; mProj[5] = (float)(f); mProj[6] = 0.0f; mProj[7] = 0.0f;
  mProj[8] = 0.0f; mProj[9] = 0.0f; mProj[10] = (float)((zFar+zNear)/(zFar-zNear)); mProj[11] = 1.0f;
  mProj[12] = 0.0f; mProj[13] = 0.0f; mProj[14] = (float)((2.0*zNear*zFar)/(zNear-zFar)) ; mProj[15] = 0.0f;

  glLoadMatrixf(mProj);

}

void GLToolkit::LookAt(const Vector3d& Eye, const Vector3d& camPos, const Vector3d& Up, const double& handedness) {
	//handedness =  -1: left handed
	// Create and load a left- or right-handed view matrix

	Vector3d Z = (camPos - Eye).Normalized();
	Vector3d X = -handedness * CrossProduct(Up, Z).Normalized();
	Vector3d Y = -handedness * CrossProduct(Z, X);
  

	double dotXE = Dot(Eye,X);
	double dotYE = Dot(Eye, Y);
	double dotZE = Dot(Eye, Z);

	float glViewMatrix[] = 
						{ (float) X.x,   (float) Y.x,   (float) Z.x,   0.0f,
						  (float) X.y,   (float) Y.y,   (float) Z.y,   0.0f,
						  (float) X.z,   (float) Y.z,   (float) Z.z,   0.0f,
						  (float)-dotXE, (float)-dotYE, (float)-dotZE, 1.0f };

	glLoadMatrixf(glViewMatrix);
                             
}

void GLToolkit::SetMaterial(GLMATERIAL *mat) {

  float acolor[] = { mat->Ambient.r, mat->Ambient.g, mat->Ambient.b, mat->Ambient.a };
  glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, acolor);
  float dcolor[] = { mat->Diffuse.r, mat->Diffuse.g, mat->Diffuse.b, mat->Diffuse.a };
  glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, dcolor);
  float scolor[] = { mat->Specular.r, mat->Specular.g, mat->Specular.b, mat->Specular.a };
  glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, scolor);
  float ecolor[] = { mat->Emissive.r, mat->Emissive.g, mat->Emissive.b, mat->Emissive.a };
  glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, ecolor);
  glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, mat->Power);
  glColor4f(mat->Ambient.r, mat->Ambient.g, mat->Ambient.b, mat->Ambient.a);

}

void GLToolkit::printGlError(GLenum glError) {

  switch( glError ) {
    case GL_INVALID_ENUM:
      Log("OpenGL failure: An unacceptable value is specified for an enumerated argument. The offending function is ignored, having no side effect other than to set the error flag.\n");
      break;
    case GL_INVALID_VALUE:
      Log("OpenGL failure: A numeric argument is out of range. The offending function is ignored, having no side effect other than to set the error flag.\n");
      break;
    case GL_INVALID_OPERATION:
      Log("OpenGL failure: The specified operation is not allowed in the current state. The offending function is ignored, having no side effect other than to set the error flag.\n");
      break;
    case GL_STACK_OVERFLOW:
      Log("OpenGL failure: This function would cause a stack overflow. The offending function is ignored, having no side effect other than to set the error flag.\n");
      break;
    case GL_STACK_UNDERFLOW:
      Log("OpenGL failure: This function would cause a stack underflow. The offending function is ignored, having no side effect other than to set the error flag.\n");
      break;
    case GL_OUT_OF_MEMORY:
      Log("OpenGL failure: There is not enough memory left to execute the function. The state of OpenGL is undefined, except for the state of the error flags, after this error is recorded.\n");
      break;
  }
}

void GLToolkit::CheckGLErrors(const char *compname) {
	static bool savedOnGLError=false;   
	GLenum glError = glGetError();
       if( glError != GL_NO_ERROR ) { 
		   char tmp[256];
		   sprintf(tmp,"OpenGL: Error painting %s.",compname);
         GLToolkit::Log(tmp);
         GLToolkit::printGlError(glError); 
         //Exit();
		 DEBUG_BREAK;
		 /*if (!savedOnGLError) {
			 extern SynRad*mApp;
			 mApp->AutoSave();
			 savedOnGLError = true;
		 }*/
		 throw Error(tmp);
       }
}