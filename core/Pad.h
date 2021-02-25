//
//  Pad.h
//  MarkPad Project
//
//  (c) Eric Lecolinet - http://www.telecom-paristech.fr/~elc
//  (c) Bruno Fruchard - http://brunofruchard.com/
//  Copyright (c) 2017/2020. All rights reserved.
//

#ifndef MarkPad_Pad
#define MarkPad_Pad

#import <string>
#import <cmath>
#import "MTouch.h"
#import "Shortcut.h"
using namespace std;

class DataLogger;

/// Trackpad configuration.
class PadConf {
public:
  PadConf(float padWidth, float padHeight);
  const float padWidth, padHeight, padRatio, padRatio2;
};

/** A physical Trackpad.
 * Note that MarkPad can support multiple trackpads.
 *
 * IMPORTANT: les coordonnées X et Y sont normalisée entre 0. et 1. quelle que
 * soit la taille réelle du trackpad dans les deux directions.
 *
 * => tous les calculs d'ANGLES et de DISTANCES doivent tenir compte du "padRatio"
 *    (padRatio = padHeight/padWidth) sous peine d'etre complètement faux !!!
 *
 *  => les methodes getAngle() et distance() le font automatiquement
 */
class Pad : public PadConf {
  friend class MarkPad;
  friend class ShortcutMenu;
public:
  /// creates a new pad.
  Pad(MTDevice*, const PadConf&);
  
  /// returns the device corresponding to this Pad.
  MTDevice* device() const {return device_;}
  
  DataLogger& dataLogger() const {return dataLogger_;}
  
  /// returns the main menu of this pad.
  ShortcutMenu* mainMenu() const {return mainMenu_;}

  void initActiveBorders();
  void updateOverlay();

  /// maximum number of simultanous Touches.
  static const int MaxTouchCount = 10;
  
  const MTTouch** touches() {return touches_;}
  unsigned int touchCount() const {return touchcount_;}

  void touchCallback(const MTTouch* touches, int touchCount);
  
  /// Cancels the current touch gesture; closes the menu if _closeMenu_ is true.
  void cancelTouchGesture(bool closeMenu);
  
  Shortcut* isInShortcut(const MTPoint& pos, BoxPart::Value&);
  
  BoxPart::Value isInBox(const MTPoint& pos, Shortcut&) const;
  
  BoxPart::Value isInTitle(const MTPoint& pos, Shortcut&) const;
  
  float distance2(const MTPoint& p1, const MTPoint& p2) const {
    // NB: takes into account padRatio to get correct distances.
    return (p1.x - p2.x) * (p1.x - p2.x)
    + (p1.y - p2.y) * (p1.y - p2.y) * padRatio2;
  }
  
  static bool isClose(const MTPoint& pos, float x, float y, float xtol, float ytol) {
    return pos.x >= x-xtol && pos.x <= x+xtol
    && pos.y >= y-ytol && pos.y <= y+ytol;
  }
  
  static bool isInside(const MTPoint& pos, const MTRect& rect) {
    return pos.x >= rect.x && pos.x <= rect.x + rect.width
    && pos.y >= rect.y && pos.y <= rect.y + rect.height;
  }
  
  static bool isInside(const MTPoint& pos, const MTRect& rect, float xtol, float ytol) {
    return pos.x >= rect.x-xtol && pos.x <= rect.x + rect.width + xtol
    && pos.y >= rect.y-ytol && pos.y <= rect.y + rect.height + ytol;
  }
  
  static bool isInside(const MTRect& r, const MTRect& rect, float xtol, float ytol) {
    return r.x >= rect.x-xtol && r.x+r.width <= rect.x+rect.width+xtol
    && r.y >= rect.y-ytol && r.y+r.height <= rect.y+rect.height+ytol;
  }
  
  int16_t getAngle(const MTTouch& p1, const MTTouch& p2) const {
    float dx = p2.norm.pos.x - p1.norm.pos.x;
    // NB: takes into account padRatio to get correct angles.
    float dy = (p2.norm.pos.y - p1.norm.pos.y) / padRatio;
    double val = std::atan(dy / dx) * 180 / M_PI;
    if (dx < 0) val += 180; else if (dy < 0) val += 360;
    return int16_t(val);
  }
  
  static int16_t diffAngle(int16_t a1, int16_t a2) {
    int d1 = a2 - a1;
    int d2 = 0;
    if (d1 < 0) {d2 = -d1; d1 = 360+d1;} else d2 = 360-d1;
    return int16_t(d1 < d2 ? d1 : d2);
  }
  
private:
  Pad(const Pad&) = delete;             ///< Pad can't be copied.
  Pad& operator=(const Pad&) = delete;  ///< Pad can't be copied.
  MTDevice       *device_{};
  int            touchcount_{0};
  const MTTouch  *touches_[MaxTouchCount];
  ShortcutMenu   *mainMenu_{}, *activeBorders_{};
  Shortcut       *leftBorder_{}, *rightBorder_{}, *topBorder_{}, *bottomBorder_{};
  bool           validTouch_{false}, unvalidTouch_{false}, cancelled_{false};
  MTPoint        touch1_{};
  int32_t        touch1Id_{-1};
  double         touchTime_{0.};
  MTTouch        selTouch_{};
  Shortcut       *opener_{}, *superopener_{};
  DataLogger     &dataLogger_;
};

#endif
