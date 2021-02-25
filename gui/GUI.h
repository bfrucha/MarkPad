//
//  GUI.h
//  MarkPad Project
//
//  (c) Eric Lecolinet - http://www.telecom-paristech.fr/~elc
//  (c) Bruno Fruchard - http://brunofruchard.com/
//  Copyright (c) 2017/18. All rights reserved.
//

#ifndef MARKPAD_GUI
#define MARKPAD_GUI

#include <string>
#include "MTouch.h"

class Overlay;
class Shortcut;
class ShortcutMenu;
class Action;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

/// Different parts of a box.
struct BoxPart {
  enum Value {
    Outside = 0,
    Inside = 1<<0, Title = 1<<1, MenuBtn = 1<<2,
    Top = 1<<3, Bottom=1<<4, Left=1<<5, Right=1<<6,
    HCenter = 1<<7, VCenter = 1<<8,
    TopLeft = (Top|Left), TopRight = (Top|Right),
    BottomLeft = (Bottom|Left), BottomRight = (Bottom|Right),
    OnBorders = (Top|Left|Bottom|Right)
  };
};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

/// API for interacting with the GUI from the functional core.
class GUI {
  GUI();
  
public:
  static GUI instance;
  static class MarkPad& mp;
  class Settings& settings;
  class Editor& edit;
  
  void init();
  
  /// opens the alert dialog; returns a number indicating which button was pressed.
  static int alert(const std::string& msg, const std::string& info,
                   const std::string& buttons = "");

  /// opens an alert dialog at this location.
  /// xpos, etc. not taken into account if negative
  static int alert(const std::string& msg, const std::string& info,
                   const std::string& buttons,
                   float xpos, float ypos);
  
  static bool isMouseDown();
  static MTPoint currentMousePos();

  enum OverlayMode {OverlayHide, OverlayMenus, OverlayEdit};
  
  void initOverlayGeometry(class Pad*);
  void showFeedback(const std::string& feedback);
  void showOverlay(OverlayMode);
  void updateOverlay(bool update_shortcuts = false);

  void openCloseMenu();
  static void openManual();
  static void openURL(const std::string& url);
  static void openFinder();
  static void openWebBrowser();
  static void showHelp();
  static void showDesktop(bool state);
  static void closeSettings();

  void completeShortcutCreation(ShortcutMenu&, Shortcut&);
  void layoutShortcut(Shortcut&);
  void updateShortcut(Shortcut*);
  void updateShortcutSize(Shortcut*);
  void updateRunState(bool running);

  /// called by MarkPad::edit().
  /// caution: use MarkPad::edit() to open/close the editor, not this method!
  void editCB(bool state);
  
  /// called by MarkPad to hide the Editor temporarily.
  /// caution: use MarkPad::edit() to close the editor, not this method!
  void hideEditorCB(bool state);
  
  /// called when the overlay intercepts a key event.
  void keyOnOverlayCB(uint32_t keycode, uint16_t key, uint32_t modifiers);

  /// called when a double click is performed on the overlay;
  void doubleClickOnOverlayCB(const Shortcut*, const MTPoint& pos, BoxPart::Value);
};

#endif
