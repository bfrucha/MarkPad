//
//  Pad.mm
//  MarkPad Project
//
//  (c) Eric Lecolinet - http://www.telecom-paristech.fr/~elc
//  (c) Bruno Fruchard - http://brunofruchard.com/
//  Copyright (c) 2017/2020. All rights reserved.
//

#include <iostream>
#include <cmath>
#include "Conf.h"
#include "MarkPad.h"
#include "Pad.h"
#include "Actions.h"
#include "DataLogger.h"
#include "Services.h"

static MarkPad& mp = MarkPad::instance;

PadConf::PadConf(float width, float height) :
padWidth(width),
padHeight(height),
padRatio(width/height),
padRatio2(padRatio*padRatio) {}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

Pad::Pad(MTDevice* device, const PadConf& padconf) :
PadConf(padconf),
device_(device),
dataLogger_(*new DataLogger(*this))
{  
  mainMenu_ = Conf::instance.mainMenu_;
  if (!mainMenu_) {
    Conf::instance.mainMenu_ = mainMenu_ = new ShortcutMenu;
    Conf::mustSave();
    mp.info("Creating a new MainMenu");
  }
  mainMenu_->setMainMenu();
  mp.setCurrentMenu(mainMenu_);
}

void Pad::initActiveBorders() {
  if (!activeBorders_) {
    activeBorders_ = new ShortcutMenu;
    (leftBorder_ = activeBorders_->addNewShortcut("Left"))->cannotEdit_ = true;
    (rightBorder_ = activeBorders_->addNewShortcut("Right"))->cannotEdit_ = true;
    (topBorder_ = activeBorders_->addNewShortcut("Top"))->cannotEdit_ = true;
    (bottomBorder_ = activeBorders_->addNewShortcut("Bottom"))->cannotEdit_ = true;
  }
  auto& b = Conf::k.activeBorders;
  leftBorder_->setArea(0., 0, b.left, 1.);
  rightBorder_->setArea(1-b.right, 0, b.right, 1.);
  bottomBorder_->setArea(0., 0., 1., b.bottom);
  topBorder_->setArea(0., 1.-b.top, 1., b.top);
}

void Pad::updateOverlay() {
  // postponed to fix threads
  Services::postpone([](){GUI::instance.updateOverlay(false);});
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//
// 1) ne pas confondre rawTouches[] le parametre de cette fonction et _touches[]:
//    - rawTouches[] est le vecteur retourné par la Touch API
//    - _touches[] est une variable d'instance du Pad qui pointe sur les elements
//      de rawTouches[]
//
// 2) _touches[] est filtré comme suit:
//    - sa taille max est MaxTouchCount (toujours suffisante en pratique)
//    - on élimine les touches dont la taille est trop petite car ca provoque des
//      détections indésirables ou des plantages de l'affichage du menu
//    - on ne conserve que les touches dont "state" est 2 ou 4 cad. celles qui
//      touchent le trackpad ou qui en sont très proche *avant* qu'on le touche.
//      Ca permet d'éviter de manquer des interactions un peu rapides.
//    - on supprime éventuellement le touch correspondant a menuTouchId, cad. le
//      touch qui a permis d'ouvrir ce menu. Ceci permet de selectiooner l'item
//      avec un autre doigt.
//
// 3) le capteur n'est pas homogène, en particulier sur les bords verticaux où la
//    valeur min et max de X depend en réalité de Y !
//
// 4) les coordonnées X et Y des éléments du vecteur sont normalisée entre 0. et 1.
//    Comme le trackpad n'est pas nécessairement carré, il faut en tenir compte
//    dans les calculs d'ANGLES et de DISTANCES. C'est fait grace à la variable
//    d'instance padRatio = padHeight/padWidth (cf. getAngle() et distance())

void Pad::touchCallback(const MTTouch* rawTouches, int rawTouchCount)
{
  // maximum number of touches (= size of array _touches[])
  if (rawTouchCount > MaxTouchCount) rawTouchCount = MaxTouchCount;
  
  const MTTouch** dst = touches_;
  const MTTouch* src_end = rawTouches + rawTouchCount;
  
  // touch qui correspond maintenant à firstTouch (meme ID)
  const MTTouch* currentTouch = nullptr;
  
  // filtrage
  for (const MTTouch* src = rawTouches; src < src_end; ++src) {
    if (src->phase >= MTPhase::Start && src->phase <= MTPhase::Touch
        && src->majorAxis > Conf::k.minTouchSize
        && src->minorAxis > Conf::k.minTouchSize
        && src->majorAxis < Conf::k.maxTouchSize
        && src->minorAxis < Conf::k.maxTouchSize
        ) {
      if (validTouch_ && src->ident == touch1Id_) currentTouch = src;
      *dst++ = src;
    }
  }
  // le nombre de touches restant apres filtrage
  touchcount_ = int(dst - touches_);
  
  // pas d'action si on edite le menu mais au besoin on met a jour l'affichage
  if (mp.isEditing()) {
    // refresh the overlay if the touches are shown
    if (mp.canShowFingers()) updateOverlay();
    return;
  }
  
  // cancelTouchGesture() was called
  if (cancelled_) {
    
    // si on relache (y compris le doigt qui tenait le menu ouvert)
    if (touchcount_ == 0) {
      validTouch_ = unvalidTouch_ = cancelled_ = false;
      opener_ = superopener_ = nullptr;
      mp.curShortcut_ = nullptr;
      Services::enableDeviceForCursor(device());
    }
    
    // refresh the overlay if the menu is shown
    if (mp.isOverlayShown()) updateOverlay();
    return;
  }
  
  // pas d'interaction si le first touch est hors zone
  if (unvalidTouch_) {
    
    // pour afficher les doigts quand menu pas affiché
    if (mp.hotkeyWantsMenu_) updateOverlay();
    
    if (touchcount_ == 0) {
      unvalidTouch_ = false;
      validTouch_ = unvalidTouch_ = cancelled_ = false;
      Services::enableDeviceForCursor(device());
    }
    return;
  }
  
  // ne garder que cas ou touchCount vaut reellement 0, pas ceux dus au filtrage
  if (touchcount_ == 0 && rawTouchCount > 0) return;
  
  if (touchcount_ > 0) {
    
    if (!validTouch_) {         // detect first touch
      currentTouch = touches_[0];

      // tester si touch dans le bord
      if (currentTouch->norm.pos.x <= Conf::k.activeBorders.left
          || currentTouch->norm.pos.x >= 1.-Conf::k.activeBorders.right
          || currentTouch->norm.pos.y <= Conf::k.activeBorders.bottom
          || currentTouch->norm.pos.y >= 1.-Conf::k.activeBorders.top
          ) {
        validTouch_ = true;
      }
      
      // cas ou le touch n'est pas dans le bord mais:
      // - soit hotkey appuyé
      // - soit shortcut special qui ne nécessite pas de partir du bord
      Shortcut* s = mainMenu_->findShortcut(*currentTouch);
      if (s && (mp.hotkeyWantsMenu_ || !s->touchFromBorder_)) {
        validTouch_ = true;
        opener_ = s;
        // bloquer curseur sauf si shorcut special car peut etre au centre
        if (opener_->touchFromBorder_) Services::disableDeviceForCursor(device());
      }

      if (!validTouch_) {
        unvalidTouch_ = true;
        return;
      }
      else {
        touch1_ = currentTouch->norm.pos;
        touch1Id_ = currentTouch->ident;
        touchTime_ = currentTouch->time;
        if (Conf::k.logData && !mp.isEditing()) {
          dataLogger_.startGesture(s, currentTouch->norm.pos);
        }
      }
    } // endif(!validTouch_)
    
    if (!currentTouch) return;
  
    // on selectionne le shortcut si bord deja touché et distance suffisante
    if (validTouch_ && !opener_
        && distance2(touch1_, currentTouch->norm.pos)
        >= Conf::k.minMovement*Conf::k.minMovement) {
      if (Shortcut* s = mainMenu_->findShortcut(*currentTouch)) {
        opener_ = s;
        Services::disableDeviceForCursor(device());
      }
    }
    
    ShortcutMenu* menu = nullptr;
    if (!opener_  || !(menu = opener_->submenu_)) return;  // menu pas trouvé
    /*
    if (Conf::k.logData) {
      dataLogger_.recordGestureMove(currentTouch->norm.pos,
                                    mp.isOverlayShown() && !mp.isEditing());
    }*/
    
    // si currentTouch est l'opener courant on ouvre le menu (si nécessaire)
    // si hotkey appuyé ou temps > menuDelay (s'applique aussi aux sousmenus)
    
    if ((!mp.isMenuShown_ || menu != mp.currentMenu())
        && isInside(currentTouch->norm.pos, opener_->area_)
        && (mp.hotkeyWantsMenu_
            || (opener_->touchOpenMenu_
                && currentTouch->time - touchTime_ > Conf::k.menuDelay)
            )
        ) {
      menu->openMenu();
    }
        
    // si le shortcut appartient au menu
    else if (Shortcut* s = menu->findShortcut(*currentTouch)) {
      mp.curShortcut_ = s;
      selTouch_ = *currentTouch;

      // si c'est un menu cascadé on garde l'opener du supermenu dans superopener
      if (s->submenu_) {
        superopener_ = opener_;
        opener_ = s;
        touchTime_ = currentTouch->time;
        // si parent menu deja ouvert alors ouvrir submenu sans attendre delai
        if (s->parentmenu_ == mp.currentMenu()) s->openMenu();
      }
      
      // actions continues
      //else if (s->triggeredBy(Shortcut::Motion)) doAction(Shortcut::Down);
    }
    
    // si le shortcut n'appartient pas au menu
    else {
      // pour eviter de garder des sous-items selectionnés
      if (mp.curShortcut_) mp.curShortcut_->setSelected(false);
      mp.curShortcut_ = nullptr;
      
      // fermer supermenu et revenir au menu parent
      if (superopener_ && isInside(currentTouch->norm.pos, superopener_->area_)) {
        mp.curShortcut_ = opener_ = superopener_;
        opener_->openMenu();
        superopener_->setSelected(false);
        superopener_ = nullptr;
      }
    }
    
    // sauver le temps du dernier Tap pour detecter les DoubleTap
    // lastTapTime_ = currentTouch->time;
  }
  
  else { // touchcount == 0 (on a relaché)
    Services::enableDeviceForCursor(device());

    // executer l'action au release si un shortcut a ete trouvé
    if (mp.curShortcut_ && opener_ && !cancelled_ && !unvalidTouch_) {

      if (Conf::k.logData && !mp.isEditing()) {
        dataLogger_.endGesture(mp.curShortcut_, selTouch_.norm.pos, mp.isOverlayShown());
      }

      if (Conf::k.showFeedback) {
        auto& s = *mp.curShortcut_;
        if (s.feedback_) {
          if (*s.feedback_ != "none") GUI::instance.showFeedback(*s.feedback_);
        }
        else GUI::instance.showFeedback(s.name_);
      }

      // fermer l'overlay avant d'excuter a cause des alertes de securite
      if (mp.isOverlayShown()) mp.showOverlay(false);

      // executer l'action
      Actions::instance.exec(*mp.curShortcut_, selTouch_, Shortcut::Up);
    }

    if (mp.hotkeyWantsMenu_) {
      // pour revenir au main menu apres avoir fermé un menu quand hotkey est appuyé
      mp.setCurrentMenu();
    }
    else {
      // sinon (hotkey pas appuyé) tout fermer
      if (mp.isOverlayShown()) mp.showOverlay(false);
    }

    validTouch_ = unvalidTouch_ = cancelled_ = false;
    opener_ = superopener_ = nullptr;
    mp.curShortcut_ = nullptr;
    Conf::saveIfNeeded();
  }
  
  // refresh the overlay if the menu is shown
  if (mp.isOverlayShown()) updateOverlay();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void Pad::cancelTouchGesture(bool closeMenu) {
  cancelled_ = true;
  
  if (validTouch_) {
    validTouch_ = false;
    opener_ = superopener_ = nullptr;
    mp.curShortcut_ = nullptr;
    
    if (closeMenu) {
      mp.showOverlay(false);
      mp.setCurrentMenu();
    }
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

BoxPart::Value Pad::isInBox(const MTPoint& pos, Shortcut& s) const {
  // should be HandleSize/2 but enlarged to make selection easier
  float xtol = Conf::k.pickTolerance;
  float ytol = Conf::k.pickTolerance * padRatio;
  MTRect a = s.area();
  
  if (!isInside(pos, a, xtol, ytol)) return BoxPart::Outside;
  if (isInside(pos, a, -xtol, -ytol)) return BoxPart::Inside;
  
  if (isClose(pos, a.x, a.y, xtol, ytol))
    return BoxPart::BottomLeft;
  
  else if (isClose(pos, a.x+a.width, a.y, xtol, ytol))
    return BoxPart::BottomRight;
  
  else if (isClose(pos, a.x, a.y+a.height, xtol, ytol))
    return BoxPart::TopLeft;
  
  else if (isClose(pos, a.x+a.width, a.y +a.height, xtol, ytol))
    return BoxPart::TopRight;
  
  else if (isInside(pos, MTRect{a.x-xtol, a.y, 2*xtol, a.height}))
    return BoxPart::Left;
  
  else if (isInside(pos, MTRect{a.x+a.width-xtol, a.y, a.width+2*xtol, a.height}))
    return BoxPart::Right;
  
  else if (isInside(pos, MTRect{a.x, a.y-ytol, a.width, 2*ytol}))
    return BoxPart::Bottom;
  
  else if (isInside(pos, MTRect{a.x, a.y+a.height-ytol, a.width, a.height+2*ytol}))
    return BoxPart::Top;

  else return BoxPart::Outside;  // should not happen
}

BoxPart::Value Pad::isInTitle(const MTPoint& pos, Shortcut& s) const {
  if (isInside(pos, s.nameArea())) return BoxPart::Title;
  // for shortcuts that open menus
  if (s.menu()) {
    MTRect menu_btn = {
      s.nameX() + s.nameWidth(),
      s.nameY(),
      s.nameHeight() / padRatio,  // height => not the same res => padRatio
      s.nameHeight()
    };
    if (isInside(pos, menu_btn)) return BoxPart::MenuBtn;
  }
  return BoxPart::Outside;
}

Shortcut* Pad::isInShortcut(const MTPoint& touch, BoxPart::Value& boxpart) {
  Shortcut* s = nullptr;
  boxpart = BoxPart::Outside;
  ShortcutMenu* menu = mp.currentMenu();

  // d'abord le label car il peut etre contenu dans la boite ou une autre boite
  if (menu) {
    for (auto& it : menu->shortcuts()) {
      if ((boxpart = isInTitle(touch, *it)) != BoxPart::Outside) {
        s = it;
        break;
      }
    }
  }
  
  // puis chercher dans le dernier selectionne s'il l'est toujours
  if (!s && mp.curShortcut_) {
    if ((boxpart = isInTitle(touch, *mp.curShortcut_)) != BoxPart::Outside
        || (boxpart = isInBox(touch, *mp.curShortcut_)) != BoxPart::Outside) {
      s = mp.curShortcut_;
    }
  }
  
  // puis chercher les boites (et les bords dans les boites)
  if (!s) {
    if (menu) {
      for (auto& it : menu->shortcuts()) {
        if ((boxpart = isInBox(touch, *it)) != BoxPart::Outside) {
          s = it;
          break;
        }
      }
    }
  }
  
  // puis chercher dans le menu opener s'il existe
  if (!s) {
    Shortcut* menuOpener = mp.currentMenuOpener();
    if (menuOpener) {
      if ((boxpart = isInTitle(touch, *menuOpener)) != BoxPart::Outside
          || (boxpart = isInBox(touch, *menuOpener)) != BoxPart::Outside
          ) {
        s = menuOpener;
      }
    }
  }

  return s;
}
