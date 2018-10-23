// Copyright (c) 2011 rubicon IT GmbH
#include "GLWindow.h"
#include "GLCombo.h"
#include "GLToolkit.h"
#include "GLWindowManager.h"
#include "GLList.h"
#include "GLTextField.h"
#include <string>
#include <algorithm>

// Popup -----------------------------------------------------------

class GLComboPopup : public GLWindow {

public:

  GLComboPopup(GLCombo *parent):GLWindow() {
    this->parent = parent;
    SetAnimatedFocus(false);
    rCode=0;
  }

  void ProcessMessage(GLComponent *src,int message) {
    if( message == MSG_LIST ) {
      rCode = 1; // Select
      GLWindow::ProcessMessage(NULL,MSG_CLOSE);
    }
  }

  void ManageEvent(SDL_Event *evt) {
    if( evt->type == SDL_MOUSEBUTTONDOWN ) {
      int mx = GetX(NULL,evt);
      int my = GetY(NULL,evt);
      if(mx>=0 && mx<=width && my>=0 && my<=height) {
        GLWindow::ManageEvent(evt);
      } else {
        // Click outside => cancel
        rCode = 2;
        GLWindow::ProcessMessage(NULL,MSG_CLOSE);
      }
    } else {
      GLWindow::ManageEvent(evt);
    }
  }

  int  rCode;
  GLCombo *parent;

};

GLCombo::GLCombo(int compId):GLComponent(compId) {

  wnd = new GLComboPopup(this);
  list = new GLList(compId);
  list->SetMotionSelection(true);
  wnd->Add(list);
  text = new GLTextField(compId,"");
  selectedRow = -1;
  m_Editable = false;
  text->SetEditable_NoBG(m_Editable);
  dropped = false;
  SetBorder(BORDER_BEVEL_IN);
  text->SetBorder(BORDER_NONE);
  Add(text);

}

void GLCombo::Clear() {
  list->Clear();
}

GLList *GLCombo::GetList() {
  return list;
}

void GLCombo::SetSize(size_t nbRow) {
  if( nbRow==0 )
    list->Clear();
  else
    list->SetSize(1,nbRow);
}

void GLCombo::SetValueAt(size_t row,const char *value,int userValue) {
  list->SetValueAt(0,row,value,userValue);
}

int GLCombo::GetUserValueAt(size_t row) {
  return list->GetUserValueAt(0,row);
}

std::string GLCombo::GetValueAt(size_t row) {
  return list->GetValueAt(0,row);
}

size_t GLCombo::GetNbRow() {
  return list->GetNbRow();
}

void GLCombo::SetSelectedValue(const char *value) {
  text->SetText(value);
  selectedRow = -1;
}

std::string GLCombo::GetSelectedValue() {
  return text->GetText();
}

void GLCombo::ScrollTextToEnd() {
  text->SetCursorPos((int)text->GetText().length());
  text->ScrollToVisible();
}

void GLCombo::SetSelectedIndex(int idx) {
	if (idx == -1) {
		text->SetText("");
	}
	else {
		text->SetText(list->GetValueAt(0, idx));
	}
  selectedRow = idx;
}

int GLCombo::GetSelectedIndex() {
  return selectedRow;
}

void GLCombo::SetEditable(bool editable) {
  m_Editable = editable;
  SetEnabled(editable);
  text->SetEditable(m_Editable);
}

void GLCombo::Paint() {

  if(!parent) return;
  int backGroundColor=enabled?240:210;
  GLToolkit::DrawTextBack(posX,posY,width,height+2,backGroundColor,backGroundColor,backGroundColor);
  GLComponent::PaintComponents();
  GLFont2D *font = GLToolkit::GetDialogFont();

  // Draw down button
  if( this->IsEnabled() ) font->SetTextColor(0.0f,0.0f,0.0f);
  else                    font->SetTextColor(0.5f,0.5f,0.5f);
  GLToolkit::DrawSmallButton(posX+width-16,posY+1,dropped);
  if(dropped) {
    //GLToolkit::DrawBox(posX+width-16,posY+1,15,height-2,rBack,gBack,bBack,true,true);
    font->DrawText(posX+width-11,posY+4,"\211",false);
  } else {
    //GLToolkit::DrawBox(posX+width-16,posY+1,15,height-2,rBack,gBack,bBack,true);
    font->DrawText(posX+width-12,posY+3,"\211",false);
  }
  GLToolkit::CheckGLErrors("GLCombo::Paint()");
}

void GLCombo::Drop() {

  dropped = true;
  Paint();

  // Old clipping
  GLint   old_v[4];
  GLfloat old_m[16];
  glGetFloatv( GL_PROJECTION_MATRIX , old_m );
  glGetIntegerv( GL_VIEWPORT , old_v );

  int x = GetWindow()->GetScreenX(this);
  int y = GetWindow()->GetScreenY(this);
  int h = (int)list->GetNbRow()*15+4;
  bool needScroll = false;
  if( h>116 ) { h=116;needScroll=true; }

  int ws[] = { width-4 };
  list->SetColumnWidths(ws);
  list->SetVScrollVisible(needScroll);
  list->SetHScrollVisible(false);
  list->SetBounds(1,1,width-2,h-2);
  list->SetFocus(true);

  // Place drop list
  int wScr,hScr;
  GLToolkit::GetScreenSize(&wScr,&hScr);
  if( y+height+h<hScr ) {
    wnd->SetBounds(x,y+height+1,width,h);
  } else {
    wnd->SetBounds(x,y-h-1,width,h);    
  }

  wnd->RestoreDeviceObjects();
  wnd->DoModal();

  if(wnd->rCode==1) {
    int sRow = list->GetSelectedRow();
    if(sRow>=0) {
      SetSelectedValue(list->GetValueAt(0,sRow));
      selectedRow = sRow;
    }
    parent->ProcessMessage(this,MSG_COMBO);
  }

  wnd->InvalidateDeviceObjects();
  GLWindowManager::FullRepaint();

  glMatrixMode( GL_PROJECTION );
  glLoadMatrixf(old_m);
  glViewport(old_v[0],old_v[1],old_v[2],old_v[3]);

  dropped = false;

}

void GLCombo::SetFocus(bool focus) {
  if(!focus) {
    dropped=false;
  }
  text->SetFocus(focus);
  GLComponent::SetFocus(focus);
}

void GLCombo::ManageEvent(SDL_Event *evt) {

  if(!parent) return;

  int mx = GetWindow()->GetX(this,evt);
  int my = GetWindow()->GetY(this,evt);

  if( mx>=width-16 || !m_Editable ) {
    size_t nbRow = list->GetNbRow();
	if( evt->type == SDL_MOUSEBUTTONDOWN ) {
      
      
        Drop();
      
    }

	if (evt->type == SDL_MOUSEWHEEL) {
#ifdef __APPLE__
		int appleInversionFactor = -1; //Invert mousewheel on Apple devices
#else
		int appleInversionFactor = 1;
#endif
		if (evt->wheel.y!=0) { //Vertical scroll
			int newPos = selectedRow - evt->wheel.y * appleInversionFactor;
			if (nbRow > 0 && newPos >= 0 && newPos <= (nbRow-1)) {
				SetSelectedIndex(newPos);
				if (parent) parent->ProcessMessage(this, MSG_COMBO);
			}
		}
	}
    return;
  }

  text->ManageEvent(evt);

}

void GLCombo::SetBounds(int x,int y,int width,int height) {

  // Place text field
  text->SetBounds(x+2,y+2,width-18,height-2);
  GLComponent::SetBounds(x,y,width,height);

}

void GLCombo::SetParent(GLContainer *parent) {
  text->SetParent(parent);
  GLComponent::SetParent(parent);
}
