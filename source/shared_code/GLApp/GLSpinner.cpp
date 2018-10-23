// Copyright (c) 2011 rubicon IT GmbH
#include "GLWindow.h"
#include "GLSpinner.h"
#include "GLToolkit.h"
#include "GLFont.h"
#include "MathTools.h" //Saturate
#include <cstring> //strcpy, etc.

GLSpinner::GLSpinner(int compId):GLComponent(compId) {

  value = 0.0;
  increment = 1.0;
  SetMinMax(0.0,100.0);
  strcpy(format,"%.0f");
  SetBackgroundColor(240,240,240);
  SetBorder(BORDER_BEVEL_IN);
  state = 0;

}

void GLSpinner::SetValue(double value) {
  this->value = value;
}

double GLSpinner::GetValue() {
  return value;
}

void GLSpinner::SetIncrement(double inc) {
  increment = inc;
}

void GLSpinner::SetFormat(const char *format) {
  if(format) strcpy(this->format,format);
}

void GLSpinner::SetMinMax(double min,double max) {
  this->min = min;
  this->max = max;
}

void GLSpinner::Paint() {

  if(!parent) return;
  GLComponent::Paint();

  GLFont2D *font = GLToolkit::GetDialogFont();
  char valueStr[256];
  sprintf(valueStr,format,value);

  int xPos = posX+width - GLToolkit::GetDialogFont()->GetTextWidth(valueStr) - 20;
  font->SetTextColor(0.0f,0.0f,0.0f);
  font->DrawText(xPos,posY+3,valueStr,false);

  // Draw up button
  int h2 = (height/2) - 1;
  GLToolkit::DrawBox(posX+width-16,posY+1,15,h2,208,208,208,true,(state==1));
  font->DrawText(posX+width-13,posY-2,"\212",false);

  // Draw down button
  GLToolkit::DrawBox(posX+width-16,posY+2+h2,15,h2,208,208,208,true,(state==2));
  font->DrawText(posX+width-13,posY+h2-1,"\211",false);

}

void GLSpinner::ManageEvent(SDL_Event *evt) {

  int mx = GetWindow()->GetX(this,evt);
  int my = GetWindow()->GetY(this,evt);

  if( evt->type == SDL_MOUSEBUTTONDOWN || evt->type == SDL_MOUSEBUTTONDBLCLICK ) {
    if( mx>=width-16 ) {
      int h2 = (height/2) - 1;
      if( my < h2 ) {
        state = 1;
        value += increment;
        Saturate(value,min,max);
        parent->ProcessMessage(this,MSG_SPINNER);
      } else {
        state = 2;
        value -= increment;
        Saturate(value,min,max);
        parent->ProcessMessage(this,MSG_SPINNER);
      }
    }
  }

  if( evt->type == SDL_MOUSEBUTTONUP ) {
    state = 0;
  }

}
