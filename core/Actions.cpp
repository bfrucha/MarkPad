//
//  Actions.cpp
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
#include "Actions.h"
#include "CurrentAction.h"
#include "GUI.h"
#include "Conf.h"
#include "Services.h"
using namespace std;
using namespace ccuty;

const std::string
Actions::CurrentApp("*CurrentApp*"),
Actions::CurrentFile("*CurrentFile*");

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// open url if openURL is true, open File or App otherwise
void Actions::setCurrentFileOrURL(Shortcut* s, bool openURL) {
  string path, title;

  if (openURL) {
    if (Services::getBrowserUrl(path) && !path.empty()) {
      s->setArg(path);
      if (Services::getBrowserTitle(title) && !title.empty()) {
        if (title.size() > 30) {title = title.substr(0,30); title += "...";}
        s->setName(title, true, false);
      }
      else s->setName("URL", true, false);
    }
  }
  else {
    if (Services::getFinderFile(path) && !path.empty()) {
      s->setArg(path);
      s->setName(basename(path), true, false);
    }
  }
  if (path.empty()) {
    s->setArg("<No Selection>");
    // setText(del->actionCommand, "<No Selection>");
  }
  Conf::mustSave();
  GUI::instance.updateShortcut(s);
}

void Actions::quitBrowser() {
  GUI::showDesktop(false);
  //Shortcut* s = MarkPad::instance.currentShortcut();
  //if (s && s->arg_.empty()) setBrowserSelection(s, helperOpenURL);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// menus must be defined before Actions::instance

static Helper* fileMenu = new Helper {
  "Choose File",
  {
    {"Current File in Finder"},
    {"Search in Finder"},
    {"Show Desktop"},
    //{"Open Chosen File"}
  },
  [] (Helper& h, Shortcut* s, const Command* c, int index) {
    if (!s || !c || index < 0) return;
    switch (index) {
      case 0:
        Actions::setCurrentFileOrURL(s, false);
        break;
      case 1:
        GUI::openFinder();
        break;
      case 2:
        GUI::showDesktop(true);
        break;
      case 3:
        GUI::showDesktop(true);
        Services::openFile(s->arg());
        break;
    }
  }
};

static Helper* appMenu = new Helper {
  "Choose App",
  {
    {"Current App in Finder"},
    {"Search in Finder"},
    {"Show Desktop"},
    //{"Open Chosen App"}, dangereux sous Mojave!
    //"[Will Open Current File]",
    //"[Will Open Custom File]",
  },
  [] (Helper& h, Shortcut* s, const Command* c, int index) {
    if (!s || !c || index < 0) return;
    switch (index) {
      case 0:
        Actions::setCurrentFileOrURL(s, false);
        break;
      case 1:
        GUI::openFinder();  // ouvrir dans "/Applications"
        break;
      case 2:
        GUI::showDesktop(true);
        break;
      case 3:
        GUI::showDesktop(true);
        Services::openApp(s->arg());
        break;
        /*
      case 4:
        // current file
        if (s->arg().empty()) s->setArg("Choose Application First!");
        else s->setArg(s->arg() + " : " + Actions::CurrentFile);
        break;
      case 5:
        // custom file
        if (s->arg().empty()) s->setArg("Choose Application First!");
        else s->setArg(s->arg() + " : Replace by Desired File");
        break;
         */
    }
  }
};

static Helper* urlMenu = new Helper {
  "Choose URL",
  {
    {"Current URL in Web Browser"},
    {"Show Web Browser"},
    {"Show Desktop"},
    //{"Open Chosen URL"}
  },
  [] (Helper&, Shortcut* s, const Command* c, int index) {
    if (!s || !c || index < 0) return;
    switch (index) {
      case 0:
        Actions::setCurrentFileOrURL(s, true);
        break;
      case 1:
        GUI::openWebBrowser();
        break;
      case 2:
        GUI::showDesktop(true);
        break;
      case 3:
        GUI::showDesktop(true);
        Services::openUrl(s->arg());
        break;
    }
  }
};

static Helper* dirMenu = new Helper {
  "Choose Directory",
  {
    //"Parent Directory (⌘  )",
    {"Recent Files (⇧⌘F)","⇧⌘F"},
    {"Documents (⇧⌘O)","⇧⌘O"},
    {"Desktop (⇧⌘D)","⇧⌘D"},
    {"Downloads (⌥⌘L)" "⌥⌘L"},
    {"Home (⇧⌘H)", "⇧⌘H"},
    {"Library (⇧⌘L)","⇧⌘L"},
    {"Computer (⇧⌘C)","⇧⌘C"},
    {"AirDrop (⇧⌘R)","⇧⌘R"},
    {"Network (⇧⌘K)","⇧⌘K"},
    {"iCloud Drive (⇧⌘I)","⇧⌘I"},
    {"Applications (⇧⌘A)","⇧⌘A"},
    {"Utilities (⇧⌘Z)","⇧⌘Z"},
    {"Go To Directory (⇧⌘G)","⇧⌘G"},
    {"Connect To Server (⌘K)","⌘K"}
  },
  [] (Helper& h, Shortcut* s, const Command* c, int index) {
    if (!s || !c || index < 0) return;
    s->setArg(h.menu[index].command);
  }
};

static Helper* volMenu = new Helper {
  "Choose Volume",
  {
    {"Volume +20","+20"},
    {"Volume +15","+15"},
    {"Volume +10","+10"},
    {"Volume  +5","+5"},
    {"Mute","mute"},
    {"Volume  -5","-5"},
    {"Volume -10","-10"},
    {"Volume -15","-15"},
    {"Volume -20","-20"},
  },
  [] (Helper& h, Shortcut* s, const Command* c, int index) {
    if (!s || !c || index < 0) return;
    s->setArg(h.menu[index].command);
  }
};

static Helper* moveMenu = new Helper {
  "Choose Area",
  {
    {"Left","left"},
    {"Top","top"},
    {"Right","right"},
    {"Bottom","bottom"},
    {"Top Left","top-left"},
    {"top Right","top-right"},
    {"Bottom Right","bottom-right"},
    {"Bottom Left","bottom-left"},
  },
  [] (Helper& h, Shortcut* s, const Command* c, int index) {
    if (!s || !c || index < 0) return;
    s->setArg(h.menu[index].command);
  }
};

static Helper* commandMenu = new Helper {
  "Choose App",
  {
    {"iTunes","iTunes"},
    {"Chrome","Chrome"},
    {"Safari","Safari"},
    {"DVD Player","DVD Player"},
    {"MacMote","MacMote"},
    {"Current App", "*CurrentApp*"}
  },
  [] (Helper& h, Shortcut* s, const Command* c, int index) {
    if (!s || !c || index < 0) return;
    string cmd = h.menu[index].command;
    if (c->name == "command") s->setArg(cmd + " : ");
    else s->setArg(cmd);
  }
};

static Helper* configMenu = new Helper {
  "Choose Menu",
  {  // automatically created, will contain all menu names
  },
  [] (Helper& h, Shortcut* s, const Command* c, int index) {
    if (!s || !c || index < 0 || index >= h.menu.size()) return;
    s->setArg(h.menu[index].item);
  }
};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

Actions Actions::instance;  // Actions singleton

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

Actions::Actions() :
current(*new CurrentAction()),
actions
{    // note that command names must be lowercased!
  
  {  // must be first!
    "Menu", "menu.png", "menu-selected.png", Action::SubMenu,
  },
  
  {
    "Open URL, File or Application", "app.png", "app-selected.png", Action::Standard,
    [this](){current.doOpen();},
    {
      {"openurl",    "Open URL", "", "?", urlMenu},
      {"openfile",   "Open File or Directory", "", "?", fileMenu},
      {"openhideapp","Open/Hide Application", "", "?", appMenu},
      //{"openapp",    "Open Application", "", "?", appMenu},
      {"opensysdir", "Open Predefined Directory", "", "?", dirMenu},
    }
  },

  {
    "Zoom and Windows", "resize.png", "resize-selected.png", Action::Standard,
    [this](){current.doWindow();},
    {
      {"nextwin", "Show Next App Window"},
      {"appwins", "Show All App Windows"},
      {"resize", "Move Window at Location", "", "?", moveMenu},
      {"fullscreen", "App in Full Screen"},
      {"zoom", "Zoom Current App"},
      {"unzoom", "Unzoom Current App"},
    }
  },

  {
    "Commands for iTunes, Chrome, Safari, etc.", "media.png", "media-selected.png",
    Action::Standard, [this]{current.doCommand();},
    {
      {"next",    "Next Track or Page", "", "?", commandMenu},
      {"previous", "Previous Track or Page", "", "?", commandMenu},
      {"showhide", "Show/Hide", "", "?", commandMenu},
      {"playpause", "Play/Pause", "", "?", commandMenu},
      {"quitapp", "Quit", "", "?", commandMenu},
      {"command",  "Custom Command", "(ex: iTunes : PlayPause)", "?", commandMenu},
    }
  },

  {
    "Extended Copy & Paste", "clipboard.png", "clipboard-selected.png",
    Action::Standard, [this](){current.doClipboard();},
    {
      //{"copyws", "Copy Without Style"},
      {"pastews", "Paste Without Style"},
      {"write", "Paste this String", "(type string to be pasted)", "?"},
      {"store", "Copy to Alt. Clipboard", "(number or any name)", "?"},  // textfield shown if arg is "?"
      {"retrieve","Paste from Alt. Clipboard","(number or any name)", "?"},
      {"cut",  "Cut"},
      {"copy", "Copy"},
      {"paste","Paste"},
    }
  },

  {
    "Application Hotkeys", "keyboard.png", "keyboard-selected.png", Action::Hotkey,
    [this](){current.doHotkey();},
    {
      {"keystroke", "Custom Hotkey", "(select modifiers and enter key)", "?"},  // textfield shown if arg is "?"
      {"redo", "Redo (⌘⇧Z/⌘Y)","(hotkey adapts to app)"},
      {"undo", "Undo (⌘Z)", "", "⌘z"},
      {"colors", "Colors (⌘⇧C)", "", "⌘⇧c"},
      {"fonts", "Fonts (⌘T)", "", "⌘t"},
      {"find", "Find (⌘F)", "", "⌘f"},
      {"hide", "Hide (⌘H)", "", "⌘h"},
      {"highlight", "Highlight (⌘⇧H)", "", "⌘⇧h"},
      {"new", "New (⌘N)", "", "⌘n"},
      {"open", "Open ⌘O)", "", "⌘o"},
      {"quit", "Quit (⌘Q)", "", "⌘q"},
      {"run", "Run (⌘R)",  "", "⌘r"},
      {"save", "Save (⌘S)", "", "⌘s"},
    },
  },

  {
    "Desktop Commands", "view.png", "view-selected.png", Action::Standard,
    [this](){current.doWindow();},
    {
      {"volume", "Volume", "", "?", volMenu},
      {"grabselect", "Grab Screen Area"},
      {"grabscreen", "Grab Entire Screen"},
      {"mcontrol", "Mission Control"},
      {"desktop", "Show Desktop"},
      {"dock", "Show/Hide Dock"},
    }
  },

  {
    "AppleScript and Unix Commands", "script.png", "script-selected.png",
    Action::Standard, [this](){current.doCommand();},
    {
      {"appcmd", "Application Command", "(ex: iTunes : PlayPause)", "?"},
      {"applecmd", "AppleScript", "(ex: tell application \"iTunes\" to playPause)", "?"},
      {"unixcmd", "Unix", "(ex: ls /Applications | open -f)", "?"},
      {"scriptfile", "Script File", "(Unix or Apple script)", "?", fileMenu}
    }
  },
  /*
  {
    "Open Menu in Editing Mode", "edit.png", "edit-selected.png",
    Action::Config, [this]{current.openEditor();},
    {
      {"edit", "Open Main Menu in Editing Mode", ""},
      {"editmenu", "Open Submenu in Editing Mode", "", "?", configMenu},
    }
   }
   */

  /*
   {
   "Click Mouse", "click.png", "click-selected.png",
   Action::Standard,
   [this]{current.doClicks();},
   {}
   },
   */
}
{
  int index = 0;
  
  for (auto& a : actions) {
    a.actionID = index++;
  
    for (auto& c : a.commands) {
      c.action = &a;
      auto it = commandmap.find(c.name);
      if (it == commandmap.end()) commandmap[c.name] = &c;
      else MarkPad::warning("Duplicate command name '"+c.name+"'");
    }
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

std::vector<Action> & Actions::getActions()  {return actions;}

Action* Actions::getActionFromIndex(int index) {
  if (index < 0 || index >= int(actions.size())) return nullptr;
  else return &actions[index];
}

Action* Actions::getActionFromCommand(const string& cmd) {
  auto it = commandmap.find(cmd);
  if (it == commandmap.end()) return nullptr;
  else return it->second->action;
}

const Command* Action::command(const Shortcut& s) const {
  if (s.comindex_ < 0 || s.comindex_ >= int(commands.size())) return nullptr;
  return &commands[s.comindex_];
}

const Command* Action::command(int index) const {
  if (index < 0 || index >= int(commands.size())) return nullptr;
  return &commands[index];
}

const string& Action::commandName(int index) const {
  static const string empty, menu("menu");
  if (type & Action::SubMenu) return menu;
  if (index < 0 || index >= int(commands.size())) return empty;
  else return commands[index].name;
}

int Action::findCommandFromName(const string& confname) const {
  if (confname.empty()) return -1;
  for (int k = 0; k < int(commands.size()); ++k) {
    if (commands[k].name == confname) return k;
  }
  return -1;
}

int Action::findCommandFromTitle(const string& atitle) const {
  if (title.empty()) return -1;
  for (int k = 0; k < (int)commands.size(); ++k) {
    if (commands[k].title == atitle) return k;
  }
  return -1;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void Actions::run(bool state) {
  if (state) current.connect(); else current.disconnect();
}

void Actions::exec(Shortcut& s, const MTTouch& touch, Shortcut::State touchState) {
  current.exec(s, touch, touchState);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// convert format with ⇧⌃⌘⌥ OR format with ctrl+alt+shift
// to generic modifier mask + following chars
static void convertHotkey(const string& from, uint8_t& modifiers, string& to) {
  modifiers = 0;

  if (from.size() >= 3) {
    string s = from.substr(0,3);
    if (s=="⇧" || s=="⌃" || s=="⌘" || s=="⌥") {
      Services::modStringToModifiers(from, modifiers, to);
      //cerr << " => converted to " << args<<" with mods " << int(modifiers) << endl;
      return;
    }
  }
  
  // no else!
  Services::stringToModifiers(from, modifiers, to);
  //cerr << " => converted_bis to " << args << endl;
}
  
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

bool Actions::setShortcutAction(Shortcut&s,
                                std::string const& command,
                                std::string const& arg) {
  Action* a = getActionFromCommand(command);
  if (!a) return false;
  s.action_ = a;
  s.arg_ = arg;
  s.comindex_ = int16_t(a->findCommandFromName(tolower(command)));

  uint8_t modifiers = 0;
  if (command == "keystroke") convertHotkey(arg, modifiers, s.arg_ );
  s.modifiers_ = modifiers;
  return true;
}


bool Actions::setShortcutActionFromConfFile(Shortcut& s,
                                            const string& keyword,
                                            const string& confarg) {
  string cmd, args;
  //uint8_t modifiers = 0;

  if (keyword == "!") {
    ccuty::strsplit(cmd, args, confarg, "");
    //if (cmd=="keystroke") convertHotkey(args, modifiers, args);
  }
  else {
    // compatibility
    Conf::mustSave();

    switch (strid(keyword)) {
      case strid("hotkey"):
        ccuty::strsplit(cmd, args, confarg, "");
        //if (cmd=="keystroke") convertHotkey(args, modifiers, args);
        break;
      case strid("resize"):
        cmd = "resize";
        args = confarg;
        break;
      case strid("volume"):
        cmd = "volume";
        args = confarg;
        break;
      case strid("openurl"):
        cmd = "openurl";
        args = confarg;
        break;
      case strid("open"):
        cmd = "openfile";
        args = confarg;
        break;
      case strid("openapp"):
        cmd = "openhideapp";
        args = confarg;
        break;
      case strid("quitapp"):
        cmd = "quitapp";
        args = confarg;
        break;
      case strid("app"):
        ccuty::strsplit(cmd, args, confarg, "");
        if (cmd=="openhide") cmd = "openhideapp";
        else if (cmd=="open") cmd = "openapp";
        else if (cmd=="quit") cmd = "quitapp";
        break;
      case strid("keystroke"): {
        uint8_t modifiers = 0;
        cmd = "keystroke";
        Services::appleModStringToModifiers("keystroke "+confarg, modifiers, args);
        Conf::mustSave();
      } break;
      case strid("keycode"): {
        uint8_t modifiers = 0;
        cmd = "keystroke";
        Services::appleModStringToModifiers("keycode "+confarg, modifiers, args);
        Conf::mustSave();
      } break;
      case strid("script"):
        ccuty::strsplit(cmd, args, confarg, "");
        if (cmd == "simple") cmd = "appcmd";
        else if (cmd == "regular") cmd = "applecmd";
        else if (cmd == "unix") cmd = "unixcmd";
        else if (cmd == "file") cmd = "scriptfile";
        args = confarg;
        break;

      case strid("tellapp"):
        cmd = "appcmd";
        args = confarg;
        Conf::mustSave();
        break;
      case strid("osascript"):
        cmd = "applecmd";
        args = confarg;
        Conf::mustSave();
        break;
      case strid("system"):
        cmd = "unixcmd";
        args = confarg;
        Conf::mustSave();
        break;
      case strid("applescript"):
        cmd = "scriptfile";
        args = confarg;
        Conf::mustSave();
        break;
        
      default:
        ccuty::strsplit(cmd, args, confarg, "");        
        break;
    }
  }
  
  if (setShortcutAction(s, cmd, args)) return true;
  else {
    MarkPad::warning("Shortcut '"+s.name_+"': Invalid action: "+confarg);
    return false;
  }
}

