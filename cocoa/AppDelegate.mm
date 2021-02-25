//
//  AppDelegate.mm
//  MarkPad Project
//
//  (c) Eric Lecolinet - http://www.telecom-paristech.fr/~elc
//  (c) Bruno Fruchard - http://brunofruchard.com/
//  Copyright (c) 2017/2020. All rights reserved.


#import <iostream>
#import <vector>
#import "AppDelegate.h"
#import "MarkPad.h"
#import "Overlay.h"
#import "Actions.h"
#import "GUI.h"
#import "AWL.h"
#import "Editor.h"
#import "Settings.h"
using namespace std;

static GUI& gui = GUI::instance;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

@interface HelpItemView : NSButton {
@public
  NSTextField* text;
  const HelpItem* help;
}
- (void)clickCallback:(id)sender;
@end

@implementation HelpItemView

- (void)clickCallback:(id)sender {
  if (text.hidden) {
    text.stringValue = toNativeString(help->content);
    text.hidden = false;
  }
  else {
    text.hidden = true;
  }
}
@end

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

class Widgets {
  AppDelegate* del;
  NSFont* font;

public:
  Widgets(AppDelegate* del, NSFont* font) : del(del), font(font) {}
  
  NSButton* newButton(AWPanel* pane, const std::string& name, SEL sel) {
    NSButton* btn = [NSButton buttonWithTitle:toNativeString(name)
                                       target:del
                                       action:sel];
    btn.controlSize = NSControlSizeSmall;
    btn.font = font;
    [pane addView:btn inGravity:NSStackViewGravityCenter];
    return btn;
  }
  
  NSButton* newActionButton(AWPanel* pane, Action& a, SEL sel) {
    NSButton* btn = [NSButton buttonWithImage: AWL::newImage(Conf::imageDir() + a.icon)
                                       target: del
                                       action: sel];
    btn.alternateImage = AWL::newImage(Conf::imageDir() + a.selectedIcon);
    btn.buttonType = NSButtonTypeToggle;
    btn.bezelStyle = NSBezelStyleRegularSquare;
    [pane addView:btn inGravity:NSStackViewGravityCenter];
    /*
     NSTrackingArea* trackingArea =
     [[NSTrackingArea alloc]
     initWithRect: [btn bounds]
     options: NSTrackingMouseEnteredAndExited | NSTrackingActiveAlways
     owner: del
     userInfo: @{@"sender" : a}];
     [btn addTrackingArea:trackingArea];
     */
    return btn;
  }

  AWCombo* newCombo(AWPanel* pane, SEL sel) {
    AWCombo* menu = [[NSPopUpButton alloc] initWithFrame:NSMakeRect(0, 0, 10, 10)
                                               pullsDown:false];
    menu.controlSize = NSControlSizeSmall;
    menu.font = font;
    menu.action = sel;
    [pane addView:menu inGravity:NSStackViewGravityCenter];
    return menu;
  }
  
  NSTextField* newTextField(AWPanel* pane) {
    NSTextField* field = [NSTextField new];
    field.delegate = del;
    field.font = font;
    [pane addView:field inGravity:NSStackViewGravityCenter];
    return field;
  }

  NSStackView* newModifierPane(AWPanel* pane, ModifierButtons& mod, SEL sel) {
    NSStackView* modpane = [NSStackView new];
    modpane.distribution = NSStackViewDistributionFillEqually;
    modpane.spacing = 1.;
    
    mod.cmdBtn = newButton(modpane, "⌘", sel);
    mod.cmdBtn.bezelStyle = NSBezelStyleRoundRect;
    mod.cmdBtn.buttonType = NSButtonTypePushOnPushOff;
    
    mod.shiftBtn = newButton(modpane, "⇧", sel);
    mod.shiftBtn.bezelStyle = NSBezelStyleRoundRect;
    mod.shiftBtn.buttonType = NSButtonTypePushOnPushOff;
    
    mod.ctrlBtn = newButton(modpane, "⌃", sel);
    mod.ctrlBtn.bezelStyle = NSBezelStyleRoundRect;
    mod.ctrlBtn.buttonType = NSButtonTypePushOnPushOff;
    
    mod.altBtn = newButton(modpane, "⌥", sel);
    mod.altBtn.bezelStyle = NSBezelStyleRoundRect;
    mod.altBtn.buttonType = NSButtonTypePushOnPushOff;
    
    [pane addView:modpane inGravity:NSStackViewGravityCenter];
    return modpane;
  }
  
  HelpItemView* newHelpItem(AWPanel* pane, const HelpItem& helpitem)
  {
    HelpItemView* it = [[HelpItemView alloc] init];
    it->help = &helpitem;
    it.title = toNativeString(helpitem.title);
    [it setTarget:it];
    [it setAction:@selector(clickCallback:)];
    it.controlSize = NSControlSizeSmall;
    it.font = font;
    [pane addView:it inGravity:NSStackViewGravityCenter];
      
    it->text = [NSTextField new];
    it->text.editable = false;
    [pane addView:it->text inGravity:NSStackViewGravityCenter];
    it->text.hidden = true;
    return it;
  }
};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

@implementation AppDelegate

- (id)init {
  // NSString * appVersion
  // = [[NSBundle mainBundle] objectForInfoDictionaryKey:@"CFBundleShortVersionString"];

  NSLog(@"MarkPad Version %s", Conf::version().c_str());
  if ((self = [super init])) {
    menuBarItem = nil;
    AWL::init(self);   // initialize delegate in the GUI
  }
  return self;
}

- (void)awakeFromNib {
  // init. menu and items in the global menubar
  menuBarItem = [[NSStatusBar systemStatusBar] statusItemWithLength:NSVariableStatusItemLength];
  menuBarItem.button.image = AWL::newImage(Conf::imageDir()+"menubarIcon.icns");
  menuBarItem.button.toolTip = @"MarkPad";
  [menuBarItem setHighlightMode:YES];
  [menuBarItem setMenu:menuBarMenu];
  [menuBarItem.button.image setTemplate:YES];
  
  // these windows are on top of all application windows
  helpWin.level = NSScreenSaverWindowLevel+1;
  settingsWin.level = NSScreenSaverWindowLevel+1;
  editWin.level = NSScreenSaverWindowLevel+1;

  // show windows on all virtual desktops
  helpWin.collectionBehavior = NSWindowCollectionBehaviorCanJoinAllSpaces;
  settingsWin.collectionBehavior = NSWindowCollectionBehaviorCanJoinAllSpaces;
  editWin.collectionBehavior = NSWindowCollectionBehaviorCanJoinAllSpaces;

  // makes it possible to enable menu items manually
  [editMenu setAutoenablesItems:NO];

  Widgets w(self, showDesktopBtn.font);
  commandMenu = [[NSMenu alloc] initWithTitle:@"Commands"];
  [commandMenu setAutoenablesItems:NO];
  commandMenu.font = showDesktopBtn.font;
  commandCB = @selector(commandMenuCallback:);

  helperMenu = [[NSMenu alloc] initWithTitle:@"Helpers"];
  [helperMenu setAutoenablesItems:NO];
  helperMenu.font = showDesktopBtn.font;
  helperCB = @selector(helperMenuCallback:);
  
  std::vector<NSButton*> buttons;
  // create the icon buttons in the action panel
  for (auto& a : Actions::instance.getActions()) {
    NSButton* b = w.newActionButton(actionPane, a, @selector(actionCallback:));
    buttons.push_back(b);
    gui.edit.addActionButton(b);
  }

  double size = (actionPane.frame.size.width - 10.) / Actions::instance.getActions().size();
  //cerr << "actionPane width " << actionPane.frame.size.width<< " / size " << size<< endl;
  
  // noter que les tailles des widgets sont n'importe quoi !
  int k = 0;
  for (auto& a : Actions::instance.getActions()) {
    [buttons[k] addTrackingRect: NSRect{0., 0., size, size}
                          owner: self
                       userData: &a
                   assumeInside: NO];
    k++;
  }
  
  // create the items in the action pane (order matters: they are added in this order!)
  //browserDoneBtn = w.newButton(commandPane, "Return to MarkPad", @selector(quitBrowserCallback:));
  helperBtn = w.newButton(commandPane, "Helper", @selector(helperBtnCallback:));
  showMenuBtn = w.newButton(commandPane, "Show Menu", @selector(showMenuCallback:));

  modifierButtons = new ModifierButtons;
  modifierPane = w.newModifierPane(commandPane, *modifierButtons, @selector(modifierCallback:));
  commandField = w.newTextField(commandPane);
  
  helpPane = [NSStackView new];
  helpWin.contentView = helpPane;
  helpPane.orientation = NSUserInterfaceLayoutOrientationVertical;
  helpPane.alignment = NSLayoutAttributeLeading;
  
  for (auto& it : Strings::k.helpitems) w.newHelpItem(helpPane, it);
  
  NSText* filler = [NSText new];
  filler.drawsBackground = false;
  filler.editable = false;
  [helpPane addView:filler inGravity:NSStackViewGravityCenter];

  // - - - starts MarkPad (must be done once widgets are created)
  
  MarkPad::instance.init();
  //Overlay::instance.fullscreen(false);  // for testing
}

// called when an icon button is entered
- (void)mouseEntered:(NSEvent*)e {
  /*
  cerr << "mouseEntered win: " << e.window.windowNumber
  << " track: " << (unsigned long)e.trackingArea
  << " data: " << (unsigned long)e.userData <<endl;
   */
  gui.edit.showActionTip((Action*)e.userData, true);
}

// called when an icon button is exited
- (void)mouseExited:(NSEvent*)e {
  /*
  cerr << "mouseExited win: " << e.window.windowNumber
  << " track: " << (unsigned long)e.trackingArea
  << " data: " << (unsigned long)e.userData <<endl;
   */
  gui.edit.showActionTip((Action*)e.userData, false);
}

- (void)actionCallback:(id)sender {
  [sender setState:false];
  auto state = gui.edit.actionCB(sender);
  if (state) [sender setState:true];
  if (state == Editor::ActionWithMenu) {
    NSPoint loc{0, [sender frame].size.height};
    [commandMenu popUpMenuPositioningItem:nil atLocation:loc inView:sender];
  }
}

- (void)commandMenuCallback:(id)sender {
  gui.edit.commandCB(sender);
}

- (void)helperMenuCallback:(id)sender {
  gui.edit.helperCB(sender);
}

- (void)helperBtnCallback:(id)sender {
  NSPoint loc{0, [sender frame].size.height};
  [helperMenu popUpMenuPositioningItem:nil atLocation:loc inView:sender];
}

- (void)showMenuCallback:(id)sender {
  gui.openCloseMenu();
}

- (IBAction)openCloseMenuCallback:(id)sender {
  gui.openCloseMenu();
}

- (void)quitBrowserCallback:(id)sender {
  Actions::instance.quitBrowser();
}

- (void)modifierCallback:(id)sender {
  if (Shortcut* s = MarkPad::instance.currentShortcut()) {
    uint8_t states = modifierButtons->getStates();
    s->setModifiers(states);
  }
}

- (IBAction)debugCallback:(id)sender {
  Overlay::instance.fullscreen(![sender state]);
}

- (IBAction)bgImageCallback:(id)sender {
  bool state = [sender state];
  Conf::change(&Conf::showBgImage, state);
  Overlay::instance.update();
}

// when the program terminates
- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication*)sender {
  Conf::saveIfNeeded();
  return NSTerminateNow;
}

/*
 //pose pbm!!! when the overlay window is opened/closed
 - (void)windowDidChangeOcclusionState:(NSNotification *)notification {
 if (!(editWin.occlusionState & NSWindowOcclusionStateVisible)) {
 mp.editMenu(false);
 }}
 */

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Editor commands

- (IBAction)showMarkpadDirectoryCallback:(id)sender {
  GUI::closeSettings();
  GUI::showDesktop(true);
  Services::openFile(Services::getUserConfDir());
}

- (IBAction)showManualCallback:(id)sender {
  gui.openManual();
}

- (IBAction)helpCallback:(id)sender {
  gui.showHelp();
}

- (IBAction)quitCallback:(id)sender {
  Conf::saveIfNeeded();
  [NSApp terminate:self];
}

- (IBAction)restartCallback:(id)sender {
  MarkPad::instance.restartApp();
}

- (IBAction)runPauseCallback:(id)sender {
  MarkPad::instance.run(!MarkPad::instance.isRunning());
}

// called via delegate of editWin and settingsWin when closing them
- (BOOL)windowShouldClose:(id)sender {
  if (sender == editWin) [self editorDoneCallback:sender];
  else if (sender == settingsWin) [self settingsCloseCallback:sender];
  return true;
}

- (IBAction)showDesktopCallback:(id)sender {
  GUI::showDesktop([sender state]);
}

- (IBAction)showFingersCallback:(id)sender {
  Conf::change(&Conf::showFingers, (bool)showFingersBtn.state);
  MarkPad::instance.showFingers([sender state]);
  Overlay::instance.update();
}

- (IBAction)showGridCallback:(id)sender {
  Conf::change(&Conf::showGrid, (bool)showGridBtn.state);
  Overlay::instance.update();
}

- (IBAction)editorOpenCallback:(id)sender {
  gui.edit.openEditor();
}

- (IBAction)editorDoneCallback:(id)sender {
  if (MarkPad::instance.desktopMode()) GUI::showDesktop(false);
  else gui.edit.closeEditor();
  [[NSColorPanel sharedColorPanel] close];
  [[NSFontPanel sharedFontPanel] close];
}

- (IBAction)gotoCallback:(id)sender {
  gui.edit.attractEditor();
}

- (IBAction)showMoreCallback:(id)sender {
  gui.edit.showMore([sender state]);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// shortcuts

static MTPoint mousePos;

/*
- (IBAction)newCallback:(id)sender {
  // 2step mode if clicking on newBtn, otherwise use mouse pos if using context menu
  MarkPad::instance.createShortcut(mousePos, (sender == newBtn));
}*/

- (IBAction)setCurrentURLAction:(id)sender {
  if (auto* s = MarkPad::instance.currentShortcut()) {
    if (Actions::instance.setShortcutAction(*s, "openurl",""))
      Actions::setCurrentFileOrURL(s, true);
  }
}

- (IBAction)setCurrentFileAction:(id)sender {
  if (auto* s = MarkPad::instance.currentShortcut()) {
    if (Actions::instance.setShortcutAction(*s, "openfile",""))
      Actions::setCurrentFileOrURL(s, false);
  }
}

- (IBAction)setCurrentAppAction:(id)sender {
  if (auto* s = MarkPad::instance.currentShortcut()) {
    if (Actions::instance.setShortcutAction(*s, "openhideapp",""))
      Actions::setCurrentFileOrURL(s, false);
  }
}

// called by all TextFields
- (void)controlTextDidChange:(NSNotification*)notif {
  if (notif.object == shortcutName)
    gui.edit.nameFieldCB(AWL::getText(shortcutName));
  else if (notif.object == commandField)
    gui.edit.actionFieldCB(AWL::getText(commandField));
  else
    gui.edit.sizeFieldCB(notif.object, AWL::getFloat(notif.object));
}

- (IBAction)stepperCallback:(id)sender {
  gui.edit.sizeStepperCB(sender);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// cut & paste

- (IBAction)copyCallback:(id)sender {
  gui.edit.copyShortcuts();
}

- (IBAction)cutCallback:(id)sender {
  gui.edit.cutShortcuts();
}

- (IBAction)pasteCallback:(id)sender {
  gui.edit.pasteShortcuts();
}

- (IBAction)deleteCallback:(id)sender {
  gui.edit.deleteShortcuts();
}

- (IBAction)undeleteCallback:(id)sender {
  gui.edit.undeleteShortcuts();
}

- (IBAction)modesCallback:(id)sender {
  if (auto* s = MarkPad::instance.currentShortcut()) {
    // forme some reason les items ne changent pas d'état tout seuls !
    if (sender == touchOpenMenu) {
      touchOpenMenu.state = s->touchOpenMenu_ = !s->touchOpenMenu_;
      Conf::mustSave();
    }
    else if (sender == touchFromBorder) {
      touchFromBorder.state = s->touchFromBorder_ = !s->touchFromBorder_;
      if (!s->touchFromBorder_) {
        touchOpenMenu.state = s->touchOpenMenu_ = false;
        Conf::mustSave();
      }
    }
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Alignment

- (void)menuWillOpen:(NSMenu*)menu {
  if (menu == contextMenu) {
    // mousePos is used for creating shorcuts by newCallback()
    mousePos = gui.currentMousePos();
    gui.edit.hideEditor(true);
  }
}

- (void)menuDidClose:(NSMenu*)menu {
  if (menu == contextMenu) gui.edit.hideEditor(false);
}

- (IBAction)alignTitlesComboCallback:(id)sender {
  BoxPart::Value part = BoxPart::Outside;
  switch (AWL::selectedIndex((AWCombo*)sender)) {
    case 2:
      part = BoxPart::Left;
      break;
    case 3:
      part = BoxPart::HCenter;
      break;
    case 4:
      part = BoxPart::Right;
      break;
    case 6:
      part = BoxPart::Top;
      break;
    case 7:
      part = BoxPart::VCenter;
      break;
    case 8:
      part = BoxPart::Bottom;
      break;
  }
   if (part != BoxPart::Outside) gui.edit.alignTitles(part);
}

- (IBAction)alignBoxesComboCallback:(id)sender {
  BoxPart::Value part = BoxPart::Outside;
  switch (AWL::selectedIndex((AWCombo*)sender)) {
    case 2:
      part = BoxPart::Left;
      break;
    case 3:
      part = BoxPart::HCenter;
      break;
    case 4:
      part = BoxPart::Right;
      break;
    case 6:
      part = BoxPart::Top;
      break;
    case 7:
      part = BoxPart::VCenter;
      break;
    case 8:
      part = BoxPart::Bottom;
      break;
  }
  if (part != BoxPart::Outside) gui.edit.alignBoxes(part);
}

- (IBAction)adjustBoxesComboCallback:(id)sender {
  BoxPart::Value part = BoxPart::Outside;
  BoxPart::Value part2 = BoxPart::Outside;
  
  switch (AWL::selectedIndex((AWCombo*)sender)) {
    case 2:
      part = BoxPart::Left;
      break;
    case 3:
      part = BoxPart::Left;
      part2 = BoxPart::Right;
      break;
    case 4:
      part = BoxPart::Right;
      break;
    case 6:
      part = BoxPart::Top;
      break;
    case 7:
      part = BoxPart::Top;
      part2 = BoxPart::Bottom;
      break;
    case 8:
      part = BoxPart::Bottom;
      break;
  }
  if (part != BoxPart::Outside) gui.edit.adjustBoxes(part, part2);
}

- (IBAction)alignTitlesCallback:(id)sender {
  BoxPart::Value part = BoxPart::Outside;
  if (sender == alignTitleBoxCenterIt || sender == centerTitleInBoxBtn) {
    part = BoxPart::Title;
  }
  else if (sender == alignTitleLeftIt) {
    part = BoxPart::Left;
  }
  else if (sender == alignTitleRightIt) {
    part = BoxPart::Right;
  }
  else if (sender == alignTitleBottomIt) {
    part = BoxPart::Bottom;
  }
  else if (sender == alignTitleTopIt) {
    part = BoxPart::Top;
  }
  else if (sender == alignTitleHCenterIt) {
    part = BoxPart::HCenter;
  }
  else if (sender == alignTitleVCenterIt) {
    part = BoxPart::VCenter;
  }
  if (part != BoxPart::Outside) gui.edit.alignTitles(part);
}

- (IBAction)alignBoxesCallback:(id)sender {
  BoxPart::Value part = BoxPart::Outside;
  if (sender == alignBoxLeftIt) {
    part = BoxPart::Left;
  }
  else if (sender == alignBoxBottomIt) {
    part = BoxPart::Bottom;
  }
  else if (sender == alignBoxRightIt) {
    part = BoxPart::Right;
  }
  else if (sender == alignBoxTopIt) {
    part = BoxPart::Top;
  }
  else if (sender == alignBoxHCenterIt) {
    part = BoxPart::HCenter;
  }
  else if (sender == alignBoxVCenterIt) {
    part = BoxPart::VCenter;
  }
  if (part != BoxPart::Outside) gui.edit.alignBoxes(part);
}

- (IBAction)adjustBoxesCallback:(id)sender {
  BoxPart::Value part = BoxPart::Outside;
  BoxPart::Value part2 = BoxPart::Outside;
  
  if (sender == adjustBoxToGuidesIt || sender == snapToGuidesBtn) {
    gui.edit.snapToGuides();
    return;
  }
  else if (sender == adjustBoxLeftIt) {
    part = BoxPart::Left;
  }
  else if (sender == adjustBoxRightIt) {
    part = BoxPart::Right;
  }
  else if (sender == adjustBoxBottomIt) {
    part = BoxPart::Bottom;
  }
  else if (sender == adjustBoxTopIt) {
    part = BoxPart::Top;
  }
  else if (sender == adjustBoxVertEdgesIt) {
    part = BoxPart::Left;
    part2 = BoxPart::Right;
  }
  else if (sender == adjustBoxHorEdgesIt) {
    part = BoxPart::Bottom;
    part2 = BoxPart::Top;
  }
  if (part != BoxPart::Outside) gui.edit.adjustBoxes(part, part2);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Settings Window

- (IBAction)settingsOpenCallback:(id)sender {
  gui.settings.openSettingsWindowCB();
}

- (IBAction)settingsCloseCallback:(id)sender {
  gui.settings.closeSettingsWindowCB();
  [[NSColorPanel sharedColorPanel] close];
  [[NSFontPanel sharedFontPanel] close];
}

- (IBAction)feedbackCallback:(id)sender {
  gui.settings.feedbackCB(true);
}

- (IBAction)accessibilityCallback:(id)sender {
  gui.settings.accessibilityCB();
}

- (IBAction)settingsCallback:(id)sender {
  gui.settings.settingsCB();
}

- (IBAction)feedbackColorCallback:(id)sender {
  [feedbackColorWell activate:YES]; // receive color change events
  NSColorPanel *panel = [NSColorPanel sharedColorPanel];
  [panel setTarget:self];
  [panel setContinuous:NO];
  [panel setShowsAlpha:YES];
  [panel setLevel:NSScreenSaverWindowLevel];
  [panel orderFront:settingsWin];
  [panel setColor: Overlay::instance.getFeedbackColor()];
  [panel setAction:@selector(changeFeedbackColor)]; // to update color in Conf
  [panel makeKeyAndOrderFront:sender];
}

- (void)changeFeedbackColor {
  NSColor* c = [[NSColorPanel sharedColorPanel] color];
  Overlay::instance.setFeedbackColor(c);
  Conf::changeFeedback(&Conf::Feedback::color,
                       to_string([c redComponent])+","+to_string([c greenComponent])
                       +","+ to_string([c blueComponent])+","+to_string([c alphaComponent]));
}

- (IBAction)feedbackFontCallback:(id)sender {
  NSFontManager *fontManager = [NSFontManager sharedFontManager];
  [fontManager setAction:@selector(changeFeedbackFont)]; // to update font in Conf
  [fontManager setTarget:self];
  
  NSFontPanel* panel = [fontManager fontPanel:YES];
  [panel setPanelFont: Overlay::instance.getFeedbackFont() isMultiple:NO];
  [panel setLevel:NSScreenSaverWindowLevel];
  [panel makeKeyAndOrderFront:sender];
}

- (void)changeFeedbackFont {
  NSFont* oldFont = Overlay::instance.getFeedbackFont();
  NSFont* newFont = [[NSFontPanel sharedFontPanel] panelConvertFont:oldFont];
  string fontname = [[newFont fontName] UTF8String];
  string fontsize = to_string(int([newFont pointSize]));
  
  AWL::setText(feedbackFontNameField, fontname);
  AWL::setText(feedbackFontSizeField, fontsize);
  Overlay::instance.setFeedbackFont(newFont);
  Conf::changeFeedback(&Conf::Feedback::font, fontname+", "+fontsize);
}

@end
