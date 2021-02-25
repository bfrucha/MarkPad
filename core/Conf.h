//
//  Conf.h
//  MarkPad Project
//
//  (c) Eric Lecolinet - http://www.telecom-paris.fr/~elc
//  (c) Bruno Fruchard - http://brunofruchard.com/
//  Copyright (c) 2017/2020. All rights reserved.
//

#ifndef MarkPad_Conf
#define MarkPad_Conf
 
#include <string>
#include <vector>
#include "MTouch.h"

/// MarkPad configuration.
class Conf {
public:
  
  /// Conf singleton (CConf used for compatibility issues).
  static class CConf instance;
  
  /// read-only access to the Conf singleton.
  static const Conf& k;
  
  static std::string version() {return "5.6";}  ///< MarkPad Version.
  static const std::string& confDir();      ///< configuration directory.
  static const std::string& resourceDir();  ///< bundle resources.
  static std::string imageDir();            ///< image directory.
  static std::string scriptDir();           ///< script directory.
  static std::string bufferDir();           ///< where copy buffers are stored.
  static std::string shortcutFile();        ///< file containing configuration and shortcuts.
  static std::string guideFile();           ///< file containing shortcut guides.
  static const std::string& execPath();     ///< path of the MarkPad executable.

  /// requests the configuration to be saved (when saveIfNeeded() will be called).
  static void mustSave();
  
  /// saves the configuration (only if mustSave() was previously called).
  static void saveIfNeeded();

  /// changes the value of a variable, calls mustSave() if its value was changed.
  template <typename T>
  static void change(T Conf::*variable, const T& value);
  
  /// Feedback after activating a command.
  struct Feedback {
    /// how long the feedback appears on the screen, 0 means no feedback.
    float delay = 0.6;
    std::string font = "Helvetica-Bold, 48";
    std::string color = "0.,0.59,1.,0.56";
  };

  struct ActiveBorder {
    float left = 0.04f, right = 0.04f, top = 0.04f, bottom = 0.04f;
  };
  
  template <typename T>
  static void changeFeedback(T Feedback::*variable, const T& value);
  
  template <typename T>
  static void changeBorder(T ActiveBorder::*variable, const T& value);
  
  // - - - Variables saved in the config file  - - - - - - - - - - - - - - - - -
  
  /// Version of the configuration file.
  std::string fileVersion;

  /// Hotkey that shows the menus (requires Acessibility).
  /// Value = key1[+key2] with key = fn, ctrl, shift, alt, cmd.
  std::string showHotkey = "fn+ctrl";
  std::string editHotkey = "shift";

  /// Theme of the application (dark, light or transparent).
  std::string theme;

  /// Hostname of the multimedia server (if any), format is hostname:port.
  std::string mediaHost;
  
  /// Gestures must start from this area (except for Non-Border Shortcuts).
  ActiveBorder activeBorders;
  
  // Delay for opening menus (except for Non-Border menus).
  float menuDelay = 0.6f;       ///< how long the touchpad must be touched to open a menu.
  
  /// Gestures must be larger than this value, menus no opened otherwise.
  float minMovement = 0.f;
  
  // Finger size:
  float minTouchSize = 4.f;        ///< Min default radius of a finger touch.
  float maxTouchSize = 12.f;       ///< Max default radius of a finger touch.

  // Modes:
  bool allowAccessibility = true; ///< accessibility is needed for using hotkeys.
  bool developerMode = true;      ///< more options allowed when true.
  bool logData = false;           ///< log touch data (uses much disk space!).
  
  /// Feedback after activating a command.
  bool showFeedback = true;       ///< show feedback after activating a command.
  Feedback feedback;

  /// Background image.
  bool showBgImage = true;
  std::string bgImage = "bgimage.png";

  // What is shown on the screen:
  bool showFingers = false;       ///< show finger positions in editor.
  bool showGrid = false;          ///< show grid in editor.
  
  class ShortcutMenu *mainMenu_{nullptr};

  // - - - Not saved in config file  - - - - - - - - - - - - - - - - - - - - - -
  
  MTSize  minShortcutSize{0.01f, 0.01f};    ///< min size of a shortcut.
  MTSize  newShortcutSize{0.15f, 0.20f};    ///< size of a new shortcut.
  MTSize  pasteOffset{0.03f, 0.03f};        ///< pasted shortcut shifted this amount.
  MTFloat pickTolerance{0.01f};             ///< tolerance when picking borders.
  MTFloat nameSpacing{0.004f};              ///< min spacing around the shortcut name.
  float   stepperIncrement{0.001f};         ///< increment of NSSteppers
  double  doubleClickDelay{0.35};  ///< delay between 2 mouse clicks for a doubleclick.
  
protected:
  friend class MarkPad;
  friend class ConfImpl;
  friend class GUI;
  friend int main(int, char**);
  
  Conf() = default;  // not public!

  /// must be called from main().
  void init(const std::string& execpath);

  /// writes the configuration file.
  void write();

  bool changed_{false}, previousConfSaved_{false};
  std::string confdir, resdir, execpath;    ///< see corresponding methods.
};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

/// MarkPad configuration: for compability with previous versions.
class CConf : public Conf {
private:
  friend class MarkPad;
  friend class ConfImpl;
  friend class Conf;
  
  /// reads the configuration file (allows obsolete settings).
  void read();
  
  /// for compatibility with older versions.
  std::vector<class ShortcutMenu*> menus_;
};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename T>
void Conf::change(T Conf::*variable, const T& value) {
  if (instance.*variable != value) {
    instance.*variable = value;
    instance.changed_ = true;
  }
}

template <typename T>
void Conf::changeFeedback(T Feedback::*variable, const T& value) {
  if (instance.feedback.*variable != value) {
    instance.feedback.*variable = value;
    instance.changed_ = true;
  }
}

template <typename T>
void Conf::changeBorder(T ActiveBorder::*variable, const T& value) {
  if (instance.activeBorders.*variable != value) {
    instance.activeBorders.*variable = value;
    instance.changed_ = true;
  }
}

#endif
