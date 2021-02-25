//
//  GUIString.mm
//  MarkPad Project
//
//  (c) Eric Lecolinet - http://www.telecom-paristech.fr/~elc
//  (c) Bruno Fruchard - http://brunofruchard.com/
//  Copyright (c) 2017/2020 All rights reserved.
//

#include <iostream>
#include "Conf.h"
#include "GUI.h"
#include "Strings.h"
using namespace std;

Strings Strings::instance;
const Strings& Strings::k = Strings::instance;

static string printkey(const string& s) {
  return s.empty() ? "*Not Set*" : s;
}

void Strings::updateHotKey(const std::string& hotkey, const std::string& editkey)
{
  instance.helpitems[1].content =
  "To Show All Menus: Press ["+printkey(hotkey)+"] hotkey\n"
  "(see 'Settings' to change hotkey)\n"
  "\n"
  "To Show a Menu: As before, or touch Menu label and wait for delay\n"
  "\n"
  "To Execute Command: Move finger from Menu label to Command\n"
  "(Menu don't appear if done quickly)";
  
  instance.helpitems[2].content =
  "To Start Edit mode: press ["+printkey(editkey)+"] when a Menu is shown,\n"
  "or click on the MarkPad icon in Mac menubar\n"
  "or \n"
  "\n"
  "To Open Menu (in Edit mode): click on [blue arrow] in Menu label,\n"
  "or press [Return] key. [Escape] key closes Menu.\n"
  "\n"
  "To Create Shortcut: Open desired menu, then double-click\n"
  " on a dotted area, or click [New Shortcut] button\n"
  "\n"
  "To Create Menu: Open main view, then create Shortcut on the \n"
  "border of the touchpad)";
}

Strings::Strings() :
helpitems{
  {
    "About",
    "MarkPad version " + Conf::version() + "\n\n"
    "(c) 2017-2020 Eric Lecolinet & Bruno Fruchard - Télécom Paris\n\n"
    "Web Page and Manual: "
    "http://brunofruchard.com/markpad.html"
  },
  {
    "Showing Menus",  //  filled by updateHotKey()
    ""
  },
  {
    "Editing Menus",
    ""
  },
  {
    "Selecting, Resizing, Copying, Deleting, etc.",
    "To Select Shortcut: click on its area\n"
    "\n"
    "To Select several Shortcuts: SHIFT-click on their areas\n"
    "\n"
    "To Move Shortcut Title: ALT-press on title\n"
    "\n"
    "To Resize Shortcut: press its borders\n"
    "\n"
    "To Align Shortcuts: click editor [More] button, or RIGHT-click\n"
    "to open context menu\n"
    "\n"
    "To Delete, Cut, Copy, Paste: press CTRL-D, CTRL-X, CTRL-C, CTRL-V,\n"
    "or click editor [Edit] button, or open context menu"
  },
  {
    "Editor Buttons",
    " - [Desktop] button: Makes desktop in/visible\n\n"
    " - [Fingers] button: Shows/Hides Finger positions\n\n"
    " - [home] button (or SPACE key): Moves Editor to area\n\n"
    " - [zoom] button (or RETURN key): Opens/closes menu\n\n"
    " - [wheel] button: Opens Settings dialog\n\n"
    " - [i] button: Opens Help dialog"
  },
  {
    "Modes and Non-Border Menus",
    " If [Touchs Open Menu] is disabled (click on [More] first),\n"
    " the menu is not shown when touching its area\n"
    "\n"
    " Menu labels must be located on borders in the main view\n"
    " except if [Touch From Border] is disabled\n"
    "\n"
    "In both cases these menus can be shown only by pressing hotkeys"
  },
  {
    "Settings",
    "Click on editor [wheel] button to open Settings panel\n"
    "\n"
    "Finger size: Minimum/maximum size of user's fingertips\n"
    "Action Feedback: Visual feedback when an action if performed\n"
    "\n"
    "Hotkeys Open Menus: the hotkeys for showing and editing menus\n"
    "Note: Hotkeys require Accessibility access (used only for this purpose)\n"
    "\n"
    "Touchs Open Menus: Touch gestures open Menus if they:\n"
    "   * Last a certain Delay\n"
    "   * Start from Active Borders (see below)\n"
    "\n"
    "Edit: Allows changing Active Borders and Guides\n"
    "   * Active Borders should be small to avoid involuntary activations\n"
    "   * Guides specify where dotted areas are located\n"
    "\n"
    "Theme: To change the theme of the application:\n"
    "\n"
    "Log: If enabled, actions are logged in ~/Library/MarkPad/data_log\n"
    "Note: Uses a lot of space!"
  },
},
ShowBrowser{"Show Browser"},
NewMenu{"New Menu"},
NewShortcut{"New Shortcut"},
OpenMenu{"Open Menu"},
CloseMenu{"Close Menu"},
ClickDesired{"Click on desired shortcut"},
ShowMenus{"Show Menu"},
EditMenus{"Edit Menu"},
AccessibilityMsg{"Accessibility Required"},
AccessibilityInfo{
  "MarkPad requires Accessibility to detect Hotkeys. "
  "MarkPad does not process key events for any other purpose.\n\n"
  "Enable Accessibility?"},
AccessibilityBtns{"Enable, See Later, Permanently Disable"},
EnableAcessibilityMsg{"Enable Acessibility THEN close this window"},
EnableAcessibilityInfo{
  " - 1) Leave this dialog box open until the final step\n"
  " - 2) Click on \"Open System Preferences\" in the other dialog\n"
  " - 3) In the newly opened window, select \"Privacy\" then \"Accessibility\"\n"
  " - 4) Add \"MarkPad\" to authorized applications\n"
  " - 5) Close both windows\n"},
EnableAcessibilityBtns{"Done"},
ClickToCreateShortcut1{"Click on desired area to create shortcut (Esc to cancel)"},
ClickToCreateShortcut2{"Click to create shortcut at this position (Esc to cancel)"},
EmptyMenu{"Empty Menu (Add Shortcuts)"}
{
  updateHotKey("","");
}



