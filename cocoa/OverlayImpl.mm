//
//  OverlayImpl.mm (Cocoa impl.)
//  MarkPad Project
//
//  (c) Eric Lecolinet - http://www.telecom-paristech.fr/~elc
//  (c) Bruno Fruchard - http://brunofruchard.com/
//  Copyright (c) 2017/2020. All rights reserved.
//

#include <iostream>
#include <Cocoa/Cocoa.h>
#include <Carbon/Carbon.h>
#include "Conf.h"
#include "MarkPad.h"
#include "Pad.h"
#include "Overlay.h"
#include "OverlayImpl.h"
#include "Theme.h"

static Overlay& overlay = Overlay::instance;
static const float HANDLE_SIZE = 6.f;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

/// window that displays the shortcuts.
@interface CocoaOverlayWindow : NSWindow {
@public
  NSTimer *timer;
}
- (id)init;
- (void)hide;
@end

/// view of the window that displays the shortcuts.
@interface CocoaOverlayView : NSView {}
- (id)init;
- (void)resolutionChanged:(NSNotification*)notif;
@end

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

@implementation CocoaOverlayWindow

- (BOOL)acceptsFirstResponder {return YES;}
- (BOOL)resignFirstResponder  {return YES;}
- (BOOL)becomeFirstResponder  {return YES;}
- (BOOL)canBecomeKeyWindow    {return YES;}

- (id)init {
  NSRect winframe = [[NSScreen mainScreen] frame];
  self = [super initWithContentRect: winframe
                          styleMask: NSWindowStyleMaskBorderless
                            backing: NSBackingStoreBuffered
                              defer: NO];  // create the window now
  if (self) {
    timer = nil;
    self.collectionBehavior = NSWindowCollectionBehaviorCanJoinAllSpaces;
    // now set in show(), depends whether editing or not
    //self.level = NSStatusWindowLevel;
    self.hasShadow = NO;
    self.opaque = NO;
    self.ignoresMouseEvents = NO;
    self.acceptsMouseMovedEvents = YES;
    self.contentView.needsDisplay = YES;
  }
  return self;
}

- (void)hide {
  [self orderOut: self];
}

- (void)endFeedback{
  Overlay::instance.impl->endFeedback();
}

// catches key events
- (void)keyDown:(NSEvent*)event {
  //cerr <<"keyDown "<<event.keyCode<<" " << long(event.modifierFlags)<< endl;

  if (MarkPad::instance.isEditing()) {
    if (event.type != NSEventTypeKeyDown) return;
    NSString* s = [event charactersIgnoringModifiers];
    if (!s) return;
    unichar c = 0;
    if (s.length > 0) c = [s characterAtIndex:0];
    GUI::instance.keyOnOverlayCB(event.keyCode, c, (uint32_t)event.modifierFlags);
  }
  else if (event.keyCode == AWL::EscapeKeycode) {
    //cerr <<"ESCAPE "<<event.keyCode<< endl;
    [self hide];
  }
  /* ne recoit pas les shift, etc.
  else if (event.modifierFlags & AWL::ShiftModifier) {
    cerr <<"SHIFT "<<event.modifierFlags<< endl;
  }
   */
  //else {
  //  mp.keyCallback([s UTF8String]);
  //}
}

@end

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

@implementation CocoaOverlayView

- (id)init {
  NSRect frame = [[NSScreen screens][0] frame];
  self = [super initWithFrame:frame];
  
  if (self) {
    [[NSNotificationCenter defaultCenter]
     addObserver:self
     selector:@selector(resolutionChanged:)
     name:NSApplicationDidChangeScreenParametersNotification
     object:nil];
    
    NSTrackingArea*
    trackingArea = [[NSTrackingArea alloc]
                    initWithRect:frame
                    options: (NSTrackingMouseEnteredAndExited
                              | NSTrackingMouseMoved
                              | NSTrackingActiveWhenFirstResponder)
                    owner:self userInfo:nil];
    [self addTrackingArea:trackingArea];
  }
  return self;
}

// catches mouse events when the window isn't the key window
- (BOOL)acceptsFirstMouse:(NSEvent*)event {
  return YES;
}

- (void)mouseDown:(NSEvent*)e {
  overlay.mouseDown(e);
}

- (void)mouseUp:(NSEvent*)e {
  overlay.mouseUp(e);
}

- (void)mouseDragged:(NSEvent*)e {
  overlay.mouseDragged(e);
}

- (void)mouseMoved:(NSEvent*)e {
  overlay.mouseMoved(e);
}

- (void)mouseEntered:(NSEvent*)e {}

- (void)mouseExited:(NSEvent*)e {}

- (void)drawRect:(NSRect)rect {
  overlay.impl->xscale = self.frame.size.width;
  overlay.impl->yscale = self.frame.size.height;
  overlay.draw();
}

// need to resize overlay if resolution changed
- (void)resolutionChanged:(NSNotification *)notification {
  NSRect r = [[NSScreen screens][0] frame];
  [self rescale:r];
  // need to update parent window
  [[self window] setFrame:r display:YES];
}

- (void)rescale:(const NSRect&)newframe {
  overlay.mustUpdateGeometry();
  [self setFrame: newframe];
  overlay.impl->xscale = newframe.size.width;
  overlay.impl->yscale = newframe.size.height;
  
  NSBezierPath* grid = [NSBezierPath bezierPath];
  overlay.impl->grid = grid;
  int gridcount = overlay.theme->gridCount;
  for (int k = 1; k < gridcount; ++k) {
    [grid moveToPoint:NSPoint{k * newframe.size.width / gridcount, 0}];
    [grid lineToPoint:NSPoint{k * newframe.size.width / gridcount, newframe.size.height}];
  }
  for (int k = 1; k < gridcount; ++k) {
    [grid moveToPoint:NSPoint{0, k * newframe.size.height / gridcount}];
    [grid lineToPoint:NSPoint{newframe.size.width, k * newframe.size.height / gridcount}];
  }
  CGFloat pattern[] = {6., 6.};
  [grid setLineDash:pattern count:2 phase:0.0];
}

@end

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

TextAttr::TextAttr(AWFont* font, AWColor* color, AWColor* bgcolor) :
font(font),
dict([NSDictionary
      dictionaryWithObjects:@[font, color, bgcolor]
      forKeys:@[NSFontAttributeName,
                NSForegroundColorAttributeName,
                NSBackgroundColorAttributeName]]) {
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

OverlayImpl::OverlayImpl() {
  owin = [[CocoaOverlayWindow alloc] init];    // creates the overlay window
  oview = [[CocoaOverlayView alloc] init];
  [owin.contentView addSubview:oview];
}

void OverlayImpl::show(bool editmode, NSColor* bg) {
  if (editmode) {
    owin.opaque = true;
    // window below Alerts: needed otherwise the editor will block
    // if Mojave opens a security alert
    // note: must hide menubar beacuse still visible when using this level
    owin.level = NSMainMenuWindowLevel;
  }
  else {
    owin.opaque = false;
    // window on the top of (almost) everything, including Alerts, which is safe
    // because the action is executed *after* closing the overlay
    owin.level = NSStatusWindowLevel;
  }

  owin.backgroundColor = bg;
  [owin.contentView display];
  [owin makeKeyAndOrderFront: owin];
}

void OverlayImpl::hide() {
  //cerr << "hide " << endl;
  //[NSMenu setMenuBarVisible:true];
  [owin orderOut: owin];
}

void OverlayImpl::fullscreen(bool state) {
  NSRect r = [[NSScreen screens][0] frame];
  if (state) {
    owin.hasShadow = NO;
    // cf aussi NSWindowStyleMaskFullScreen et toggleFullScreen
    owin.styleMask = NSWindowStyleMaskBorderless;
    [oview rescale:r];
    [owin setFrame:r display:YES];
  }
  else {
    owin.hasShadow = YES;
    owin.styleMask = NSWindowStyleMaskTitled | NSWindowStyleMaskResizable;
    r.origin.x = 0;
    r.origin.y = 0;
    r.size.width = r.size.width/2.;
    r.size.height = r.size.height/2.;
    [oview rescale:r];
    [owin setFrame:r display:YES];
  }
}

void OverlayImpl::showFeedback() {
  //cerr << "\nOverlayImpl::showFeedback "<<endl;
  owin.ignoresMouseEvents = YES;
  owin.acceptsMouseMovedEvents = NO;
  if (owin->timer) [owin->timer invalidate];
  show(false, AWL::ClearColor);  // not opaque
  needsDisplay();
  owin->timer =
  [NSTimer scheduledTimerWithTimeInterval:Conf::k.feedback.delay
                                   target:owin
                                 selector:@selector(endFeedback)
                                 userInfo:nil
                                  repeats:NO];
}

void OverlayImpl::endFeedback() {
  //cerr << "OverlayImpl::endFeedback "<<endl;
  [owin hide];
  owin->timer = nil;
  owin.ignoresMouseEvents = NO;
  owin.acceptsMouseMovedEvents = YES;
  overlay.feedbackMode = false;
}

void OverlayImpl::needsDisplay() {
  [oview setNeedsDisplay:YES];
}

void OverlayImpl::setContextMenu(NSMenu* contextMenu) {
  oview.menu = contextMenu;
}

double OverlayImpl::getWidth() {
  return oview.frame.size.width;
}

double OverlayImpl::getHeight() {
  return oview.frame.size.height;
}

MTPoint OverlayImpl::getMousePos(NSEvent* event) {
  NSPoint pos = event.locationInWindow;
  MTPoint mouse {
    float(pos.x / oview.frame.size.width),
    float(pos.y / oview.frame.size.height)};
  return mouse;
}

void OverlayImpl::rescale() {
  [oview rescale:oview.frame];
}

void OverlayImpl::setCursor(AWCursor* c) {
  [c set];
}

// - - - - - - -

void OverlayImpl::drawLine(const MTPoint& from, const MTPoint& to, AWColor* color) {
  [color set];
  NSBezierPath* arrow = [NSBezierPath bezierPath];
  [arrow moveToPoint: NSPoint{from.x * xscale, from.y * yscale}];
  [arrow lineToPoint: NSPoint{to.x * xscale, to.y * yscale}];
  [arrow setLineWidth: overlay.theme->linkThickness];
  [arrow stroke];
}

void OverlayImpl::drawImage(NSImage* image, float x, float y, float w, float h) {
  NSRect imarect {
    {x * xscale, y * yscale},
    {w * xscale, h * yscale}
  };
  [image drawInRect:imarect];
}

void OverlayImpl::drawIcon(NSImage* image, float x, float y, float s) {
  NSRect imarect {
    {x * xscale, y * yscale},
    {s * yscale, s * yscale}   // yscale,yscale => squared
  };
  [image drawInRect:imarect];
}

void OverlayImpl::drawHandle(float x, float y) {
  NSRect box{
    {x * xscale - HANDLE_SIZE/2.f, y * yscale - HANDLE_SIZE/2.f},
    {HANDLE_SIZE, HANDLE_SIZE}   // yscale,yscale => squared
  };
  [overlay.theme->handleBg set];
  NSRectFillUsingOperation(box, NSCompositingOperationSourceOver);
  [overlay.theme->handleColor set];
  NSBezierPath* path = [NSBezierPath bezierPathWithRect:box];
  [path stroke];
}

void OverlayImpl::drawSelectionBox(const MTRect& rect) {
  NSRect box {
    {rect.x * xscale, rect.y * yscale},
    {rect.width * xscale, rect.height * yscale}
  };
  [overlay.theme->selectionBg set];
  NSRectFillUsingOperation(box, NSCompositingOperationSourceOver);
  [overlay.theme->selectionColor set];
  NSBezierPath* path = [NSBezierPath bezierPathWithRect:box];
  [path stroke];
}

void OverlayImpl::drawBox(const MTRect& rect, BoxTheme& attr) {
  NSRect box {
    {rect.x * xscale, rect.y * yscale},
    {rect.width * xscale, rect.height * yscale}
  };
  // avoid drawing illdefined areas
  if (box.size.width <= 0 || box.size.height <= 0) return;
  
  if ((attr.drawn & Theme::DrawFilled) && attr.boxBg) {
    [attr.boxBg set];
    NSRectFillUsingOperation(box, NSCompositingOperationSourceOver);
  }
  
  if ((attr.drawn & (Theme::DrawSolid | Theme::DrawDashed)) && attr.boxColor) {
    [attr.boxColor set];
    NSBezierPath* path = [NSBezierPath bezierPathWithRect:box];
    [path setLineWidth: overlay.theme->boxThickness];
    if (attr.drawn & Theme::DrawDashed) {
      CGFloat pattern[] = {4., 4.};
      [path setLineDash:pattern count:2 phase:0.];
    }
    [path stroke];
  }
}

void OverlayImpl::layoutTitle(MTRect& textarea, const std::string& text, TextAttr* attr) {
  NSSize textsize = [toNativeString(text) sizeWithAttributes:attr->dict];
  textarea.width = float(textsize.width / xscale);
  textarea.height = float(textsize.height / yscale);
}

void OverlayImpl::drawTitle(MTRect& textarea, const std::string& text, TextAttr* attr) {
  NSString* s = toNativeString(text);
  NSSize textsize = [s sizeWithAttributes:attr->dict];
  NSPoint texpos = {textarea.x * xscale, textarea.y * yscale};
  [s drawAtPoint:texpos withAttributes: attr->dict];
  textarea.width = float(textsize.width / xscale);
  textarea.height = float(textsize.height / yscale);
}

void OverlayImpl::drawInfo(const std::string& text, TextAttr* attr, double heightRatio) {
  NSString* s = toNativeString(text);
  NSSize textsize = [s sizeWithAttributes: attr->dict];
  NSPoint texpos {getWidth()/2. - textsize.width/2., getHeight()*heightRatio};
  [s drawAtPoint: texpos withAttributes: attr->dict];
}

void OverlayImpl::drawFingers() {
  Pad* pad = overlay.pad;
  if (!pad) return;
  NSSize viewSize = oview.frame.size;
  [overlay.theme->fingerColor set];

  const MTTouch** end = pad->touches() + pad->touchCount();
  for (const MTTouch** t = pad->touches(); t < end; ++t) {
    double majorAxis = (*t)->majorAxis * viewSize.width / pad->padWidth / 2;
    double minorAxis = (*t)->minorAxis * viewSize.height / pad->padHeight / 2;
    double x = (*t)->norm.pos.x * viewSize.width - minorAxis / 2.;
    double y = (*t)->norm.pos.y * viewSize.height - majorAxis / 2.;
    /* deja teste dans Pad.mm
     trop petit (on est en train de relacher) => risque de planter
     if (minorAxis < 0.01 || majorAxis < 0.01) continue;
     */
    NSGraphicsContext* context = [NSGraphicsContext currentContext];
    [context saveGraphicsState];
    
    //CGContextRef c = (CGContextRef) [context graphicsPort];
    CGContextRef c = context.CGContext;
    CGContextTranslateCTM(c, x+minorAxis/2., y+majorAxis/2.);
    CGContextRotateCTM(c, (*t)->angle+M_PI/2);
    CGContextTranslateCTM(c, -(x+minorAxis/2.), -(y+majorAxis/2.));
    
    NSRect r = NSMakeRect(x, y, minorAxis, majorAxis);
    NSBezierPath* path = [NSBezierPath bezierPathWithOvalInRect:r];
    [path setLineWidth: overlay.theme->fingerThickness];
    [path stroke];
    [context restoreGraphicsState];
  }
}

void OverlayImpl::drawGrid() {
  [overlay.theme->gridColor set];
  [grid stroke];
}

