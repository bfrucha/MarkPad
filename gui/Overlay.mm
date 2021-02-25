//
//  Overlay.mm
//  MarkPad Project
//
//  (c) Eric Lecolinet - http://www.telecom-paristech.fr/~elc
//  (c) Bruno Fruchard - http://brunofruchard.com/
//  Copyright (c) 2017/2020. All rights reserved.
//

#include <iostream>
#include <string>
#include "Conf.h"
#include "Strings.h"
#include "MarkPad.h"
#include "Pad.h"
#include "Overlay.h"
#include "Theme.h"
#include "OverlayImpl.h"
#include "Services.h"

Overlay Overlay::instance;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static MarkPad& mp = MarkPad::instance;

Overlay::Overlay() :
impl(nullptr),
theme(nullptr),
pad(nullptr),
pressedShortcut(nullptr),
modifierMask(0),
isEditing(false), mouseIsDown(false), selectionMode(false), 
updateGeometry(true), showHandles(false),
pressedPart{}, lastEnteredPart{},
lastMouseUpTime(0),
selectionBox{},
normal(nullptr), selected(nullptr) {
  feedback.color = nullptr;
  feedback.font = nullptr;
  feedback.textAttr = nullptr;
  feedback.updateAttr = true;
}

void Overlay::init(AWMenu* contextMenu) {
  theme = Theme::getTheme(Conf::k.theme);
  impl = new OverlayImpl();
  pad = nullptr;
  impl->setContextMenu(contextMenu);
  impl->rescale();
  updateGeometry = true;
}

void Overlay::setTheme(const std::string& themename) {
  theme = Theme::getTheme(themename);
  if (isEditing) impl->show(true/*editmode*/, theme->overlayEditBg);
}

void Overlay::mustUpdateGeometry() {
  updateGeometry = true;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// called only if the overlay is displayed (= in edit mode)
void Overlay::mouseDown(AWMouseEvent* event) {
  if (!pad || !mp.isEditing()) return;
  
  mouseIsDown = true;
  showHandles = false;
  selectionMode = false;
  // needed because isCreatingShortcut() will be false after selectShortcut()
  bool isCreatingShortcut = mp.isCreatingShortcut();
  
  // a mousepress cancels the recognition of a touch gesture
  // pad->cancelTouchGesture(false);
  
  MTPoint mouse = impl->getMousePos(event);
  modifierMask = AWL::getModifiers(event);
  mouseLast = mouse;
  mouseInit = mouse;  // initial position of the mouse
  pressedShortcut = pad->isInShortcut(mouse, pressedPart);
  
  // mouse outside shortcuts
  if (!pressedShortcut) {
    selectionMode = true;
    selectionBox = {mouse.x, mouse.y, 0.f, 0.f};
    mp.selectShortcut(nullptr);
    impl->setCursor(AWL::arrowCursor);
    impl->needsDisplay();
    return;
  }
  
  // MenuBtn will open/close menu on release. It must remain active even if creating shortcut
  if (pressedPart == BoxPart::MenuBtn && pressedShortcut->menu()) {
    impl->setCursor(AWL::arrowCursor);
    return;
  }
  
  // single selection:
  // nothing selected || (shift, cmd not pressed && not in selection)
  if (!mp.isShortcutSelected()
      || ((modifierMask & (AWL::ShiftModifier|AWL::CommandModifier)) == 0
          && ((pressedPart & (BoxPart::Inside|BoxPart::Title))
              || !mp.isInMultiSelection(mouse.x, mouse.y,
                                        Conf::k.pickTolerance,
                                        Conf::k.pickTolerance))
          )
      ){
    // can show properties => perform mp.selectShortcut(s)
    mp.selectShortcut(pressedShortcut);
    // dont allow moving shortcut in CreatingShortcut mode
    if (isCreatingShortcut) {
      pressedShortcut = nullptr;
      impl->setCursor(AWL::arrowCursor);
    }
    else {
      showHandles = true;
      setCursorOnPart(pressedPart);
    }
  }
  
  // multi-selection (not in isCreatingShortcut mode)
  else if (!isCreatingShortcut) {
    if (pressedPart & (BoxPart::Inside|BoxPart::Title))
      mp.multiSelects(pressedShortcut, !pressedShortcut->isSelected());
    else
      mp.multiSelects(pressedShortcut, true);

    // cannot show properties => mp.currentShortcut() is null
    pressedShortcut = nullptr;
    impl->setCursor(AWL::openHandCursor);
  }
  
  impl->needsDisplay();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void Overlay::mouseUp(AWMouseEvent* event) {
  if (!mp.isEditing()) return;
  
  mouseIsDown = false;
  MTPoint mouse = impl->getMousePos(event);
  modifierMask = AWL::getModifiers(event);
  mouseLast = mouse;
  showHandles = false;
  selectionMode = false;

  if (!pad) return;
  
  if (!pressedShortcut) {
    if (AWL::getTimestamp(event) - lastMouseUpTime < Conf::k.doubleClickDelay
        && fabs(mouseInit.x - mouse.x) < 0.001
        && fabs(mouseInit.y - mouse.y) < 0.001) {
      GUI::instance.doubleClickOnOverlayCB(nullptr, mouse, BoxPart::Outside);
    }
    lastMouseUpTime = AWL::getTimestamp(event);
    impl->needsDisplay();
    return;
  }
  
  // a release on the menu button opens/closes the menu
  if (pressedShortcut->menu() && (pressedPart == BoxPart::MenuBtn)) {
    pressedShortcut->openCloseMenu();
    return;
  }

  // a double click moves the editor or opens/closes the menu, etc.
  if (AWL::getTimestamp(event) - lastMouseUpTime < Conf::k.doubleClickDelay
      && fabs(mouseInit.x - mouse.x) < 0.001
      && fabs(mouseInit.y - mouse.y) < 0.001) {
    GUI::instance.doubleClickOnOverlayCB(pressedShortcut, mouse, pressedPart);
  }
  
  if (!mp.multiSelection().empty()) impl->setCursor(AWL::arrowCursor);
  
  pressedShortcut = nullptr;
  pressedPart = BoxPart::Outside;
  lastMouseUpTime = AWL::getTimestamp(event);
  impl->needsDisplay();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void Overlay::mouseDragged(AWMouseEvent* event) {
  if (!pad || !mp.isEditing()) return;
  
  MTPoint mouse = impl->getMousePos(event);
  modifierMask = AWL::getModifiers(event);

  if (mp.editMode() == mp.EditBorders) {
    if (pressedShortcut && (pressedPart & BoxPart::OnBorders)) dragBorderHandle(mouse);
    mouseLast = mouse;
    return;
  }
  
  // select shortcuts in selectionBox
  if (selectionMode) {
    float x1 = min(mouseInit.x, mouse.x);
    float x2 = max(mouseInit.x, mouse.x);
    float y1 = min(mouseInit.y, mouse.y);
    float y2 = max(mouseInit.y, mouse.y);
    selectionBox = {x1, y1, x2-x1, y2-y1};
    
    if (ShortcutMenu* menu = mp.currentMenu()) {
      for (Shortcut* s : menu->shortcuts()) {
        if (Pad::isInside(s->area(), selectionBox,
                          Conf::k.pickTolerance, Conf::k.pickTolerance))
          mp.addToMultiSelection(s);
        else
          mp.removeFromMultiSelection(s);
      }
    }
    
    impl->needsDisplay();
    return;
  }
  
  float deltax = mouse.x - mouseLast.x;
  float deltay = mouse.y - mouseLast.y;
  
  // no curshortcut => move multiselection if any
  if (!pressedShortcut) {
    auto& multisel = mp.multiSelection();
    if (!multisel.empty()) {
      for (auto& it : multisel) it->move(deltax, deltay);
      updateShortcuts();
    }
    mouseLast = mouse;
    return;
  }
  
  if (pressedPart == BoxPart::Outside) {
    // nop
  }
  // cursor inside shortcut => move this shortcut or its title
  else if (pressedPart == BoxPart::Inside || pressedPart == BoxPart::Title) {
    // move title if Alt is pressed
    if (pressedPart == BoxPart::Title && (modifierMask & AWL::AltModifier)) {
      pressedShortcut->moveName(deltax, deltay, false);
    }
    else {
      pressedShortcut->move(deltax, deltay);  // move box
    }
    updateShortcuts();
  }
  // drag handle
  else dragHandle(mouse);

  mouseLast = mouse;
}


void Overlay::dragHandle(const MTPoint& mouse) {
  float dx = mouse.x - mouseLast.x;
  float dy = mouse.y - mouseLast.y;
  switch (pressedPart) {
    case BoxPart::Left:
      pressedShortcut->changeWidth(-dx, true);
      break;
    case BoxPart::Right:
      pressedShortcut->changeWidth(dx, false);
      break;
    case BoxPart::Bottom:
      pressedShortcut->changeHeight(-dy, true);
      break;
    case BoxPart::Top:
      pressedShortcut->changeHeight(dy, false);
      break;
    case BoxPart::BottomLeft:
      pressedShortcut->changeWidth(-dx, true);
      pressedShortcut->changeHeight(-dy, true);
      break;
    case BoxPart::BottomRight:
      pressedShortcut->changeWidth(dx, false);
      pressedShortcut->changeHeight(-dy, true);
      break;
    case BoxPart::TopLeft:
      pressedShortcut->changeWidth(-dx, true);
      pressedShortcut->changeHeight(dy, false);
      break;
    case BoxPart::TopRight:
      pressedShortcut->changeWidth(dx, false);
      pressedShortcut->changeHeight(dy, false);
      break;
    default:
      break;
  }
  updateShortcuts();
}

void Overlay::dragBorderHandle(const MTPoint& mouse) {
  float dx = mouse.x - mouseLast.x;
  float dy = mouse.y - mouseLast.y;
  switch (pressedPart) {
    case BoxPart::Left:
      if (pressedShortcut->name_== "Right") {
        pressedShortcut->changeWidth(-dx, true);
        Conf::instance.activeBorders.right = pressedShortcut->area_.width;
      }
      break;
    case BoxPart::Right:
      if (pressedShortcut->name_== "Left") {
        pressedShortcut->changeWidth(dx, false);
        Conf::instance.activeBorders.left = pressedShortcut->area_.width;
      }
      break;
    case BoxPart::Bottom:
      if (pressedShortcut->name_== "Top") {
        pressedShortcut->changeHeight(-dy, true);
        Conf::instance.activeBorders.top = pressedShortcut->area_.height;
      }
      break;
    case BoxPart::Top:
      if (pressedShortcut->name_== "Bottom") {
        pressedShortcut->changeHeight(dy, false);
        Conf::instance.activeBorders.bottom = pressedShortcut->area_.height;
      }
      break;
    default:
      break;
  }
  updateShortcuts();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void Overlay::mouseMoved(AWMouseEvent* event) {
  if (!pad || !mp.isEditing()) return;
  
  Shortcut* s = mp.currentShortcut();
  if (!s) {
    //impl->setCursor(AWL::arrowCursor); couteux!
    return;
  }
  
  MTPoint mouse = impl->getMousePos(event);
  BoxPart::Value part = pad->isInTitle(mouse, *s);
  if (part == BoxPart::Outside) part = pad->isInBox(mouse, *s);
  
  if (part == lastEnteredPart) return;
  setCursorOnPart(part);
}

void Overlay::setCursorOnPart(BoxPart::Value part) {
  lastEnteredPart = part;

  switch (part) {
    case BoxPart::Outside:
    case BoxPart::MenuBtn:
      impl->setCursor(AWL::arrowCursor);
      break;
    case BoxPart::Inside:
    case BoxPart::Title:
      impl->setCursor(AWL::openHandCursor);
      break;
    case BoxPart::Left:
    case BoxPart::Right:
      impl->setCursor(AWL::resizeHorCursor);
      break;
    case BoxPart::Bottom:
    case BoxPart::Top:
      impl->setCursor(AWL::resizeVerCursor);
      break;
    case BoxPart::BottomLeft:
      impl->setCursor(AWL::resizeDiagPlus45Cursor);
      break;
    case BoxPart::BottomRight:
      impl->setCursor(AWL::resizeDiagMinus45Cursor);
      break;
    case BoxPart::TopLeft:
      impl->setCursor(AWL::resizeDiagMinus45Cursor);
      break;
    case BoxPart::TopRight:
      impl->setCursor(AWL::resizeDiagPlus45Cursor);
      break;
    default:
      impl->setCursor(AWL::arrowCursor);
      break;
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void Overlay::getTitle(const Shortcut& s, string& title) {
  title = s.name();
}

void Overlay::layoutShortcut(Shortcut& s) {
  string title;
  getTitle(s, title);
  if (impl) impl->layoutTitle(s.nameAreaRef(), title, theme->titleAttr);
}

void Overlay::drawShortcut(Shortcut* s, BoxTheme* attr, bool opened) {
  if (!impl || !s) return;
  string title;
  getTitle(*s, title);
  
  if (attr->drawn & (Theme::DrawSolid | Theme::DrawDashed | Theme::DrawFilled)) {
    impl->drawBox(s->area(), *attr);
  }
  if (attr->drawn & Theme::DrawTitle) {
    // note: drawText updates nameArea size
    impl->drawTitle(s->nameAreaRef(), title, attr->titleAttr);
    
    // draw menuBtn for shortcuts that open menus
    if (s->menu()) {             // NB drawIcon() draws a squared icon
      impl->drawIcon(opened ? theme->menuBtnUp : theme->menuBtnDown,
                     s->nameX() + s->nameWidth(), s->nameY(), s->nameHeight());
    }
  }
}

void Overlay::drawLink(const Shortcut* from, const Shortcut* to, AWColor* color) {
  MTPoint frompos {
    (from->nameX() + from->nameWidth()/2.f),
    (from->nameY() + from->nameHeight()/2.f)
  };
  MTPoint topos = {
    (to->nameX() + to->nameWidth()/2.f),
    (to->nameY() + to->nameHeight()/2.f)
  };
  impl->drawLine(frompos, topos, color);
}

void Overlay::drawHandles(const Shortcut& s) {
  MTRect a = s.area();
  
  impl->drawHandle(a.x, a.y); // BottomLeft
  impl->drawHandle(a.x+a.width/2.f, a.y);  // Bottom
  impl->drawHandle(a.x+a.width, a.y); // BottomRight
  
  impl->drawHandle(a.x, a.y+a.height); // TopLeft
  impl->drawHandle(a.x+a.width/2.f, a.y+a.height);  // Top
  impl->drawHandle(a.x+a.width, a.y+a.height); // TopRight
  
  impl->drawHandle(a.x, a.y+a.height/2.f); // Left
  impl->drawHandle(a.x+a.width, a.y+a.height/2.f);  // Right
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void Overlay::drawMenu(const ShortcutMenu& menu, bool editing, bool drawlinks,
                       bool drawopener) {
  //cerr << "Overlay::drawMenu "<<endl;
  if (!pad) return;
  
  // geometry must be updated by layoutShortcut if positions have changed
  if (updateGeometry) {
    updateGeometry = false;
    for (auto s : menu.shortcuts()) layoutShortcut(*s);
  }
  
  // draw links first
  if (drawlinks) {
    AWColor* linkColor = normal->linkColor;
    for (auto s : menu.shortcuts()) {
      if (menu.opener()) drawLink(menu.opener(), s, linkColor);
    }
  }
  
  // will point on selected shortcut
  Shortcut* selsh = nullptr;
  
  // draw all shortcuts except the selected shortcut
  for (Shortcut* s : menu.shortcuts()) {
    if (s == mp.currentShortcut()) {
      selsh = s;  // check that selected shortcut is in this menu
      continue;   // don't paint selected shortcut now
    }
    drawShortcut(s, s->isSelected() ? selected : normal);
  }
  
  // draw the main selected shortcut on top on the other ones
  if (selsh) {
    ShortcutMenu* parent = selsh->parentMenu();
    Shortcut *opener = nullptr;
  
    if (parent && (opener = parent->opener())) {
      // draw stroke origin
      if (editing) drawShortcut(opener, &theme->editParent);
      drawLink(opener, selsh, selected->linkColor);
    }
    
    // draw box and handles of selsh, the selected shortcut
    drawShortcut(selsh, selected);
    if (editing) drawHandles(*selsh);
  }
  
  Shortcut* opener = menu.opener();
  if (opener && drawopener) {
    // in edit mode, draw menu opener
    if (editing) drawShortcut(opener, &theme->editParent, true/*opened*/);
    else drawShortcut(opener, selected, true/*opened*/);
      
    if (editing && opener == mp.currentShortcut()) drawHandles(*opener);
    
    ShortcutMenu* parent = opener->parentMenu();
    Shortcut *super_opener = nullptr;
    
    // superparent (cascaded menus)
    if (parent && (super_opener = parent->opener())) {
      if (editing) drawShortcut(super_opener, &theme->editParent, true/*opened*/);
      else drawShortcut(super_opener, selected, true/*opened*/);
      drawLink(super_opener, opener, selected->linkColor);
    }
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void Overlay::draw() {
  //cerr << "Overlay::draw "<<endl;

  if (feedbackMode) {
    impl->drawInfo(feedback.text, feedback.textAttr, 1./2.);
    return;
  }
  
  if (!pad) return;  // shortcuts not displayed if pad is null
  ShortcutMenu* menu = mp.currentMenu();
  if (!menu) goto END;;
  
  if (mp.isEditing()) {

    // draw active borders
    if (mp.currentMenu()->isMainMenu()) {
      auto& b = Conf::k.activeBorders;
      impl->drawLine(MTPoint{b.left, 0.f}, MTPoint{b.left, 1.f}, theme->activeBorderColor);
      impl->drawLine(MTPoint{1.f-b.right, 0.f}, MTPoint{1.f-b.right, 1.f}, theme->activeBorderColor);
      impl->drawLine(MTPoint{0.f, b.bottom}, MTPoint{1.f, b.bottom}, theme->activeBorderColor);
      impl->drawLine(MTPoint{0.f, 1.f-b.top}, MTPoint{1.f, 1.f-b.top}, theme->activeBorderColor);
    }
    
    // draw grid
    if (Conf::k.showGrid) impl->drawGrid();
  }

  else { // Show Menus mode

    // draw background image
    if (Conf::k.showBgImage && theme->bgImage) {
      impl->drawImage(theme->bgImage, 0, 0, 1.f, 1.f);
    }
    
    normal = &theme->show;
    selected = &theme->showSelected;
    drawMenu(*menu, false, true, true);

    if (Services::isAccessibilityEnabled()) {
      impl->drawInfo("Press "+Conf::k.editHotkey+" to Edit",theme->showInfoAttr, 0.8);
    }
    
    impl->drawFingers();
    return;
  }
  
  if (!isEditing) goto END;;

  // Create Shortcut
  if (mp.isCreatingShortcut()) {
    
    // draw guide (if available) then menus
    if (mp.isCreatingFromGuide() || mp.editMode() == mp.EditGuides) {
      normal = &theme->guidesCreate;
      selected = &theme->guidesCreate;
      drawMenu(*menu, false, false, false);
    }
    
    normal = &theme->editCreate;
    selected = &theme->editCreate;
    drawMenu(*mp.creatingActualMenu(), false, true/*Conf::k.showLinks*/, true);

    if (mp.isCreatingFromGuide())
      impl->drawInfo(Strings::k.ClickToCreateShortcut1, theme->editInfoAttr, 3./4.);
    else
      impl->drawInfo(Strings::k.ClickToCreateShortcut2, theme->editInfoAttr, 3./4.);
  }
  
  // Edit Guide
  else if (mp.editMode() == mp.EditGuides) {
    normal = &theme->guidesEdit;
    selected = &theme->guidesEditSelected;
    drawMenu(*menu, true, false, true);
    impl->drawInfo("Guides", theme->editInfoAttr, 3./4.);
  }

  // Edit Borders
  else if (mp.editMode() == mp.EditBorders) {
    normal = &theme->activeBorders;
    selected = &theme->activeBorders;
    drawMenu(*menu, true, false, false);
    impl->drawInfo("Active Borders", theme->editInfoAttr, 3./4.);
  }
  
  // Edit Shortcuts
  else {

    // draw background image
    if (Conf::k.showBgImage && theme->bgImage) {
      impl->drawImage(theme->bgImage, 0, 0, 1.f, 1.f);
    }

    // draw templates then menus
    ShortcutMenu* templ = mp.findGuide(*menu);
    if (templ) {
      normal = &theme->guides;
      selected = &theme->guides;
      drawMenu(*templ, false, false, false);
    }
    
    normal = &theme->edit;
    selected = &theme->editSelected;
    drawMenu(*menu, true, true/*showLinks*/, true);

    if (menu->shortcuts().empty()) {
      impl->drawInfo(Strings::k.EmptyMenu, theme->editInfoAttr, 3./4.);
    }
  }
  
  if (selectionMode) impl->drawSelectionBox(selectionBox);
  
END:
  if (mp.canShowFingers()) impl->drawFingers();  // draw touches on the top
}

void Overlay::show(GUI::OverlayMode mode) {
  //cerr << "Overlay::show "<<mode<<endl;
  
  // dont do anything if feedbackMode otherwise the feedback will be lost
  if (!impl || feedbackMode) return;
    
  pressedShortcut = nullptr;
  isEditing = false;
  showHandles = false;
  selectionMode = false;

  switch (mode) {
    case GUI::OverlayHide:
      impl->hide();
      pad = nullptr;  // wont draw shortcuts
      break;
    case GUI::OverlayMenus:
      pad = mp.currentPad();  // wont draw shortcuts otherwise
      if (!pad) return;
      impl->show(false,theme->overlayShowBg);  // not opaque
      break;
    case GUI::OverlayEdit:
      pad = mp.currentPad();  // wont draw shortcuts otherwise
      if (!pad) return;
      isEditing = true;
      impl->show(true/*editmode*/, theme->overlayEditBg);   // opaque
      break;
  }
}

void Overlay::initGeometry(Pad* ppad) {
  pad = ppad;
  ShortcutMenu* menu = pad->mainMenu();
  if (!menu) return;
  
  for (auto s : menu->shortcuts()) {
    if (s) {
      layoutShortcut(*s);
      if (ShortcutMenu* m = s->menu()) {
        for (auto s2 : m->shortcuts()) layoutShortcut(*s2);
      }
    }
  }
  pad = nullptr;
}

void Overlay::updateShortcuts() {
  Conf::mustSave();
  updateGeometry = true;
  GUI::instance.updateShortcutSize(pressedShortcut);
  if (pad && impl) impl->needsDisplay();
}

void Overlay::update(bool update_shortcuts) {
  //cerr << "Overlay::update "<<endl;
  if (!mp.currentShortcut() && impl) {
    showHandles = false;
    impl->setCursor(AWL::arrowCursor);
  }
  if (update_shortcuts) {
    pressedShortcut = MarkPad::instance.currentShortcut();
    updateShortcuts();
  }
  if (pad && impl) impl->needsDisplay();
}

void Overlay::fullscreen(bool state) {
  if (impl) impl->fullscreen(state);
  update();
}

// - - - - - -

void Overlay::showFeedback(const std::string& text) {
  if (!Conf::k.showFeedback || Conf::k.feedback.delay <= 0 || !impl) return;
  feedbackMode = true;
  showHandles = false;
  selectionMode = false;

  if (feedback.updateAttr) {
    feedback.updateAttr = false;
    delete feedback.textAttr;
    // nb getXxx() inits font & color if needed
    feedback.textAttr = new TextAttr(getFeedbackFont(), getFeedbackColor(), AWL::ClearColor);
  }
  feedback.text = text;
  //cerr << "Overlay::showFeedback "<<feedback.text<<endl;
  //impl->setFeedbackMode(true);
  //impl->show(AWL::ClearColor, false);  // not opaque
  impl->showFeedback();
}

void Overlay::setFeedbackColor(AWColor* c) {
  feedback.updateAttr = true;
  feedback.color = c;
}

void Overlay::setFeedbackFont(AWFont* f) {
  feedback.updateAttr = true;
  feedback.font = f;
}

AWColor* Overlay::getFeedbackColor() {
  if (!feedback.color) feedback.color = AWL::newColor(Conf::k.feedback.color);
  return feedback.color;
}

AWFont* Overlay::getFeedbackFont() {
  if (!feedback.font) feedback.font = AWL::newFont(Conf::k.feedback.font);
  return feedback.font;
}
