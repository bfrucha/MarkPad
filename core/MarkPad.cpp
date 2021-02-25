//
//  MarkPad: main class
//  MarkPad Project
//
//  (c) Eric Lecolinet - http://www.telecom-paristech.fr/~elc
//  (c) Bruno Fruchard - http://brunofruchard.com/
//  Copyright (c) 2017/18. All rights reserved.
//

#include <unistd.h>
#include <iostream>
#include <chrono>
#include "ccuty/ccstring.hpp"
#include "Conf.h"
#include "MarkPad.h"
#include "Pad.h"
#include "Actions.h"
#include "Services.h"
using namespace std;
using namespace ccuty;

MarkPad MarkPad::instance;  // MarkPad singleton.

void MarkPad::info(const std::string& msg) {
  cout << msg << endl;
}

void MarkPad::warning(const std::string& msg) {
  cerr << "Warning: " << msg << endl;
}

void MarkPad::log(const std::string& msg) {
  Services::log(msg);
}

// - - - init & pads - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void MarkPad::init() {
  // note that orders matters!!
  
  // reads the configuration file
  // CConf, not conf, for compatibility with older versions
  // CConf is init in main() to create user files and init paths (incl. exec path)
  // Note that read() requires AWL to be init. because it may call alert()
  CConf::instance.read();

  // init services (start callback notifiers & accesibility).
  Services::init();
  
  // inits GUI (after reading the Conf file)
  GUI::instance.init();

  // the application will be restarted after a computer Wake
  // because the touch API may not work properly after some time
  Services::setWakeCallback(MarkPad::restartApp);
  
  // save before a computer sleep to avoid loosing config changes
  Services::setSleepCallback(Conf::saveIfNeeded);
  
  // called when show and edit hotkeys are pressed or released
  Services::setHotkeyCallback([](uint32_t hotkey) {
    MarkPad::instance.hotkeyCallback(hotkey);
  });

  // searches for trackpads; quits app if no trackpad could be found
  run(true);
}

void MarkPad::restartApp() {
  log("MarkPad::restartApp");
  Conf::saveIfNeeded();
  // start a new instance of MarkPad then close the current instance
  int status = ccuty::system("(sleep 5; "+Conf::execPath() + ")&");
  log("Status: "+to_string(int(status)));
  sleep(1);
  exit(0);
}

void MarkPad::run(bool state) {
  //log("run: state="+to_string(int(state)));
  
  if (state == isRunning_) return;
  isRunning_ = state;
  GUI::instance.updateRunState(state);
  
  if (!state) {
    MTReader::stop(*this);
    Actions::instance.run(false);  // stop media server
    std::cout << "MarkPad was paused\n" << std::endl;
  }
  else {
    //std::cout << "Starting MarkPad..." << std::endl;
    int count = MTReader::start(*this);
    if (count <= 0) {
      GUI::alert("MarkPad can't be started: touchpad not found", "Quit");
      exit(1);
    }
    
    // init geometry of pad menus
    Pad* pad = currentPad();
    if (pad) GUI::instance.initOverlayGeometry(pad);
    else warning("Could not init Pad geometry");
    
    // start Actions (connect media server)
    Actions::instance.run(true);
  }
}
  
bool MarkPad::addPad(MTDevice* device, float width, float height) {
  if (width <= 0 ||  height <= 0) return false;
  
  // search if there is a configuration that matches this trackpad
  PadConf* padconf = nullptr;
  /*
   for (auto p : Conf::instance.padConfs) {
   if (p->padWidth == width && p->padHeight == height) {padconf = p; break;}
   }
   */
  // creates a default padconf if no matching trackpad
  if (!padconf) padconf = new PadConf(width, height);
  
  //cout << "add pad" << endl;
  Pad* pad  = new Pad(device, *padconf);
  pads_.push_back(pad);
  if (!curPad_) curPad_ = pad;
  return true;
}

// callback for trackpad events.
int MarkPad::touchCallback(MTDevice* device, const MTTouch* touches, int touchCount,
                           double /*timestamp*/, int /*frame*/) {
#if TEST
  cout << "\nTouch Count: " << touchCount << endl;
  for (int i=0; i<touchCount; i++){
    const MTTouch& t = touches[i];
    cout << " @ frame="<< t.frame << " id=" << t.ident << " state=" << t.phase
    << "\n - x=" << t.norm.pos.x << " y=" << t.norm.pos.y
    << " / vx=" << t.norm.velocity.x << " vy=" << t.norm.velocity.y
    << "\n - size=" << t.size << " angle=" << t.angle
    << " / minor axis=" << t.minorAxis << " major axis=" << t.majorAxis << endl;
  }
#endif
  MarkPad& mp = MarkPad::instance;
  
  if (mp.pads_.size() == 1) {
    mp.curPad_ = mp.pads_[0];
    mp.curPad_->touchCallback(touches, touchCount);
  }
  else {
    Pad* pad{nullptr};
    for (const auto& p : mp.pads_) {
      if (p->device_ == device) {pad = p; break;}
    }
    if (pad) {
      mp.curPad_ = pad;
      pad->touchCallback(touches, touchCount);
    }
  }
  
  return 0;
}

// - - - keys and hotkeys - - - - - - - - - - - - - - - - - - - - - - - - - - -

void MarkPad::setHotkeys(uint32_t showMask, uint32_t editMask) {
  showHotkeyMask_ = showMask;
  editHotkeyMask_ = editMask;
}

void MarkPad::hotkeyCallback(uint32_t hotkey) {
  if (hotkey) {
    if (hotkey == showHotkeyMask_) {
      if (!isMenuShown_) showOverlayOnHotkey(true);
    }
    else if (isMenuShown_ && (hotkey & editHotkeyMask_)) {
      // - dont call edit() while editing, may block the app!
      // - need to save current menu because it is reset by edit()
      if (editMode_ == EditNone) {
        auto* menu = currentMenu();
        edit(MarkPad::EditShortcuts);
        setCurrentMenu(menu);
      }
    }
  }
  else if (isMenuShown_) showOverlayOnHotkey(false);
}

// - - - menus - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

ShortcutMenu* MarkPad::mainMenu() const {
  if ((isCreatingShortcut() || editMode_ == MarkPad::EditGuides) && guide_)
    return guide_;
  else if (editMode_ == MarkPad::EditBorders)
    return curPad_->activeBorders_;
  else if (curPad_)
    return curPad_->mainMenu_;
  else return nullptr;
}

void MarkPad::setCurrentMenu(ShortcutMenu* menu) {
  curMenus_[0] = menu ? menu : mainMenu();
  curMenus_[1] = nullptr;
  
  if (curShortcut_) {
    curShortcut_->setSelected(false);
    curShortcut_ = nullptr;
  }
  GUI::instance.updateShortcut(curShortcut_);
}

ShortcutMenu* MarkPad::createMenu(Shortcut& opener) {
  ShortcutMenu* menu = new ShortcutMenu();
  Conf::instance.mustSave();
  opener.setMenu(menu);
  return menu;
}

ShortcutMenu* MarkPad::findMenu(const string& menuname) {
  ShortcutMenu* mainmenu = mainMenu();
  if (!mainmenu || menuname.empty()) return nullptr;
  for (auto& s : mainmenu->shortcuts()) {
    if (s && s->submenu_ && s->name() == menuname) return s->submenu_;
  }
  return nullptr;
}

// - - - menus and overlays - - - - - - - - - - - - - - - - - - - - - - - - - - 

bool MarkPad::opensCurrentMenu(Shortcut& s) {
  return s.submenu_ && s.submenu_ == curMenus_[0];
}

void MarkPad::showOverlay(bool state) {
  mpWantMenu_ = state;
  if (hotkeyWantsMenu_ || isEditing()) return;
  isMenuShown_ = mpWantMenu_ || hotkeyWantsMenu_;
  auto mode = state ? GUI::OverlayMenus : GUI::OverlayHide;
  Services::postpone([mode]{GUI::instance.showOverlay(mode);});
}

void MarkPad::showOverlayOnHotkey(bool state) {
  if (hotkeyWantsMenu_ == state) return;
  hotkeyWantsMenu_ = state;
  if (mpWantMenu_ || isEditing()) return;
  
  setCurrentMenu();
  isMenuShown_ = mpWantMenu_ || hotkeyWantsMenu_;
  auto mode = state ? GUI::OverlayMenus : GUI::OverlayHide;
  Services::postpone([mode]{GUI::instance.showOverlay(mode);});
}

void MarkPad::edit(EditMode mode) {
  editMode_ = mode;
  mpWantMenu_ = hotkeyWantsMenu_ = false;
  if (!curPad_) return;
  curPad_->cancelTouchGesture(false);

  if (mode == EditNone) {
    isMenuShown_ = false;
  }
  else {
    isMenuShown_ = true;
    Services::enableDeviceForCursor(curPad_->device());
    Services::makeMarkPadFront();  // give focus to MarkPad to intercept events
    if (mode == EditGuides && guide_) guideChanged_ = true;
    else if (mode == EditBorders) curPad_->initActiveBorders();
  }
  //Services::postpone([state](){GUI::instance.editCB(mode != EditNone);});
  GUI::instance.editCB(mode != EditNone);
}

void MarkPad::postEdit(ShortcutMenu* menu) {
  // should be called from the main thread
  Services::postpone([menu](){
    MarkPad::instance.edit(MarkPad::EditShortcuts);
    if (menu) menu->openMenu(true);
  });
}

// - - - selection - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

bool MarkPad::isShortcutSelected() const {
  return curShortcut_!=nullptr || !selectionBuffer_.empty();
}

void MarkPad::selectShortcut(Shortcut* s) {
  if (curShortcut_) curShortcut_->setSelected(false);
  clearMultiSelection();

  if (isCreatingShortcut_) {   // creating shortcut interactively
    curShortcut_ = createShortcutSecondStep(s);
    if (curShortcut_) curShortcut_->setSelected(true);
    GUI::instance.updateShortcut(curShortcut_);
    return;
  }
  
  if (!s) {
    curShortcut_ = nullptr;
  }
  else {
    s->setSelected(true);
    curShortcut_ = s;
  }
  GUI::instance.updateShortcut(curShortcut_);
}

void MarkPad::multiSelects(Shortcut* s, bool add) {
  if (isCreatingShortcut_) {  // creating shortcut interactively
    selectShortcut(s);
    return;
  }
  
  if (s) {
    if (add) {
      // if curShortcut_ not null, add it to the multiselection buffer
      if (curShortcut_) {
        // ! selected must be false otherwise the shortcut wont be added !
        curShortcut_->setSelected(false);
        addToMultiSelection(curShortcut_);
      }
      addToMultiSelection(s);
    }
    else {
      if (curShortcut_) curShortcut_->setSelected(false);
      removeFromMultiSelection(s);
    }
  }
  
  // curShortcut_ must be null if several shortcuts selected!
  curShortcut_ = nullptr;
  GUI::instance.updateShortcut(curShortcut_);
}

void MarkPad::addToMultiSelection(Shortcut* s) {
  if (!s) return;
  
  // should be already in the list if selected
  if (s->isSelected()) return;
  
  if (opensCurrentMenu(*s)) {   // dont add current menu!!
    s->setSelected(false);
    return;
  }
  s->setSelected(true);
  if (find(selectionBuffer_.begin(), selectionBuffer_.end(), s) == selectionBuffer_.end()) {
    selectionBuffer_.push_back(s);
  }
}

void MarkPad::removeFromMultiSelection(Shortcut* s) {
  if (!s) return;
  auto it = find(selectionBuffer_.begin(), selectionBuffer_.end(), s);
  if (it != selectionBuffer_.end()) {
    s->setSelected(false);
    selectionBuffer_.erase(it);
  }
}

bool MarkPad::isInMultiSelection(float x, float y, float xtol, float ytol) {
  MTPoint point{x,y};
  for (auto& it : selectionBuffer_) {
    if (Pad::isInside(point, it->area_, xtol, ytol)) return true;
  }
  return false;
}

void MarkPad::clearMultiSelection() {
  if (selectionBuffer_.empty()) return;
  for (auto& it : selectionBuffer_) {
    it->setSelected(false);
    if (it == curShortcut_) curShortcut_ = nullptr;
  }
  selectionBuffer_.clear();
}

// - - - shortcut management - - - - - - - - - - - - - - - - - - - - - - - - - -

void MarkPad::createShortcut(const MTPoint& pos, bool twoSteps) {
  ShortcutMenu* menu = currentMenu();
  // menu needed and do nothing if alraedy creating shortcut in 2steps
  if (!menu || isCreatingShortcut_) return;
  
  if (twoSteps) {  // create in 2 steps
    ShortcutMenu* guide = nullptr;
    if (editMode_ != MarkPad::EditGuides && guide_) guide = findGuide(*menu);
    createShortcutFirstStep(*menu, guide);
  }
  
  else {  // create in one step if possible
    
    if (editMode_ != MarkPad::EditGuides && guide_) {
      if (Shortcut* guideArea = findGuideArea(*menu, pos))
        createShortcutFromGuide(*menu, *guideArea);   // guide area found => use it
      else {
        ShortcutMenu* guide = findGuide(*menu);
        createShortcutFirstStep(*menu, guide);     // guide area not found => two steps
      }
    }
    else {  // not in guide mode, create at (x,y) location
      createShortcutAtPos(*menu, pos);
    }
  }
  GUI::instance.updateOverlay();
}

// initiates shortcut creation, selectShortcut() will call createShortcutSecondStep()
void MarkPad::createShortcutFirstStep(ShortcutMenu& menu, ShortcutMenu* guide) {
  if (guide) {
    // order matters: creatingActualMenu_ must be set after selectShortcut()
    isCreatingFromGuide_ = true;
    setCurrentMenu(guide);  // guide will be displayed instead ofthe menu
    selectShortcut(nullptr);
  }
  // if no guide, the mouse position will be used
  isCreatingShortcut_ = true;
  creatingActualMenu_ = &menu;
  GUI::instance.hideEditorCB(true);   // hide editor while selecting template or position
}

Shortcut* MarkPad::createShortcutSecondStep(const Shortcut* guideArea) {
  Shortcut* s = nullptr;
  
  if (isCreatingShortcut_ && creatingActualMenu_) {
    // quit CreatingShortcut mode now otherwise infine loop through selectShortcut()!
    isCreatingShortcut_ = false;
    
    // not in guide mode, create at the mouse location
    if (!isCreatingFromGuide_) {
      s = createShortcutAtPos(*creatingActualMenu_, GUI::instance.currentMousePos());
    }
    // in guide mode, create according to template
    else {
      // attention: ne pas selectionner les openers quand les menus sont affiches
      if (!guideArea || (guideArea->submenu_ && !creatingActualMenu_->isMainMenu())) s = nullptr;
      else {
        s = createShortcutFromGuide(*creatingActualMenu_, *guideArea);
      }
    }
  }
  cancelCreateShortcut();
  return s;
}

void MarkPad::cancelCreateShortcut() {
  isCreatingShortcut_ = false;
  isCreatingFromGuide_ = false;
  if (creatingActualMenu_) setCurrentMenu(creatingActualMenu_);
  creatingActualMenu_ = nullptr;
  GUI::instance.hideEditorCB(false); // show editor again
  GUI::instance.updateOverlay();
}

Shortcut* MarkPad::createShortcutFromGuide(ShortcutMenu& menu, const Shortcut& guideArea) {
  Conf::mustSave();
  Shortcut* s = menu.addNewShortcut("NoName");
  s->copyArea(guideArea.area());
  s->setNameX(guideArea.nameX());
  s->setNameY(guideArea.nameY());
  if (guideArea.submenu_) s->setMenu(new ShortcutMenu());
  GUI::instance.completeShortcutCreation(menu, *s);
  return s;
}

Shortcut* MarkPad::createShortcutAtPos(ShortcutMenu& menu, const MTPoint& pos) {
  Conf::mustSave();
  Shortcut* s = menu.addNewShortcut("NoName");
  s->setArea(pos.x - Conf::k.newShortcutSize.width/2.f,
             pos.y - Conf::k.newShortcutSize.height/2.f,
             Conf::k.newShortcutSize.width, Conf::k.newShortcutSize.height);
  s->centerName(true, true);
  GUI::instance.completeShortcutCreation(menu, *s);
  return s;
}

// - - - guides - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

ShortcutMenu* MarkPad::findGuide(ShortcutMenu& menu) {
  if (!guide_) return nullptr;
  if (menu.isMainMenu()) return guide_;
  
  // find the guide corresponding to this opener
  Shortcut* opener = menu.opener();
  if (!opener) return nullptr;
  
  MTPoint opcenter{
    opener->x() + opener->width() /2.f,
    opener->y() + opener->height() /2.f
  };
  double mindist = 1.;
  ShortcutMenu* templ = nullptr;
  
  for (auto& it : guide_->shortcuts()) {
    MTPoint center{
      it->x() + it->width() /2.f,
      it->y() + it->height() /2.f
    };
    float d = currentPad()->distance2(center, opcenter);
    if (d < mindist) {mindist = d; templ = it->submenu_;}
  }
  return templ;
}

Shortcut* MarkPad::findGuideArea(ShortcutMenu& menu, const MTPoint& pos) {
  ShortcutMenu* guide_menu = findGuide(menu);
  if (!guide_menu) return nullptr;
  for (auto& s : guide_menu->shortcuts()) {
    if (Pad::isInside(pos, s->area())) return s;
  }
  return nullptr;
}

// - - - deletion, cut, paste - - - - - - - - - - - - - - - - - - - - - - - - -

void MarkPad::deleteShortcuts() {
  if (curShortcut_) {
    deleteShortcutImpl(*curShortcut_);
  }
  for (auto& it : selectionBuffer_) {
    deleteShortcutImpl(*it);
  }
  selectionBuffer_.clear();
}

bool MarkPad::deleteShortcutImpl(Shortcut& s) {
  // dont delete current menu and undeletable shortcuts
  if (opensCurrentMenu(s) || s.cannotEdit_) return false;
  Conf::mustSave();
  if (&s == curShortcut_) curShortcut_ = nullptr;
  s.setSelected(false);
  deleteBuffer_.push_back(&s);
  if (ShortcutMenu* menu = s.parentMenu()) {
    menu->removeShortcut(s);    // removes but doesn't destroy
    return true;
  }
  else return false;
}

Shortcut* MarkPad::undeleteShortcuts() {
  if (deleteBuffer_.empty()) return nullptr;
  Conf::mustSave();
  Shortcut* s = deleteBuffer_.back();
  deleteBuffer_.pop_back();
  if (ShortcutMenu* menu = s->parentMenu()) menu->addShortcut(*s);
  return s;
}

void MarkPad::copyShortcuts() {
  // clear PasteBuffer
  for (auto& it : pasteBuffer_) delete it;
  pasteBuffer_.clear();
  pasteCount_ = 1;
  // duplicates submenus if any
  if (curShortcut_ && !opensCurrentMenu(*curShortcut_)) {    // dont copy current menu!!
    pasteBuffer_.push_back(new Shortcut(*curShortcut_, true));
  }
  for (auto& it : selectionBuffer_) {
    if (!opensCurrentMenu(*it) && !it->cannotEdit_) {    // dont copy current menu!!
      pasteBuffer_.push_back(new Shortcut(*it, true));
    }
  }
}

void MarkPad::cutShortcuts() {
  copyShortcuts();
  pasteCount_ = 0;  // paste will place items at the same location
  deleteShortcuts();
}

Shortcut* MarkPad::pasteShortcuts(ShortcutMenu& menu) {
  if (pasteBuffer_.empty()) return nullptr;
  Conf::mustSave();
  Shortcut* dup = nullptr;
  for (auto& it : pasteBuffer_) {
    dup = new Shortcut(*it, true);   // duplicate submenu if any
    dup->move(pasteCount_ * Conf::k.pasteOffset.width,
              pasteCount_ * Conf::k.pasteOffset.height);
    menu.addShortcut(*dup);
  }
  pasteCount_++;
  return dup;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Adjust & Align

static void setmin(const std::list<Shortcut*>& buffer,
                   function< MTFloat (Shortcut*) > get,
                   function< void (Shortcut*, MTFloat) > set) {
  if (buffer.empty()) return;
  MTFloat minval = 1.;
  for (Shortcut* s : buffer) {
    if ((get)(s) < minval) minval = (get)(s);
  }
  if (minval < 0.) minval = 0.;
  for (Shortcut* s : buffer) {(set)(s, minval);}
}

static void setmax(const std::list<Shortcut*>& buffer,
                   function< MTFloat (Shortcut*) > get,
                   function< void (Shortcut*, MTFloat) > set) {
  if (buffer.empty()) return;
  float maxval = 0.;
  for (Shortcut* s : buffer) {
    if ((get)(s) > maxval) maxval = (get)(s);
  }
  if (maxval > 1.) maxval = 1.;
  for (Shortcut* s : buffer) {(set)(s, maxval);}
}

static void setmean(const std::list<Shortcut*>& buffer,
                   function< MTFloat (Shortcut*) > get,
                   function< void (Shortcut*, MTFloat) > set) {
  if (buffer.empty()) return;
  float val = 0.;
  for (Shortcut* s : buffer) {
    val += (get)(s);
  }
  val /= buffer.size();
  for (Shortcut* s : buffer) {(set)(s, val);}
}

void MarkPad::adjustBoxes(BoxPart::Value part) {
  switch (part) {
    case BoxPart::Left:
      setmin(selectionBuffer_,
             [](Shortcut* s) {return s->x();},
             [](Shortcut* s, MTFloat val) {
               s->setWidth(s->width() + s->x()-val);
               s->setX(val, false);
             });
      break;
    case BoxPart::Bottom:
      setmin(selectionBuffer_,
             [](Shortcut* s) {return s->y();},
             [](Shortcut* s, MTFloat val) {
               s->setHeight(s->height() + s->y()-val);
               s->setY(val, false);
             });
      break;
    case BoxPart::Right:
      setmax(selectionBuffer_,
             [](Shortcut* s) {return s->x() + s->width();},
             [](Shortcut* s, MTFloat val) {s->setWidth(val - s->x());});
      break;
    case BoxPart::Top:
      setmax(selectionBuffer_,
             [](Shortcut* s) {return s->y() + s->height();},
             [](Shortcut* s, MTFloat val) {s->setHeight(val - s->y());});
      break;
    default:
      break;
  }
}

void MarkPad::adjustBoxToGuide(Shortcut& s) {
  if (!s.parentmenu_) return;
  MTPoint middle {
    s.x() + s.width()/2.f,
    s.y() + s.height()/2.f
  };
  
  if (Shortcut* guide_s = findGuideArea(*s.parentmenu_, middle)) {
    s.area_.x = guide_s->x();
    s.area_.y = guide_s->y();
    s.area_.width = guide_s->width();
    s.area_.height = guide_s->height();
  }
}

void MarkPad::alignBoxes(BoxPart::Value part) {
  switch (part) {
    case BoxPart::Left:
      setmin(selectionBuffer_,
             [](Shortcut* s) {return s->x();},
             [](Shortcut* s, MTFloat val) {s->setX(val);});
      break;
    case BoxPart::Bottom:
      setmin(selectionBuffer_,
             [](Shortcut* s) {return s->y();},
             [](Shortcut* s, MTFloat val) {s->setY(val);});
      break;
    case BoxPart::Right:
      setmax(selectionBuffer_,
             [](Shortcut* s) {return s->x() + s->width();},
             [](Shortcut* s, MTFloat val) {s->setX(val - s->width());});
      break;
    case BoxPart::Top:
      setmax(selectionBuffer_,
             [](Shortcut* s) {return s->y() + s->height();},
             [](Shortcut* s, MTFloat val) {s->setY(val - s->height());});
      break;
    case BoxPart::HCenter:
      setmean(selectionBuffer_,
              [](Shortcut* s) {return s->x() + s->width()/2.;},
             [](Shortcut* s, MTFloat val) {s->setX(val - s->width()/2.f);});
      break;
    case BoxPart::VCenter:
      setmean(selectionBuffer_,
             [](Shortcut* s) {return s->y() + s->height()/2.;},
             [](Shortcut* s, MTFloat val) {s->setY(val - s->height()/2.f);});
      break;
    default:
      break;
  }
}
  
void MarkPad::alignTitles(BoxPart::Value part) {
  switch (part) {
    case BoxPart::Left:
      setmin(selectionBuffer_,
             [](Shortcut* s) {return s->nameX();},
             [](Shortcut* s, MTFloat val) {s->setNameX(val);});
      break;
    case BoxPart::Bottom:
      setmin(selectionBuffer_,
             [](Shortcut* s) {return s->nameY();},
             [](Shortcut* s, MTFloat val) {s->setNameY(val);});
      break;
    case BoxPart::Right:
      setmax(selectionBuffer_,
             [](Shortcut* s) {return s->nameX() + s->nameWidth();},
             [](Shortcut* s, MTFloat val) {s->setNameX(val - s->nameWidth());});
      break;
    case BoxPart::Top:
      setmax(selectionBuffer_,
             [](Shortcut* s) {return s->nameY() + s->nameHeight();},
             [](Shortcut* s, MTFloat val) {s->setNameY(val - s->nameHeight());});
      break;
    case BoxPart::HCenter:
      setmean(selectionBuffer_,
              [](Shortcut* s) {return s->nameX() + s->nameWidth()/2.;},
              [](Shortcut* s, MTFloat val) {s->setNameX(val - s->nameWidth()/2.f);});
      break;
    case BoxPart::VCenter:
      setmean(selectionBuffer_,
              [](Shortcut* s) {return s->nameY() + s->nameHeight()/2.;},
              [](Shortcut* s, MTFloat val) {s->setNameY(val - s->nameHeight()/2.f);});
      break;
    default:
      break;
  }
}
