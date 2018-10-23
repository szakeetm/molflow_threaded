// Copyright (c) 2011 rubicon IT GmbH
#include "GLWindow.h"
#include "GLToggle.h"
#include "GLToolkit.h"
#include "GLFont.h"
#include <cstring> //strcpy, etc.

GLToggle::GLToggle(int compId,const char *text):GLComponent(compId) {
  if(text) 
    strcpy(this->text,text);
  else
    strcpy(this->text,"");
  state=0;
  allowMixedState = false;
  SetBorder(BORDER_NONE);
  SetTextColor(0,0,0);
}

int GLToggle::GetState() {
  return state;
}

void GLToggle::AllowMixedState(bool setAllow) {
	allowMixedState = setAllow;
}

void GLToggle::SetState(int setState) {
  state=setState;
}

void GLToggle::SetText(std::string text) {
	strcpy(this->text, text.c_str());
}

void GLToggle::Paint() {

  if(!parent) return;
  GLFont2D *font = GLToolkit::GetDialogFont();

  GLComponent::Paint();

  font->SetTextColor(rText,gText,bText);
  font->DrawText(posX+16,posY+2,text,false);

  GLToolkit::DrawToggle(posX+2,posY+3);
  if(state==1) {
    font->SetTextColor(0.0f,0.0f,0.0f);
    font->DrawText(posX+5,posY+1,"\215",false);
  }
  else if (state == 2) {
	  font->SetTextColor(0.7f, 0.7f, 0.7f);
	  font->DrawText(posX + 5, posY + 1, "\215", false);
  }
  GLToolkit::CheckGLErrors("GLToggle::Paint()");
}

void GLToggle::SetTextColor(int r,int g,int b) {
  rText = r/255.0f;
  gText = g/255.0f;
  bText = b/255.0f;
}

void GLToggle::SetEnabled(bool enable) { //GLComponent override
	enabled = enable;
	int color = enable ? 0 : 120;
	SetTextColor(color, color, color);
}

void GLToggle::ManageEvent(SDL_Event *evt) {

  if(!parent) return;

  int x,y,w,h;
  GetWindow()->GetBounds(&x,&y,&w,&h);

  if( evt->type == SDL_MOUSEBUTTONDOWN ) {
    if( evt->button.button == SDL_BUTTON_LEFT ) {
      if (!allowMixedState) state = !state; //inverse state
	  else state = (state + 2) % 3; //cycle states
      parent->ProcessMessage(this,MSG_TOGGLE);
    }
  }

}
