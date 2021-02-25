//
//  Services.h
//  Created by elc on 10/12/2017.
//  Copyright © 2017/2020 elc. All rights reserved.
//

#ifndef Markpad_Services_h
#define Markpad_Services_h

#include <string>
#include <functional>
#include "MTouch.h"

struct Modifiers {
  enum {Shift=1<<0, Control=1<<1, Alt=1<<2, Command=1<<3, Function=1<<4};
};

class Services {  
public:
  
  // init services (start callback notifiers & accesibility).
  static void init();

  static void log(const std::string& msg);

  /// returns the resource directory of the application.
  /// On MacOSX this dir is located in the app bundle.
  static std::string getResourceDir();
  static std::string getUserConfDir();

  /// call _fun_ later in the proper thread.
  static void postpone(std::function<void()> fun);
  
  /// This callback function will be called when the computer is woken up.
  static void setWakeCallback(std::function<void()> fun);
  
  /// This callback function will be called when the computer is going to sleep.
  static void setSleepCallback(std::function<void()> fun);
  
  /// This callback function will be called when the hotkey modifiers are pressed/released.
  /// Note: will work only if accessibility is enabled.
  static void setHotkeyCallback(std::function<void(uint32_t hotkey)> fun);
  
  /// Request accessibility (needed on MacOSX to detect modifier changes)
  static bool askAccessibility();
  
  /// Is accessibility enabled?.
  static bool isAccessibilityEnabled();
  
  // Auto-hide the OS X menu bar and the dock when the app is frontmost.
  static void autoHideMenubarAndDock();
  
  /// disable enable cursor (e.g., during a slide)
  static void disableDeviceForCursor(MTDevice *dev);
  static void enableDeviceForCursor(MTDevice *dev);

  /// Makes the app frontmost (= activate app on MacOSX).
  static void makeMarkPadFront();
  
  // Actions
  static bool getFrontAppPath(std::string& str);
  static bool getFrontAppName(std::string& str);
  
  static const std::string& getDefaultWebBrowser();
  static void openDefaultWebBrowser();
  
  /// use getDefaultWebBrowser if second argument is empty.
  static bool setBrowserUrl(std::string const& url, const std::string& browser = "");
  static bool getBrowserUrl(std::string& url, const std::string& browser = "");
  static bool getBrowserTitle(std::string& url, const std::string& browser ="");
  static bool getFinderFile(std::string& path);
  
  static void openFile(const std::string& path);
  static void openUrl(const std::string& url);
  static void openApp(const std::string& path);
  static void openApp(const std::string& path, const std::string& arg);
  static void hideApp(const std::string& path);
  static void tellApp(const std::string& path, const std::string& command);
  static void changeVolume(int value);
  static void setVolume(unsigned int value);
  static void muteVolume();
  
  // Advanced cut and paste
  static void copyWithoutStyle();
  static void pasteWithoutStyle();
  static void copyToBuffer(const std::string& bufferName);
  static void pasteFromBuffer(const std::string& bufferName);
  static void pasteString(const std::string& str);
  
  /// Converts string with fn+ctrl+alt+shift to generic modifier mask.
  /// hotkey : the hotkey as a string
  /// Returns:
  /// - _modifiers_ : the generic modifier mask (see Modifiers)
  /// - _chars_ : the remaining characters
  /// - returned value : the native *PLATFORM* modifier mask.
  /// Caution: the platform modifiers may not be the GUI tookit modifiers (e.g. for Qt on MacOSX)
  static uint32_t stringToModifiers(const std::string& hotkey,
                                    uint8_t& modifiers, std::string& chars);
  
  /// Converts generic modifier mask to string with fn+ctrl+alt+shift.
  /// _modifiers_ : the generic modifier mask (see Modifiers)
  /// Returns:
  /// - the _hotkey_ as a string
  /// - returned value: the native *PLATFORM* modifier mask.
  /// Caution: the platform modifiers may not be the GUI tookit modifiers (e.g. for Qt on MacOSX)
  static uint32_t modifiersToString(uint8_t modifiers, std::string& hotkey);
  
  /// convert string with UTF characters representing modifiers to modifier mask.
  static void modStringToModifiers(const std::string& modchars,
                                   uint8_t& modifiers, std::string& chars);

  /// convert modifier mask to string with UTF characters representing modifiers.
  static void modifiersToModString(uint8_t modifiers, std::string& hotkey);
  
  /// convert apple string with 'using {}' to modifier mask.
  static void appleModStringToModifiers(const std::string& modchars,
                                        uint8_t& modifiers, std::string& chars);
  
  /// Send ascii characters to the frontmost application
  static void sendChar(char c, uint8_t modifiers);
  static void sendChars(const std::string& chars, uint8_t modifiers);
  
  /// Send characters to the frontmost application; _chars_ can contain textual modifiers.
  static void sendModChars(const std::string& chars);
  
  /// Send keycode to the frontmost application
  static void sendKeycode(uint32_t keycode, uint8_t modifiers);
  
  /// converts ascii char to keycode (note: Carbon framework required).
  static uint32_t charToKeycode(char c);
  
  static void printKeyCodes();
};

#endif
