//
//  GUI.mm
//  MarkPad Project
//
//  (c) Eric Lecolinet - http://www.telecom-paristech.fr/~elc
//  (c) Bruno Fruchard - http://brunofruchard.com/
//  Copyright (c) 2017/18. All rights reserved.
//

#include <iostream>
#include "ccuty/ccstring.hpp"
#include "ccuty/ccpath.hpp"
#include "MarkPad.h"
#include "AWL.h"
#include "GUI.h"
#include "Strings.h"
#include "Editor.h"
#include "Settings.h"
#include "Overlay.h"
#include "Services.h"
using namespace std;
using namespace ccuty;

GUI GUI::instance;
MarkPad& GUI::mp = MarkPad::instance;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

GUI::GUI() :
settings(Settings::instance), edit(*new Editor()) {}

void GUI::init() {
  // auto-hide the OS X menu bar and the dock when the app is frontmost
  // useful only in Qt version!!
  //Services::autoHideMenubarAndDock();
  
  // must be created first (inits overlay and attaches the contextual menu)
  Overlay::instance.init(AWL::getDelegate()->contextMenu);
  
  // init. settings dialog (also sets accessibility and hotkeys)
  settings.init();

  // init. editor dialog
  edit.init();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

bool GUI::isMouseDown() {
  return Overlay::instance.isMouseDown();
}

MTPoint GUI::currentMousePos() {
  return MTPoint{
    float(AWL::getCurrentMouseX() / AWL::getScreenWidth()),
    float(AWL::getCurrentMouseY() / AWL::getScreenHeight())
  };
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// HELP and ALERT

int GUI::alert(const std::string& msg, const std::string& info,
               const std::string& buttons) {
  return AWL::alert(msg, info, buttons);
}

int GUI::alert(const std::string& msg, const std::string& info,
               const std::string& buttons, float x, float y) {
  return AWL::alert(msg, info, buttons, x, y);
}


void GUI::openManual() {
  MarkPad::instance.edit(MarkPad::EditNone);
  Services::openUrl("http://brunofruchard.com/markpad.html");
}

void GUI::openURL(const std::string& url) {
  Services::openUrl(url);
}

void GUI::openFinder() {
  instance.edit.openFinder();
}

void GUI::openWebBrowser() {
  instance.edit.openWebBrowser();
}

void GUI::showHelp() {
 instance.edit.show(AWL::getDelegate()->helpWin, true);
}

void GUI::closeSettings(){
   instance.settings.closeSettings();
}

/*
void GUI::showHelp(HelpCategory type) {
  AWDelegate* del = AWL::getDelegate();
  AWL::select(del->helpChoices, (int(type)));
  AWL::setText(del->helpField, Strings::k.help[int(type)]);
  edit.show(del->helpWin, true);
}*/

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// OVERLAY

void GUI::initOverlayGeometry(Pad* pad) {
  Overlay::instance.initGeometry(pad);
}

void GUI::showOverlay(OverlayMode mode) {
  Overlay::instance.show(mode);
}

void GUI::updateOverlay(bool update_shortcuts) {
  Overlay::instance.update(update_shortcuts);
}

void GUI::showFeedback(const std::string& feedback) {
 Overlay::instance.showFeedback(feedback);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// EDITOR

void GUI::updateRunState(bool running) {
  settings.updateRunState(running);
}

void GUI::editCB(bool state) {
  GUI::instance.edit.editCB(state);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Menus

void GUI::openCloseMenu() {
  if (Shortcut* s = mp.currentShortcut()) {
    s->openMenu(true);
  }
  else if (ShortcutMenu* m = mp.currentMenu()) {
    m->openMenu(false);
  }
}

void GUI::showDesktop(bool state) {
  GUI::instance.edit.showDesktop(state);
}

void GUI::hideEditorCB(bool state) {
  edit.hideEditor(state);
}

void GUI::completeShortcutCreation(ShortcutMenu& menu, Shortcut& s) {
  edit.completeShortcutCreation(menu, s);
}

void GUI::layoutShortcut(Shortcut& s) {
  Overlay::instance.layoutShortcut(s);
}

void GUI::updateShortcut(Shortcut* s) {
  edit.updateShortcut(s);
}

void GUI::updateShortcutSize(Shortcut* s) {
  edit.updateShortcutSize(s);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// overlay

// double click on overlay
void GUI::doubleClickOnOverlayCB(const Shortcut* s, const MTPoint& pos, BoxPart::Value boxpart) {
  //if (s) edit.attractEditor();
  if (s) edit.snapToGuides();
  else mp.createShortcut(currentMousePos(), false);
}

// key on overlay
void GUI::keyOnOverlayCB(uint32_t keycode, uint16_t key, uint32_t modifiers) {
  key = std::tolower(key);
  if (keycode == AWL::BackspaceKeycode)
    edit.deleteShortcuts();
  else if (keycode == AWL::SpaceKeycode)
    edit.attractEditor();
  else if (keycode == AWL::ReturnKeycode)
    openCloseMenu();
  else if (keycode == AWL::EscapeKeycode) {
    ShortcutMenu* menu = mp.currentMenu();
    if (mp.isCreatingShortcut()) mp.cancelCreateShortcut();
    else if (menu == mp.mainMenu()) mp.edit(MarkPad::EditNone); // close editor
    else {
      mp.setCurrentMenu(); // close menu
      if (menu && menu->opener()) mp.selectShortcut(menu->opener());
    }
  }
  
  else if (modifiers & (AWL::ControlModifier|AWL::CommandModifier)) {
    switch (key) {
      case 'n':
        mp.createShortcut(currentMousePos(), false);
        break;
      case 'm':
        openCloseMenu();
        break;
      case 'd':
        edit.deleteShortcuts();
        break;
      case 'u':
      case 'z':
        edit.undeleteShortcuts();
        break;
      case 'c':
        edit.copyShortcuts();
        break;
      case 'x':
        edit.cutShortcuts();
        break;
      case 'v':
        edit.pasteShortcuts();
        break;
      case 'g':
        edit.attractEditor();
        break;
      case '=':
        edit.snapToGuides();
        break;
      case ':':
        edit.alignTitles(BoxPart::Title);
        break;
    }
  }
}
