//
//  AppDelegate.h
//  MarkPad Project
//
//  (c) Eric Lecolinet - http://www.telecom-paristech.fr/~elc
//  (c) Bruno Fruchard - http://brunofruchard.com/
//  Copyright (c) 2017/2020. All rights reserved.
//

#import <Cocoa/Cocoa.h>
class Shortcut;

/// Markpad application delegate.
@interface AppDelegate
: NSObject<NSApplicationDelegate, NSWindowDelegate, NSMenuDelegate, NSTextFieldDelegate, NSComboBoxDelegate>
{
@public
  // menubar
  IBOutlet NSMenu *menuBarMenu;
  IBOutlet NSMenuItem *showMenusItem, *runPauseItem, *editMenusItem;
  NSStatusItem *menuBarItem;
  
  // edit
  IBOutlet NSWindow *editWin;
  IBOutlet NSButton *showDesktopBtn, *showFingersBtn, *showGridBtn, *doneBtn,
  *newBtn, *gotoBtn, *menuBtn, *showMoreBtn, *showMoreBtn2,
  *touchOpenMenu, *touchFromBorder;
  IBOutlet NSMenu *editMenu;
  IBOutlet NSMenuItem *deleteItem, *undeleteItem, *copyItem, *cutItem, *pasteItem;
  IBOutlet NSTextField *shortcutName;
  IBOutlet NSMenu* contextMenu;   //contextual menu
  
  // actions
  IBOutlet NSStackView *actionPane, *commandPane;
  IBOutlet NSTextField *actionTitle, *actionCommand;
  NSButton *helperBtn, *showMenuBtn;
  NSStackView *modifierPane;
  NSTextField *commandField;
  NSMenu *commandMenu, *helperMenu;
  SEL commandCB, helperCB;
  struct ModifierButtons *modifierButtons;
  
  // more
  IBOutlet NSBox *morePane;
  
  // position
  IBOutlet NSBox *positionBox;
  IBOutlet NSStepper *areaX, *areaY, *areaW, *areaH, *nameX, *nameY;
  IBOutlet NSTextField *areaXField, *areaYField, *areaWField, *areaHField, *nameXField, *nameYField;
  
  // align
  IBOutlet NSButton *snapToGuidesBtn, *centerTitleInBoxBtn;
  IBOutlet NSMenuItem *adjustBoxToGuidesIt, *adjustBoxLeftIt, *adjustBoxRightIt,
  *adjustBoxBottomIt, *adjustBoxTopIt, *adjustBoxVertEdgesIt, *adjustBoxHorEdgesIt;
  IBOutlet NSMenuItem *alignBoxLeftIt, *alignBoxRightIt, *alignBoxBottomIt,
  *alignBoxTopIt, *alignBoxHCenterIt, *alignBoxVCenterIt;
  IBOutlet NSMenuItem *alignTitleLeftIt, *alignTitleRightIt, *alignTitleBottomIt,
  *alignTitleTopIt, *alignTitleHCenterIt, *alignTitleVCenterIt, *alignTitleBoxCenterIt;

  // help
  IBOutlet NSWindow *helpWin;
  NSStackView *helpPane;

  // settings window
  IBOutlet NSWindow *settingsWin;
  IBOutlet NSButton *fnBtn, *shiftBtn, *ctrlBtn, *altBtn, *cmdBtn,
  *fnBtn2, *shiftBtn2, *ctrlBtn2, *altBtn2, *cmdBtn2,
  *accessibilityBtn, *feedbackBtn, *feedbackColorBtn, *feedbackFontBtn,
  *darkThemeBtn, *lightThemeBtn, *transpThemeBtn,
  *editShortcutsBtn, *editBordersBtn, *editGuidesBtn, *logDataBtn,
  *debugBtn, *showBgImageBtn;
  IBOutlet NSTextField *minTouchSizeField, *maxTouchSizeField, *menuDelayField,
  *minMovementField,
  *feedbackDelayField, *feedbackFontNameField, *feedbackFontSizeField;
  IBOutlet NSColorWell *feedbackColorWell;
}
@end
