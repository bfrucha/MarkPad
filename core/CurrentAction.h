//
//  CurrentAction.hpp
//  MarkPad project
//
//  (c) Eric Lecolinet - http://www.telecom-paristech.fr/~elc
//  (c) Bruno Fruchard - http://brunofruchard.com/
//  Copyright (c) 2017/18. All rights reserved.
//

#ifndef CurrentAction_h
#define CurrentAction_h

#include "Actions.h"

class CurrentAction {
public:
  class Cnx* cnx{nullptr};
  enum Status {OK, NoFeedback, Error} status{OK};
  Shortcut* shortcut{nullptr};
  const Action* action{nullptr};
  const Command* command{nullptr};
  Shortcut::State touchstate{};
  MTTouch touch{};
  std::string feedback;
  
  void exec(Shortcut&, const MTTouch&, Shortcut::State);
  void doOpen();
  void doCommand();
  void doWindow();
  void doHotkey();
  void doClipboard();
  void doClicks();
  void openEditor();
  void setVolume(const std::string& vol);
  void openApp(const std::string& appname);
  void openHideApp(const std::string& appname);
  string getFile(const std::string& file);
  string getApp(const std::string& app);
  unsigned long long int getAppID(const std::string& app);
  void redoKey();
  bool connected();
  void connect();
  void disconnect();
  void macmote(const string& command);
  /*
   const string& arg() const {return shortcut_->arg();}
   float globalX() const {return touch_.norm.pos.x;}
   float globalY() const {return touch_.norm.pos.y;}
   float localX()  const {return touch_.norm.pos.x - shortcut_->area().x;}
   float localY()  const {return touch_.norm.pos.y - shortcut_->area().y;}
   */
};

#endif
