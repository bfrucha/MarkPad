//
//  AWL: Abstract Widget Layer
//  (c) Eric Lecolinet - http://www.telecom-paristech.fr/~elc
//  Copyright (c) 2018. All rights reserved.
//

#ifndef MARKPAD_AWIDGETS
#define MARKPAD_AWIDGETS

#include <string>
#include <functional>

#if WITH_QT
#include "QtDelegate.h"
using AWDelegate = QtDelegate;
using AWString = QString;
using AWColor = QColor;
using AWFont = QFont;
using AWImage = QImage;
using AWCursor = QCursor;
using AWKeyEvent = QKeyEvent;
using AWMouseEvent = QMouseEvent;
using AWWidget = QWidget;
using AWBox = class AWLDontExist1;  // doesn't exist
//using AWPanel = QWidget;
using AWPanel = class AWLDontExist3;// doesn't exist
using AWDialog = QDialog;
using AWLabel = QLabel;
using AWButton = QAbstractButton;
using AWTextField = QLineEdit;
using AWTextArea = QTextEdit;
using AWTabWidget = QTabWidget;
using AWTabBar = class AWLDontExist2;  // doesn't exist
using AWSpinBox = QDoubleSpinBox;
using AWMenu = QMenu;
using AWMenuItem = QAction;
using AWCombo = QComboBox;
using AWGroupBox = QGroupBox;
using AWColorWidget = QFrame;
using AWActionField = QLineEdit;

#else

#include "AppDelegate.h"
using AWCallback = SEL;
using AWDelegate = AppDelegate;
using AWString = NSString;
using AWColor = NSColor;
using AWFont = NSFont;
using AWImage = NSImage;
using AWCursor = NSCursor;
using AWKeyEvent = NSEvent;
using AWMouseEvent = NSEvent;
using AWWidget = NSControl;
using AWView = NSView;       // Cocoa only
using AWBox = NSBox;
using AWPanel = NSStackView;
using AWDialog = NSWindow;
using AWButton = NSButton;
using AWTextField = NSTextField;
using AWTextArea = NSText;
using AWTabWidget = NSTabView;;
using AWTabBar = NSSegmentedControl;
using AWSpinBox = NSStepper;   // Note: not exactly a SPinBox!
using AWMenu = NSMenu;
using AWMenuItem = NSMenuItem;
using AWCombo = NSPopUpButton;
using AWGroupBox = NSBox;
using AWColorWidget = NSColorWell;

NSString* toNativeString(const char*);
NSString* toNativeString(const std::string&);

#endif

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

class AWL {
protected:
  /// delegate: contains pointers to the application widgets.
  static AWDelegate* del;
  
public:
  static AWColor *ClearColor, *WhiteColor, *BlackColor, *GrayColor,
  *RedColor, *GreenColor, *BlueColor, *YellowColor, *OrangeColor,
  *MagentaColor, *CyanColor;
  
  static AWCursor *arrowCursor, *crosshairCursor,
  *closedHandCursor, *openHandCursor, *pointingHandCursor,
  *resizeVerCursor, *resizeHorCursor,
  *resizeDiagPlus45Cursor, *resizeDiagMinus45Cursor;
  
  static const uint32_t BackspaceKeycode, SpaceKeycode, ReturnKeycode, EscapeKeycode;
  
  static const uint32_t ShiftModifier, ControlModifier, AltModifier, CommandModifier, FunctionModifier;
  
  // - - - -
  
  static void init(AWDelegate*);

  static AWDelegate* getDelegate() {return del;}

  static uint32_t getModifiers(const AWMouseEvent*);
  static double getTimestamp(const AWMouseEvent*);
  
  static double getCurrentMouseX();
  static double getCurrentMouseY();
  
  static double getScreenWidth();
  static double getScreenHeight();
  
  /// opens an alert dialog.
  static int alert(const std::string& msg, const std::string& info = "",
                   const std::string& buttons = "");

  /// opens an alert dialog at this location.
  /// xpos, etc. not taken into account if negative
  static int alert(const std::string& msg, const std::string& info,
                   const std::string& buttons,
                   float xpos, float ypos);
  
  // - - - -

  /// name = fontface, fontsize.
  static AWFont* newFont(const std::string& qualified_name);
  static AWFont* newFont(const std::string& face, float size);

  /// name = r,g,b,a.
  static AWColor* newColor(const std::string& qualified_name);
  static AWColor* newColor(float red, float green, float blue, float alpha = 1.f);

  static AWCursor* newCursor(const std::string& filename, float xhotspot, float yhotspot);

  static AWImage* newImage(const std::string& filename);
  
  // - - - -

  static double getX(AWWidget *);
  static double getY(AWWidget *);
  
  static double getWidth(AWWidget *);
  static double getWidth(AWBox *);
  static double getWidth(AWDialog *);

  static double getHeight(AWWidget *);
  static double getHeight(AWBox *);
  static double getHeight(AWDialog *);
  
  static void show(AWWidget *, bool state);
  static void show(AWBox *, bool state);
  static void show(AWPanel *, bool state);
  static void show(AWDialog*, bool state);
  
  // makes the content visible or invisible
  void showContent(AWTextField* w, bool state);
  
  static void move(AWWidget*, double x, double y);
  static void move(AWDialog*, double x, double y);
  static void changeHeight(AWDialog*, double delta);
  static bool showOpenDialog(std::string& filename);

  static void highlight(AWWidget*, bool state);
  static void setFocus(AWWidget*);
  static void setFocus(AWDialog*);  // does nothing with Cococa

  static void enable(AWWidget*, bool state);
  static void enable(AWMenuItem*, bool state);
  
  static void setState(AWButton*, bool state);
  static void setState(AWMenuItem*, bool state);
  static bool getState(AWButton*);
  static bool getState(AWMenuItem*);
  
  template <class T> static void setColor(T*, AWColor*);
  template <class T> static void setBgColor(T*, AWColor*);
  
  template <class T> static void setText(T*, const std::string& str);
  template <class T> static void setText(T*, double val);
  
  static void selectText(AWTextField*);
  static std::string getText(AWTextField*);
  static double getDouble(AWTextField*);
  static float  getFloat(AWTextField*);

  // a SpinBox or a Stepper
  static void setValue(AWSpinBox*, double);
  static double getValue(AWSpinBox*);
  static void setDecimals(AWSpinBox*, int number);
  static void setRange(AWSpinBox*, double min, double max);
  static void setStep(AWSpinBox*, double step);

  static int  getCount(AWTabWidget*);
  static void select(AWTabWidget*, int index);
  static int  selectedIndex(AWTabWidget*);
  
  static int  getCount(AWTabBar*);
  static void select(AWTabBar*, int index);
  static int  selectedIndex(AWTabBar*);
  
  static void addItem(AWMenu*, AWCallback, const std::string& title);
  static void removeAllItems(AWMenu*);
  static int  itemIndex(AWMenu*, AWMenuItem*);

  static int  getCount(AWCombo*);
  static void addItem(AWCombo*, const std::string& title);
  static void removeAllItems(AWCombo*);
  static void enable(AWCombo* w, int index, bool state);
  static void select(AWCombo*, int index);
  static int  selectedIndex(AWCombo*);
};

#endif
