// Copyright (c) 2011 rubicon IT GmbH
#ifndef _GLTEXTFIELDH_
#define _GLTEXTFIELDH_

#include "GLComponent.h"
#include <string>

#define MAX_TEXT_SIZE 1024

class GLTextField : public GLComponent {

public:

  // Construction
  GLTextField(int compId,const char *text);

  // Component methods
  void SetText(const char *text);
  void SetText(std::string string);
  void SetText(const double &val);
  void SetText(const int &val);
  void SetText(const size_t &val);
  std::string GetText();
  void ScrollToVisible();
  void SetCursorPos(int pos);
  int  GetCursorPos();
  int  GetTextLength();
  void SetEditable(bool editable);
  bool IsEditable();
  void SetEditable_NoBG(bool editable);
  void SetTextColor(float r, float g, float b);
  void Clear();
  

  bool GetNumber(double *num);
  bool GetNumberInt(int *num);
  bool GetNumberSizeT(size_t *num);
  void SelectAll();
  bool IsCaptured();

  // Implementation
  void Paint();
  void ManageEvent(SDL_Event *evt);
  void SetFocus(bool focus);

private:

  void   CopyClipboardText();
  void   PasteClipboardText();
  void   UpdateXpos();
  void   InsertString(const char *lpszString);
  void   DeleteString(int count);
  void   MoveCursor(int newPos);
  void   RemoveSel();
  void   MoveSel(int newPos);
  void   DeleteSel();
  void   UpdateText(const char *text);
  void	 ProcessEnter();
  int    GetCursorLocation(int px);

  char    m_Text[MAX_TEXT_SIZE];
  int      m_Start;
  int      m_Stop;
  int      m_Length;
  int      m_CursorPos;
  short    m_XPos[MAX_TEXT_SIZE];
  int      m_CursorState;
  int      m_Captured;
  int      m_LastPos;
  int     m_Zero;
  bool    m_Editable;
  float   rText, gText, bText;

};

#endif /* _GLTEXTFIELDH_ */