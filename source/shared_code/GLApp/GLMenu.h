// Copyright (c) 2011 rubicon IT GmbH
#ifndef _GLMENUH_
#define _GLMENUH_

#include "GLComponent.h"

#define MAX_MENU_ITEM 64
#define MAX_ITEM_LGTH 128

class GLMenu;
class GLMenuBar;

// Menu item struct ----------------------------

typedef struct {

  char         itemName[MAX_ITEM_LGTH];
  int          itemId;
  int          x;
  int          y;
  int          width;
  int          height;
  bool         isSeparator;
  bool         checked;
  bool         enabled;
  char         shortcut;
  int          sctPos;
  int          sctWidth;
  GLMenu      *subMenu;
  char        *accName;
  int          accWidth;
  int          iconX;
  int          iconY;

} MENUITEM;

// Menu Popup ---------------------------------

class GLMenu : public GLComponent {

public:

  // Construction
  GLMenu();
  ~GLMenu();

  // Menu items
  GLMenu* Add(const char *itemName,int itemId=0,int accKeyCode=0,int accKeyModifier=0);
  int     GetNbItem();
  GLMenu *GetSubMenu(const char *itemName);
  void    SetCheck(int itemId,bool checked);
  bool    GetCheck(int itemId);
  void    SetEnabled(int itemId,bool enabled);
  void    SetIcon(int itemId,int x,int y);
  void    Clear();

  // Components method
  void    Close();
  void    Drop(GLContainer *parent,int x,int y);
  int     Track(GLWindow *parent,int x,int y);
  void    SetParentMenuBar(GLMenuBar *p);
  void    SetParentMenu(GLMenu *p);

  // Implementation
  void Paint();
  void ManageEvent(SDL_Event *evt);
  void ProcessKeyboardShortcut(int accId);

private:

  bool  IsInItem(MENUITEM *p,int mx,int my);
  int   GetMenu(int mx,int my);
  int   GetMenu(int id);
  void  CloseSub(bool resetSel=true);
  void  DropSub(int m);
  bool  HasSub(int s);
  void  ProcessMenuItem(int m);
  bool  ProcessShortcut(SDL_Event *evt);

  GLMenuBar *pBar;     // Parent menubar

  MENUITEM *items;
  int       nbItem;

  int       selMenu;   // Selected menu
  int       rCode;
  GLMenu   *pMenu;

  bool      hasAcc;

};

#endif /* _GLMENUH_ */
