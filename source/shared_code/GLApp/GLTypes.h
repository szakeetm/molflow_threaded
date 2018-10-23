/*
Program:     MolFlow+ / Synrad+
Description: Monte Carlo simulator for ultra-high vacuum and synchrotron radiation
Authors:     Jean-Luc PONS / Roberto KERSEVAN / Marton ADY
Copyright:   E.S.R.F / CERN
Website:     https://cern.ch/molflow

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

Full license text: https://www.gnu.org/licenses/old-licenses/gpl-2.0.en.html
*/
#pragma once

// Messages

#define MSG_NULL     0    // No message
#define MSG_CLOSE    1    // Close window
#define MSG_ICONIFY  2    // Iconify window
#define MSG_MAXIMISE 3    // Maximise window
#define MSG_BUTTON   4    // Button pressed
#define MSG_TOGGLE   5    // Toggle state change
#define MSG_TEXT     6    // Return pressed 
#define MSG_TEXT_UPD 7    // Text change
#define MSG_SCROLL   8    // Scroll change
#define MSG_LIST     9    // List select
#define MSG_LIST_DBL 10   // List DBL click
#define MSG_LIST_COL 11   // List column move
#define MSG_COMBO    12   // Combo select
#define MSG_MENU     13   // Menu select
#define MSG_TAB      14   // Tab window change
#define MSG_SPINNER  15   // Spinner change
#define MSG_PANELR   16   // Panel open/close

#define MSG_USER     256

// SDL extensions

/*
#define SDLK_CTRLC 3
#define SDLK_CTRLX 24
#define SDLK_CTRLV 22
*/

#define SDL_MOUSEBUTTONDBLCLICK SDL_USEREVENT + 0

// Macros
#define DELETE_LIST(l) if(l) { glDeleteLists(l,1);l=0; }
#define DELETE_TEX(t)  if(t) { glDeleteTextures(1,&t);t=0; }
#define SAFE_DELETE(x) if(x) { delete x;x=NULL; }
#define SAFE_FREE(x) if(x) { free(x);x=NULL; }
#define SAFE_CLEAR(vect) if(vect) {vect.clear();}
#define IVALIDATE_DLG(dlg) if(dlg && !dlg->IsVisible()) dlg->InvalidateDeviceObjects();
#define RVALIDATE_DLG(dlg) if(dlg && !dlg->IsVisible()) dlg->RestoreDeviceObjects();
#define ZEROVECTOR(_vector) std::fill(_vector.begin(),_vector.end(),0);
#define WRITEBUFFER(_value,_type) *((_type *)buffer)=_value;buffer += sizeof(_type)
#define READBUFFER(_type) *(_type*)buffer;buffer+=sizeof(_type)

// Constants

#define GL_OK   1
#define GL_FAIL 0

// Type definitions
typedef unsigned char BYTE;

class Error {

public:
	Error(const char *message);
	const char *GetMsg();

private:
	char msg[1024];

};

typedef struct {

  float r;
  float g;
  float b;
  float a;

} GLCOLOR;

typedef struct {

  GLCOLOR   Diffuse;
  GLCOLOR   Ambient;
  GLCOLOR   Specular;
  GLCOLOR   Emissive;
  float     Power;

} GLMATERIAL;

typedef struct {

  int x;
  int y;
  int width;
  int height;

} GLVIEWPORT;