//
//  MarkPad: main class
//  MarkPad Project
//
//  (c) Eric Lecolinet - http://www.telecom-paristech.fr/~elc
//  (c) Bruno Fruchard - http://brunofruchard.com/
//  Copyright (c) 2017/2020. All rights reserved.//
//
// NOTES:
// - Accessibility must be enabled to detect Function hotkeys
// - Services::getFinderFile() and getBrowserUrl(s) called at the beginning
//   to allow accesing the Desktop and the Web browsEr
// In Info.plist
//  - UIElement=YES => item in the mac menubar but no dock icon
//  - Privacy - AppleEvents Sending Usage Description = YES to allow sending key events
//  !!! Service::sendChars() crashes with last OS version !!!

#ifndef MarkPad_MarkPad
#define MarkPad_MarkPad

#include <list>
#include <vector>
#include "GUI.h"
#include "Shortcut.h"
using namespace std;
class Pad;

/** Main class of the MarkPad application.
 * there is only one MarkPad instance which can control several trackpads (@see class Pad).
 */
class MarkPad {
public:
  /// The MarkPad singleton.
  static MarkPad instance;
  
  static void info(const std::string& msg);
  static void warning(const std::string& msg);
  static void log(const std::string& msg);
  
  // - - - init, run/pause, restart

  // CAUTION: must be called once widgets are created
  void init();
  
  /// run MarkPad is arg is true, pause otherwise, exits if no pad can be found.
  void run(bool state);
  
  bool isRunning() const {return isRunning_;}
  
  /// restarts the MarkPad app.
  static void restartApp();

  /// sets the hotkeys.
  void setHotkeys(uint32_t showMask, uint32_t editMask);

  // - - - pads
  
  /// adds a trackpad.
  bool addPad(MTDevice*, float width, float height);
  
  /// returns the Pad that is currently used.
  Pad* currentPad() const {return curPad_;}
  
  /// return pads.
  const std::vector<Pad*>& getPads() const {return pads_;}
  
  // - - - states
  
  enum EditMode {EditNone, EditShortcuts, EditBorders, EditGuides};

  /// starts/stops editing the menus.
  void edit(EditMode);
  void postEdit(ShortcutMenu*);

  EditMode editMode() const {return editMode_;}

  /// true if the menu is being edited.
  bool isEditing() const {return editMode_ != EditNone;}
  
  /// can the fingers be shown?.
  bool canShowFingers() const {return editMode_ == EditNone || wantFingers_;}
  
  /// the gui wants fingers.
  void showFingers(bool state) {wantFingers_ = state;}
  
  // - - - menus

  /// true if a menu is shown on the overlay.
  bool isOverlayShown() const {return isMenuShown_;}
  
  /// shows a menu on the overlay.
  void showOverlay(bool state);
  
  /// the hotkey shows the menu.
  void showOverlayOnHotkey(bool state);
  
  /// returns the main menu.
  ShortcutMenu* mainMenu() const;
  
  /// returns the menu that is currently shown.
  ShortcutMenu* currentMenu() const {return curMenus_[0];}
  
  /// returns the opener of the current menu.
  Shortcut* currentMenuOpener() const {return curMenus_[0] ? curMenus_[0]->opener() : nullptr;}
  
  /// does this shortcut open the current menu?.
  bool opensCurrentMenu(Shortcut&);
  
  /// selects the current menu and the current shortcut.
  /// Selects the main menu if _menu_ il null.
  /// Caution: sets current shorcut to null!
  void setCurrentMenu(ShortcutMenu* menu = nullptr);
  
  /// creates a new menu.
  ShortcutMenu* createMenu(Shortcut& opener);
  
  /// searches menu.
  ShortcutMenu* findMenu(const std::string& menu_name);
  
  // - - - current selection

  /// returns the current shortcut.
  /// returns null if several shortcuts are selected (ie multiSelection is not empty).
  Shortcut* currentShortcut() const {return curShortcut_;}

  /// returns true if at least one shortcut is selected.
  /// returns true if currentShortcut is not null or multiSelection is not empty.
  bool isShortcutSelected() const;
  
  /// selects this shortcut.
  void selectShortcut(Shortcut*);
  
  void multiSelects(Shortcut*, bool add);
  
  /// multiselection buffer.
  /// note that it does not contain curshortcut_ if curshortcut_ is not null
  void addToMultiSelection(Shortcut*);
  void removeFromMultiSelection(Shortcut*);
  void clearMultiSelection();
  bool isInMultiSelection(float x, float y, float xtol, float ytol);

  /// NOTE: this buffer is empty if only one shortcut (ie curentShortcut()) is selected.
  const std::list<Shortcut*>& multiSelection() const {return selectionBuffer_;}

  // - - - manage shortcuts

  void createShortcut(const MTPoint& pos, bool twoSteps);
  void cancelCreateShortcut();
  
  /// copy, cut and paste shortcuts.
  void copyShortcuts();
  void cutShortcuts();
  Shortcut* pasteShortcuts(ShortcutMenu& menu);
  bool pasteBufferEmpty() const {return pasteBuffer_.empty();}
  
  /// deletes/undelete shortcuts.
  void deleteShortcuts();
  bool deleteShortcutImpl(Shortcut&);
  Shortcut* undeleteShortcuts();
  bool deleteBufferEmpty() const {return deleteBuffer_.empty();}
  
  // - - - guide and layout
  
  void adjustBoxToGuide(Shortcut&);
  void adjustBoxes(BoxPart::Value);
  void alignBoxes(BoxPart::Value);
  void alignTitles(BoxPart::Value);
  
  ShortcutMenu* guide() {return guide_;}
  ShortcutMenu* findGuide(ShortcutMenu& menu);
  Shortcut* findGuideArea(ShortcutMenu& menu, const MTPoint& pos);
  
  bool isCreatingShortcut() const {return isCreatingShortcut_;}
  bool isCreatingFromGuide() const {return isCreatingFromGuide_;}
  ShortcutMenu* creatingActualMenu() const {return creatingActualMenu_;}

  /// called when the trackpad is touched (static is required by the Touch API).
  static int touchCallback(MTDevice*, const MTTouch* touches, int touchCount,
                           double timestamp, int frame);
  
  /// called when a key is pressed in the overlay.
  void keyCallback(const std::string& key);

  void setDesktopMode(bool state) {desktopMode_ = state;}
  bool desktopMode() const {return desktopMode_;}

private:
  friend class Conf;
  friend class CConf;
  friend class Pad;
  
  MarkPad() = default;                    // private constr.
  MarkPad(const MarkPad&) = delete;       // can't be copied.
  MarkPad& operator=(MarkPad&) = delete;  // can't be copied.

  // called when show and edit hotkeys are pressed or released.
  void hotkeyCallback(uint32_t hotkey);
  
  void createShortcutFirstStep(ShortcutMenu&, ShortcutMenu* guide);
  Shortcut* createShortcutSecondStep(const Shortcut* guideArea);
  Shortcut* createShortcutFromGuide(ShortcutMenu&, const Shortcut& guideArea);
  Shortcut* createShortcutAtPos(ShortcutMenu&, const MTPoint& pos);

  bool isRunning_{}, isMenuShown_{},
  hotkeyWantsMenu_{}, mpWantMenu_{}, wantFingers_{}, desktopMode_{};
  uint32_t showHotkeyMask_{0}, editHotkeyMask_{0};
  
  std::vector<Pad*> pads_;
  Pad* curPad_{nullptr};                    // current pad
  
  std::vector<ShortcutMenu*> curMenus_{2};  // current menus
  ShortcutMenu* guide_{nullptr};

  Shortcut *curShortcut_{nullptr};          // current shortcut.
  std::list<Shortcut*> deleteBuffer_, pasteBuffer_, selectionBuffer_;
  int pasteCount_{0};
  
  EditMode editMode_{EditNone};
  bool guideChanged_{false}, isCreatingShortcut_{false}, isCreatingFromGuide_{false};
  ShortcutMenu* creatingActualMenu_{nullptr};
};

#endif
