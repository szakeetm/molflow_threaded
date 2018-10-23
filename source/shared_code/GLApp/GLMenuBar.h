// Copyright (c) 2011 rubicon IT GmbH
#ifndef _GLMENUBARH_
#define _GLMENUBARH_

#include "GLMenu.h" //MENUITEM

class GLMenuBar : public GLComponent {

public:

  // Construction
  GLMenuBar(int compId);
  ~GLMenuBar();

  // Component methods
  void    Add(const char *itemName);
  GLMenu *GetSubMenu(const char *itemName);
  void    Close();

  // Implementation
  void Paint();
  void ManageEvent(SDL_Event *evt);
  void SetFocus(bool focus);
  void ProcessKeyboardShortcut(int accId);

  // Expert usage
  void    GoLeft();
  void    GoRight();

private:

  bool  IsInItem(MENUITEM *p,int mx,int my);
  int   GetMenu(int mx,int my);
  void  Drop(int sel);

  MENUITEM items[MAX_MENU_ITEM];
  int nbItem;

  int  selMenu;   // Selected menu
  bool autoDrop; // auto drop flag

};

#endif /* _GLMENUBARH_ */