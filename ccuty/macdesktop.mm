//
//  macdesktop: Useful functions for managing the MacOSX desktop.
//  (C) Eric Lecolinet 2017 - www.telecom-paristech.fr/~elc
//

#import <Carbon/Carbon.h>
#import <Cocoa/Cocoa.h>
#import <iostream>
#import "ccstring.hpp"
#import "ccpath.hpp"
#import "macdesktop.hpp"
using std::string;
using namespace std;

namespace ccuty {

  MacDesktop MacDesktop::instance;

  int MacDesktop::system(const std::string& command) {
    return ::system(command.c_str());
  }
  
  int MacDesktop::system(const std::string& command, std::string& result) {
    result.clear();
    FILE* f = ::popen(command.c_str(), "r");
    if (!f) return -1;
    char buf[500] = "";
    while (fgets(buf, sizeof(buf), f)) {
      if (::strlen(buf) > 0) result += buf; else break;
    }
    ::pclose(f);
    if (result.size() > 0 && result[result.size()-1] == '\n') result.pop_back();
    return 0;
  }
  
  void MacDesktop::open(const string& path) {
    if (path.empty()) return;
    system("open \"" + path + "\"");
  }
  
  void MacDesktop::openUrl(const string& path) {
    if (path.empty()) return;
    NSString* s = nil;
    if (path.compare(0, 4, "http")==0 || path.compare(0, 5, "file:")==0) {
      s = [NSString stringWithUTF8String:path.c_str()];
    }
    else {  // make it work if prefix is missing
      string path2;
      if (path.compare(0, 2, "//")==0) path2 = "http:" + path;
      else path2 = "http://" + path;
      s = [NSString stringWithUTF8String:path2.c_str()];
    }
    NSURL* url = nil;
    if (s && (url=[NSURL URLWithString:s])) [[NSWorkspace sharedWorkspace] openURL:url];
  }
  
  void MacDesktop::openFile(const string& path) {
    if (path.empty()) return;
    // edit scripts (apple scripts are open in editor but not unix scripts)
    string ext = extname(path);
    if (ext == ".sh" || ext == ".bash" || ext == ".zsh")
      system("open -e \"" + path + "\"");
    else system("open \"" + path + "\"");
  }
  
  void MacDesktop::openApp(const string& path) {
    if (path.empty()) return;
    string ext = extname(path);
    // execute scripts as apps
    if (ext == ".scpt" || ext == ".sh" || ext == ".bash" || ext == ".zsh") {
      string file = path;
      if (path.compare(0, 5, "file:")==0) file.erase(0,5);
      if (ext == ".scpt") system("osascript " + file + "&");
      else system(". " + file + "&");
    }
    else if (NSString* s = [NSString stringWithUTF8String:path.c_str()])
      [[NSWorkspace sharedWorkspace] launchApplication:s];
  }
  
  void MacDesktop::openApp(const string& path, const string& args) {
    if (path.empty()) return;
    string ext = extname(path);
    // execute scripts as apps
    if (ext == ".scpt" || ext == ".sh" || ext == ".bash" || ext == ".zsh") {
      string file = path;
      if (path.compare(0, 5, "file:")==0) file.erase(0,5);
      if (ext == ".scpt") system("osascript " + file + " " + args + "&");
      else system(". " + file + " " + args + "&");
    }
    else system("open -a \"" + path + "\" " + args);
  }
  
  void MacDesktop::hideApp(const string& appName) {
    if (appName.empty()) return;
    system("osascript -e 'tell application \"System Events\" to tell process \""
           + appName + "\" to set visible to false' 2> /dev/null");
  }
  
  /*
   void MacDesktop::openHideApp(const string& appName) {
   if (appName.empty()) return;
   if (appName == frontMostApp()) hideApp(appName); else openApp(appName);
   }
   */
  
  bool MacDesktop::getFrontAppPath(std::string& str) {
    NSRunningApplication* a = [[NSWorkspace sharedWorkspace] frontmostApplication];
    str = a ? [[a executableURL] fileSystemRepresentation] : "";
    return !str.empty();
  }
  
  bool MacDesktop::getFrontAppName(std::string& str) {
    NSRunningApplication* a = [[NSWorkspace sharedWorkspace] frontmostApplication];
    str = a ? [[[a executableURL] lastPathComponent] UTF8String]: "";
    return !str.empty();
  }
  
  bool MacDesktop::getFrontAppLocName(std::string& str) {
    NSRunningApplication* a = [[NSWorkspace sharedWorkspace] frontmostApplication];
    str = a ? [[a localizedName] UTF8String]: "";
    return !str.empty();
  }
  
  void MacDesktop::activateApp(const string& appName) {
    if (appName.empty()) return;
    system("osascript -e 'tell application \"" + trimmed(appName) + "\" to activate' 2>&1>/dev/null");
  }
  
  void MacDesktop::tellApp(const string& appName, const string& command, bool inBackground) {
    if (appName.empty()) return;
    system("osascript -e 'tell application \"" + trimmed(appName) + "\" to " + command
           + "'" + (inBackground ? "&" : ""));
  }
  
  void MacDesktop::tellApp(const string& appName, const string& command, string& result) {
    if (appName.empty()) return;
    system("osascript -e 'tell application \"" + trimmed(appName) + "\" to " + command + "'",
           result);
  }
  
  /*
  void MacDesktop::doScript(const string& cmd) {
    if (cmd.empty()) return;
    tellApp("Terminal", "do script " + cmd + " in window frontmost");
  }
  */
  
  void MacDesktop::showHideDock() {
    sendKeyStroke("\"d\" using {command down, option down}");
  }
  
  /*
   void MacDesktop::centerCurrentWindow() {
   const char* cmd =
   "osascript<<END\n\
   tell application \"Finder\"\n\
   set screenSize to bounds of window of desktop\n\
   set screenWidth to item 3 of screenSize\n\
   set screenHeight to item 4 of screenSize\n\
   end tell\n\
   tell application \"System Events\" to set myFrontMost to name of the first process whose frontmost is true\n\
   tell application myFrontMost to set bounds of window 1 to {40, 100, screenWidth, screenHeight-100}\n\
   END";   // must be on the beginning of the line
   ::system(cmd);
   }
   */
  
  void MacDesktop::wakeScreen() {
    ::system("caffeinate -u -t 3 &");
  }
  
  void MacDesktop::beep(int seconds) {
    char cmd[50];
    ::sprintf(cmd, "osascript -e 'beep %d' &", seconds);
    ::system(cmd);
  }
  
  void MacDesktop::changeVolume(int val, bool displayNotification) {
    if (val == 0) return;
    string sign;
    if (val > 0) { sign = "+"; }
    if (val < 0) { sign = "-"; val = -val; }
    system("osascript -e 'set volume output volume (output volume of (get volume settings)"
           + sign + std::to_string(val) + ")'");
    if (displayNotification) {
      system("osascript -e 'display notification \"Volume set to \" & (output volume of (get volume settings))'");
    }
  }
  
  void MacDesktop::setVolume(unsigned int val, bool displayNotification) {
    // cf aussi: osascript -e 'set volume 2' (value in (0,7) and ... output muted true
    char buf[500];
    sprintf(buf, "osascript -e 'set volume output volume %d'", val);
    ::system(buf);
    if (displayNotification) {
      sprintf(buf, "osascript -e 'display notification \"Volume set to %d \"'", val);
      ::system(buf);
    }
  }
  
  void MacDesktop::muteVolume() {
    system("osascript -e 'set curVolume to get volume settings\n"
           "if output muted of curVolume is false then\n"
           "set volume with output muted\n"
           "display notification \"Volume muted\"\n"
           "else\n"
           "set volume without output muted\n"
           "display notification \"Volume unmuted\"\n"
           "end if'");
  }
  
  void MacDesktop::playPause() {
    system("osascript -e 'tell application \"iTunes\"\n"
           "playpause\n"
           "if player state is paused then\n"
           "display notification \"Song paused\" with title \"iTunes\"\n"
           "else\n"
           "display notification \"Song playing\" with title \"iTunes\"\n"
           "end if\n"
           "end tell'");
  }
  
  void MacDesktop::say(const string& sentence) {
    system("say '" + sentence + "' &");
  }
  
  // either album or artist may be empty (resp. meaning any album from this artist
  // or any album with this name
  
  bool MacDesktop::playAlbum(const string& album, const string& artist) {
    string request;
    if (!album.empty()) request = "album contains \"" + album + "\"";
    if (!artist.empty()) {
      if (!request.empty()) request += " and ";
      request += "artist contains \"" + artist + "\"";
    }
    if (request.empty()) return false;
    
    system("osascript<<END\n\
           tell application \"iTunes\"\n\
           set TMP_PLISTS to (user playlists whose name = \"@Tmp\")\n\
           repeat with p in TMP_PLISTS\n\
           delete p\n\
           end repeat\n\
           set TMP_PLAYLIST to (make user playlist with properties {name:\"@Tmp\"})\n\
           duplicate (tracks whose " + request + ") to TMP_PLAYLIST\n\
           play TMP_PLAYLIST\n\
           reveal TMP_PLAYLIST\n\
           end tell\n\
           END");   // must be on the beginning of the line
    return true;
  }
  
  
  bool MacDesktop::playPlaylist(const string& playlist, const string& track) {
    if (playlist.empty()) return false;
    if (track.empty())
      tellApp("iTunes","play playlist \"" + playlist + '"');
    else
      tellApp("iTunes","play first item of (search user playlist \""
              + playlist + "\" for \"" + track + "\")");
    return true;
  }
  
  /*
   command:
   osascript -e 'tell application "iTunes" to return {artist of current track, name of current track, "Album", album of current track, current stream title, kind of current track}'
   or:
   tell application "iTunes"
   set _title to name of current track
   set _artist to artist of current track
   set _album to album of current track
   log {"Artist:", _artist, "Title:", _title, "Album", _album}
   end tell
   */
  bool MacDesktop::getCurrentTrack(string& album, string& artist, string& track, bool isPlaying) {
    const char* cmd = NULL;
    if (isPlaying)
      cmd = "osascript -e 'tell application \"iTunes\" to return {artist of current track,\
      \"\t\", name of current track, \"\t\", album of current track, \"\t\", current stream title,\
      \"\t\", kind of current track}' 2> /dev/null";
    else
      cmd = "osascript -e 'tell application \"iTunes\" to return {artist of selection,\
      \"\t\", name of selection, \"\t\", album of selection, \"\t\", \"***\",\
      \"\t\", kind of selection}' 2> /dev/null";
    
    FILE* f = ::popen(cmd, "r");
    if (!f) return false;
    char buf[500] = "";
    fgets(buf, sizeof(buf), f);
    pclose(f);
    
    std::vector<string> tokens;
    ccuty::strsplit(tokens, buf, "\t");
    for (auto& s : tokens) ccuty::strtrim(s, ", ");  // remove spaces and commas
    
    if (tokens.size() < 5) return false;
    else if (tokens[4].find("Flux") == 0) {
      artist = "";
      album = tokens[1];
      track = tokens[3];
    }
    else {
      artist = tokens[0];
      track = tokens[1];
      album = tokens[2];
    }
    return true;
  }
  
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // KEYBOARD EVENTS
  
  void MacDesktop::sendKeyStroke(const string& keystroke) {
    if (keystroke.empty()) return;
    system("osascript -e 'tell application \"System Events\" to keystroke " + keystroke + "'");
  }

  void MacDesktop::sendKeycode(uint32_t keycode, uint8_t mods, uint8_t type) {
    CGEventFlags flags = 0;
    if (mods) {
      if (mods & Alt) flags |= kCGEventFlagMaskAlternate;
      if (mods & Control) flags |= kCGEventFlagMaskControl;
      if (mods & Command) flags |= kCGEventFlagMaskCommand;
      if (mods & Shift) flags |= kCGEventFlagMaskShift;
    }
    cerr << flags << " " << keycode << endl;
    if (type & KeyDown) {
      CGEventRef e = CGEventCreateKeyboardEvent(NULL, CGKeyCode(keycode), true);
      if (mods) CGEventSetFlags(e, flags);
      CGEventPost(kCGHIDEventTap, e);
      CFRelease(e);
    }
    if (type & KeyUp) {
      CGEventRef e = CGEventCreateKeyboardEvent(NULL, CGKeyCode(keycode), false);
      if (mods) CGEventSetFlags(e, flags);
      CGEventPost(kCGHIDEventTap, e);
      CFRelease(e);
    }
  }

  void MacDesktop::sendLeftKey()   {sendKeycode(kVK_LeftArrow);}
  void MacDesktop::sendRightKey()  {sendKeycode(kVK_RightArrow);}
  void MacDesktop::sendUpKey()     {sendKeycode(kVK_UpArrow);}
  void MacDesktop::sendDownKey()   {sendKeycode(kVK_DownArrow);}
  void MacDesktop::sendTabKey()    {sendKeycode(kVK_Tab);}
  void MacDesktop::sendBacktabKey(){sendKeycode(kVK_Tab, Shift);}
  void MacDesktop::sendSpaceKey()  {sendKeycode(kVK_Space);}
  void MacDesktop::sendReturnKey() {sendKeycode(kVK_Return);}
  void MacDesktop::sendEscapeKey() {sendKeycode(kVK_Escape);}
  void MacDesktop::sendDeleteKey() {sendKeycode(kVK_Delete);}
  
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  
  void MacDesktop::getMainScreenSize(size_t& width, size_t& height) {
    width = CGDisplayPixelsWide(kCGDirectMainDisplay);
    height = CGDisplayPixelsHigh(kCGDirectMainDisplay);
  }

  /*
  size_t MacDesktop::mainDisplayWidth() const {
    return CGDisplayPixelsWide(kCGDirectMainDisplay);
  }
  size_t MacDesktop::mainDisplayHeight() const {
    return CGDisplayPixelsHigh(kCGDirectMainDisplay);
  }
  */
  
  void MacDesktop::hideMouse(bool hide) {
    if (hide) CGDisplayHideCursor(kCGNullDirectDisplay);
    else CGDisplayShowCursor(kCGNullDirectDisplay);
  }
  
  void MacDesktop::getMouse(double& x, double& y) {
    CGEventRef e = CGEventCreate(NULL);
    NSPoint mousePos = CGEventGetLocation(e);
    x = mousePos.x;
    y = mousePos.y;
    CFRelease(e);
  }
  
  void MacDesktop::pressMouse(double x, double y, unsigned int button) {
    CGMouseButton mouseButton = static_cast<CGMouseButton>(button);
    CGEventType mouseType;
    switch (mouseButton) {
      case kCGMouseButtonLeft:
        mouseType = kCGEventLeftMouseDown;
        break;
      case kCGMouseButtonRight:
        mouseType = kCGEventRightMouseDown;
        break;
      case kCGMouseButtonCenter:
      default:
        mouseType = kCGEventOtherMouseDown;
        break;
    }
    CGPoint mousePos = {x, y};
    CGEventRef e = CGEventCreateMouseEvent(NULL, mouseType, mousePos, mouseButton);
    CGEventSetType(e, mouseType);
    CGEventPost(kCGHIDEventTap, e);
    CFRelease(e);
  }
  
  
  void MacDesktop::releaseMouse(double x, double y, unsigned int button) {
    CGMouseButton mouseButton = static_cast<CGMouseButton>(button);
    CGEventType mouseType;
    switch (mouseButton) {
      case kCGMouseButtonLeft:
        mouseType = kCGEventLeftMouseUp;
        break;
      case kCGMouseButtonRight:
        mouseType = kCGEventRightMouseUp;
        break;
      case kCGMouseButtonCenter:
      default:
        mouseType = kCGEventOtherMouseUp;
        break;
    }
    CGPoint mousePos = {x, y};
    CGEventRef e = CGEventCreateMouseEvent(NULL, mouseType, mousePos, mouseButton);
    CGEventSetType(e, mouseType);
    CGEventPost(kCGHIDEventTap, e);
    CFRelease(e);
  }
  
  
  void MacDesktop::dragMouse(double x, double y, unsigned int button) {
    CGMouseButton mouseButton = static_cast<CGMouseButton>(button);
    CGEventType mouseType;
    switch (mouseButton) {
      case kCGMouseButtonLeft:
        mouseType = kCGEventLeftMouseDragged;
        break;
      case kCGMouseButtonRight:
        mouseType = kCGEventRightMouseDragged;
        break;
      case kCGMouseButtonCenter:
      default:
        mouseType = kCGEventOtherMouseDragged;
        break;
    }
    CGPoint mousePos = {x, y};
    CGEventRef event = CGEventCreateMouseEvent(NULL, mouseType, mousePos, mouseButton);
    CGEventSetType(event, kCGEventLeftMouseDragged);
    CGEventPost(kCGHIDEventTap, event);
    CFRelease(event);
  }
  
  void MacDesktop::moveMouse(double x, double y) {
    // see also: CGWarpMouseCursorPosition CGDisplayMoveCursorToPoint
    CGPoint mousePos = {x, y};
    CGEventRef event = CGEventCreateMouseEvent(NULL, kCGEventMouseMoved,
                                               mousePos, kCGMouseButtonLeft);
    CGEventSetType(event, kCGEventMouseMoved);
    CGEventPost(kCGHIDEventTap, event);
    CFRelease(event);
  }
  
}
