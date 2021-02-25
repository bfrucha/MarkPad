//
//  Services.mm (Mac version)
//  Created by elc on 10/12/2017.
//  Copyright © 2017/18 elc. All rights reserved.
//

#ifdef __APPLE__

#include <iostream>
#include <Foundation/Foundation.h>
#include <Cocoa/Cocoa.h>
#include <Carbon/Carbon.h>
#include "ccuty/ccstring.hpp"
#include "ccuty/ccpath.hpp"
#include "ccuty/macdesktop.hpp"
#include "Conf.h"
#include "GUI.h"
#include "Strings.h"
#include "Services.h"
#include "MTouch.h"
#include "MarkPad.h"
using namespace std;
using namespace ccuty;

#define WAIT_FOR_HOTKEY 0.2

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

@interface Notifier : NSObject {
  id globalMouseMonitor, localMouseMonitor;
  CGPoint mousePos;
}
- (id)init;
- (void)freezeMouse: (bool)state;
@end

struct ServicesImpl {
  Notifier* notifier{nil};
  NSRunningApplication* mpApp{nil};
  bool accessEnabled{false};
  std::string defaultWebBrowser, defaultWebBrowserPath;
  std::function<void()> wakeCallback;
  std::function<void()> sleepCallback;
  std::function<void(uint32_t hotkey)> hotkeyCallback;
  NSDictionary<NSString*,NSMutableArray*> *storedData{nil};
};

static ServicesImpl impl;
static MacDesktop& desk = MacDesktop::instance;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void Services::init() {
  impl.accessEnabled = AXIsProcessTrusted();
  impl.notifier = [[Notifier alloc] init];
  
  // find default web browser
  NSURL *url = nil, *browser = nil;
  if ((url = [NSURL URLWithString:@"http://www.google.com"])
      && (browser = [[NSWorkspace sharedWorkspace] URLForApplicationToOpenURL:url])) {
    impl.defaultWebBrowserPath = [browser fileSystemRepresentation];
  }
  // use Safari as a default if web browser not found
  if (impl.defaultWebBrowserPath.empty()) {
    impl.defaultWebBrowserPath = "/Applications/Safari.app";
  }
  impl.defaultWebBrowser = basename(impl.defaultWebBrowserPath, false);

  // to ask for authorization for Finder (Mojave) to avoid blocking MP later
  std::string s;// = Services::getUserConfDir();
  Services::getFinderFile(s);
  Services::getBrowserUrl(s);
}

void Services::log(const std::string& msg) {
  NSLog(@"MarkPad: %s", msg.c_str());
}

std::string Services::getResourceDir() {
  NSBundle* bundle = [NSBundle mainBundle];
  if (!bundle) return "";
  NSString* respath = [bundle resourcePath];
  if (!respath) return "";
  return [respath UTF8String] + std::string("/");
}

std::string Services::getUserConfDir() {
  return getenv("HOME") + std::string("/Library/MarkPad/");
}

void Services::postpone(std::function<void()> fun) {
  dispatch_time_t popTime = dispatch_time(DISPATCH_TIME_NOW, int64_t(0.1*NSEC_PER_SEC));
  dispatch_after(popTime, dispatch_get_main_queue(), ^(void){fun();});
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

const std::string& Services::getDefaultWebBrowser() {
  return impl.defaultWebBrowser;
}

void Services::openDefaultWebBrowser() {
  openFile(impl.defaultWebBrowserPath);
}

bool Services::getBrowserUrl(std::string& url, const std::string& browser) {
  const string& app = !browser.empty() ? browser : getDefaultWebBrowser();

  if (app == "Safari") {
    return desk.system
    ("osascript -e 'tell application \"Safari\" to return URL of front document'",
     url) >= 0;
  }
  else if (app == "Google Chrome") {
    return desk.system
    ("osascript -e 'tell application \"Google Chrome\" to return URL of active tab of front window'",
     url) >= 0;
  }
  else {
    MarkPad::warning("getBrowserUrl: " + app + " is not supported");
    return false;
  }
}

bool Services::setBrowserUrl(std::string const& url, std::string const& browser) {
  const string& app = !browser.empty() ? browser : getDefaultWebBrowser();

  if (app == "Safari") {
    return desk.system
    ("osascript -e 'tell application \"Safari\" to set URL of front document to \""
     + url + "\"'") >= 0;
  }
  else if (app == "Google Chrome") {
    return desk.system
    ("osascript -e 'tell application \"Google Chrome\" to return URL of active tab of front window to \""
     + url + "\"'") >= 0;
  }
  else {
    MarkPad::warning("setBrowserUrl: " + app + " is not supported");
    return false;
  }
}

bool Services::getBrowserTitle(std::string& url, const std::string& browser) {
  const string& app = !browser.empty() ? browser : getDefaultWebBrowser();
  if (app == "Safari") {
    return
    desk.system("osascript -e 'tell application \"Safari\" to return name of front document'",
                url) >= 0;
  }
  else if (app == "Google Chrome") {
    return
    desk.system("osascript -e 'tell application \"Google Chrome\" to return title of active tab of front window'",
                url) >= 0;
  }
  else {
    MarkPad::warning("getBrowserTitle: " + app + " is not supported");
    return false;
  }
}

bool Services::getFinderFile(std::string& path) {
  return desk.system
  ("osascript -e 'tell application \"Finder\" to return POSIX path of (selection as alias)'",
   path) >= 0;
}

void Services::makeMarkPadFront() {
  if (impl.mpApp)
    [impl.mpApp activateWithOptions: NSApplicationActivateIgnoringOtherApps];
}

void Services::autoHideMenubarAndDock() {
  [NSApp setPresentationOptions: [NSApp presentationOptions]
   | NSApplicationPresentationHideMenuBar | NSApplicationPresentationHideDock];
}

bool Services::getFrontAppPath(std::string& str) {
  return desk.getFrontAppPath(str);
}

bool Services::getFrontAppName(std::string& str) {
  return desk.getFrontAppName(str);
}

void Services::openFile(const std::string& path) {
  desk.openFile(path);
}

void Services::openUrl(const std::string& url) {
  desk.openUrl(url);
}

void Services::openApp(const std::string& path) {
  desk.openApp(path);
}

void Services::openApp(const std::string& path, const std::string& arg) {
  desk.openApp(path, arg);
}

void Services::hideApp(const std::string& path) {
  desk.hideApp(path);
}

void Services::tellApp(const std::string& path, const std::string& command) {
  desk.tellApp(path, command);
}

void Services::changeVolume(int value) {
  desk.changeVolume(value);
}

void Services::setVolume(unsigned int value) {
  desk.setVolume(value);
}

void Services::muteVolume() {
  desk.muteVolume();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// notifications

void Services::setWakeCallback(std::function<void()> fun) {
  impl.wakeCallback = fun;
}

void Services::setSleepCallback(std::function<void()> fun) {
  impl.sleepCallback = fun;
}

void Services::setHotkeyCallback(std::function<void(uint32_t hotkey)> fun) {
  impl.hotkeyCallback = fun;
}

@implementation Notifier

- (id)init {
  if ((self = [super init])) {
    globalMouseMonitor = nil;
    localMouseMonitor = nil;
    impl.mpApp = [NSRunningApplication currentApplication];
    
    NSNotificationCenter* nc = [[NSWorkspace sharedWorkspace] notificationCenter];
    // notification on computer wake up
    [nc addObserver: self
           selector: @selector(wakeNotify:)
               name: NSWorkspaceDidWakeNotification
             object: nil];
    // notification on computer sleep
    [nc addObserver: self
           selector: @selector(sleepNotify:)
               name: NSWorkspaceWillSleepNotification
             object: nil];

    
    // detect when keys are pressed (global events)
    [NSEvent addGlobalMonitorForEventsMatchingMask: NSEventMaskFlagsChanged
                                           handler: ^(NSEvent* e){[self modifierChanged:e];}];
    // detect when keys are pressed (local app events)
    [NSEvent addLocalMonitorForEventsMatchingMask: NSEventMaskFlagsChanged
                                          handler: ^(NSEvent* e){[self modifierChanged:e]; return e;}];
    /*
     // notification when an app is activated (and thus becomes the frontmost app)
     [nc addObserver:self
     selector:@selector(activateNotify:)
     name:NSWorkspaceDidActivateApplicationNotification
     object:nil];
     */
    //keyMonitor1 = nil;
    //keyMonitor2 = nil;
  }
  return self;
}

// when a modifier is pressed
- (void)modifierChanged:(NSEvent*)event {
  if (impl.hotkeyCallback && event.type == NSEventTypeFlagsChanged) {
    uint32_t modMask =
    event.modifierFlags & (NSEventModifierFlagFunction|NSEventModifierFlagShift
                           |NSEventModifierFlagControl
                           |NSEventModifierFlagOption|NSEventModifierFlagCommand);
    (impl.hotkeyCallback)(modMask);
  }
}

// when the computer is awakened.
- (void)wakeNotify: (NSNotification*)note {
  if (impl.wakeCallback) (impl.wakeCallback)();
}

// when the computer starts sleeping.
- (void)sleepNotify: (NSNotification*)note {
  if (impl.sleepCallback) (impl.sleepCallback)();
}

- (void)freezeMouse: (bool)state {
  if (state) {
    if (globalMouseMonitor) return;
    
    mousePos = {
      [NSEvent mouseLocation].x,
      CGDisplayPixelsHigh(kCGDirectMainDisplay) - [NSEvent mouseLocation].y
    };
    
    // global events
    globalMouseMonitor =
    [NSEvent addGlobalMonitorForEventsMatchingMask: NSEventMaskMouseMoved
                                           handler: ^(NSEvent* e){[self mouseMoved:e];}];
    // local app events
    localMouseMonitor =
    [NSEvent addLocalMonitorForEventsMatchingMask: NSEventMaskMouseMoved
                                          handler: ^(NSEvent* e){[self mouseMoved:e]; return e;}];
  }
  else {
    if (globalMouseMonitor) [NSEvent removeMonitor: globalMouseMonitor];
    if (localMouseMonitor) [NSEvent removeMonitor: localMouseMonitor];
    globalMouseMonitor = nil;
    localMouseMonitor = nil;
  }
}
  
- (void)mouseMoved:(NSEvent*)e {
  CGWarpMouseCursorPosition(mousePos);
}

void Services::disableDeviceForCursor(MTDevice *dev) {
  [impl.notifier freezeMouse: true];
}

void Services::enableDeviceForCursor(MTDevice *dev) {
  [impl.notifier freezeMouse: false];
}

/*
- (void)detectSpaceKey: (bool)state {
  // cerr << "detectSpaceKey " << state << endl;
  if (state) {
    // detect when keys are pressed (global events)
    if (!keyMonitor1)
    keyMonitor1 = [NSEvent addGlobalMonitorForEventsMatchingMask: NSEventMaskKeyDown
                                                         handler: ^(NSEvent* e){[self keyChanged:e];}];
    // detect when keys are pressed (local app events)
    if (!keyMonitor2)
    keyMonitor2 = [NSEvent addLocalMonitorForEventsMatchingMask: NSEventMaskKeyDown
                                                        handler: ^(NSEvent* e){[self keyChanged:e]; return e;}];
  }
  else {
    if (keyMonitor1) [NSEvent removeMonitor:keyMonitor1];
    if (keyMonitor2) [NSEvent removeMonitor:keyMonitor2];
    keyMonitor1 = nil;
    keyMonitor2 = nil;
  }
}
 
 - (void)keyChanged:(NSEvent*)event {
 if (impl.editKeyCallback && event.type == NSEventTypeKeyDown && event.keyCode == kVK_Space)
 (impl.editKeyCallback)();
 }
 */

@end

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Accessibility & detection of the Hotkeys that open the main menu

bool Services::isAccessibilityEnabled() {
  return impl.accessEnabled;
}

// accessibility is needed to access key presses
bool Services::askAccessibility() {
  if (impl.accessEnabled) return true;
  //NSLog(@"MarkPad: Services::askAccessibility");

  // alert() will appear below the overlay if the editor is opened!
  MarkPad::instance.edit(MarkPad::EditNone);
  int resp = GUI::alert(Strings::k.AccessibilityMsg,
                        Strings::k.AccessibilityInfo,
                        Strings::k.AccessibilityBtns);
  switch (resp) {
    case 3:   // "Permanently Disable"
      impl.accessEnabled = false;
      Conf::change(&Conf::allowAccessibility, false);
      break;
      
    case 2:   // "See later" => inchanged
      break;
      
    case 1: {  // Try to enable
      // opens the accessibility preference pane
      NSDictionary* opts = @{(__bridge id)kAXTrustedCheckOptionPrompt: @YES};
      AXIsProcessTrustedWithOptions((__bridge CFDictionaryRef)opts);
      
      // open popup to block until accessibilty is set
      GUI::alert(Strings::k.EnableAcessibilityMsg,
                 Strings::k.EnableAcessibilityInfo,
                 Strings::k.EnableAcessibilityBtns,
                 400, 200);
      
      if (AXIsProcessTrusted()) {
        impl.accessEnabled = true;
        Conf::change(&Conf::allowAccessibility, true);
      }
      else {
        impl.accessEnabled = false;
      }
    } break;
  }
  return impl.accessEnabled;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// adapted from Théo Winterhalter
// https://stackoverflow.com/questions/1918841/how-to-convert-ascii-character-to-cgkeycode

static NSString* keyCodeToString(CGKeyCode keyCode)
{
  TISInputSourceRef currentKeyboard = TISCopyCurrentKeyboardInputSource();
  CFDataRef uchr =
  (CFDataRef)TISGetInputSourceProperty(currentKeyboard, kTISPropertyUnicodeKeyLayoutData);
  const UCKeyboardLayout *keyboardLayout = (const UCKeyboardLayout*)CFDataGetBytePtr(uchr);
  
  if (keyboardLayout) {
    UInt32 deadKeyState = 0;
    UniCharCount maxStringLength = 255;
    UniCharCount actualStringLength = 0;
    UniChar unicodeString[maxStringLength];
    
    OSStatus status = UCKeyTranslate(keyboardLayout,
                                     keyCode, kUCKeyActionDown, 0,
                                     LMGetKbdType(), 0,
                                     &deadKeyState,
                                     maxStringLength,
                                     &actualStringLength, unicodeString);
    
    if (actualStringLength == 0 && deadKeyState) {
      status = UCKeyTranslate(keyboardLayout,
                              kVK_Space, kUCKeyActionDown, 0,
                              LMGetKbdType(), 0,
                              &deadKeyState,
                              maxStringLength,
                              &actualStringLength, unicodeString);
    }
    if (actualStringLength > 0 && status == noErr)
      return [[NSString stringWithCharacters:unicodeString
                                      length:(NSUInteger)actualStringLength] lowercaseString];
  }
  
  return nil;
}

struct CharToCode : public std::vector<uint32_t> {
  CharToCode() {
    resize(128);
    for (uint32_t i = 0; i < 128; ++i) {
      NSString* str = keyCodeToString((CGKeyCode)i);
      if (str != nil && ![str isEqualToString:@""]) {
        int c = [str characterAtIndex:0];
        if (c >= 0 && c < 128) (*this)[c] = i;
      }
    }
  }
};

uint32_t Services::charToKeycode(char c) {
  static CharToCode char_to_code;
  return char_to_code[std::tolower(c)];
}

void Services::printKeyCodes() {
  cerr << "printKeyCodes"<<endl;
  for (int c = 0; c < 128; ++c) {
    std::cout << "char: " << char(c) << " keycode: " << charToKeycode(c) << std::endl;
  }
}

void Services::sendKeycode(uint32_t keycode, uint8_t mods) {
  MacDesktop::sendKeycode(keycode, mods);
}

void Services::sendChar(char c, uint8_t mods) {
  if (std::isupper(c)) {mods |= MacDesktop::Shift; c = tolower(c);}
  MacDesktop::sendKeycode(charToKeycode(c), mods);
}

void Services::sendChars(const std::string& chars, uint8_t mods) {
  for (char c : chars) sendChar(c, mods);
}

void Services::sendModChars(const string& modchars) {
  uint8_t genmask = 0;
  size_t k = 0;
  for ( ; k < modchars.size(); k += 3) {
    string s = modchars.substr(k,3);
    if (s=="⇧") genmask |= Modifiers::Shift;
    else if (s=="⌃") genmask |= Modifiers::Control;
    else if (s=="⌘") genmask |= Modifiers::Command;
    else if (s=="⌥") genmask |= Modifiers::Alt;
    else break;
  }
  // chars must be lowerized !
  for (; k < modchars.size(); ++k) sendChar(std::tolower(modchars[k]), genmask);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

uint32_t Services::stringToModifiers(const std::string& key, uint8_t& states,
                                     std::string& chars) {
  states = 0;
  chars.clear();
  if (key.empty()) return 0;
  
  std::vector<string> v;
  ccuty::strsplit(v, key, "+");
  uint32_t mask = 0;
  
  for (auto& s : v) {
    if (iequal(s,"fn")) {
      states |= Modifiers::Function;
      mask |= NSEventModifierFlagFunction;
    }
    else if (iequal(s,"shift")) {
      states |= Modifiers::Shift;
      mask |= NSEventModifierFlagShift;
    }
    else if (iequal(s,"ctrl")) {
      states |= Modifiers::Control;
      mask |= NSEventModifierFlagControl;
    }
    else if (iequal(s,"alt")) {
      states |= Modifiers::Alt;
      mask |= NSEventModifierFlagOption;
    }
    else if (iequal(s,"cmd")) {
      states |= Modifiers::Command;
      mask |= NSEventModifierFlagCommand;
    }
    else chars += s;
  }
  return mask;
}

uint32_t Services::modifiersToString(uint8_t mods, std::string& hotkey) {
  uint32_t mask = 0;
  hotkey = "";
  if (mods & Modifiers::Function) {
    mask |= NSEventModifierFlagFunction;
    hotkey += "fn+";
  }
  if (mods & Modifiers::Command) {
    mask |= NSEventModifierFlagCommand;
    hotkey += "cmd+";
  }
  if (mods & Modifiers::Shift) {
    mask |= NSEventModifierFlagShift;
    hotkey += "shift+";
  }
  if (mods & Modifiers::Control) {
    mask |= NSEventModifierFlagControl;
    hotkey += "ctrl+";
  }
  if (mods & Modifiers::Alt) {
    mask |= NSEventModifierFlagOption;
    hotkey += "alt+";
  }
  if (!hotkey.empty()) hotkey.pop_back();  // remove last +
  return mask;
}

void Services::modifiersToModString(uint8_t mods, std::string& hotkey) {
  hotkey = "";
  //if (mods & Modifiers::Function) {   !!!!
  //  hotkey += "fn+";
  //}
  if (mods & Modifiers::Command) {
    hotkey += "⌘";
  }
  if (mods & Modifiers::Shift) {
    hotkey += "⇧";
  }
  if (mods & Modifiers::Control) {
    hotkey += "⌃";
  }
  if (mods & Modifiers::Alt) {
    hotkey += "⌥";
  }
}

void Services::modStringToModifiers(const std::string& modchars,
                                    uint8_t& genmask, std::string& chars) {
  genmask = 0;
  size_t k = 0;
  for ( ; k < modchars.size(); k += 3) {
    string s = modchars.substr(k,3);
    if (s=="⇧") genmask |= Modifiers::Shift;
    else if (s=="⌃") genmask |= Modifiers::Control;
    else if (s=="⌥") genmask |= Modifiers::Alt;
    else if (s=="⌘") genmask |= Modifiers::Command;
    else break;
  }
  chars = modchars.substr(k);
}

void Services::appleModStringToModifiers(const string& fromkey,
                                         uint8_t& genmask, string& tokey) {
  std::vector<string> v;
  strsplit(v, fromkey, strdelim("{,}").spaces().quotes());
  genmask = 0;
  
  for (auto& s : v) {
    if (s=="shift") genmask |= Modifiers::Shift;
    else if (s=="control") genmask |= Modifiers::Control;
    else if (s=="option") genmask |= Modifiers::Command;
    else if (s=="command") genmask |= Modifiers::Alt;
  }
  if (v.size() < 2) tokey = fromkey;
  else tokey = v[0] + " " + v[1];
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// copy all pasteboard items (a pasteboard item cannot be shared)
static NSMutableArray* copyPasteboardItems(NSPasteboard *pb) {
  // copy info with sytle in temporary pasteboard
  NSMutableArray *pasteboardItems = [NSMutableArray array];
  
  for (NSPasteboardItem *item in [pb pasteboardItems]) {
    //Create new data holder
    NSPasteboardItem *dataHolder = [[NSPasteboardItem alloc] init];
    
    //For each type in the pasteboard's items
    for (NSString *type in [item types]) {
     
      //Get each type's data and add it to the new dataholder
      NSData *data = [[item dataForType:type] mutableCopy];
      if (data) {
        [dataHolder setData:data forType:type];
      }
    }
    [pasteboardItems addObject:dataHolder];
  }
  return pasteboardItems;
}

// copy text without style or paste without style what is already in the pasteboard
static void copyPasteWithoutStyle(bool copy) {
  NSPasteboard *pb = [NSPasteboard generalPasteboard];
  
  // no data to read in case of paste
  if (!copy && [[pb pasteboardItems] count] == 0) { return; }
  // copy data using copy hotkey (need time to process event)
  else if (copy) {
    MacDesktop::instance.sendKeycode(kVK_ANSI_C, MacDesktop::Command);
    [NSThread sleepForTimeInterval: WAIT_FOR_HOTKEY];
  }
  
  // copy pasteboard items with style
  // NSPasteboard *pprev = [NSPasteboard pasteboardWithUniqueName];
  // [pprev writeObjects:copyPasteboardItems(pb)];
  NSMutableArray *items = copyPasteboardItems(pb);
  
  // copy info without style in temporary string
  NSString *nostyle = [pb stringForType:NSPasteboardTypeString];
  [pb clearContents];
  [pb setString:nostyle forType:NSPasteboardTypeString];
  
  // paste data and restore data with style
  if (!copy) {
    MacDesktop::instance.sendKeycode(kVK_ANSI_V, MacDesktop::Command);
    [NSThread sleepForTimeInterval: WAIT_FOR_HOTKEY];
    [pb clearContents];
    [pb writeObjects:items];
  }
}

void Services::copyWithoutStyle() {
  copyPasteWithoutStyle(true);
}

void Services::pasteWithoutStyle() {
  copyPasteWithoutStyle(false);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static NSString* toNSString(const std::string& s) {
  return s.empty() ? @"" : [NSString stringWithUTF8String:s.c_str()];
}

// copy/paste data into/from storing area
static void handleBuffer(const std::string& bufferName, bool store) {
  if (impl.storedData == nil) {impl.storedData = [NSDictionary new]; };
  
  // create folder to store data
  NSString *dir = toNSString(Conf::bufferDir());
  if(![[NSFileManager defaultManager] fileExistsAtPath: dir]) {
    [[NSFileManager defaultManager]
     createDirectoryAtPath: dir
     withIntermediateDirectories:NO attributes:nil error:nil];
  }
  
  //FIXME 1. only text data recorded yet, 2. should clean directory from time to time
  //NSString *areaName = [NSString stringWithUTF8String:arg.c_str()];
  //NSString *filepath = [dir stringByAppendingFormat:@"%@", areaName];
  
  NSString *filepath = toNSString(Conf::bufferDir()+bufferName);
  if (store) {
    desk.sendKeycode(kVK_ANSI_C, MacDesktop::Command);
    [NSThread sleepForTimeInterval: WAIT_FOR_HOTKEY];
    NSPasteboard *pb = [NSPasteboard generalPasteboard];
    [[NSFileManager defaultManager]
     createFileAtPath: filepath
     contents: [pb dataForType: NSPasteboardTypeString]
     attributes: nil];
  }
  else if ([[NSFileManager defaultManager] fileExistsAtPath:filepath]) {
    NSPasteboard *pb = [NSPasteboard generalPasteboard];
    NSMutableArray *items = copyPasteboardItems(pb);
    NSData *content = [[NSFileManager defaultManager] contentsAtPath:filepath];
    [pb clearContents];
    [pb setData:content forType:NSPasteboardTypeString];
    
    desk.sendKeycode(kVK_ANSI_V, MacDesktop::Command);
    [NSThread sleepForTimeInterval:WAIT_FOR_HOTKEY];
    [pb clearContents];
    [pb writeObjects:items];
  }
}

void Services::copyToBuffer(const std::string& bufferName) {
  handleBuffer(bufferName, true);
}

void Services::pasteFromBuffer(const std::string& bufferName) {
  handleBuffer(bufferName, false);
}

void Services::pasteString(const std::string& str) {
  NSPasteboard *pb = [NSPasteboard generalPasteboard];
  NSMutableArray *items = copyPasteboardItems(pb);
  
  [pb clearContents];
  [pb setString:[NSString stringWithUTF8String: str.c_str()]
        forType:NSPasteboardTypeString];
  
  desk.sendKeycode(kVK_ANSI_V, MacDesktop::Command);
  [NSThread sleepForTimeInterval:WAIT_FOR_HOTKEY];
  [pb clearContents];
  [pb writeObjects:items];
}

#endif
