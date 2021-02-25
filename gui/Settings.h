//
//  Settings.h
//  MarkPad Project
//
//  (c) Eric Lecolinet - http://www.telecom-paristech.fr/~elc
//  (c) Bruno Fruchard - http://brunofruchard.com/
//  Copyright (c) 2017/18. All rights reserved.
//

#ifndef MARKPAD_SETTINGS
#define MARKPAD_SETTINGS

#include "AWL.h"
#include "GUI.h"
#include "MarkPad.h"
#include "Actions.h"


struct ModifierButtons {
  uint8_t getStates();
  void setStates(uint8_t mods);
  void enableButtons(bool);
  AWButton *fnBtn{}, *shiftBtn{}, *ctrlBtn{}, *altBtn{}, *cmdBtn{};
};


class Settings : public AWL {
  ModifierButtons showHotkeyBtns{}, editHotkeyBtns{};
  std::string showHotkey, editHotkey, defaultEditHotkey = "shift";
  uint8_t showHotkeyStates{0}, editHotkeyStates{0};   // button states
  
public:
  static Settings instance;
  
  void init();
  
  void updateRunState(bool running);
  void updateHotkeys();
  void openSettingsWindowCB();
  void closeSettings();
  void closeSettingsWindowCB();
  void settingsCB();
  void feedbackCB(bool called_by_btn);
  void accessibilityCB();
  
private:
  friend class GUI;
  GUI& gui = GUI::instance;

  bool settings_window_opened{false};
  
  // get keys and masks according to buttons
  void setHotkeys(uint8_t showstates, uint8_t sedittates);
};

#endif

