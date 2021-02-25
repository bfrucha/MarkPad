//
//  Overlay.h
//  MarkPad Project
//
//  (c) Eric Lecolinet - http://www.telecom-paristech.fr/~elc
//  (c) Bruno Fruchard - http://brunofruchard.com/
//  Copyright (c) 2017/18. All rights reserved.
//

#ifndef MARKPAD_OVERLAY
#define MARKPAD_OVERLAY

#include <string>
#include "AWL.h"
#include "MTouch.h"
#include "GUI.h"

class Shortcut;

class Overlay {
public:
  static Overlay instance;
  
  void init(AWMenu* contextMenu);
  void setTheme(const std::string& themename);
  void initGeometry(class Pad*);
  void update(bool updateShortcuts = false);
  void show(GUI::OverlayMode);
  void fullscreen(bool state);

  void showFeedback(const std::string& text);
  AWColor* getFeedbackColor();
  AWFont*  getFeedbackFont();
  void setFeedbackColor(AWColor*);
  void setFeedbackFont(AWFont*);
  
  bool isMouseDown() const {return mouseIsDown;}
  int32_t getModifiers() const {return modifierMask;}

  // - - -  Impl. - - -

  // called in OverlayImpl:
  void draw();
  void mustUpdateGeometry();
  void mouseDown(AWMouseEvent*);
  void mouseUp(AWMouseEvent*);
  void mouseDragged(AWMouseEvent*);
  void mouseMoved(AWMouseEvent*);
  void layoutShortcut(Shortcut&);
  void updateShortcuts();

  class OverlayImpl* impl{nullptr};
  class Theme* theme{nullptr};
  Pad* pad{nullptr};
  bool feedbackMode{false};

private:
  friend class GUI;
  friend class OverlayImpl;

  Overlay();
  void getTitle(const Shortcut&, std::string& title);
  void drawShortcut(Shortcut*, class BoxTheme*, bool opened = false);
  void drawLink(const Shortcut* from, const Shortcut* to, AWColor* color);
  void drawMenu(const class ShortcutMenu&, bool editing, bool drawlinks, bool drawopener);
  void drawHandles(const Shortcut&);
  void setCursorOnPart(BoxPart::Value);
  void dragHandle(const MTPoint& mouse);
  void dragBorderHandle(const MTPoint& mouse);

  Shortcut *pressedShortcut;
  int32_t modifierMask;
  bool isEditing, mouseIsDown, selectionMode, updateGeometry, showHandles;
  BoxPart::Value pressedPart, lastEnteredPart;
  MTPoint mouseLast, mouseInit;
  double lastMouseUpTime;
  MTRect selectionBox;
  class BoxTheme *normal, *selected;
  
  struct {
    std::string text;
    AWColor* color;
    AWFont* font;
    class TextAttr *textAttr;
    bool updateAttr;
  } feedback;
};

#endif
