//
//  Actions.h
//  MarkPad Project
//
//  (c) Eric Lecolinet - http://www.telecom-paristech.fr/~elc
//  (c) Bruno Fruchard - http://brunofruchard.com/
//  Copyright (c) 2017/18. All rights reserved.
//
 
#ifndef MarkPad_Actions
#define MarkPad_Actions

#include <string>
#include <map>
#include <vector>
#include <functional>
#include "Shortcut.h"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

class Helper {
public:
  struct Item {
    std::string item, command;
  };
  std::string title;
  std::vector<Item> menu;
  void (*fun)(Helper&, Shortcut*, const class Command*, int helperindex);
};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

class Command {
public:
  std::string name, title, help, arg;
  Helper* helper{nullptr};
  class Action* action{nullptr};
  int index{0};
};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

class Action {
public:
  enum Type {Standard=0, SubMenu=1<<1, Config=1<<2, Hotkey=1<<3};

  bool isHotkey() const {return (type & Hotkey) != 0;}
  const Command* command(const Shortcut&) const;
  const Command* command(int index) const;
  const string& commandName(int index) const;
  int findCommandFromName(const string& confname) const;
  int findCommandFromTitle(const string& title) const;

  std::string title, icon, selectedIcon;
  unsigned int type{Standard};
  std::function<void()> fun{nullptr};
  std::vector<Command> commands;
  int actionID{0};     // index in the list of actions
};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

class Actions {
public:
  static const std::string CurrentApp, CurrentFile;

  /// Action singleton
  static Actions instance;
  
  /// run or pause depending on _state_.
  void run(bool state);
  
  /// executes the action of this Shortcut.
  /// the shortcut name is displayed as feedback, except if changed by the action.
  void exec(Shortcut&, const MTTouch&, Shortcut::State);

  static void setCurrentFileOrURL(Shortcut*, bool openURL);

  std::vector<Action> & getActions();
  Action* getActionFromCommand(const string& command);
  Action* getActionFromIndex(int actionindex);

  bool setShortcutAction(Shortcut&s,
                         std::string const& command, std::string const& args);

  bool setShortcutActionFromConfFile(Shortcut&, const string& keyword,
                                     const string& config);
  void quitBrowser();

private:
  Actions();
  Actions(const Actions&) = delete;       // can't be copied.
  Actions& operator=(Actions&) = delete;  // can't be copied.
  class CurrentAction& current;
  std::vector<Action> actions;
  std::map<std::string,Command*> commandmap;
};

#endif
