//
//  Theme.h
//  MarkPad Project
//  (C) eric lecolinet 2018.
//

#ifndef Theme_hpp
#define Theme_hpp
#include "AWL.h"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

struct ThemeDraw {
  enum {DrawNone=0, DrawTitle=1<<1, DrawSolid=1<<2, DrawDashed=1<<3, DrawFilled=1<<4};
};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

class BoxTheme : public ThemeDraw {
public:
  void complete(class Theme*); // completes titleAttr and missing components;

  unsigned int drawn{0};  // drawn components
  int boxThickness{1}, linkThickness{1};
  AWColor *boxColor{}, *boxBg{}, *titleColor{}, *titleBg{}, *linkColor{};
  AWFont  *titleFont{};
  class TextAttr* titleAttr{}; // computed from font and color by complete()
};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

class Theme : public AWL, public ThemeDraw {
public:
  ~Theme();

  static Theme* getTheme(const std::string& name);
  void complete(); // completes titleAttr and missing components;

  std::vector<std::string> angleSymbols, countSymbols;
  AWImage *menuBtnUp{}, *menuBtnDown{}, *bgImage{};
  
  AWFont *titleFont{}, *infoFont{};
  AWColor *titleColor{}, *showInfoColor{}, *editInfoColor{},
  *selectionColor{}, *selectionBg{}, *handleColor{}, *handleBg{},
  *activeBorderColor{};
  class TextAttr *titleAttr{}, *showInfoAttr{}, *editInfoAttr{};

  int gridCount{4}, gridThickness{1}, borderThickness{1}, fingerThickness{3},
  boxThickness{1}, linkThickness{1};
  AWColor *overlayShowBg{}, *overlayEditBg{}, *gridColor{}, *borderColor{}, *fingerColor{};
  
  BoxTheme show{}, showSelected{}, edit{}, editSelected{}, editParent{}, editCreate{},
  guides{}, guidesEdit{}, guidesEditSelected{}, guidesCreate{}, activeBorders{};
};

#endif
