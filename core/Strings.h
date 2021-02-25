//
//  GUIStrings.h
//  MarkPad Project
//
//  (c) Eric Lecolinet - http://www.telecom-paristech.fr/~elc
//  (c) Bruno Fruchard - http://brunofruchard.com/
//  Copyright (c) 2017/18. All rights reserved.
//

#ifndef MARKPAD_STRINGS
#define MARKPAD_STRINGS

#include <string>
#include <vector>

struct HelpItem {
  std::string title, content;
};

class Strings {
  Strings();
  static Strings instance;

public:
  /// read-only access to strings.
  static const Strings& k;

  static void updateHotKey(const std::string& hotkey, const std::string& editkey);

  std::vector<HelpItem> helpitems;
  
  std::string
  ShowBrowser,
  NewMenu,
  NewShortcut,
  OpenMenu,
  CloseMenu,
  ClickDesired,
  ShowMenus,
  EditMenus,

  AccessibilityMsg,
  AccessibilityInfo,
  AccessibilityBtns,
  EnableAcessibilityMsg,
  EnableAcessibilityInfo,
  EnableAcessibilityBtns,
  
  ClickToCreateShortcut1,
  ClickToCreateShortcut2,
  EmptyMenu;
};

#endif

