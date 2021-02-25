//
//  Settings.mm
//  MarkPad Project
//
//  (c) Eric Lecolinet - http://www.telecom-paristech.fr/~elc
//  (c) Bruno Fruchard - http://brunofruchard.com/
//  Copyright (c) 2017/18. All rights reserved.
//

#include <iostream>
#include "ccuty/ccstring.hpp"
#include "Conf.h"
#include "Strings.h"
#include "Settings.h"
#include "Services.h"
#include "Overlay.h"
using namespace ccuty;

Settings Settings::instance;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void ModifierButtons::enableButtons(bool enabled) {
  if (fnBtn) AWL::enable(fnBtn, enabled);
  AWL::enable(shiftBtn, enabled);
  AWL::enable(ctrlBtn, enabled);
  AWL::enable(altBtn, enabled);
  AWL::enable(cmdBtn, enabled);
}

void ModifierButtons::setStates(uint8_t mods) {
  if (fnBtn) AWL::setState(fnBtn, mods & Modifiers::Function);
  AWL::setState(shiftBtn, mods & Modifiers::Shift);
  AWL::setState(ctrlBtn, mods & Modifiers::Control);
  AWL::setState(altBtn, mods & Modifiers::Alt);
  AWL::setState(cmdBtn, mods & Modifiers::Command);
}

uint8_t ModifierButtons::getStates() {
  uint8_t mods = 0;
  if (fnBtn && AWL::getState(fnBtn)) mods |= Modifiers::Function;
  if (AWL::getState(shiftBtn)) mods |= Modifiers::Shift;
  if (AWL::getState(ctrlBtn)) mods |= Modifiers::Control;
  if (AWL::getState(altBtn)) mods |= Modifiers::Alt;
  if (AWL::getState(cmdBtn)) mods |= Modifiers::Command;
  return mods;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void Settings::init() {  
  // hotkey checkboxes (order matters!)
  showHotkeyBtns = {del->fnBtn, del->shiftBtn, del->ctrlBtn, del->altBtn, del->cmdBtn};
  editHotkeyBtns = {del->fnBtn2, del->shiftBtn2, del->ctrlBtn2, del->altBtn2, del->cmdBtn2};

  // note: values depend on config file, which must have been read!
  setState(del->showGridBtn, Conf::k.showGrid);
  setState(del->showFingersBtn, Conf::k.showFingers);
  MarkPad::instance.showFingers(Conf::k.showFingers);

  // these buttons are only available in this mode
  enable(del->logDataBtn, Conf::k.developerMode);
  enable(del->debugBtn, Conf::k.developerMode);

  // open accesssibility dialog if accessibility is not already allowed
  if (!Services::isAccessibilityEnabled() && Conf::k.allowAccessibility) {
    Services::askAccessibility();
  }
  
  // change keys and masks according to Conf
  if (Services::isAccessibilityEnabled()) {
    showHotkey = Conf::k.showHotkey;
    editHotkey = Conf::k.editHotkey;
    if (editHotkey.empty()) editHotkey = defaultEditHotkey;
    string chars;
    gui.mp.setHotkeys(Services::stringToModifiers(showHotkey, showHotkeyStates, chars),
                      Services::stringToModifiers(editHotkey, editHotkeyStates, chars));
  }
  updateHotkeys();
}

void Settings::updateRunState(bool running) {
  setText(del->runPauseItem, running ? "Pause" : "Run");
}

void Settings::updateHotkeys() {
  // update SettingsWin
  showHotkeyBtns.enableButtons(Services::isAccessibilityEnabled());
  editHotkeyBtns.enableButtons(Services::isAccessibilityEnabled());
  showHotkeyBtns.setStates(showHotkeyStates);
  editHotkeyBtns.setStates(editHotkeyStates);

  // update help
  Strings::updateHotKey(showHotkey, editHotkey);
  
  // update statusbar
  setText(del->showMenusItem, Strings::k.ShowMenus
          + (showHotkey.empty() ? "" : (" ["+showHotkey+"]")));
  setText(del->editMenusItem, Strings::k.EditMenus
          + (editHotkey.empty() ? "" : (" ["+editHotkey+" (when shown)]")));
}

void Settings::openSettingsWindowCB() {
  settings_window_opened = true;
  setState(del->accessibilityBtn, Services::isAccessibilityEnabled());
  
  setText(del->menuDelayField,  Conf::k.menuDelay);
  setText(del->minMovementField,  Conf::k.minMovement);
  setText(del->minTouchSizeField, Conf::k.minTouchSize);
  setText(del->maxTouchSizeField, Conf::k.maxTouchSize);
  setState(del->feedbackBtn, Conf::k.showFeedback);
  setState(del->showBgImageBtn, Conf::k.showBgImage);
  feedbackCB(false);
  
  // setThemeButtons
  /*
  setState(del->darkThemeBtn, false);
  setState(del->lightThemeBtn, false);
  setState(del->transpThemeBtn, false);
  if (Conf::k.theme == "dark") setState(del->darkThemeBtn, true);
  else if (Conf::k.theme == "light") setState(del->lightThemeBtn, true);
  else if (Conf::k.theme == "transparent") setState(del->transpThemeBtn, true);
  */

  if (MarkPad::instance.editMode() == MarkPad::EditGuides)
    setState(del->editGuidesBtn, true);
  else if (MarkPad::instance.editMode() == MarkPad::EditBorders)
    setState(del->editBordersBtn, true);
  else
    setState(del->editShortcutsBtn, true);

  setState(del->logDataBtn, Conf::k.logData);
  show(del->settingsWin, true);
}


void Settings::setHotkeys(uint8_t showstates, uint8_t editstates) {
  if (Services::isAccessibilityEnabled()) {
    showHotkeyStates = showstates;
    editHotkeyStates = editstates;
    gui.mp.setHotkeys(Services::modifiersToString(showHotkeyStates, showHotkey),
                      Services::modifiersToString(editHotkeyStates, editHotkey));
    Conf::change(&Conf::showHotkey, showHotkey);
    Conf::change(&Conf::editHotkey, editHotkey);
  }
  else {
    showHotkeyStates = 0;
    editHotkeyStates = 0;
    gui.mp.setHotkeys(0, 0);
    showHotkey = "";
    editHotkey = "";
  }
}

void Settings::closeSettingsWindowCB() {
  closeSettings();
}

void Settings::closeSettings() {

  if (!settings_window_opened) return;
  settings_window_opened = false;
  show(del->settingsWin, false);
  
  // save hotkeys if changed
  setHotkeys(showHotkeyBtns.getStates(), editHotkeyBtns.getStates());
  updateHotkeys();

  Conf::change(&Conf::menuDelay, getFloat(del->menuDelayField));
  Conf::change(&Conf::minMovement, getFloat(del->minMovementField));
  Conf::change(&Conf::minTouchSize, getFloat(del->minTouchSizeField));
  Conf::change(&Conf::maxTouchSize, getFloat(del->maxTouchSizeField));
  Conf::changeFeedback(&Conf::Feedback::delay, getFloat(del->feedbackDelayField));
  Conf::change(&Conf::logData, getState(del->logDataBtn));
  Conf::change(&Conf::showBgImage, getState(del->showBgImageBtn));
}

void Settings::settingsCB() {
  std::string theme;
  /*
  if (getState(del->darkThemeBtn)) theme = "dark";
  else if (getState(del->lightThemeBtn)) theme = "light";
  else if (getState(del->transpThemeBtn)) theme = "transparent";
  Conf::change(&Conf::theme, theme);
  Overlay::instance.setTheme(theme);
   */
  
  if (getState(del->editShortcutsBtn)) {
    MarkPad::instance.edit(MarkPad::EditShortcuts);
  }
  else if (getState(del->editBordersBtn)) {
    MarkPad::instance.edit(MarkPad::EditBorders);
  }
  else if (getState(del->editGuidesBtn)) {
    if (!MarkPad::instance.guide()) {
      setState(del->editGuidesBtn, false);
      MarkPad::instance.edit(MarkPad::EditNone);
      GUI::alert("Cannot edit Guide","Because file "+Conf::guideFile()+" is missing", "OK");
    }
    else MarkPad::instance.edit(MarkPad::EditGuides);
  }
  Overlay::instance.update();
}

void Settings::feedbackCB(bool called_by_btn) {
  bool state = getState(del->feedbackBtn);
  if (called_by_btn) Conf::change(&Conf::showFeedback, state);
  
  enable(del->feedbackColorBtn, state);
  enable(del->feedbackDelayField, state);
  setText(del->feedbackDelayField, Conf::k.feedback.delay);
  
  enable(del->feedbackFontBtn, state);
  enable(del->feedbackFontNameField, state);
  enable(del->feedbackFontSizeField, state);
  string name, size;
  ccuty::strsplit(name, size, Conf::k.feedback.font, ",");
  setText(del->feedbackFontNameField, name);
  setText(del->feedbackFontSizeField, size);
  enable(del->feedbackColorBtn, state);
  setColor(del->feedbackColorWell, Overlay::instance.getFeedbackColor());
}

void Settings::accessibilityCB() {
  if (Services::isAccessibilityEnabled()) {
    setState(del->accessibilityBtn, true);
  }
  else {
    bool accept = Services::askAccessibility();
    setState(del->accessibilityBtn, accept);
    updateHotkeys();
    MarkPad::instance.edit(MarkPad::EditShortcuts);  // because alert closed the MP menu
    show(del->settingsWin, true);
  }
}
