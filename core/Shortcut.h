//
//  Shortcut.h
//  MarkPad Project
//
//  (c) Eric Lecolinet - http://www.telecom-paristech.fr/~elc
//  (c) Bruno Fruchard - http://brunofruchard.com/
//  Copyright (c) 2017/18. All rights reserved.
//

#ifndef MarkPad_Shortcut
#define MarkPad_Shortcut

#include <string>
#include <vector>
#include <list>
#include <map>
#include "MTouch.h"

using std::string;
class Pad;
class ShortcutMenu;
class Action;
class Command;

/** MarkPad shortcut.
 */
class Shortcut {
public:
  /// shortcut states.
  enum State {Idle, Up, Down, Move};
  
  // needed for serialization
  Shortcut() = default;
  
  Shortcut(ShortcutMenu&, const string& name);
  
  // note: keeps the parent menu
  Shortcut(const Shortcut& s, bool duplicate_submenu);
  
  // note: deletes submenu if any
  ~Shortcut();
  
  bool isSelected() const {return selected_;}
  void setSelected(bool state) {selected_ = state;}

  /// name of the shortcut
  const string& name() const {return name_;}
  void setName(const string& name, bool xcenter, bool ycenter);

  /// associated action.
  const Action* action() const {return action_;}
  Action* setAction(int index);

  /// associated command.
  const Command* command() const;
  int16_t commandIndex() const {return comindex_;} // -1 if undefined).
  const string& commandName() const;
  void setCommand(int16_t comindex);

  /// argument of the action.
  const string& arg() const {return arg_;}
  void setArg(const string& arg);
  
  bool isMenuOpened() const;
  void openMenu(bool state = true);
  void openCloseMenu();
  
  /// when applicable, the menu that is opened by this shortcut.
  ShortcutMenu* menu() const {return submenu_;}
  void setMenu(ShortcutMenu*);
  
  /// the menu that contains this shortcut.
  ShortcutMenu* parentMenu() const {return parentmenu_;}
  void setParentMenu(ShortcutMenu*);
  
  uint8_t modifiers() const {return modifiers_;}
  void setModifiers(uint8_t value)  {modifiers_ = value;}

  float x() const {return area_.x;}
  float y() const {return area_.y;}
  float width() const {return area_.width;}
  float height() const {return area_.height;}
  const MTRect& area() const {return area_;}
  bool contains(float x, float y);
  
  void setX(float x, bool move_name = true);
  void setY(float y, bool move_name = true);
  void setPos(float x, float y);
  void move(float deltax, float deltay);

  void setWidth(float w, bool move_name = true);
  void setHeight(float h, bool move_name = true);
  void setSize(float width, float height);
  void changeWidth(float delta_width, bool from_left);
  void changeHeight(float delta_height, bool from_bottom);

  void setArea(float x, float y, float width, float height);
  void copyArea(const MTRect& area);  ///< just a copy, does not check anything.

  float nameX() const {return namearea_.x;}
  float nameY() const {return namearea_.y;}
  float nameWidth() const {return namearea_.width;}
  float nameHeight() const {return namearea_.height;}
  const MTRect& nameArea() const {return namearea_;}
  MTRect& nameAreaRef() const {return namearea_;}  ///< returns a modifiable reference.

  void setNameX(float x);
  void setNameY(float y);
  void moveName(float deltax, float deltay, bool constrained);
  void centerName(bool xcenter, bool ycenter);
  
  Shortcut(const Shortcut&) = default;

  std::string    name_;
  int16_t        comindex_{-1};
  bool           selected_{false}, cannotEdit_{false},
                 touchOpenMenu_{true}, touchFromBorder_{true};
  uint8_t        modifiers_{0};
  Action*        action_{nullptr};
  std::string    arg_, *feedback_{nullptr};
  ShortcutMenu*  parentmenu_{nullptr};
  ShortcutMenu*  submenu_{nullptr};
  MTRect         area_{};
  mutable MTRect namearea_{};
};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

using Shortcuts = std::vector<Shortcut*>;

/** shortcut menu.
 */
class ShortcutMenu {
public:
  ShortcutMenu() = default;
  ShortcutMenu(const ShortcutMenu&);
  ~ShortcutMenu();

  bool isMenuOpened() const;
  bool isCascaded() const;
  void openMenu(bool state = true);
  void openCloseMenu();
  
  bool isMainMenu() const {return isMainMenu_;}
  void setMainMenu() {isMainMenu_ = true;}
  
  Shortcut* opener() const {return opener_;}
  int  shortcutNum() const {return shortcutNum_;}
  void setShortcutNum(int num) {shortcutNum_ = num;}
  
  bool addShortcut(Shortcut&);
  Shortcut* addNewShortcut(const string& shortcut_name);
  void removeShortcut(Shortcut&);  ///< removes this shortcut without destroying it.
  
  bool containsShortcut(Shortcut&);
  Shortcut* findShortcut(const string& name) const;
  Shortcut* findShortcut(const MTTouch&) const;
  
  long size()  const {return shortcuts_.size();}
  bool empty() const {return shortcuts_.empty();}
  Shortcuts& shortcuts() {return shortcuts_;}
  const Shortcuts& shortcuts() const {return shortcuts_;}

private:
  friend class ConfImpl;
  friend class Shortcut;
  bool isMainMenu_{false};
  int  shortcutNum_{0};
  Shortcut* opener_{nullptr};
  std::vector<Shortcut*> shortcuts_;
};

#endif
