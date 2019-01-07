// Copyright (c) 2011 rubicon IT GmbH
#include "GLWindow.h"
#include "GLTextField.h"
#include "GLToolkit.h"
#include "MathTools.h" //Min max
#include "GLFont.h"
#include <cstring> //strcpy, etc.

GLTextField::GLTextField(int compId,const char *text):GLComponent(compId) {

  SetBorder(BORDER_BEVEL_IN);
  strcpy(m_Text,"!!!Don't remove this default value!!!");
  SetText(text);
  SetBackgroundColor(240,240,240);
  SetTextColor(0.0f, 0.0f, 0.0f);
  m_CursorState=0;
  m_Editable=true;
  SetCursor(CURSOR_TEXT);
}

std::string GLTextField::GetText() {
  return m_Text;
}

void GLTextField::Clear() {
  SetText("");
}

void GLTextField::SetCursorPos(int pos) {
  if(pos>=0 && pos<=(int)strlen(m_Text)) {
    m_Start=0;
    m_Stop =0;
    m_CursorPos=pos;
    m_Captured=0;
    ScrollToVisible();
  }
}

int GLTextField::GetCursorPos() {
  return m_CursorPos;
}

int GLTextField::GetTextLength() {
  return m_Length;
}

void GLTextField::SetEditable(bool editable) {
  m_Editable = editable;
  if( m_Editable ) {
	  SetCursor(CURSOR_TEXT);
	  SetBackgroundColor(240,240,240);
  }
  else {
	  SetCursor(CURSOR_DEFAULT);
	  SetBackgroundColor(210,210,210);
  }
}

bool GLTextField::IsEditable() {
	return m_Editable;
}

void GLTextField::SetEditable_NoBG(bool editable) { //for combo boxes
  m_Editable = editable;
  if( m_Editable ) {
	  SetCursor(CURSOR_TEXT);
  }
  else {
	  SetCursor(CURSOR_DEFAULT);
  }
}

bool GLTextField::IsCaptured() {
  return m_Captured;
}

void GLTextField::SetText(const char *text) {
  if(text && strcmp(text,m_Text)==0) return;
  UpdateText(text);
  m_Start=0;
  m_Stop =0;
  m_CursorPos=0;
  m_Captured=0;
  m_Zero=0;
}

void GLTextField::SetText(std::string string) {
	SetText(string.c_str());
}

void GLTextField::SetText(const double &val) {
	char tmp[256];
	sprintf(tmp, "%.10g", val);
	SetText(tmp);
}

void GLTextField::SetText(const int &val) {
	char tmp[256];
	sprintf(tmp, "%d", val);
	SetText(tmp);
}

void GLTextField::SetText(const size_t &val) {
	char tmp[256];
	sprintf(tmp, "%zd", val);
	SetText(tmp);
}

void GLTextField::UpdateText(const char *text) {
  if(text) {
    strncpy(m_Text,text,MAX_TEXT_SIZE);
    m_Text[MAX_TEXT_SIZE-1]=0;
  } else
    strcpy(m_Text,"");
  UpdateXpos();
}

bool GLTextField::GetNumber(double *num) {

  int conv = sscanf(m_Text,"%lf",num);
  return (conv==1);

}

bool GLTextField::GetNumberInt(int *num) {

  int conv = sscanf(m_Text,"%d",num);
  return (conv==1);

}

bool GLTextField::GetNumberSizeT(size_t *num) {

	int conv = sscanf(m_Text, "%zd", num);
	return (conv == 1);

}

void GLTextField::UpdateXpos() {

  char c[2];
  int  sum;

  m_Length=(int)strlen(m_Text);
  
  c[1]=0;
  sum=3;

  for(int i=0;i<=m_Length;i++) {
    m_XPos[i]=sum;
    c[0]=m_Text[i];
    sum+=GLToolkit::GetDialogFont()->GetTextWidth(c);
  }

}

void GLTextField::ScrollToVisible()
{
  int sx = m_XPos[m_Zero];
  int cx = m_XPos[m_CursorPos];

  // Scroll forward
  while((cx-sx)>width-7 && m_Zero<=m_CursorPos) {
    m_Zero++;
    sx = m_XPos[m_Zero];
  }

  // Scroll backward
  if(m_Zero>m_CursorPos) {
    m_Zero=m_CursorPos;
  }

}

void GLTextField::InsertString(const char *lpszString)
{
  char tmp[MAX_TEXT_SIZE];
  if(!m_Editable) return;

  DeleteSel();

  if( (strlen(lpszString) + strlen(m_Text)) < MAX_TEXT_SIZE ) {
    strcpy(tmp,m_Text);
    tmp[m_CursorPos]=0;
    strcat(tmp,lpszString);
    strcat(tmp,m_Text+m_CursorPos);
    m_CursorPos+=(int)strlen(lpszString);
    m_Start=m_CursorPos;
    m_Stop=m_CursorPos;
    m_CursorState=1;
    UpdateText(tmp);
  }

  ScrollToVisible();

}

void GLTextField::DeleteString(int count)
{
  if(!m_Editable) return;

  std::string tmp = m_Text;
  tmp.erase(m_CursorPos, count);

  m_CursorState = 1;
  UpdateText(tmp.c_str());
  ScrollToVisible();
}

void GLTextField::Paint() {

  if(!parent) return;

  int  x,y;

  // Draw background

  if(border) {
    GLToolkit::DrawTextBack(posX,posY,width,height+2,rBack,gBack,bBack);
	GetWindow()->Clip(this,2,1,2,1);
  } else {
    GLComponent::Paint();
	GetWindow()->Clip(this,0,0,0,0);
}

  // Draw selection background
  int sx = m_XPos[m_Zero] - 2;

  if( m_Start!=m_Stop ) {

    int min = Min(m_Stop,m_Start);
    int max = Max(m_Stop,m_Start);

    int tMargin = (border?2:1);

    GLToolkit::DrawBox(
      m_XPos[min]-sx,
      tMargin,
      m_XPos[max]-m_XPos[min],
      height-4
      ,160,160,255);

  }

  // Draw text
  y = (border?2:1);
  x = 1;
  
  GLToolkit::GetDialogFont()->SetTextColor(rText,gText,bText);
  GLToolkit::GetDialogFont()->DrawText(x,y,m_Text+m_Zero,false);

  // Draw cursor
  if( m_CursorState && m_Editable ) {
    glColor3f(0.0f,0.0f,0.0f);
    glDisable(GL_CULL_FACE);
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_TEXTURE_2D);
    glBegin(GL_LINES);
    _glVertex2i(m_XPos[m_CursorPos]-sx,1);
    _glVertex2i(m_XPos[m_CursorPos]-sx,height-2);
    glEnd();
  }

  // Restore clip
  GetWindow()->ClipToWindow();
  GLToolkit::CheckGLErrors("GLTextField::Paint()");
}

void GLTextField::MoveCursor(int newPos)
{
  if( m_CursorState ) {
    m_CursorState=0;
  }
  m_CursorPos=newPos;
  m_CursorState=1;
  m_Start=m_CursorPos;
  m_Stop=m_CursorPos;
  ScrollToVisible();
}

void GLTextField::MoveSel(int newPos)
{
  m_Stop=newPos;
  m_CursorPos=newPos;
  ScrollToVisible();
}

void GLTextField::RemoveSel()
{
  m_Start=m_CursorPos;
  m_Stop=m_CursorPos;
}

void GLTextField::DeleteSel()
{
  int min,max;

  if(m_Start==m_Stop) {
    m_Start=m_CursorPos;
    m_Stop=m_CursorPos;
    return;
  }

  if(m_Start>m_Stop) { min=m_Stop;max=m_Start; } 
  else               { min=m_Start;max=m_Stop; }

  m_CursorPos=min;
  DeleteString(max-min);
  m_Start=m_CursorPos;
  m_Stop=m_CursorPos;
  ScrollToVisible();
}

void  GLTextField::CopyClipboardText() {

 char    tmp[MAX_TEXT_SIZE];
 int     min,max;

 if( m_Start==m_Stop ) return;

 if(m_Start>m_Stop) { min=m_Stop;max=m_Start; } 
 else               { min=m_Start;max=m_Stop; }

 strcpy(tmp,&(m_Text[min]));
 tmp[max-min]=0;

 GLToolkit::CopyTextToClipboard(tmp);

}

void GLTextField::PasteClipboardText() {
	if (!SDL_HasClipboardText()) return;
	char* text = SDL_GetClipboardText();
	//Cut trailing whitespace characters (Paste from Excel, for example)
	for (int cursorPos = (int)strlen(text); cursorPos >= 0; cursorPos--) {
		if (text[cursorPos] == '\t' || text[cursorPos] == '\r' || text[cursorPos] == '\n') {
			text[cursorPos] = '\0';
		}
	}
	InsertString(text);

	/*
#ifdef _WIN32

  if( OpenClipboard(NULL) ) {
    HGLOBAL hMem;
    if(hMem = GetClipboardData(CF_TEXT)) {
      LPVOID ds = GlobalLock(hMem);
      if (ds) {
		  char* text = (char*)ds;
		  //Cut trailing whitespace characters (Paste from Excel, for example)
		  for (int cursorPos = (int)strlen(text); cursorPos >= 0; cursorPos--) {
			  if (text[cursorPos] == '\t' || text[cursorPos] == '\r' || text[cursorPos] == '\n') {
				  text[cursorPos] = NULL;
			  }
		  }
          InsertString(text);
          GlobalUnlock(hMem);
      }
    }
    CloseClipboard();
  }

#endif
*/

}

void GLTextField::SetFocus(bool focus) {

  if(focus) {
    m_CursorState=1;
  } else {
    m_CursorState=0;
    m_Start=0;
    m_Stop =0;
    m_Captured=0;
  }

  GLComponent::SetFocus(focus);

}

void GLTextField::SelectAll() {

  m_Start=0;
  m_CursorPos=0;
  MoveSel(m_Length);

}

int GLTextField::GetCursorLocation(int px) {

  int i=m_Zero,found=0;
  int mxZ = m_XPos[m_Zero];
  
  // Search cursor position
  double pxd = (double)(px - 2);
  while(i<m_Length && !found)
  {
    double l0 = ((i>0)?(double)(m_XPos[i]-m_XPos[i-1]):10.0);
    double l1 = (double)(m_XPos[i]-mxZ);
    double l2 = ((i<m_Length)?(double)(m_XPos[i+1]-m_XPos[i]):0.0);
    found = (pxd >= l1 - l0/2.0f) && (pxd <= l1 + l2/2.0f);
    if(!found) i++;
  }

  if( found ) return i;
  else        return -1;

}

void GLTextField::ProcessEnter() {

	return;
}

void GLTextField::ManageEvent(SDL_Event *evt) {

  if(!parent) return;

  int px = GetWindow()->GetX(this,evt);
  int py = GetWindow()->GetY(this,evt);

  if( evt->type == SDL_MOUSEBUTTONUP ) {
    if( evt->button.button == SDL_BUTTON_LEFT ) {
        m_Captured=0;
    }
  }

  if( evt->type == SDL_MOUSEBUTTONDOWN ) {
    if( evt->button.button == SDL_BUTTON_LEFT ) {

        // Invalidate old cursor
        if( m_CursorState ) {
          m_CursorState=0;
          m_CursorState=1;
        }

        int i = GetCursorLocation(px);
  
        if( i>=0 ) m_CursorPos=i;
        else       m_CursorPos=m_Length;

        RemoveSel();
        m_Captured=1;
        m_LastPos=m_CursorPos;

    }
  }

  if( evt->type == SDL_MOUSEBUTTONDBLCLICK ) {
    if( evt->button.button == SDL_BUTTON_LEFT ) {
      SelectAll();
    }
  }

  if( evt->type == SDL_MOUSEMOTION ) {

    if( m_Captured ) {

      // Search cursor position
      int i=GetCursorLocation(px);

      if( i<0 ) {
        if( px<=m_XPos[0] ) i=0;
        else                i=m_Length;
      }

      if( m_LastPos!=i ) {
        m_Start=i;
        m_LastPos=i;
        m_CursorPos=i;
      }
      ScrollToVisible();

    }

  }

  // Handle key presses
  if( evt->type == SDL_KEYDOWN )
  {
    
    char c[2];
    c[1]=0;

    switch(evt->key.keysym.sym) {
      
      case SDLK_RIGHT:
      if( m_CursorPos<m_Length ) {
        if( GetWindow()->IsShiftDown() ) {
          MoveSel(m_CursorPos+1);
        } else {
          RemoveSel();
          MoveCursor(m_CursorPos+1);
        }
      } else {
        if( !GetWindow()->IsShiftDown() ) RemoveSel();
      }
      break;

	  case SDLK_END:
		  if (m_CursorPos<m_Length) {
			  if (GetWindow()->IsShiftDown()) {
				  MoveSel(m_Length);
			  }
			  else {
				  RemoveSel();
				  MoveCursor(m_Length);
			  }
		  }
		  else {
			  if (!GetWindow()->IsShiftDown()) RemoveSel();
		  }
	 break;

      case SDLK_LEFT:
      if( m_CursorPos>0 ) {
        if( GetWindow()->IsShiftDown() ) {
          MoveSel(m_CursorPos-1);      
        } else {
          RemoveSel();
          MoveCursor(m_CursorPos-1);
        }
      } else {
        if( !GetWindow()->IsShiftDown() ) RemoveSel();
      }
      break;

	  case SDLK_HOME:
		  if (m_CursorPos>0) {
			  if (GetWindow()->IsShiftDown()) {
				  MoveSel(0);
			  }
			  else {
				  RemoveSel();
				  MoveCursor(0);
			  }
		  }
		  else {
			  if (!GetWindow()->IsShiftDown()) RemoveSel();
		  }
	  break;

      case SDLK_DELETE:
      if( m_Start!=m_Stop ) {
        DeleteSel();
        parent->ProcessMessage(this,MSG_TEXT_UPD);
      } else {
        if( m_CursorPos<m_Length ) {
          DeleteString(1);
          parent->ProcessMessage(this,MSG_TEXT_UPD);
        }
      }
      break;

      case SDLK_BACKSPACE:
      if( m_Start!=m_Stop ) {
        DeleteSel();
      } else {
        if( m_CursorPos>0 ) {
          m_CursorPos--;
          DeleteString(1);
          parent->ProcessMessage(this,MSG_TEXT_UPD);
        }
      }
      break;

      case SDLK_v: //CTRL+V
		  if (GetWindow()->IsCtrlDown()) {
			  PasteClipboardText();
			  parent->ProcessMessage(this, MSG_TEXT_UPD);
		  }
      break;

      case SDLK_c: //CTRL+C
		  if (GetWindow()->IsCtrlDown()) {
			  CopyClipboardText();
		  }
      break;

      case SDLK_x: //CTRL+X
		  if (GetWindow()->IsCtrlDown()) {
			  CopyClipboardText();
			  DeleteSel();
			  parent->ProcessMessage(this, MSG_TEXT_UPD);
		  }
      break;

	  case SDLK_a: //CTRL+A
		  if (GetWindow()->IsCtrlDown()) {
			  SelectAll();
		  }
		  break;

      case SDLK_RETURN:
	  case SDLK_KP_ENTER:
        parent->ProcessMessage(this,MSG_TEXT);
        m_CursorPos=0;
        ScrollToVisible();
        break;
	
		/*
      default:
      if( (evt->key.keysym.sym >=32 && evt->key.keysym.sym <=255) ) {
        c[0]= evt->key.keysym.sym;
        InsertString(c);
        parent->ProcessMessage(this,MSG_TEXT_UPD);
      }
      break;
	  */
    }

	

    //Debug
    //char tmp[128];
    //sprintf(tmp,"%d (%c)",unicode,unicode);
    //SetText(tmp);

  }
  if (evt->type == SDL_TEXTINPUT) {
	  InsertString(evt->text.text);
	  parent->ProcessMessage(this, MSG_TEXT_UPD);
  }

}

void   GLTextField::SetTextColor(float r, float g, float b) {
	rText = r;
	gText = g;
	bText = b;
}