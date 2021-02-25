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
#include "Actions.h"
#include "CurrentAction.h"
#include "GUI.h"
#include "Conf.h"
#include "Services.h"
using namespace std;
using namespace ccuty;

Actions Actions::instance;  // Actions singleton

static const char* appname_or_empty =
"(enter app name, applies on current app otherwise)";

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

Actions::Actions() :
current(*new CurrentAction()),
actions {    // note that command names must be lowercased!

  {  // must be first!
    "menu",                              // name
    "Open SubMenu",                      // title
    "menu.png", "menu-selected.png",     // icons
    Action::MarkpadMenu,                 // type
    nullptr,                             // fun
    {}                                   // commands
  },
  
  {
    "openurl",
    "Open Web Page",
    "web.png", "web-selected.png",
    Action::HelperMenu | Action::TextField, // type
    [this]{current.openUrl();},             // fun
    {
      {"", "Get Current URL"},
      {"", "Open Browser"},
      {"", "Show Desktop"},
      //{"", "Clear"},
    },
    [this](Shortcut* s, int index){current.urlHelper(s, index);}  // interactive helper
  },
  
  {
    "open",
    "Open File, Directory or Script",
    "file.png", "file-selected.png",
    Action::HelperMenu|Action::TextField,
    [this](){current.openFile();},
    {
      {"", "Get Current File"},
      {"", "Open Finder"},
      {"", "Show Desktop"},
      //{"", "Clear"},
    },
    [this](Shortcut* s, int index){current.fileHelper(s,index);}  // interactive helper
  },
  
  {
    "app",
    "Open, Hide, Quit Application",
    "app.png", "app-selected.png",
    Action::CommandMenu|Action::TextField,
    [this](){current.doApp();},
    {
      {"openhide", "Open/Hide App", "", "@finder"},  // @finder: shows openFinderBtn
      {"open", "Open App", "", "@finder"},
      {"quit", "Quit App", "", "@finder"},
    }
  },

  {
    "media",
    "Commands for iTunes, Chrome, Safari, etc.",
    "media.png", "media-selected.png",
    Action::CommandMenu|Action::TextFieldOnRequest,
    [this]{current.playMedia();},
    {
      {"next", "Next", appname_or_empty, "?"},
      {"previous", "Previous", appname_or_empty, "?"},
      {"playpause", "Play/Pause", appname_or_empty, "?"},
      {"showhide", "Show/Hide", appname_or_empty, "?"},
      {"command", "Custom Command", "(Exemple: iTunes : playPause)", "?"},
    },
    /*
     "The Application can be iTunes, DVD Player, Google Chrome, "
     "Safari, MediaServer or MusicMedia: \n\n"
     " - 'MediaServer' sends the command to a remote media server\n"
     " - 'MusicMedia' uses the media server if connected otherwise iTunes\n"
     */
  },
  
  {
    "hotkey",
    "Application Hotkeys",
    "keyboard.png", "keyboard-selected.png",
    Action::CommandMenu|Action::TextFieldOnRequest,
    [this](){current.hotkey();},
    {
      {"keystroke", "Custom Hotkey", "(enter key)", "?@hotkey"},  // textfield shown if arg is "?"
      {"redo", "Generic Redo","(⌘⇧z/⌘y)"},
      {"undo", "Undo","(⌘z)", "⌘z"},
      {"highlight", "Highlight","(⌘⇧h)", "⌘⇧h"},
      {"colors", "Colors","(⌘⇧c)", "⌘⇧c"},
      {"fonts", "Fonts","(⌘t)", "⌘t"},
      {"find", "Find","(⌘f)", "⌘f"},
      {"hide", "Hide","(⌘h)", "⌘h"},
      {"new", "New","(⌘n)",  "⌘n"},
      {"open", "Open","(⌘o)", "⌘o"},
      {"quit", "Quit","(⌘q)", "⌘q"},
      {"run", "Run","(⌘r)",  "⌘r"},
      {"save", "Save","(⌘s)", "⌘s"},
    },
  },
  
  {
    "clipboard",
    "Copy and Paste With(out) Style",
    "clipboard.png", "clipboard-selected.png",
    Action::CommandMenu|Action::TextFieldOnRequest, // textfield shown if arg is "?"
    [this](){current.clipboard();},
    {
      {"cut",  "Cut"},
      {"copy", "Copy with Style"},
      {"paste","Paste with Style"},
      {"copyws", "Copy without Style"},
      {"pastews", "Paste without Style"},
      {"store",   "Copy to Buffer", "(enter buffer name)", "?"},  // textfield shown if arg is "?"
      {"retrieve","Paste from Buffer","(enter buffer name)", "?"},
      {"write", "Paste String", "(enter string)", "?"},
    }
  },
  
  {
    "view",
    "Screen and Windows",
    "view.png", "view-selected.png",
    Action::CommandMenu,
    [this](){current.manageView();},
    {
      {"zoom", "Zoom +"},
      {"unzoom", "Zoom -"},
      {"appwins", "Show App Windows"},
      {"nextwin", "Next App Window"},
      {"mcontrol", "Mission Control"},
      {"desktop", "Show Desktop"},
      {"dock", "Show/Hide Dock"},
      {"grabselect", "Grab Screen Part"},
      {"grabscreen", "Grab All Screen"},
    }
  },
    
  {
    "resize",
    "Move and Resize Windows", //"(may not work with some apps)",
    "resize.png", "resize-selected.png",
    Action::CommandMenu,
    [this](){current.resizeWin();},
    {
      {"fullscreen", "Full Screen"},
      {"left", "Move Left"},
      {"top", "Move Top"},
      {"right", "Move Right"},
      {"bottom", "Move Bottom"},
      {"top-left", "Move Top-Left"},
      {"top-right", "Move Top-Right"},
      {"bottom-right", "Move Bottom-Right"},
      {"bottom-left", "Move Bottom-Left"},
    }
  },

  {
    "volume",
    "Volume",
    "volume.png", "volume-selected.png",
    Action::CommandMenu,
    [this](){current.setVolume();},
    {
      {"+20", "Volume +20"},
      {"+15", "Volume +15"},
      {"+10", "Volume +10"},
      {"+5", "Volume +5"},
      {"mute", "Mute"},
      {"-5", "Volume -5"},
      {"-10", "Volume -10"},
      {"-15", "Volume -15"},
      {"-20", "Volume -20"}
    }
  },

  {
    "script",
    "AppleScript and Unix Commands",
    "script.png", "script-selected.png",
    Action::CommandMenu|Action::TextField,
    [this](){current.doScript();},
    {
      {"simple", "Application Command", "(exemple: iTunes : playPause)"},
      {"regular", "AppleScript Command", "(ex: tell application \"iTunes\" to playPause)"},
      {"unix", "Unix Command", "(exemple: ls /Applications | open -f)"},
      {"file", "Script File", "(select Unix or Apple Script)", "@finder"}
    }
  },
  
  /*
  {
    "click",
    "Click Mouse",
   "click.png", "click-selected.png",
    Action::TextField,
    [this]{current.doClicks();},
    {}
  },
   */
  
  {
    "editmenu",
    "Open Menu in Editing Mode",
    "edit.png", "edit-selected.png",
    Action::CommandMenu|Action::TextFieldOnRequest,
    [this]{current.openEditor();},
    {
      {"edit", "Open Main Menu", "(main menu will be in editing mode)"},
      {"editmenu", "Open Other Menu", "(this menu will be in editing mode)", "?"},
    }
  }
}
{
  int index = 0;
  for (auto& it : actions) it.actionID = index++;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

std::vector<Action> & Actions::getActions()  {return actions;}

Action* Actions::getAction(int index) {
  if (index < 0 || index >= int(actions.size())) return nullptr;
  else return &actions[index];
}

Action* Actions::getAction(const string& confname) {
  for (auto& it : actions) {
    if (it.name == confname) return &it;
  }
  return nullptr;
}

const Command* Action::getCommand(const Shortcut& s) const {
  if (s.command() < 0 || s.command() >= int(commands.size())) return nullptr;
  return &commands[s.command()];
}

const Command* Action::getCommand(int index) const {
  if (index < 0 || index >= int(commands.size())) return nullptr;
  return &commands[index];
}

const string& Action::commandName(int index) const {
  static const string empty;
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
static void convertHotkey(const string& confargs, uint8_t& modifiers, string& args) {
  modifiers = 0;
  string keyword, value;
  strsplit(keyword, value, confargs, "");
  
  if (keyword.empty() || keyword != "keystroke") {
    args = confargs;  // unchanged
    return;
  }
  
  string chars;

  if (value.size() >= 3) {
    string s = value.substr(0,3);
    if (s=="⇧" || s=="⌃" || s=="⌘" || s=="⌥") {
      Services::modStringToModifiers(value, modifiers, chars);
      args = "keystroke " + chars;
      //cerr << " => converted to " << args<<" with mods " << int(modifiers) << endl;
      return;
    }
  }
  
  // no else!
  Services::stringToModifiers(value, modifiers, chars);
  args = "keystroke " + chars;
  //cerr << " => converted_bis to " << args << endl;
}
  
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

bool Actions::setShortcutAction(Shortcut& s, const string& confname,
                                const string& confargs) {
  string name, args;
  uint8_t modifiers = 0;
  
  // compatibility and hotkeys:
  switch (strid(confname)) {
      
    case strid("hotkey"):
      name = "hotkey";
      convertHotkey(confargs, modifiers, args);
      break;
      
    case strid("openapp"):
      name = "app";
      args = "openhide "+confargs;
      Conf::mustSave();
      break;
    case strid("quitapp"):
      name = "app";
      args = "quit "+confargs;
      Conf::mustSave();
      break;
      
    case strid("keystroke"):
      name = "hotkey";
      Services::appleModStringToModifiers("keystroke "+confargs, modifiers, args);
      Conf::mustSave();
      break;
      
    case strid("keycode"): {
      name = "hotkey";
      Services::appleModStringToModifiers("keycode "+confargs, modifiers, args);
      Conf::mustSave();
    } break;
      
    case strid("applescript"):
      name = "script";
      args = confargs;
      Conf::mustSave();
      break;
    case strid("system"):
      name = "script";
      args = "unix "+confargs;
      Conf::mustSave();
      break;
    case strid("tellapp"):
      name = "script";
      args = "simple "+confargs;
      Conf::mustSave();
      break;
    case strid("osascript"):
      name = "script";
      args = "regular "+confargs;
      Conf::mustSave();
      break;
    case strid("mediaserver"):
      name = "media";
      args = "command:media "+confargs;
      Conf::mustSave();
      break;
      
    case strid("desktop"):
    case strid("zoom"):
      name = "view";
      args = confargs;
      Conf::mustSave();
      break;
    case strid("edit"):
      name = "editmenu";
      args = confargs;
      Conf::mustSave();
      break;
    case strid("action"):
      name = "clipboard";
      args = confargs;
      Conf::mustSave();
      break;
      
    default:
      name = confname;
      args = confargs;
      break;
  }
  
  Action* a = getAction(name);
  if (!a) return false;
  s.action_ = a;
  
  if (a->type & Action::CommandMenu) {
    string cmd;
    strsplit(cmd, s.arg_, args, "");
    s.command_ = int16_t(a->findCommandFromName(tolower(cmd)));
    s.modifiers_ = modifiers;
    /*
    cerr << "CommandMenu: " << s.name()
    << " / act: " << name
    << " / com: " << s.command_
    << " / sharg: " << s.arg_ << " / line: " << args << endl;
     */
  }
  else if (a->type & Action::HelperMenu) {
    s.command_ = 0;
    s.arg_ = args;
    s.modifiers_ = modifiers;
  }
  else {
    s.command_ = -1;
    s.arg_ = args;
    s.modifiers_ = modifiers;
  }
  return true;
}
