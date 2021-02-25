//
//  AWL.mm (Cocoa impl.)
//  (c) Eric Lecolinet - http://www.telecom-paristech.fr/~elc
//  Copyright (c) 2018. All rights reserved.
//

#include <iostream>
#include <vector>
#include <Carbon/Carbon.h>
#include "ccuty/ccstring.hpp"
#include "AWL.h"
#include "Conf.h"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

NSString* toNativeString(const std::string& s) {
  return s.empty() ? @"" : [NSString stringWithUTF8String:s.c_str()];
}

NSString* toNativeString(const char* s) {
  return !s ? @"" : [NSString stringWithUTF8String:s];
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

AWDelegate* AWL::del{}; // app widgets

AWColor
* AWL::ClearColor = [NSColor clearColor],
* AWL::WhiteColor = [NSColor whiteColor],
* AWL::BlackColor = [NSColor blackColor],
* AWL::GrayColor  = [NSColor grayColor],
* AWL::RedColor   = [NSColor redColor],
* AWL::GreenColor = [NSColor greenColor],
* AWL::BlueColor  = [NSColor blueColor],
* AWL::YellowColor= [NSColor yellowColor],
* AWL::OrangeColor= [NSColor orangeColor],
* AWL::MagentaColor = [NSColor magentaColor],
* AWL::CyanColor  = [NSColor cyanColor];


AWCursor
*AWL::arrowCursor = [NSCursor arrowCursor],
*AWL::crosshairCursor = [NSCursor crosshairCursor],
*AWL::openHandCursor = [NSCursor openHandCursor],
*AWL::closedHandCursor = [NSCursor closedHandCursor],
*AWL::pointingHandCursor = [NSCursor pointingHandCursor],
*AWL::resizeVerCursor = [NSCursor resizeUpDownCursor],
*AWL::resizeHorCursor = [NSCursor resizeLeftRightCursor],
*AWL::resizeDiagPlus45Cursor = nil,
*AWL::resizeDiagMinus45Cursor = nil;

const uint32_t
AWL::BackspaceKeycode = kVK_Delete,
AWL::SpaceKeycode  = kVK_Space,
AWL::ReturnKeycode = kVK_Return,
AWL::EscapeKeycode = kVK_Escape;

const uint32_t
AWL::ShiftModifier = NSEventModifierFlagShift,
AWL::ControlModifier = NSEventModifierFlagControl,
AWL::AltModifier = NSEventModifierFlagOption,
AWL::CommandModifier = NSEventModifierFlagCommand,
AWL::FunctionModifier = NSEventModifierFlagFunction;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void AWL::init(AWDelegate* d) {
  del = d;
  
  // MUSt be init here for some reason!
  arrowCursor = [NSCursor arrowCursor];
  
  // OK becuase Conf::init() is called at the beginning of main
  resizeDiagPlus45Cursor
  = newCursor(Conf::imageDir()+"cursorResizeDiagonalRight.ai", 8, 8);
  
  resizeDiagMinus45Cursor
  = newCursor(Conf::imageDir()+"cursorResizeDiagonalLeft.ai", 8, 8);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

int AWL::alert(const std::string& msg, const std::string& info,
               const std::string& buttons) {
  return alert(msg, info, buttons, -1., -1.);
}

int AWL::alert(const std::string& msg, const std::string& info,
               const std::string& buttons,
               float x, float y)
{
  std::cerr << "MarkPad: " << msg << ":\n" << info << std::endl;
  if (!del) return 0;
  
  NSAlert* alert = [[NSAlert alloc] init];
  //[alert setMessageText: toNativeString("MarkPad: " + msg)];
  [alert setMessageText: toNativeString(msg)];
  [alert setInformativeText: toNativeString(info)];

  if (buttons.empty()) [alert addButtonWithTitle:@"OK"];
  else {
    std::vector<std::string> v;
    ccuty::strsplit(v, buttons, ",");
    for (auto& s : v) [alert addButtonWithTitle: toNativeString(s)];
  }
  if (x >= 0 && y >= 0) {
    [alert.window makeKeyAndOrderFront:AWL::del];
    [alert.window setFrameOrigin:NSPoint{x,y}];
  }
    
  // PROBLEME risque de plantage ou de blocage !!!
  //mp().edit(false); // must close because alert may appear below overlay !!
  
  return (int)[alert runModal] - NSAlertFirstButtonReturn + 1;;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

AWCursor* AWL::newCursor(const std::string& filename, float xhotspot, float yhotspot) {
  NSPoint hotspot = NSMakePoint(xhotspot, yhotspot);
  return [[NSCursor new] initWithImage: newImage(filename) hotSpot:hotspot];
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

NSImage* AWL::newImage(const std::string& filename) {
  return [[NSImage new] initByReferencingFile: toNativeString(filename.c_str())];
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

NSFont* AWL::newFont(const std::string& face, float size) {
  // fully specified family-face name, such as Helvetica-BoldOblique or Times-Roman.
  return [NSFont fontWithName: toNativeString(face) size:size];
}

AWFont* AWL::newFont(const std::string& name) {
  std::string face, sizestr;
  float size = 12.f;
  ccuty::strsplit(face, sizestr, name, ",");

  try {size = std::stof(sizestr);}
  catch (...) {std::cerr << "MarkPad: font has wrong size: "<< name << std::endl;}
  return newFont(face, size);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

NSColor* AWL::newColor(float red, float green, float blue, float alpha) {
  //return [NSColor colorWithCalibratedRed:red green:green blue:blue alpha:alpha];
  return [NSColor colorWithRed:red green:green blue:blue alpha:alpha];
}

AWColor* AWL::newColor(const std::string& name) {
  if (name.empty()) {
    std::cerr << "MarkPad: color has empty name"<< std::endl;
    return newColor(0, 0, 0, 1);
  }
  
  if (::isdigit(name[0])) {
    std::vector<std::string> v;
    auto count = ccuty::strsplit(v, name, ",");
    float r=1.f, g=1.f, b=1.f, a=1.f;
    
    if (count != 4)
      std::cerr << "MarkPad: color has wrong format: "<< name << std::endl;
    else {
      try {
        r = std::stof(v[0]);
        g = std::stof(v[1]);
        b = std::stof(v[2]);
        a = std::stof(v[3]);
      }
      catch (...) {
        std::cerr << "MarkPad: color has wrong format: "<< name << std::endl;
      }
    }
    return newColor(r, g, b, a);
  }
  
  // search color
  if (NSString* key = toNativeString(name)) {
    auto* list = [NSColorList availableColorLists];
    for (int k = 0; k < [list count]; ++k) {
      NSColor* c = [list[k] colorWithKey:key];
      if (c) return c;
    }
  }
  std::cerr << "MarkPad: color "<< name << " not found" << std::endl;
  return BlackColor;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

uint32_t AWL::getModifiers(const AWMouseEvent* event) {
  return (uint32_t)event.modifierFlags;
}

double AWL::getTimestamp(const AWMouseEvent* event) {
  return event.timestamp;
}

double AWL::getCurrentMouseX() {
  return [NSEvent mouseLocation].x;
}

double AWL::getCurrentMouseY() {
  return [NSEvent mouseLocation].y;
}

double AWL::getScreenWidth() {
  return CGDisplayPixelsWide(kCGDirectMainDisplay);
}

double AWL::getScreenHeight() {
  return CGDisplayPixelsHigh(kCGDirectMainDisplay);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

double AWL::getX(AWWidget* w) {return w.frame.origin.x;}
double AWL::getY(AWWidget* w) {return w.frame.origin.y;}

double AWL::getWidth(AWWidget* w) {return w.frame.size.width;}
double AWL::getWidth(AWBox* w) {return w.frame.size.width;}
double AWL::getWidth(AWDialog* w) {return w.frame.size.width;}

double AWL::getHeight(AWWidget* w) {return w.frame.size.height;}
double AWL::getHeight(AWBox* w) {return w.frame.size.height;}
double AWL::getHeight(AWDialog* w) {return w.frame.size.height;}

void AWL::move(AWWidget* w, double x, double y)  {
  [w setFrameOrigin: NSPoint{x, y}];
}

void AWL::move(AWDialog* w, double x, double y)  {
  [w setFrameTopLeftPoint: NSPoint{x, y}];
}

void AWL::show(AWWidget* w, bool state) {w.hidden = !state;}
void AWL::show(AWBox* w, bool state) {w.hidden = !state;}
void AWL::show(AWPanel* w, bool state) {w.hidden = !state;}

void AWL::show(AWDialog* win, bool state) {
  if (state) [win makeKeyAndOrderFront:del];
  else [win orderOut:del];
}

void AWL::showContent(NSTextField* w, bool state) {
  w.drawsBackground = state;
  w.bordered = state;
  w.bezeled = state;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void AWL::changeHeight(AWDialog* win, double delta) {
  NSRect rect = win.frame;
  CGFloat h = rect.size.height;
  rect.size.height += delta;
  rect.origin.y -= rect.size.height - h;
  [win setFrame:rect display:YES animate:YES];
}

bool AWL::showOpenDialog(std::string& filename) {
  NSOpenPanel* panel = [NSOpenPanel openPanel];
  [panel setCanChooseDirectories:YES];
  NSModalResponse result = [panel runModal];
  
  //if (result != NSFileHandlingPanelOKButton) return false;
  if (result != NSModalResponseOK) return false;
  else {
    NSURL* url = [[panel URLs] count] > 0 ? [[panel URLs] objectAtIndex:0] : nil;
    NSString* path = nil;
    if (!url || !(path = [url path])) return false;
    filename = [path UTF8String];
    return true;
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void AWL::enable(AWWidget* w, bool state) {w.enabled = state;}
void AWL::enable(AWMenuItem* w, bool state) {w.enabled = state;}

void AWL::highlight(AWWidget* w, bool state) {w.highlighted = state;}

void AWL::setFocus(AWWidget* w) {[w becomeFirstResponder];}
void AWL::setFocus(AWDialog* w) {  /* illegal thus nop */}

void AWL::setState(AWButton* w, bool state) {w.state = state;}
bool AWL::getState(AWButton* w) {return w.state;}

void AWL::setState(AWMenuItem* w, bool state) {w.state = state;}
bool AWL::getState(AWMenuItem* w) {return w.state;}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <> void AWL::setColor(AWTextArea* w, AWColor* c) {
  w.textColor = c;
}

template <> void AWL::setColor(AWTextField* w, AWColor* c) {
  w.textColor = c;
}

template <> void AWL::setColor(AWColorWidget* w, AWColor* c) {
  w.color = c;
}

template <> void AWL::setBgColor(AWTextArea* w, AWColor* c) {
  w.backgroundColor = c;
}

template <> void AWL::setBgColor(AWTextField* w, AWColor* c) {
  w.backgroundColor = c;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <> void AWL::setColor(AWButton* w, AWColor* c) {
  w.bezelColor = c;
}

template <> void AWL::setText(AWButton* w, const std::string& str) {
  w.title = toNativeString(str);
}

template <> void AWL::setText(AWMenuItem* w, const std::string& str) {
  w.title = toNativeString(str);
}

template <> void AWL::setText(AWTextArea* w, const std::string& str) {
  w.string = toNativeString(str);
}

template <> void AWL::setText(AWTextField* w, const std::string& str) {
  w.stringValue = toNativeString(str);
}

template <> void AWL::setText(AWTextField* w, double val) {
  w.doubleValue = val;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void AWL::selectText(AWTextField* w) {
  [w selectText:del];
}

std::string AWL::getText(AWTextField* w) {
  return [w.stringValue UTF8String];
}

float AWL::getFloat(AWTextField* w) {
  return w.floatValue;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void AWL::setValue(AWSpinBox* w, double val) {
  if (w) w.doubleValue = val;
}

double AWL::getValue(AWSpinBox* w) {
  return w ? w.doubleValue : 0.;
}

void AWL::setDecimals(AWSpinBox* w, int number) {
  //nop
}

void AWL::setRange(AWSpinBox* w, double min, double max) {
  if (w) {
    w.minValue = min;
    w.maxValue = min;
  }
}

void AWL::setStep(AWSpinBox* w, double step) {
  if (w) w.increment = step;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

int AWL::getCount(AWTabWidget* w) {
  return int(w.numberOfTabViewItems);
}

void AWL::select(AWTabWidget* w, int index) {
  [w selectTabViewItemAtIndex:index];
}

//int AWL::selectedIndex(AWTabWidget* w) {
//  return int(w.selectedSegment);
//}

int AWL::getCount(AWTabBar* w) {
  return int(w.segmentCount);
}

void AWL::select(AWTabBar* w, int index) {
  [w setSelected:true forSegment:index];
}

int AWL::selectedIndex(AWTabBar* w) {
  return int(w.selectedSegment);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

int AWL::getCount(AWCombo* w) {
  return int(w.numberOfItems);
}

void AWL::addItem(AWCombo* w, const std::string& title) {
  [w addItemWithTitle: toNativeString(title)];
}

void AWL::removeAllItems(AWCombo* w) {
  [w removeAllItems];
}

void AWL::select(AWCombo* w, int index) {
  [w selectItemAtIndex:index];
}

int AWL::selectedIndex(AWCombo* w) {
  return int(w.indexOfSelectedItem);
}

void AWL::enable(AWCombo* w, int index, bool state) {
  auto* item = [w itemAtIndex:index];
  if (item) enable(item, state);
}

void AWL::addItem(AWMenu* w, AWCallback callback, const std::string& title) {
  //[w addItemWithTitle: toNativeString(title) action:del->commandSelector keyEquivalent:@""];
  [w addItemWithTitle: toNativeString(title) action:callback keyEquivalent:@""];
}

void AWL::removeAllItems(AWMenu* w) {
  [w removeAllItems];
}

int AWL::itemIndex(AWMenu* w, AWMenuItem* item) {
  return (int)[w indexOfItem:item];
}
