//
//  OverlayImpl.h
//  MarkPad Project
//
//  (c) Eric Lecolinet - http://www.telecom-paristech.fr/~elc
//  (c) Bruno Fruchard - http://brunofruchard.com/
//  Copyright (c) 2017/18. All rights reserved.
//

#ifndef OVERLAY_IMPL_H
#define OVERLAY_IMPL_H
#include "Overlay.h"
#include "MTouch.h"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#ifdef WITH_QT

#include <QtWidgets/QWidget>
#include <QTimer>

class TextAttr {
public:
  TextAttr(AWFont* font, AWColor* color, AWColor* bgcolor);
  AWFont *font{};
  AWColor *color{}, *bgcolor{};
};

#else   // COCOA

#include <Cocoa/Cocoa.h>
@class CocoaOverlayWindow;
@class CocoaOverlayView;

class TextAttr {
public:
  TextAttr(AWFont* font, AWColor* fg, AWColor* bg);
  AWFont *font{};
  NSDictionary *dict{};
};

#endif

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

class OverlayImpl
#ifdef WITH_QT
     : public QWidget
#endif
{
public:
  explicit OverlayImpl();
  void setContextMenu(AWMenu*);
  
  void show(bool editmode, AWColor* bgcolor);
  void fullscreen(bool state);
  void needsDisplay();
  /*
  void setFeedbackMode(bool state);
  void feedbackEnd();
   */
  void showFeedback();
  void endFeedback();
  
  MTPoint getMousePos(AWMouseEvent*);
  double getWidth();
  double getHeight();
  double xscale{0.}, yscale{0.};
  void rescale();
  void setCursor(AWCursor*);

  void layoutTitle(MTRect& area, const std::string& text, TextAttr*);
  void drawTitle(MTRect& area, const std::string& text, TextAttr*);
  void drawInfo(const std::string& text, TextAttr*, double heightRatio);
  void drawLine(const MTPoint& from, const MTPoint& to, AWColor* color);
  void drawBox(const MTRect&, class BoxTheme&);
  void drawSelectionBox(const MTRect&);
  void drawHandle(float x, float y);
  void drawImage(AWImage*, float x, float y, float w, float h);
  void drawIcon(AWImage*, float x, float y, float size); // size = w = h (squared)
  void drawGrid();
  void drawFingers();
  
#ifdef WITH_QT
  void paintEvent(QPaintEvent*);
  void mousePressEvent(QMouseEvent*);
  void mouseMoveEvent(QMouseEvent*);
  void mouseReleaseEvent(QMouseEvent*);
  void keyPressEvent(QKeyEvent*);
  QPainter* painter{nullptr};
  QPen dashedPen;
  QTimer timer;
#else
  void hide();
  CocoaOverlayWindow* owin{nil};
  CocoaOverlayView* oview{nil};
  NSBezierPath *grid{nil};
#endif
};

#endif
