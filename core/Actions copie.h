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
#include <vector>
#include <functional>
#include "Shortcut.h"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

class Command {
public:
  std::string name, title, help, arg;
  int index{0};
};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

class Action {
public:
  enum Type {
    None=0, MarkpadMenu=1<<1, HelperMenu=1<<2, CommandMenu=1<<3,
    TextField=1<<4, TextFieldOnRequest=1<<5
  };

  const Command* getCommand(const Shortcut&) const;
  const Command* getCommand(int index) const;
  const string& commandName(int index) const;
  int findCommandFromName(const string& confname) const;
  int findCommandFromTitle(const string& title) const;
  
  std::string name, title, icon, selectedIcon;
  unsigned int type{None};
  std::function<void()> fun{nullptr};
  std::vector<Command> commands;
  std::function<void(Shortcut* s, int)> helper{nullptr};
  int actionID{0};     // index in the list of actions
};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

class Actions {
public:
  /// Action singleton
  static Actions instance;
  
  /// run or pause depending on _state_.
  void run(bool state);
  
  /// executes the action of this Shortcut.
  /// the shortcut name is displayed as feedback, except if changed by the action.
  void exec(Shortcut&, const MTTouch&, Shortcut::State);
  
  std::vector<Action> & getActions();
  Action* getAction(const string& confname);
  Action* getAction(int actionindex);
  bool setShortcutAction(Shortcut&, const string& confname, const string& confargs);
  
private:
  Actions();
  Actions(const Actions&) = delete;       // can't be copied.
  Actions& operator=(Actions&) = delete;  // can't be copied.
  class CurrentAction& current;
  std::vector<Action> actions;
};

#endif
