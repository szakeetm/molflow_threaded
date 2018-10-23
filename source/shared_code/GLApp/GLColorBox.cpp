// Copyright (c) 2011 rubicon IT GmbH
#include "GLWindow.h"
#include "GLLabel.h"
#include "GLTitledPanel.h"
#include "GLTextField.h"
#include "GLColorBox.h"
#include "GLButton.h"
#include "GLToolkit.h"
#include "GLApp.h"
#include "MathTools.h" //Saturate
//#include <malloc.h>
#define cimg_use_png 1
#include <CImg/CImg.h>
using namespace cimg_library;

const size_t sColors[] = {
  0xFFFFFF,0xCCFFFF,0xCCCCFF,0xCCCCFF,0xCCCCFF,0xCCCCFF,0xCCCCFF,0xCCCCFF,0xCCCCFF,0xCCCCFF,0xCCCCFF,0xFFCCFF,0xFFCCCC,0xFFCCCC,0xFFCCCC,0xFFCCCC,0xFFCCCC,0xFFCCCC,0xFFCCCC,0xFFCCCC,0xFFCCCC,0xFFFFCC,0xCCFFCC,0xCCFFCC,0xCCFFCC,0xCCFFCC,0xCCFFCC,0xCCFFCC,0xCCFFCC,0xCCFFCC,0xCCFFCC,
  0xCCCCCC,0x99FFFF,0x99CCFF,0x9999FF,0x9999FF,0x9999FF,0x9999FF,0x9999FF,0x9999FF,0x9999FF,0xCC99FF,0xFF99FF,0xFF99CC,0xFF9999,0xFF9999,0xFF9999,0xFF9999,0xFF9999,0xFF9999,0xFF9999,0xFFCC99,0xFFFF99,0xCCFF99,0x99FF99,0x99FF99,0x99FF99,0x99FF99,0x99FF99,0x99FF99,0x99FF99,0x99FFCC,
  0xCCCCCC,0x66FFFF,0x66CCFF,0x6699FF,0x6666FF,0x6666FF,0x6666FF,0x6666FF,0x6666FF,0x9966FF,0xCC66FF,0xFF66FF,0xFF66CC,0xFF6699,0xFF6666,0xFF6666,0xFF6666,0xFF6666,0xFF6666,0xFF9966,0xFFCC66,0xFFFF66,0xCCFF66,0x99FF66,0x66FF66,0x66FF66,0x66FF66,0x66FF66,0x66FF66,0x66FF99,0x66FFCC,
  0x999999,0x33FFFF,0x33CCFF,0x3399FF,0x3366FF,0x3333FF,0x3333FF,0x3333FF,0x6633FF,0x9933FF,0xCC33FF,0xFF33FF,0xFF33CC,0xFF3399,0xFF3366,0xFF3333,0xFF3333,0xFF3333,0xFF6633,0xFF9933,0xFFCC33,0xFFFF33,0xCCFF33,0x99F433,0x66FF33,0x33FF33,0x33FF33,0x33FF33,0x33FF66,0x33FF99,0x33FFCC,
  0x999999,0x00FFFF,0x00CCFF,0x0099FF,0x0066FF,0x0033FF,0x0000FF,0x3300FF,0x6600FF,0x9900FF,0xCC00FF,0xFF00FF,0xFF00CC,0xFF0099,0xFF0066,0xFF0033,0xFF0000,0xFF3300,0xFF6600,0xFF9900,0xFFCC00,0xFFFF00,0xCCFF00,0x99FF00,0x66FF00,0x33FF00,0x00FF00,0x00FF33,0x00FF66,0x00FF99,0x00FFCC,
  0x666666,0x00CCCC,0x00CCCC,0x0099CC,0x0066CC,0x0033CC,0x0000CC,0x3300CC,0x6600CC,0x9900CC,0xCC00CC,0xCC00CC,0xCC00CC,0xCC0099,0xCC0066,0xCC0033,0xCC0000,0xCC3300,0xCC6600,0xCC9900,0xCCCC00,0xCCCC00,0xCCCC00,0x99CC00,0x66CC00,0x33CC00,0x00CC00,0x00CC33,0x00CC66,0x00CC99,0x00CCCC,
  0x666666,0x009999,0x009999,0x009999,0x006699,0x003399,0x000099,0x330099,0x660099,0x990099,0x990099,0x990099,0x990099,0x990099,0x990066,0x990033,0x990000,0x993300,0x996600,0x999900,0x999900,0x999900,0x999900,0x999900,0x669900,0x339900,0x009900,0x009933,0x009966,0x009999,0x009999,
  0x333333,0x006666,0x006666,0x006666,0x006666,0x003366,0x000066,0x330066,0x660066,0x660066,0x660066,0x660066,0x660066,0x660066,0x660066,0x660033,0x660000,0x663300,0x666600,0x666600,0x666600,0x666600,0x666600,0x666600,0x666600,0x336600,0x006600,0x006633,0x006666,0x006666,0x006666,
  0x000000,0x003333,0x003333,0x003333,0x003333,0x003333,0x000033,0x330033,0x330033,0x330033,0x330033,0x330033,0x330033,0x330033,0x330033,0x330033,0x330000,0x333300,0x333300,0x333300,0x333300,0x333300,0x333300,0x333300,0x333300,0x003300,0x003333,0x003333,0x003333,0x003333,0x333333 
};

extern GLApplication *theApp;

// Construct a message dialog box
GLColorBox::GLColorBox(const char *title,int *r,int *g,int *b):GLWindow() {

  int wD = 370;
  int hD = 350;

  if(title) SetTitle(title);
  else      SetTitle("Choose color");

  swPanel = new GLTitledPanel("Swatches");
  swPanel->SetBounds(5,hD-175,wD-10,127);
  Add(swPanel);

  swBox = new GLLabel("");
  swBox->SetOpaque(false);
  swBox->SetBounds(15,hD-155,340,98);
  Add(swBox);

  hsvPanel = new GLTitledPanel("H.S.V.");
  hsvPanel->SetBounds(5,10,225,160);
  Add(hsvPanel);

  hsBox = new GLLabel("");
  hsBox->SetOpaque(false);
  hsBox->SetBounds(15,30,128,128);
  Add(hsBox);

  vBox = new GLLabel("");
  vBox->SetOpaque(false);
  vBox->SetBounds(170,30,45,128);
  Add(vBox);

  GLTitledPanel *rgbPanel = new GLTitledPanel("R.G.B.");
  rgbPanel->SetBounds(240,10,125,160);
  Add(rgbPanel);

  GLLabel *l1 = new GLLabel("Red");
  l1->SetBounds(250,30,50,19);
  Add(l1);
  rText = new GLTextField(0,"");
  rText->SetEditable(true);
  rText->SetBounds(305,30,50,19);
  Add(rText);

  GLLabel *l2 = new GLLabel("Green");
  l2->SetBounds(250,55,50,19);
  Add(l2);
  gText = new GLTextField(0,"");
  gText->SetEditable(true);
  gText->SetBounds(305,55,50,19);
  Add(gText);

  GLLabel *l3 = new GLLabel("Blue");
  l3->SetBounds(250,80,50,19);
  Add(l3);
  bText = new GLTextField(0,"");
  bText->SetEditable(true);
  bText->SetBounds(305,80,50,19);
  Add(bText);

  GLLabel *l4 = new GLLabel("Old");
  l4->SetBounds(250,110,50,19);
  Add(l4);

  oldColor = new GLLabel("");
  oldColor->SetOpaque(true);
  oldColor->SetBorder(BORDER_NONE);
  oldColor->SetBackgroundColor(*r,*g,*b);
  oldColor->SetBounds(305,110,50,20);
  Add(oldColor);

  GLLabel *l5 = new GLLabel("New");
  l5->SetBounds(250,130,50,19);
  Add(l5);

  newColor = new GLLabel("");
  newColor->SetOpaque(true);
  newColor->SetBorder(BORDER_NONE);
  newColor->SetBounds(305,130,50,20);
  Add(newColor);

  GLButton *okButton = new GLButton(1,"OK");
  okButton->SetBounds(wD-200,hD-43,95,19);
  Add(okButton);

  GLButton *cancelButton = new GLButton(0,"Cancel");
  cancelButton->SetBounds(wD-100,hD-43,95,19);
  Add(cancelButton);

  updateColor(*r,*g,*b);
  draggV = false;

  // Center dialog
  int wS,hS;
  GLToolkit::GetScreenSize(&wS,&hS);
  if( wD > wS ) wD = wS;
  int xD = (wS-wD)/2;
  int yD = (hS-hD)/2;
  SetBounds(xD,yD,wD,hD);

  // Create objects
  RestoreDeviceObjects();

  rCode = 0;

}

GLColorBox::~GLColorBox() {
  InvalidateDeviceObjects();
}

void GLColorBox::InvalidateDeviceObjects() {

  DELETE_TEX(hsvTex);
  DELETE_TEX(sliderTex);
  GLWindow::InvalidateDeviceObjects();

}

void GLColorBox::RestoreDeviceObjects() {

  // HSV texture
  
  size_t *buff32 = (size_t *)malloc( 128*128*sizeof(size_t) );
  for(int i=0;i<128;i++) {
    for(int j=0;j<128;j++) {
      float h = ((float)i / 128.0f) * 360.0f;
      float s = (float)j / 128.0f;
      float v = 0.9f;
      buff32[i+j*128]=hsv_to_rgb(h,s,v,true);
    }
  }

  glGenTextures(1,&hsvTex);
  glBindTexture(GL_TEXTURE_2D,hsvTex);
  glTexImage2D (
    GL_TEXTURE_2D,      // Type
    0,                  // No Mipmap
    4,                  // Format RGBA
    128,                // Width
    128,                // Height
    0,                  // Border
    GL_RGBA,            // Format RGBA
    GL_UNSIGNED_BYTE,   // 8 Bit/color
    buff32              // Data
  );   

  free(buff32);

  // Slider texture  
  //CImage img;
  //if( img.LoadCImage("images/icon_slider.png") ) {
  CImg<BYTE> img("images/icon_slider.png");
  {
	  BYTE *buff32 = (BYTE *)malloc(16 * 16 * 4);
	  //BYTE *data = img.data();
	  for (int y = 0; y < 16; y++) {
		  for (int x = 0; x < 16; x++) {
			  buff32[x * 4 + 0 + y * 4 * 16] = /*data[x * 3 + 2 + y * 3 * 16]*/*(img.data(x,y,0,0));
			  buff32[x * 4 + 1 + y * 4 * 16] = /*data[x * 3 + 1 + y * 3 * 16]*/*(img.data(x, y, 0, 1));
			  buff32[x * 4 + 2 + y * 4 * 16] = /*data[x * 3 + 0 + y * 3 * 16]*/*(img.data(x, y, 0, 2));
			  if (/*data[x * 3 + 2 + y * 3 * 16]*/*(img.data(x, y, 0, 2)) == 255 && /*data[x * 3 + 1 + y * 3 * 16]*/*(img.data(x, y, 0, 1)) == 0)
				  buff32[x * 4 + 3 + y * 4 * 16] = 0;
			  else
				  buff32[x * 4 + 3 + y * 4 * 16] = 0xFF;
		  }
	  }

	  glGenTextures(1, &sliderTex);
	  glBindTexture(GL_TEXTURE_2D, sliderTex);

	  glTexImage2D(
		  GL_TEXTURE_2D,       // Type
		  0,                   // No Mipmap
		  4,                   // Format RGBA
		  16,                  // Width
		  16,                  // Height
		  0,                   // Border
		  GL_RGBA,             // Format RGBA
		  GL_UNSIGNED_BYTE,    // 8 Bit/color
		  buff32               // Data
	  );

	  free(buff32);
  }
  //}
  //img.Release();

  GLWindow::RestoreDeviceObjects();
}

void GLColorBox::Paint() {

  int x,y,w,h;

  GLWindow::Paint();

  // Paint HSV square
  hsBox->GetBounds(&x,&y,&w,&h);

  glDisable(GL_CULL_FACE);
  glDisable(GL_LIGHTING);
  glDisable(GL_DEPTH_TEST);
  glEnable(GL_TEXTURE_2D);
  glDisable(GL_BLEND);
  glBindTexture(GL_TEXTURE_2D,hsvTex);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glColor3f(1.0f,1.0f,1.0f);
  glBegin(GL_QUADS);
  glTexCoord2f(0.0f,0.0f);glVertex2i(x    ,y);
  glTexCoord2f(1.0f,0.0f);glVertex2i(x+128,y);
  glTexCoord2f(1.0f,1.0f);glVertex2i(x+128,y+128);
  glTexCoord2f(0.0f,1.0f);glVertex2i(x    ,y+128);
  glEnd();

#ifdef _DEBUG
  theApp->nbPoly++;
#endif

  glDisable(GL_TEXTURE_2D);
  glColor3f(0.0f,0.0f,0.0f);
  paintBox(x-2,y-2,131,131);
  glColor3f(1.0f,1.0f,1.0f);
  paintBox(x-1,y-1,129,129);

  int xm = (int)(curH/360.0f * 128.0f + 0.5f) + x;
  int ym = (int)(curS * 128.0f + 0.5f) + y;
  glColor3f(0.0f,0.0f,0.0f);
  glBegin(GL_LINES);
  _glVertex2i(xm-2,ym);
  _glVertex2i(xm+1,ym);
  _glVertex2i(xm,ym-2);
  _glVertex2i(xm,ym+2);
  glEnd();

  vBox->GetBounds(&x,&y,&w,&h);
  for(int i=0;i<128;i++) {
    size_t c = hsv_to_rgb(curH,curS,(float)i/128.0f);
    glColor3f(get_red(c),get_green(c),get_blue(c));
    glBegin(GL_LINES);
    _glVertex2i(x,y+i);
    _glVertex2i(x+20,y+i);
    glEnd();
  }
  glColor3f(0.0f,0.0f,0.0f);
  paintBox(x-2,y-2,23,131);
  glColor3f(1.0f,1.0f,1.0f);
  paintBox(x-1,y-1,21,129);

  // Slider
  int yPos = (int)(curV*128.0f + 0.5f) + y - 8;

  glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D,sliderTex);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glColor3f(1.0f,1.0f,1.0f);
  glBegin(GL_QUADS);
  glTexCoord2f(0.0f,0.0f);glVertex2i(x+30,yPos);
  glTexCoord2f(1.0f,0.0f);glVertex2i(x+46,yPos);
  glTexCoord2f(1.0f,1.0f);glVertex2i(x+46,yPos+16);
  glTexCoord2f(0.0f,1.0f);glVertex2i(x+30,yPos+16);
  glEnd();

#ifdef _DEBUG
  theApp->nbPoly++;
#endif

  // Paint swatches
  swBox->GetBounds(&x,&y,&w,&h);

  glDisable(GL_TEXTURE_2D);
  glDisable(GL_BLEND);
  glColor3f(0.0f,0.0f,0.0f);
  paintBox(x-3,y-4,346,105);
  glColor3f(1.0f,1.0f,1.0f);
  paintBox(x-2,y-3,344,103);
  for(int j=0;j<9;j++) {
    for(int i=0;i<31;i++) {
      glColor3f(get_red(sColors[i+j*31]),get_green(sColors[i+j*31]),get_blue(sColors[i+j*31]));
      glBegin(GL_QUADS);
      glVertex2i(x+i*11      ,y+j*11);
      glVertex2i(x+(i+1)*11-1,y+j*11);
      glVertex2i(x+(i+1)*11-1,y+(j+1)*11-1);
      glVertex2i(x+i*11      ,y+(j+1)*11-1);
      glEnd();
#ifdef _DEBUG
      theApp->nbPoly++;
#endif
      glColor3f(0.0f,0.0f,0.0f);
      glBegin(GL_LINES);
      _glVertex2i(x+(i+1)*11-1,y+j*11);
      _glVertex2i(x+(i+1)*11-1,y+(j+1)*11-2);
      _glVertex2i(x+(i+1)*11-1,y+(j+1)*11-2);
      _glVertex2i(x+i*11      ,y+(j+1)*11-2);
      glEnd();
#ifdef _DEBUG
      theApp->nbLine++;
#endif
    }
  }
  GLToolkit::CheckGLErrors("GLColorBox::Paint()");
}

void GLColorBox::paintBox(int x,int y,int w,int h) {

  glBegin(GL_LINES);
  _glVertex2i(x,y);
  _glVertex2i(x+w,y);
  _glVertex2i(x+w,y);
  _glVertex2i(x+w,y+h);
  _glVertex2i(x+w,y+h);
  _glVertex2i(x,y+h);
  _glVertex2i(x,y+h);
  _glVertex2i(x,y);
  glEnd();

#ifdef _DEBUG
  theApp->nbLine+=2;
#endif

}

int GLColorBox::get_redi( size_t c ) {
  int ret = (c & 0x00FF0000);
  ret = ret >> 16;
  return ret;
}

int GLColorBox::get_greeni( size_t c ) {
  int ret = (c & 0x0000FF00);
  ret = ret >> 8;
  return ret;
}

int GLColorBox::get_bluei( size_t c ) {
  int ret = (c & 0x000000FF);
  return ret;
}

float GLColorBox::get_red( size_t c ) {
  return (float)get_redi(c)/255.0f;
}

float GLColorBox::get_green( size_t c ) {
  return (float)get_greeni(c)/255.0f;
}

float GLColorBox::get_blue( size_t c ) {
  return (float)get_bluei(c)/255.0f;
}

// h in [0,360], s in [0,1], v in [0,1]
size_t GLColorBox::hsv_to_rgb( float h,float s,float v,bool swap ) {

	float r,g,b;
	int   ir,ig,ib;
	int   i;
  float hi,f,p,q,t;

	if( s==0.0f ) {
		r=g=b=v;
	} else {

		hi = h/60.0f;
		i=(int)hi;
		f = hi - (float)i;
		p = v * (1.0f - s);
		q = v * (1.0f - s*f);
		t = v * (1.0f - s*(1.0f-f));

		switch(i) {
		  case 0:
			r=v;g=t;b=p;
			break;
		  case 1:
			r=q;g=v;b=p;
			break;
		  case 2:
			r=p;g=v;b=t;
			break;
		  case 3:
			r=p;g=q;b=v;
			break;
		  case 4:
			r=t;g=p;b=v;
			break;
		  default:
			r=v;g=p;b=q;
			break;
		}

	}

	ir  = (int)( r*255.0 );
  ig  = (int)( g*255.0 );
  ib  = (int)( b*255.0 );
  if(!swap) {
	  size_t ret = ir << 16 | ig << 8 | ib;
	  return ret;
  } else {
	  size_t ret = ib << 16 | ig << 8 | ir;
	  return ret;
  }
}

void GLColorBox::rgb_to_hsv(int ri,int gi,int bi, float *h,float *s,float *v) {

	float min,max,delta;

	float r = (float)ri/255.0f;
  float g = (float)gi/255.0f;
  float b = (float)bi/255.0f;

  min = r;
	max = r;

  if( g<min ) min =g;
  if( b<min ) min =b;
  if( g>max ) max =g;
  if( b>max ) max =b;

	*v = max;

	delta = max-min;

	if( delta == 0 )
	{
		*v = max;
		*h = 0.0f;
		*s = 0.0f;
		return;
	}

	if( max!=0 )
		*s = delta / max;
	else {
		*s = 0;
		*h = 0;
		return;
	}

	if ( r==max )
		*h = (g-b) / delta;
	else if( g==max ) 
		*h = 2 + (b-r) / delta;
	else
		*h = 4 + (r-g) / delta;
     
	*h *= 60.0f;
	if( *h<0.0f ) *h += 360.0f;

}

void GLColorBox::ProcessMessage(GLComponent *src,int message) {
  if(message==MSG_BUTTON) {
    rCode = src->GetId();
    GLWindow::ProcessMessage(NULL,MSG_CLOSE);
    return;
  }
  if(message==MSG_TEXT) {
    int r=0,g=0,b=0;
	if (
    rText->GetNumberInt(&r) &&
	gText->GetNumberInt(&g) &&
	bText->GetNumberInt(&b)
		)
			updateColor(r,g,b);
  }
  GLWindow::ProcessMessage(src,message);
}

void GLColorBox::ManageEvent(SDL_Event *evt) {

  if(evt->type == SDL_MOUSEBUTTONUP) {
    draggV = false;
  }

  if(evt->type == SDL_MOUSEBUTTONDOWN) {
    if( IsInComp(swBox,evt->button.x,evt->button.y) ) {
      int x = GetX(swBox,evt)/11;
      int y = GetY(swBox,evt)/11;
      size_t newC = sColors[x+y*31];
      updateColor(get_redi(newC),get_greeni(newC),get_bluei(newC));
    }
    if( IsInComp(hsBox,evt->button.x,evt->button.y) ) {
      float h = GetX(hsBox,evt)/128.0f * 360.0f;
      float s = GetY(hsBox,evt)/128.0f;
      if( curV<=0.0001 ) curV = 0.8f;
      size_t newC = hsv_to_rgb(h,s,curV);
      updateColor(get_redi(newC),get_greeni(newC),get_bluei(newC));
    }
    if( IsInComp(vBox,evt->button.x,evt->button.y) ) {
      float v = GetY(vBox,evt)/128.0f;
      size_t newC = hsv_to_rgb(curH,curS,v);
      updateColor(get_redi(newC),get_greeni(newC),get_bluei(newC));
      draggV = true;
    }
  }

  if(evt->type == SDL_MOUSEMOTION) {
    if( draggV ) {
      float v = GetY(vBox,evt)/128.0f;
      Saturate(v,0.0f,1.0f);
      size_t newC = hsv_to_rgb(curH,curS,v);
      updateColor(get_redi(newC),get_greeni(newC),get_bluei(newC));
    }
  }

  GLWindow::ManageEvent(evt);
}

void GLColorBox::updateColor(int r,int g,int b) {

  char tmp[128];
  sprintf(tmp,"%d",r);
  rText->SetText(tmp);
  sprintf(tmp,"%d",g);
  gText->SetText(tmp);
  sprintf(tmp,"%d",b);
  bText->SetText(tmp);
  newColor->SetBackgroundColor(r,g,b);
  curR = r;
  curG = g;
  curB = b;
  rgb_to_hsv(r,g,b,&curH,&curS,&curV);

}

int GLColorBox::Display(const char *title,int *r,int *g,int *b) {

  GLfloat old_mView[16];
  GLfloat old_mProj[16];
  GLint   old_viewport[4];

  // Save current matrix
  glGetFloatv( GL_PROJECTION_MATRIX , old_mProj );
  glGetFloatv( GL_MODELVIEW_MATRIX , old_mView );
  glGetIntegerv( GL_VIEWPORT , old_viewport );

  // Initialise
  GLColorBox *dlg = new GLColorBox(title,r,g,b);
  dlg->DoModal();
  int ret = dlg->rCode;
  if( ret ) {
    *r = dlg->curR;
    *g = dlg->curG;
    *b = dlg->curB;
  }
  delete dlg;

  // Restore matrix
  glMatrixMode( GL_PROJECTION );
  glLoadMatrixf(old_mProj);
  glMatrixMode( GL_MODELVIEW );
  glLoadMatrixf(old_mView);
  glViewport(old_viewport[0],old_viewport[1],old_viewport[2],old_viewport[3]);

  return ret;

}
