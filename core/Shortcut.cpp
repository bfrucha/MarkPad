//
//  Shortcut.mm
//  MarkPad Project
//
//  (c) Eric Lecolinet - http://www.telecom-paristech.fr/~elc
//  (c) Bruno Fruchard - http://brunofruchard.com/
//  Copyright (c) 2017/18. All rights reserved.
//

#include <iostream>
#include <algorithm>
#include "ccuty/ccstring.hpp"
#include "Conf.h"
#include "MarkPad.h"
#include "Pad.h"
#include "Actions.h"
using namespace std;
using namespace ccuty;

Shortcut::Shortcut(ShortcutMenu& parentMenu, const string& name) :
name_(name), parentmenu_(&parentMenu) {
}

Shortcut::Shortcut(const Shortcut& s, bool duplicate_submenu) : Shortcut(s) {
  // on garde _parentMenu !!
  selected_ = false;
  if (s.submenu_ && duplicate_submenu) {
    submenu_ = new ShortcutMenu(*s.submenu_);
    setMenu(submenu_);
  }
  else submenu_ = nullptr;
}

Shortcut::~Shortcut() {
  if (submenu_) delete submenu_;
}

void Shortcut::setName(const string& name, bool xcenter, bool ycenter) {
  name_ = name;
  centerName(xcenter, ycenter);
}

void Shortcut::setParentMenu(ShortcutMenu* menu) {
  parentmenu_ = menu;
}

void Shortcut::setMenu(ShortcutMenu* menu) {
  if (submenu_) submenu_->opener_ = nullptr;
  if (menu) {
    if (menu->opener_) menu->opener_->submenu_ = nullptr;
    menu->opener_ = this;
  }
  submenu_ = menu;
}

bool Shortcut::isMenuOpened() const {
  return submenu_ == MarkPad::instance.currentMenu();
}

void Shortcut::openMenu(bool state) {
  if (submenu_) submenu_->openMenu(state);
}

void Shortcut::openCloseMenu() {
  if (submenu_) submenu_->openCloseMenu();
}

Action* Shortcut::setAction(int action_index) {
  Action* a = Actions::instance.getActionFromIndex(action_index);
  if (!a) return nullptr;
  action_ = a;
  comindex_ = -1;
  arg_ = "";
  return a;
}

const Command* Shortcut::command() const {
  if (!action_ || comindex_ < 0) return nullptr;
  return action_->command(comindex_);
}

const string& Shortcut::commandName() const {
  static const string empty;
  if (!action_) return empty;
  return action_->commandName(comindex_);
}

void Shortcut::setCommand(int16_t index) {
  comindex_ = index;
}

void Shortcut::setArg(const string& arg) {
  arg_ = arg;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

bool Shortcut::contains(float x, float y) {
  return x >= area_.x && x <= area_.x + area_.width
  && y >= area_.y && y <= area_.y + area_.height;
}

void Shortcut::setX(float x, bool move_name) {
  if (x < 0.f) x = 0.f;
  else if (x + area_.width > 1.f) {
    x = 1.f - area_.width;
    if (x >= 1.f - Conf::k.minShortcutSize.width) x = 1.f - Conf::k.minShortcutSize.width;
  }
  if (move_name) {
    namearea_.x += (x - area_.x);
    area_.x = x;
  }
  else {
    area_.x = x;
    setNameX(namearea_.x);
  }
}

void Shortcut::setY(float y, bool move_name) {
  if (y < 0.f) y = 0.f;
  else if (y + area_.height > 1.f) {
    y = 1.f - area_.height;
    if (y >= 1.f - Conf::k.minShortcutSize.height) y = 1.f - Conf::k.minShortcutSize.height;
  }
  if (move_name) {
    namearea_.y += (y - area_.y);
    area_.y = y;
  }
  else {
    area_.y = y;
    setNameY(namearea_.y);
  }
}

void Shortcut::setPos(float x, float y) {
  setX(x);
  setY(y);
}

void Shortcut::move(float deltax, float deltay) {
  setX(area_.x + deltax);
  setY(area_.y + deltay);
}

void Shortcut::moveName(float deltax, float deltay, bool constrained) {
  if (constrained) {
    setNameX(namearea_.x + deltax);
    setNameY(namearea_.y + deltay);
  }
  else {
    namearea_.x += deltax;
    namearea_.y += deltay;
  }
}

void Shortcut::setWidth(float width, bool move_name) {
  if (width < Conf::k.minShortcutSize.width) width = Conf::k.minShortcutSize.width;
  else if (area_.x + width > 1.f) width = 1.f - area_.x;
  
  float oldw = area_.width;
  area_.width = width;

  if (move_name) {
    setNameX(area_.x + (namearea_.x-area_.x) * ((width-namearea_.width) / (oldw-namearea_.width)));
  }
  else setNameX(namearea_.x);
}

void Shortcut::setHeight(float height, bool move_name) {
  if (height < Conf::k.minShortcutSize.height) height = Conf::k.minShortcutSize.height;
  else if (area_.y + height > 1.f) height = 1.f - area_.y;
  
  float oldh = area_.height;
  area_.height = height;

  if (move_name) {
    setNameY(area_.y + (namearea_.y-area_.y) * ((height-namearea_.height) / (oldh-namearea_.height)));
  }
  else setNameY(namearea_.x);
}

void Shortcut::changeWidth(float delta_width, bool from_left) {
  float newx = area_.x;
  if (from_left) {
    if (newx - delta_width < 0.f) {
      delta_width = newx;
      newx = 0.f;
    }
    else newx -= delta_width;
  }
  
  float neww = area_.width + delta_width;
  if (neww < Conf::k.minShortcutSize.width) {
    neww = Conf::k.minShortcutSize.width;
    delta_width = neww - area_.width;
    if (from_left) newx = area_.x - delta_width;
  }
  
  float oldx = area_.x, oldw = area_.width;
  area_.x = newx;
  area_.width = neww;
  setNameX(newx + (namearea_.x-oldx) * ((neww-namearea_.width) / (oldw-namearea_.width)));
}

void Shortcut::changeHeight(float delta_height, bool from_bottom) {
  float newy = area_.y;
  if (from_bottom) {
    if (newy - delta_height < 0.f) {
      delta_height = newy;
      newy = 0.f;
    }
    else newy -= delta_height;
  }
  
  float newh = area_.height + delta_height;
  if (newh < Conf::k.minShortcutSize.height) {
    newh = Conf::k.minShortcutSize.height;
    delta_height = newh - area_.height;
    if (from_bottom) newy = area_.y - delta_height;
  }
  
  float oldy = area_.y, oldh = area_.height;
  area_.y = newy;
  area_.height = newh;
  setNameY(newy + (namearea_.y-oldy) * ((newh-namearea_.height) / (oldh-namearea_.height)));
}

void Shortcut::setSize(float width, float height) {
  setWidth(width);
  setHeight(height);
}

void Shortcut::setArea(float x, float y, float width, float height) {
  setX(x);
  setY(y);
  setWidth(width);
  setHeight(height);
}

void Shortcut::setNameX(float x) {
  // when both conditions match (ie. box width is too small), name is left aligned
  if (x + namearea_.width > area_.x + area_.width - Conf::k.nameSpacing) {
    x =  area_.x + area_.width - Conf::k.nameSpacing - namearea_.width;
  }
  if (x < area_.x + Conf::k.nameSpacing) {
    x = area_.x + Conf::k.nameSpacing;
  }
  namearea_.x = x;
}

void Shortcut::setNameY(float y) {
  if (y + namearea_.height > area_.y + area_.height - Conf::k.nameSpacing) {
    y =  area_.y + area_.height - Conf::k.nameSpacing - namearea_.height;
  }
  if (y < area_.y + Conf::k.nameSpacing) {
    y = area_.y + Conf::k.nameSpacing;
  }
  namearea_.y = y;
}

void Shortcut::centerName(bool xcenter, bool ycenter) {
  GUI::instance.layoutShortcut(*this);
  if (xcenter) namearea_.x = area_.x + (area_.width/2.f) - (namearea_.width/2.f);
  if (ycenter) namearea_.y = area_.y + (area_.height/2.f) - (namearea_.height/2.f);
}

void Shortcut::copyArea(const MTRect& area) {
  area_ = area;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

ShortcutMenu::~ShortcutMenu() {
  for (auto& it : shortcuts_) delete it;
}

ShortcutMenu::ShortcutMenu(const ShortcutMenu& from) {
  for (auto& it : from.shortcuts_) {
    Shortcut* dup = new Shortcut(*it, false);
    shortcuts_.push_back(dup);
    dup->setParentMenu(this);
  }
}

bool ShortcutMenu::addShortcut(Shortcut& s) {
  auto it = find(shortcuts_.begin(), shortcuts_.end(), &s);
  if (it != shortcuts_.end()) return false;  // already in the menu
  else {
    shortcuts_.push_back(&s);
    s.setParentMenu(this);
    return true;
  }
}

Shortcut* ShortcutMenu::addNewShortcut(const string& name) {
  Shortcut* s = new Shortcut(*this, name);
  shortcuts_.push_back(s);
  s->setParentMenu(this);
  return s;
}

void ShortcutMenu::removeShortcut(Shortcut& s) {
  vector<Shortcut*>::iterator where;
  bool found = false;
  
  for (auto it = shortcuts_.begin(); it != shortcuts_.end(); ++it) {
    // iterateur du shortcut dans le menu
    if (*it == &s) {found = true; where = it;}
  }
  if (found) {
    shortcuts_.erase(where);
  }
}

bool ShortcutMenu::containsShortcut(Shortcut& s) {
  return (std::find(shortcuts_.begin(), shortcuts_.end(), &s) != shortcuts_.end());
}

Shortcut* ShortcutMenu::findShortcut(const string& name) const {
  for (auto s : shortcuts_) {
    if (s->name_ == name) return s;
  }
  return nullptr;
}

Shortcut* ShortcutMenu::findShortcut(const MTTouch& touch) const {
  for (auto s : shortcuts_) {
    if (Pad::isInside(touch.norm.pos, s->area_)) return s;
  }
  return nullptr;
}

bool ShortcutMenu::isMenuOpened() const {
  return this == MarkPad::instance.currentMenu();
}

bool ShortcutMenu::isCascaded() const {
  return (opener_ && opener_->parentmenu_ && opener_->parentmenu_->opener_);
}

void ShortcutMenu::openMenu(bool state) {
  if (state) {
    MarkPad::instance.setCurrentMenu(this);
  }
  else if (opener_ && opener_->parentmenu_) {
    MarkPad::instance.setCurrentMenu(opener_->parentmenu_);
  }
  
  // changing menu must clear selected items
  MarkPad::instance.clearMultiSelection();
  
  if (!MarkPad::instance.isOverlayShown()) MarkPad::instance.showOverlay(true);
  else GUI::instance.updateOverlay();
}

void ShortcutMenu::openCloseMenu() {
  openMenu(!isMenuOpened());
}
