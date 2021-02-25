//
//  Editor.h
//  MarkPad Project
//
//  (c) Eric Lecolinet - http://www.telecom-paristech.fr/~elc
//  (c) Bruno Fruchard - http://brunofruchard.com/
//  Copyright (c) 2017/2020. All rights reserved.
//

#ifndef MARKPAD_EDITOR
#define MARKPAD_EDITOR

#include <iostream>
#include "ccuty/ccpath.hpp"
#include "Conf.h"
#include "Strings.h"
#include "AWL.h"
#include "GUI.h"
#include "MarkPad.h"
#include "Overlay.h"
#include "Settings.h"
#include "Actions.h"
#include "Services.h"
using namespace std;

class Editor : public AWL {
public:
  enum SetActionStatus {ActionInvalid, ActionWithMenu, ActionWithoutMenu};

private:
  friend class GUI;
  GUI& gui = GUI::instance;
  bool show_more{false}, choosing_URL{false};
  AWColor* selectionColor = AWL::newColor(0.995, 0., 0.5);
  //AWColor* selectionBgColor = AWL::ClearColor;
  AWColor* tipColor = AWL::newColor(1, 0.3, 0.);
  //AWColor* tipBgColor = AWL::newColor(1, 1, 1.);

#ifdef WITH_QT
  std::vector<AWWidget*> showMoreBtns;
#else
  std::vector<AWTextField*> sizeFields;
#endif
  std::vector<AWSpinBox*> sizeSpins;
  std::vector<AWButton*> actionButtons;

  // - - - -
  
  void init() {
    sizeSpins = {
      del->areaX, del->areaY, del->areaW, del->areaH, del->nameX, del->nameY
    };
#ifdef WITH_QT
    for (auto& w : sizeSpins) {
      setDecimals(w, 4);
      setRange(w, 0., 1.);
      setStep(w, 0.0005);
    }
    showMoreBtns = {del->closeBtn, del->quitBtn, del->showMoreBtn};
#else
    sizeFields = {
      del->areaXField, del->areaYField, del->areaWField, del->areaHField,
      del->nameXField, del->nameYField
    };
#endif
    
    setState(del->showDesktopBtn, false);
    // must have different value to do something
    show_more = true;
    showMore(false);
  }

public:

  void openEditor() {
    // run again (if needed)
    GUI::mp.run(true);
    GUI::mp.setCurrentMenu();
    updateShortcut(nullptr);
    GUI::mp.edit(MarkPad::EditShortcuts);  // will call editCB()
  }
  
  void closeEditor() {
    if (choosing_URL) {
      setText(del->commandField, "");
      showDesktop(false);
    }
    else {
      MarkPad::instance.edit(MarkPad::EditNone);  // will call editCB()
      gui.settings.closeSettingsWindowCB();
    }
  }
  
  // called by MarkPad::edit(), must not be called directly!
  // note that this function should only be called from the main thread!
  void editCB(bool state) {
    if (state) showDesktop(false);
    else {
      Overlay::instance.show(GUI::OverlayHide);
      show(del->editWin, false);
      show(del->helpWin, false);
      show(del->settingsWin, false);
      Conf::saveIfNeeded();   // save files when editing is over
    }
    MarkPad::instance.setCurrentMenu();
    MarkPad::instance.selectShortcut(nullptr);
  }
  
  // hides editor temporarily
  void hideEditor(bool state) {
    if (state) show(del->editWin, false);
    else show(del->editWin, true);
  }
  
  // hides all menus (to show the desktop)
  void showDesktop(bool state) {
    MarkPad::instance.setDesktopMode(state);
    if (!state) {
      // changed when choosing url
      choosing_URL = false;
      setState(del->showDesktopBtn, false);
      setText(del->doneBtn, "Close");
      highlight(del->doneBtn, false);
      Overlay::instance.show(GUI::OverlayEdit);
      show(del->editWin, true);
    }
    else {
      setState(del->showDesktopBtn, true);
      setText(del->doneBtn, "Back to Markpad");
      highlight(del->doneBtn, true);
      Overlay::instance.show(GUI::OverlayHide);
      show(del->editWin, true);
    }
  }
  
#ifdef WITH_QT
  
  void showMore(bool state) {}   // TODO !!!
  
#else
  
  void showMore(bool state) {
    if (show_more == state) return;
    show_more = state;
    setState(del->showMoreBtn, state);
    setState(del->showMoreBtn2, state);
    show(del->morePane, state);
    
    double delta = getHeight(del->morePane);
    if (!state) delta = -delta;
    
#ifdef WITH_QT
    for (auto& btn : showMoreBtns) {
      if (btn) {
        double x = getX(btn), y = getY(btn);
        move(btn, x, y+delta);
      }
    }
#endif
    changeHeight(del->editWin, delta);
    updateShortcut(GUI::mp.currentShortcut());  // to update the action menu
  }
  
#endif
  
  void attractEditor() {
    Shortcut* s = GUI::mp.currentShortcut();
    if (!s) return;
    double w = getWidth(del->editWin);
    double h = getHeight(del->editWin);
    MTPoint pos = {
      float((s->x() + s->width() / 2.) * getScreenWidth() - w / 2.),
      float(s->y() * getScreenHeight() - 10)
    };
    if (pos.x < 100) pos.x = 100;
    else if (pos.x + w + 100 > getScreenWidth()) {
      pos.x = getScreenWidth() - w - 100;
    }
    if (pos.y - h - 100 < 0) pos.y = h + 100;
    move(del->editWin, pos.x, pos.y);
    show(del->editWin, true);
    setFocus(del->editWin);
    setFocus(del->shortcutName);
  }
  
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // Shortcut creation, cut & paste

  void completeShortcutCreation(ShortcutMenu& menu, Shortcut& s) {
    Overlay::instance.layoutShortcut(s);
    s.centerName(true, false);
    GUI::mp.selectShortcut(&s);
    if (menu.isMainMenu()) setShortcutAction(&s, 0/*actionNo*/);
    updateShortcut(&s);
  }
          
  void pasteShortcuts() {
    ShortcutMenu* menu = GUI::mp.currentMenu();
    if (!menu) return;
    if (Shortcut* dup = GUI::mp.pasteShortcuts(*menu)) {
      GUI::mp.selectShortcut(dup);
      setFocus(del->shortcutName);
      updateShortcut(dup);
    }
  }
          
  void copyShortcuts() {
    GUI::mp.copyShortcuts();
    enable(del->pasteItem, true);
  }
          
  void cutShortcuts() {
    GUI::mp.cutShortcuts();
    GUI::mp.selectShortcut(nullptr);
    enable(del->pasteItem, true);
    updateShortcut(nullptr);
  }
          
  void deleteShortcuts() {
    // don't delete the menu that is currently shown
    Shortcut* s = GUI::mp.currentShortcut();
    if (s && s->menu() == GUI::mp.currentMenu()) return;
    GUI::mp.deleteShortcuts();
    GUI::mp.selectShortcut(nullptr);
    Overlay::instance.update();
  }
          
  void undeleteShortcuts() {
    if (Shortcut* s = GUI::mp.undeleteShortcuts()) {
      ShortcutMenu* menu = s->parentMenu();
      if (menu) menu->openMenu(true);
      GUI::mp.selectShortcut(s);
      Overlay::instance.update();
    }
  }
  
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // actions
  
  void addActionButton(AWButton* button) {
     actionButtons.push_back(button);
  }
  
  enum {NoAction=-1, AllActions=-2, AllActionsBut0 = -3};
  
  void enableAction(int index) {
    if (index == NoAction) {
      for (auto& b : actionButtons) enable(b, false);
    }
    else if (index == AllActions) {
      for (auto& b : actionButtons) enable(b, true);
    }
    else if (index == AllActionsBut0) {
      int k = 0;
      for (auto& b : actionButtons) {
        if (k == 0) enable(b, false); else enable(b, true);
        k++;
      }
    }
    else {
      int k = 0;
      for (auto& b : actionButtons) {
        if (k == index) enable(b, true); else enable(b, false);
        k++;
      }
    }
  }
  
  void selectAction(int index) {
    int k = 0;
    for (auto& b : actionButtons) {
      if (k == index) setState(b, true); else setState(b, false);
      k++;
    }
  }
  
  SetActionStatus actionCB(AWButton* item) {
    int k = 0, index = -1;
    for (auto& it : actionButtons) {
      if (it == item) {index = k; break;}
      k++;
    }
    auto* a = setShortcutAction(gui.mp.currentShortcut(), index);
    if (!a) return ActionInvalid;
    else if (a->commands.empty()) return ActionWithoutMenu;
    else return ActionWithMenu;
  }
  
  Action* setShortcutAction(Shortcut* s, int actionNo) {
    auto* a = Actions::instance.getActionFromIndex(actionNo);
    if (!s || !a) return nullptr;
    
    if (s->action() == a) return a;   // do nothing if unchanged
    
    // cannot change unempty menu
    if (s->submenu_ && !s->submenu_->empty()) {
      setText(del->actionCommand, "!!! Can't change unempty Menu (delete Menu items first)");
      return nullptr;
    }
    
    s->setAction(actionNo);
    Conf::mustSave();
    setText(del->commandField, "");
    
    if (actionNo == 0) {    // action 0 is OpenMenu
      // shortcut doesnt have a submenu => create a menu
      if (!s->menu()) s->setMenu(GUI::mp.createMenu(*s));
      s->setName("Menu", true, false);  // preset title
    }
    else {
      // existing submenu => copy shortcut to save the menu
      if (s->menu()) {
        Shortcut* dup = new Shortcut(*s, false);  // dont copy submenu
        dup->setMenu(s->menu());  // own submenu
        GUI::mp.deleteShortcutImpl(*dup);
      }
      s->setMenu(nullptr);
      if (!s->command()) s->setName("", true, false);  // preset title
    }
    
    updateShortcut(s);
    return a;
  }

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // action field & commands
  
  void helperCB(AWMenuItem* item) {
    Shortcut* s = GUI::mp.currentShortcut();
    if (!s) return;
    int index = AWL::itemIndex(del->helperMenu, item);
    if (index < 0) return;
    
    const Command* c = s->command();
    if (c && c->helper) (c->helper->fun)(*c->helper, s, c, index);
    Conf::mustSave();
    updateShortcut(s);
  }
  
  void commandCB(AWMenuItem* item) {
    Shortcut* s = GUI::mp.currentShortcut();
    if (!s) return;
    int index = AWL::itemIndex(del->commandMenu, item);
    if (index < 0) return;
  
    s->setCommand(index);
    const Command* c = s->command();
    if (c) s->setName(c->title, true, false); // preset shortcut title
    Conf::mustSave();
    updateShortcut(s);
  }
  
  void actionFieldCB(const std::string& str) {
    Shortcut* s = GUI::mp.currentShortcut();
    if (!s) return;
    s->setArg(str);
    if (choosing_URL) showDesktop(false);
    Conf::mustSave();
    Overlay::instance.update();
  }
  
  // show tip when the cursor moves over the action buttons
  void showActionTip(Action* a, bool enter) {
    if (!a) return;
    if (enter) {
      setColor(del->actionTitle, tipColor);
      //setBgColor(del->actionTitle, tipBgColor);
      setText(del->actionTitle, a->title);
    }
    else {
      Shortcut* s = GUI::mp.currentShortcut();
      const Action* a = s ? s->action() : nullptr;
      if (a) {
        setColor(del->actionTitle, selectionColor);
        //setBgColor(del->actionTitle, selectionBgColor);
        setText(del->actionTitle, a->title);
      }
      else {
        setText(del->actionTitle,"");
      }
    }
  }
  
  void openFinder() {
    Shortcut* s = GUI::mp.currentShortcut();
    if (!s) return;
    showDesktop(true);
    hideEditor(true);
    std::string filename;
    if (showOpenDialog(filename)) {
      setText(del->commandField, filename);
      s->setArg(filename);
      s->setName(ccuty::basename(filename), true, false);
      Conf::mustSave();
    }
    showDesktop(false);
    hideEditor(false);  // needed by Qt - marche pas: cache l'editeur a revoir!!!
  }
  
  void openWebBrowser() {
    choosing_URL = true;
    showDesktop(true);
    Services::openDefaultWebBrowser();
    hideEditor(false);    // editor must remain visible (doesnt work with Qt?) !!!!
  }
  
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // action update
  
  // show the actionField but makes it invisible so that its fills the space
  void showActionFieldHidden() {
    enable(del->commandField, false);
    showContent(del->commandField, false);
    show(del->commandField, true);
    setText(del->commandField, "");
    highlight(del->commandField, false);
  }
  
  void setHelper(const Action* action, Helper* helper) {
    show(del->helperBtn, false);
    show(del->showMenuBtn, false);
    if (!action) return;

    if (action->type == Action::SubMenu) {
      setText(del->actionCommand, "Opens a SubMenu (click on 'Show Menu' to add items)");
      show(del->showMenuBtn, true);
      return;
    }
    
    if (!helper) return;
    removeAllItems(del->helperMenu);
    
    if (action->type == Action::Config) {
      if (auto* mainmenu = MarkPad::instance.mainMenu()) {
        helper->menu.resize(mainmenu->size());
        int k = 0;
        for (auto& s : mainmenu->shortcuts()) {
          if (s && s->submenu_) helper->menu[k++].item = s->name();
        }
      }
    }
      
    for (const auto& it : helper->menu) {
      addItem(del->helperMenu, del->helperCB, it.item);
    }
    setText(del->helperBtn, helper->title);
    show(del->helperBtn, true);
  }
  
  void updateShortcutAction(const Shortcut* s) {
    setHelper(nullptr, nullptr);
    show(del->commandPane, false);
    show(del->commandField, false);
    show(del->modifierPane, false);
    setText(del->actionTitle, "");
    setText(del->actionCommand, "");
    if (!s) {
      enableAction(NoAction);
      selectAction(NoAction);
      return;
    }
    ShortcutMenu* parent = s->parentMenu();
    
    if (!parent) enableAction(AllActions);
    else if (parent->isMainMenu()) enableAction(0);
    else if (parent->isCascaded()) enableAction(AllActionsBut0);
    else enableAction(AllActions);
    
    const Action* action = s->action();
    
    if (s->menu()) {  // must be first!
      selectAction(0);
      if ((action = Actions::instance.getActionFromIndex(0))) {   // menu action
        setColor(del->actionTitle, selectionColor);
        //setBgColor(del->actionTitle, selectionBgColor);
        setText(del->actionTitle, action->title);
      }
      show(del->commandPane, true);
      setHelper(action, nullptr);
      showActionFieldHidden();
      return;
    }
    
    if (!action) {
      selectAction(NoAction);
      setColor(del->actionTitle, selectionColor);
      //setBgColor(del->actionTitle, selectionBgColor);
      setText(del->actionTitle, "Choose Desired Action");
      setHelper(nullptr, nullptr);
      return;
    }
    
    // action exists
    selectAction(action->actionID);
    show(del->commandPane, true);
    setColor(del->actionTitle, selectionColor);
    //setBgColor(del->actionTitle, selectionBgColor);
    setText(del->actionTitle, action->title);
    enable(del->commandField, true);
    showContent(del->commandField, true);
    bool showActionField = false;
    
    if (action->type == Action::SubMenu) {
      showActionFieldHidden();  // action field shown in hidden state (for layout)
      setHelper(action, nullptr);
      return;
    }
    
    // update CommandMenu
    removeAllItems(del->commandMenu);
    for (const auto& it : action->commands) {
      addItem(del->commandMenu, del->commandCB, it.title);
    }
    
    const Command* cmd = action->command(*s);
    if (!cmd) setText(del->actionCommand, "");
    else {
      setText(del->actionCommand, cmd->title+" "+cmd->help);
      setHelper(action, cmd->helper);
    }
    
    if (cmd && !cmd->arg.empty()) {
      if (cmd->arg[0] == '?') {
        showActionField = true;
        if (action->isHotkey()) {
          del->modifierButtons->setStates(s->modifiers());
          show(del->modifierPane, true);
        }
      }
    }
    
    if (!showActionField) showActionFieldHidden();
    else {
      setText(del->commandField, s->arg());
      show(del->commandField, true);
    }
  }
  
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // shortcut update
  
  void updateShortcut(const Shortcut* s) {
    ShortcutMenu* menu = GUI::mp.currentMenu();
    bool inMainMenu = menu ? menu->isMainMenu() : true;
    
    enable(del->undeleteItem, !GUI::mp.deleteBufferEmpty());
    enable(del->pasteItem, !GUI::mp.pasteBufferEmpty());
    
    if (s) {
      enable(del->deleteItem, true);
      enable(del->copyItem, true);
      enable(del->cutItem, true);
      enable(del->gotoBtn, true);
      enable(del->touchFromBorder,true);
      enable(del->touchOpenMenu,true);
      setState(del->touchOpenMenu, s->touchOpenMenu_);
      setState(del->touchFromBorder, s->touchFromBorder_);
      
      //if (inMainMenu) enable(del->menuBtn, s->menu() != nullptr);
      //else enable(del->menuBtn, true);
      enable(del->menuBtn, s->menu() != nullptr);
      enable(del->shortcutName, true);
      setText(del->shortcutName, s->name());
      updateShortcutAction(s);
    }
    
    else {      // no shortcut
      enable(del->deleteItem, false);
      enable(del->copyItem, false);
      enable(del->cutItem, false);
      enable(del->gotoBtn, false);
      enable(del->touchFromBorder,false);
      enable(del->touchOpenMenu,false);

      if (inMainMenu) enable(del->menuBtn, false);
      else enable(del->menuBtn, true);
      enable(del->shortcutName, false);
      setText(del->shortcutName, "");
      updateShortcutAction(nullptr);
    }
    
    updateShortcutSize(s);
    // update the view as shortcut attributes have changed
    Overlay::instance.update();
  }

#ifdef WITH_QT
  
  void updateShortcutSize(const Shortcut* s) {
    if (s) {
      for (auto& w : sizeSpins) if (w) enable(w, true);
      setValue(del->areaX, s->x());
      setValue(del->areaY, s->y());
      setValue(del->areaW, s->width());
      setValue(del->areaH, s->height());
      setValue(del->nameX, s->nameX());
      setValue(del->nameY, s->nameY());
    }
    else {
      for (auto& w : sizeSpins) {
        if (w) {enable(w, false); setValue(w, 0.);}
      }
    }
    Overlay::instance.update();
  }
  
#else
  
  void setTextValue(AWTextField* w, float val) {
    char buf[10] = "";
    sprintf(buf, "%.4f", val);
    setText(w, buf);
  }
  
  void updateShortcutSize(const Shortcut* s) {
    if (s) {
      for (auto& w : sizeSpins) if (w) enable(w, true);
      for (auto& w : sizeFields) if (w) enable(w, true);
      setTextValue(del->areaXField, s->x());
      setTextValue(del->areaYField, s->y());
      setTextValue(del->areaWField, s->width());
      setTextValue(del->areaHField, s->height());
      setTextValue(del->nameXField, s->nameX());
      setTextValue(del->nameYField, s->nameY());
      for (auto& w : sizeSpins) setValue(w, 0.);
    }
    else {
      for (auto& w : sizeSpins) {
        if (w) {enable(w, false); setValue(w, 0.);}
      }
      for (auto& w : sizeFields) {
        if (w) {enable(w, false); setText(w, "");}
      }
    }
    Overlay::instance.update();
  }
#endif
  
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // Contextual menu an Alignment
  
  void alignTitles(BoxPart::Value part) {
    // BoxPart::Title active if only one shortcut selected
    if (part == BoxPart::Title && GUI::mp.currentShortcut()) {
      GUI::mp.currentShortcut()->centerName(true, true);
    }
    // any boxpart active (including Title) if multiSelection buffer not empty
    if (!GUI::mp.multiSelection().empty()) GUI::mp.alignTitles(part);
    Overlay::instance.update();
  }
  
  void alignBoxes(BoxPart::Value part) {
    // inactive (meaningless) if multiSelection buffer is empty
    if (GUI::mp.multiSelection().empty()) return;
    GUI::mp.alignBoxes(part);
    Overlay::instance.update();
  }
  
  void adjustBoxes(BoxPart::Value part, BoxPart::Value part2) {
    // inactive (meaningless) if multiSelection buffer is empty
    if (GUI::mp.multiSelection().empty()) return;
    GUI::mp.adjustBoxes(part);
    if (part2 != BoxPart::Outside) GUI::mp.adjustBoxes(part2);
    Overlay::instance.update();
  }
  
  void snapToGuides() {
    // active if only one shortcut selected
    if (GUI::mp.currentShortcut()) {
      GUI::mp.adjustBoxToGuide(*GUI::mp.currentShortcut());
    }
    // active if multiSelection buffer not empty
    if (!GUI::mp.multiSelection().empty()) {
      for (auto& it : GUI::mp.multiSelection()) GUI::mp.adjustBoxToGuide(*it);
    }
    Overlay::instance.update();
  }
  
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // shortcut attributes
  
  void nameFieldCB(const std::string& str) {
    Shortcut* s = GUI::mp.currentShortcut();
    if (!s) return;
    // we may have duplicated names but the conf format supports it
    s->setName(str, true, false);
    Conf::mustSave();
    Overlay::instance.update();
  }

#ifdef WITH_QT
  
  // for QtSpinBox. Note: areaXField etc. are not used (and null) with Qt
  void sizeSpinCB(AWSpinBox* w, float val) {
    Shortcut* s = GUI::mp.currentShortcut();
    if (!s) return;
    if (w == del->areaX) s->setX(val);
    else if (w == del->areaY) s->setY(val);
    else if (w == del->areaW) s->setWidth(val);
    else if (w == del->areaH) s->setHeight(val);
    else if (w == del->nameX) s->setNameX(val);
    else if (w == del->nameY) s->setNameY(val);
    Conf::mustSave();
    updateShortcutSize(s);
  }
  
#else
  
  // for Cocoa TextArea
  void sizeFieldCB(AWTextField* w, float val) {
    Shortcut* s = GUI::mp.currentShortcut();
    if (!s) return;
    if (w == del->areaXField) s->setX(getFloat(w));
    else if (w == del->areaYField) s->setY(getFloat(w));
    else if (w == del->areaWField) s->setWidth(getFloat(w));
    else if (w == del->areaHField) s->setHeight(getFloat(w));
    else if (w == del->nameXField) s->setNameX(getFloat(w));
    else if (w == del->nameYField) s->setNameY(getFloat(w));
    Conf::mustSave();
    Overlay::instance.update();
  }
  
  // for Cocoa NSStepper
  static float stepperIncr(AWSpinBox* stepper) {
    double val = getValue(stepper);
    setValue(stepper, 0.);
    if (val > 0.) return Conf::k.stepperIncrement;
    else if (val < 0.) return -Conf::k.stepperIncrement;
    else return 0.;
  }
  
  void sizeStepperCB(AWSpinBox* sender) {
    Shortcut* s = GUI::mp.currentShortcut();
    if (!s) return;
    if (sender == del->areaX)
      s->setX(stepperIncr(sender) + s->x());
    else if (sender == del->areaY)
      s->setY(stepperIncr(sender) + s->y());
    else if (sender == del->areaW)
      s->setWidth(stepperIncr(sender) + s->width());
    else if (sender == del->areaH)
      s->setHeight(stepperIncr(sender) + s->height());
    else if (sender == del->nameX)
      s->setNameX(stepperIncr(sender) + s->nameX());
    else if (sender == del->nameY)
      s->setNameY(stepperIncr(sender) + s->nameY());
    Conf::mustSave();
    updateShortcutSize(s);
  }
#endif
};

#endif
