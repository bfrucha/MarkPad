//
//  CurrentAction.cpp
//  MarkPad Project
//
//  (c) Eric Lecolinet - http://www.telecom-paristech.fr/~elc
//  (c) Bruno Fruchard - http://brunofruchard.com/
//  Copyright (c) 2017/18. All rights reserved.
//

#include <iostream>
#include <thread>
#include "ccuty/ccstring.hpp"
#include "ccuty/ccpath.hpp"
#include "ccuty/ccsocket.hpp"
#include "Conf.h"
#include "MarkPad.h"
#include "CurrentAction.h"
#include "Services.h"
using namespace ccuty;
using namespace std;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void CurrentAction::exec(Shortcut& s, const MTTouch& touch_, Shortcut::State touchstate_) {
  action = s.action();
  if (!action) return;
  
  int cmd = s.comindex_;
  if (cmd >= 0 && cmd < int(action->commands.size())) command = &action->commands[cmd];
  else command = nullptr;
  
  status = OK;
  shortcut = &s;
  touch = touch_;
  touchstate = touchstate_;
  feedback.clear();
  std::string f;
  
  try {
    if (action->fun) (action->fun)(); else throw 1;
    
    if (status == Error) throw 1;
    else return; 

    /*
    // if 's.feedback' is defined
    else if (s.feedback_) {
      // display if not empty and not equal to "none"
      if (!s.feedback_->empty() && *s.feedback_ != "none") {
        f = *s.feedback_;
      }
    }
    else if (status == OK) {
      // display 'feedback' if not empty, the name of the shortcut otherwise
      f = feedback.empty() ? s.name_ : feedback;
    }
    else return; // NoFeedback: dont display feeddback
     */
  }
  catch (...) {
    f = "Invalid Action: "+s.name()+" "+s.arg();
    return;
  }

  // aucun de ces solutions ne marche avec Mojave !!!
  // la premiere fait reapparaitre le menu complet
  // le premiere est que NSView.drawRect n'est pas toujours appelée
  // Services::postpone([f](){GUI::instance.showFeedback(f);});
  // GUI::instance.showFeedback(f);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

string CurrentAction::getFile(const string& file) {
  if (file != Actions::CurrentFile) return file;
  string f;
  if (!Services::getFinderFile(f) || f.empty()) return "";
  else return f;
}

string CurrentAction::getApp(const string& app) {
  if (app != Actions::CurrentApp) return app;
  string a;
  if (!Services::getFrontAppName(a) || a.empty()) return "";
  else return a;
}

unsigned long long int CurrentAction::getAppID(const string& app) {
  if (app != Actions::CurrentApp) return strid(tolower(app));
  string a;
  if (!Services::getFrontAppName(a) || a.empty()) return 0;
  else return strid(tolower(a));
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void CurrentAction::doOpen() {
  const string& sarg = shortcut->arg();
  
  switch (strid(command->name)) {
    case strid("openhideapp"):
      openHideApp(ccuty::basename(sarg, false));  // no extension!
      break;
      
    case strid("openapp"):
      openApp(sarg);
      break;
      
    case strid("openurl"):
      Services::openUrl(sarg);
      break;
      
    case strid("openfile"):
      Services::openFile(sarg);
      break;
      
    case strid("opensysdir"): {
      //std::vector<string> v;
      Services::openApp("Finder");
      //auto n = ccuty::strsplit(v, sarg, "()");
      //if (n >= 2) Services::sendModChars(v[1]);
      Services::sendModChars(sarg);
      } break;
  }
}

void CurrentAction::openApp(const std::string& appname) {
  if (appname.empty()) return;
  string app, apparg;
  auto n = ccuty::strsplit(app, apparg, appname, ":");
  if (n == 1) Services::openApp(app);
  else if (n == 2) Services::openApp(app, getFile(apparg));
}

void CurrentAction::openHideApp(const std::string& appname) {
  if (appname.empty()) return;
  string app;
  if ((Services::getFrontAppPath(app) && app == appname)
      || (Services::getFrontAppName(app) && app == appname)
      ) {        // this is the frontmost app +> hide it
    Services::hideApp(appname);
    feedback = "Hide " + shortcut->name();
  }
  else {
    Services::openApp(appname);   // not the frontmost app => open it
    feedback = "Open " + shortcut->name();
  }
}

void CurrentAction::doWindow() {
  if (!command) return;
  switch (strid(command->name)) {
    case strid("fullscreen"):
      Services::sendChar('f', Modifiers::Command|Modifiers::Control);
      break;
      
    case strid("resize"):
      if (shortcut->arg() == "fullscreen")
        Services::sendChar('f', Modifiers::Command|Modifiers::Control);
      else if (shortcut->arg() == "zoom")
        Services::sendChar('+', Modifiers::Command);
      else if (shortcut->arg() == "unzoom")
        Services::sendChar('+', Modifiers::Command);
      else
        system("osascript "+Conf::scriptDir()+"resize_window.scpt "
               + tolower(shortcut->arg()));
      break;
      
    case strid("zoom"):
      if (shortcut->arg().empty()) Services::sendChar('+', Modifiers::Command);
      else Services::sendChar(shortcut->arg()[0], Modifiers::Command);
      break;
    
    case strid("unzoom"):  // compat
      Services::sendChar('-', Modifiers::Command);
      break;
    
    case strid("volume"):
      setVolume(shortcut->arg());
      break;
    
    case strid("appwins"):
      system("open -a \"mission control\" --args 2");
      break;
    
    case strid("nextwin"):
      Services::sendChar('`', Modifiers::Command);
      break;
    
    case strid("mcontrol"):
      Services::openApp("mission control");
      break;
    
    case strid("desktop"):
      system("open -a \"mission control\" --args 1");
      break;
    
    case strid("dock"):
      // "⌘⌥d";
      Services::sendChar('d', Modifiers::Command|Modifiers::Alt);
      break;
    
    case strid("grabscreen"):
      Services::sendChar('3', Modifiers::Command|Modifiers::Shift);
      break;
    
    case strid("grabselect"):
      //"⌘⇧4"
      Services::sendChar('4', Modifiers::Command|Modifiers::Shift);
      break;
    
    case strid("switchapp"):
      //Cmd-Tab: comportement incorrect!
      Services::sendKeycode(48, Modifiers::Command);
      break;
  }
}

void CurrentAction::doHotkey() {
  if (!command) return;
  if (command->name=="redo") {
    redoKey();
  }
  else if (!command->arg.empty()) {
    if (command->arg[0] == '?')    // "?" means take arg provided by user
      Services::sendChars(shortcut->arg(), shortcut->modifiers());
    else Services::sendModChars(command->arg);
  }
}

void CurrentAction::redoKey() {
  string app;
  if (!Services::getFrontAppName(app)) return;
  switch (strid(app)) {
    case strid("Microsoft Word"):
    case strid("Microsoft Excel"):
    case strid("Microsoft PowerPoint"):
      Services::sendChar('y', Modifiers::Command);
      break;
    default:
      Services::sendChar('z', Modifiers::Command|Modifiers::Shift);
      break;
  }
}

void CurrentAction::setVolume(const std::string& vol) {
  if (!command) return;
  if (vol.empty()) return;
  
  if (connected()) {
    if (vol[0] == '+') {feedback = "Media vol+"; macmote("vol+"); }
    else if (vol[0] == '-') {feedback = "Media vol-"; macmote("vol-");}
    else if (iequal(vol, "mute")) {feedback = "Media mute"; macmote("mute");}
  }
  else if (vol[0] == '+') {
    if (vol == "+") Services::changeVolume(+10);
    else Services::changeVolume(stoi(vol));
  }
  else if (vol[0] == '-') {
    if (vol == "-") Services::changeVolume(-10);
    else Services::changeVolume(stoi(vol));
  }
  else if (isdigit(vol[0])) Services::setVolume(stoi(vol));
  else if (iequal(vol, "mute")) Services::muteVolume();
}

void CurrentAction::doCommand() {
  if (!command || shortcut->arg().empty()) return;
  const string& sarg = shortcut->arg();
  
  switch (strid(command->name)) {
    case strid("appcmd"):
    case strid("command"): {
      string app, apparg;
      auto n = ccuty::strsplit(app, apparg, sarg, ":");
      if (n <= 0) break;
      if (app == "music") {
        if (connected()) macmote(apparg);
        else Services::tellApp("iTunes", apparg);
      }
      else if (app == "macmote") macmote(apparg);
      else Services::tellApp(app, apparg);
    } break;
      
    case strid("applecmd"):
      system("osascript -e '" + sarg + "' &");
      break;
      
    case strid("unixcmd"):
      system(sarg + "&");
      break;
      
    case strid("scriptfile"):
      openApp(sarg);
      break;
      
    case strid("quitapp"):
      Services::tellApp(getApp(sarg), "quit");
      break;
      
    case strid("previous"): {
      switch (getAppID(sarg)) {
        case strid("chrome"):
        case strid("google chrome"):
          Services::tellApp("Google Chrome","go back active tab of front window");
          break;
        case strid("safari"):
          Services::sendKeycode(123, Modifiers::Command); // command left arrow
          break;
        case strid("itunes"):
          Services::tellApp("iTunes","back track");
          break;
        case strid("macmote"):
          if (connected()) macmote("previous");
          break;
        case strid("music"):
          if (connected()) macmote("previous");
          else Services::tellApp("iTunes","back track");
          break;
        case strid("dvd player"):
          Services::tellApp("DVD Player","play previous chapter");
          break;
      }
    } break;
      
    case strid("next"): {
      switch (getAppID(sarg)) {
        case strid("chrome"):
        case strid("google chrome"):
          //Services::sendChar(']', Modifiers::Command);
          Services::tellApp("Google Chrome","go forward active tab of front window");
          break;
        case strid("safari"):
          Services::sendKeycode(124, Modifiers::Command); // command left arrow
          break;
        case strid("itunes"):
          Services::tellApp("iTunes","next track");
          break;
        case strid("macmote"):
          if (connected()) macmote("next");
          break;
        case strid("music"):
          if (connected()) macmote("next");
          else Services::tellApp("iTunes","next track");
          break;
        case strid("dvd player"):
          Services::tellApp("DVD Player","play next chapter");
          break;
      }
    } break;
      
    case strid("playpause"): {
      switch (getAppID(sarg)) {
        case strid("itunes"):
          Services::tellApp("iTunes","playpause");
          break;
        case strid("macmote"):
          if (connected()) macmote("playpause");
        case strid("music"):
          if (connected()) macmote("playpause");
          else Services::tellApp("iTunes","playpause");
          break;
      }
    } break;
      
    case strid("showhide"): {
      switch (getAppID(sarg)) {
        case strid("itunes"):
          openHideApp("iTunes");
          break;
        case strid("macmote"):
          if (connected()) macmote("info");
          break;
        case strid("music"):
          if (connected()) macmote("info");
          else openHideApp("iTunes");
          break;
        default:
          openHideApp(sarg);
          break;
      }
    } break;
  }
}

void CurrentAction::openEditor() {
  if (!command) return;
  status = NoFeedback;  // sinon le feedback va fermer l'overlay!
  switch (strid(command->name)) {
    case strid("edit"):
      MarkPad::instance.postEdit(nullptr);
      break;
    case strid("editmenu"):
      MarkPad::instance.postEdit(MarkPad::instance.findMenu(shortcut->arg()));
      break;
  }
}

void CurrentAction::doClipboard() {
  if (!command) return;
  switch (strid(command->name)) {
    case strid("write"):
      Services::pasteString(shortcut->arg());
      break;
    case strid("copyws"):
      Services::copyWithoutStyle();
      break;
    case strid("pastews"):
      Services::pasteWithoutStyle();
      break;
    case strid("store"):
      Services::copyToBuffer(shortcut->arg());
      break;
    case strid("retrieve"):
      Services::pasteFromBuffer(shortcut->arg());
      break;
    case strid("cut"):
      Services::sendChar('x', Modifiers::Command);
      break;
    case strid("copy"):
      Services::sendChar('c', Modifiers::Command);
      break;
    case strid("paste"):
      Services::sendChar('v', Modifiers::Command);
      break;
  }
}

/*
void CurrentAction::doClicks() {
  const std::string&  arg = shortcut->arg();
  if (arg.empty()) return;
  
  std::vector<std::string> v;
  ccuty::strsplit(v, arg, ";");
  
  for (auto& s : v) {
    std::string left, right;
    ccuty::strsplit(left, right, s, ",");
    //cerr << "s: "<<s<< " l: " <<left<< " r: " << right<< endl;
    try {
      float x = ::stof(left), y = ::stof(right);
      //cerr << "x: "<< x << " y: " << y << endl;
      
      MacDesktop::pressMouse(x, y, MacDesktop::LeftButton);
      MacDesktop::releaseMouse(x, y, MacDesktop::LeftButton);
    }
    catch (...) {
    }
  }
}
*/
/*
void CurrentAction::volumePerCent() {
  int vol = int(globalY()*100); // continous action (called by Shortcut::Motion)
  
  if (_cnx.connected()) {
    // no feedback for Shortcut::Motion
    if (touchState() != Shortcut::Up) dontNotify();  // do nothing, display nothing
    else if (vol >= 60) {macmote("vol+"); notify("Media Vol +");}
    else if (vol <= 40) {macmote("vol-"); notify("Media Vol -");}
  }
  else {
    static int lastvol = 0;
    // takes too much time if dif is very small
    int dif = abs(vol - lastvol);
    if (dif < 5) {
      dontNotify();  // do nothing, display nothing
    }
    else {
      _desk.setVolume(vol, false);
      lastvol = vol;
      notify("Volume " + to_string(vol));
    }
  }
}
 */
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

class Cnx {
public:
  Cnx(const string& hostname_and_port);
  void connect();
  void disconnect();
  bool connected() const;
  void send(const string& data);
  
  string hostname;
  int port{0};
  bool connecting{false};
  Socket* sock{nullptr};
  SocketBuffer* sockbuf{nullptr};
};

Cnx::Cnx(const string& hostname_and_port) {
  string portname;
  ccuty::strsplit(hostname, portname, hostname_and_port, ":");
  port = atoi(portname.c_str());
}

bool Cnx::connected() const {
  return sock != nullptr && sockbuf != nullptr;
}

void Cnx::connect() {
  connecting = true;
  disconnect();
  sock = new Socket();
  int status = sock->connect(hostname, port);
  if (status >= 0) {
    sockbuf = new SocketBuffer(sock);
    cout << "*** Connected to Macmote on: "<< hostname <<endl;
  }
  else {
    MarkPad::warning("Couldn't connect to MacMote on: "+hostname);
    delete sock;
    sock = nullptr;
  }
  connecting = false;
}

void Cnx::disconnect() {
  if (sock) sock->close();
  delete sock;
  delete sockbuf;
  sock = nullptr;
  sockbuf = nullptr;
}

void Cnx::send(const string& args) {
  if (connecting) {
    MarkPad::warning("Server "+hostname+" is busy");
    return;
  }
  if (!sock) {
    connect();
    if (!sock) return;
  }
  if (sockbuf->writeLine(">"+args) < 0) {  // > needed before command
    MarkPad::warning("Couldn't send data to server "+hostname);
    disconnect();
  }
}

// - - - - -

void CurrentAction::connect() {
  if (!Conf::k.mediaHost.empty()) macmote("getstatus");
}

void CurrentAction::disconnect() {
  if (cnx) cnx->disconnect();
  cout << "Disconnected from media server "<< Conf::k.mediaHost<<endl;
}

bool CurrentAction::connected() {
  return cnx ? cnx->connected() : false;
}

// must be static because called by std::thread()
static void sendData(Cnx* cnx, const string& args) {
  cnx->send(args);
}

void CurrentAction::macmote(const string& args) {
  if (args == "panel") {
    if (cnx) Services::openUrl("http://"+ cnx->hostname+"/macmote/");
  }
  else {
    if (!cnx) cnx = new Cnx(Conf::k.mediaHost);
    // thread to avoid blocking the program
    std::thread(sendData, cnx, args).detach();
  }
}
