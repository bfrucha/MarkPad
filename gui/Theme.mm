//
//  Theme.cpp
//  MarkPad Project
//  (C) eric lecolinet 2017/2020.
//

#include <iostream>
#include "Conf.h"
#include "Theme.h"
#include "OverlayImpl.h"
using namespace std;

Theme::~Theme() {
  // leaked but we dont care !
}

void Theme::complete() {
  angleSymbols = {"➡️","↗️","⬆️","↖️","⬅️","↙️","⬇️","↘️"};
  countSymbols = {"0️⃣","1️⃣","2️⃣","3️⃣","4️⃣","5️⃣"};

  menuBtnUp = AWL::newImage(Conf::imageDir()+"menuButtonUp.png");
  menuBtnDown = AWL::newImage(Conf::imageDir()+"menuButtonDown.png");
  bgImage = AWL::newImage(Conf::confDir()+Conf::k.bgImage);

  fingerThickness = 3;
  boxThickness = 1;
  linkThickness = 1;
  
  if (!titleFont) titleFont = newFont("Helvetica-Bold", 16);
  if (!infoFont) infoFont = newFont("Helvetica-Bold", 36);
  if (!titleColor) titleColor = AWL::BlackColor;
  if (!showInfoColor) showInfoColor = AWL::GrayColor;
  if (!editInfoColor) editInfoColor = AWL::GrayColor;
  titleAttr = new TextAttr(titleFont, titleColor, AWL::ClearColor);
  showInfoAttr = new TextAttr(infoFont, showInfoColor, AWL::ClearColor);
  editInfoAttr = new TextAttr(infoFont, editInfoColor, AWL::ClearColor);

  if (!overlayShowBg) overlayShowBg = newColor(0.05, 0., 0.,0.1);
  if (!overlayEditBg) overlayEditBg = newColor(0.2, 0.2, 0.2);
  if (!gridColor) gridColor = AWL::BlueColor;;
  if (!borderColor) borderColor = AWL::BlueColor;
  if (!fingerColor) fingerColor = OrangeColor;
  
  selectionColor = newColor(0.9, 0.5, 0.0); // orange
  selectionBg = newColor(0.9, 0.5, 0.0, 0.25);
  handleColor = AWL::BlackColor;
  handleBg = AWL::WhiteColor;
  activeBorderColor = newColor(1., 0.6, 0., 0.45);
}


void BoxTheme::complete(Theme* theme) {
  if (!boxColor) boxColor = AWL::BlackColor;
  if (!boxBg) boxBg = AWL::ClearColor;
  if (!titleColor) titleColor = AWL::BlackColor;
  if (!titleBg) titleBg = AWL::ClearColor;
  if (!titleFont) titleFont = theme->titleFont;
  if (!linkColor) linkColor = AWL::BlackColor;
  titleAttr = new TextAttr(titleFont, titleColor, titleBg);
}


class DarkTheme : public Theme {
public:
  DarkTheme() {
    overlayShowBg = newColor(0.05, 0., 0.,0.1);
    overlayEditBg = newColor(0.2, 0.2, 0.2);
    editInfoColor = AWL::WhiteColor;
    complete();  // must be called before filling boxThemes
    
    { auto& bt = show;
      bt.drawn = DrawTitle;
      bt.boxColor = AWL::ClearColor;
      bt.boxBg = AWL::ClearColor;
      bt.linkColor = AWL::GrayColor;
      bt.titleColor = AWL::WhiteColor;
      bt.titleBg = newColor(0.2, 0.2, 0.2);
      bt.complete(this);
    }
    
    showSelected = show;
    showSelected.linkColor = AWL::RedColor;
    showSelected.titleBg = AWL::RedColor;
    showSelected.complete(this);

    { auto& bt = activeBorders;
      bt.drawn = DrawSolid;
      bt.boxColor = activeBorderColor;
      bt.boxBg = AWL::ClearColor;
      bt.linkColor = AWL::ClearColor;
      bt.titleColor = activeBorderColor;
      bt.titleBg = AWL::ClearColor;
      bt.complete(this);
    }
    
    { auto& bt = edit;
      bt.drawn = DrawTitle | DrawSolid |DrawFilled;
      bt.boxColor = AWL::BlackColor;
      bt.boxBg = newColor(0.85, 0.85, 0.85, 0.25);
      bt.linkColor = AWL::WhiteColor;
      bt.titleColor = AWL::WhiteColor;
      bt.titleBg = newColor(0.2, 0.2, 0.2);
      bt.complete(this);
    }
    
    { auto& bt = editSelected;
      bt = edit;
      bt.boxColor = AWL::RedColor;
      bt.linkColor = AWL::RedColor;
      bt.titleBg = AWL::RedColor;
      bt.complete(this);
    }
    
    editParent = editSelected;
    editParent.boxColor = AWL::BlueColor;
    editParent.linkColor = AWL::RedColor;
    editParent.titleBg = AWL::BlueColor;
    editParent.complete(this);
    
    editCreate = edit;
    editCreate.boxBg = newColor(0.15, 0.15, 0.15, 0.75);
    editCreate.complete(this);
    
    guides.drawn = DrawDashed;
    //guides.boxColor = newColor(0.5, 0.5, 0.5, 0.5);
    guides.boxColor = newColor(0.85, 0.85, 0.85, 0.8);
    guides.boxBg = AWL::ClearColor;
    guides.linkColor = AWL::ClearColor;
    guides.titleColor = AWL::ClearColor;
    guides.titleBg = AWL::ClearColor;
    guides.complete(this);

    guidesCreate = guides;
    guidesCreate.drawn = DrawDashed | DrawFilled;
    guidesCreate.boxBg = OrangeColor;
    guidesCreate.complete(this);

    // NB: Draw::Title needed to draw icon
    guidesEdit.drawn = DrawTitle | DrawDashed;
    guidesEdit.boxColor = AWL::GreenColor;
    guidesEdit.boxBg = AWL::ClearColor;
    guidesEdit.linkColor = AWL::WhiteColor;
    guidesEdit.titleColor = AWL::ClearColor;
    guidesEdit.titleBg = AWL::ClearColor;
    guidesEdit.complete(this);

    guidesEditSelected = guidesEdit;
    guidesEditSelected.drawn = DrawTitle | DrawSolid;
    guidesEditSelected.boxColor = AWL::RedColor;
    guidesEditSelected.linkColor = AWL::RedColor;
    guidesEditSelected.complete(this);
  }
};


class LightTheme : public Theme {
public:
  LightTheme() {
    overlayShowBg = newColor(0.05, 0., 0.,0.1);
    overlayEditBg = AWL::WhiteColor;
    editInfoColor = AWL::BlackColor;
    complete();
    
    show.drawn = DrawTitle;
    show.boxColor = AWL::ClearColor;
    show.boxBg = AWL::ClearColor;
    show.linkColor = AWL::GrayColor;
    show.titleColor = AWL::WhiteColor;
    show.titleBg = newColor(0.2, 0.2, 0.2);
    show.complete(this);

    showSelected = show;
    showSelected.linkColor = AWL::RedColor;
    showSelected.titleBg = AWL::RedColor;
    showSelected.complete(this);

    { auto& bt = activeBorders;
      bt.drawn = DrawSolid;
      bt.boxColor = activeBorderColor;
      bt.boxBg = AWL::ClearColor;
      bt.linkColor = AWL::ClearColor;
      bt.titleColor = activeBorderColor;
      bt.titleBg = AWL::ClearColor;
      bt.complete(this);
    }
    
    edit.drawn = DrawTitle | DrawSolid | DrawFilled;
    edit.boxColor = AWL::BlackColor;
    edit.boxBg = newColor(0.72, 0.70, 0.70, 0.75);
    edit.linkColor = AWL::BlackColor;
    edit.titleColor = AWL::WhiteColor;
    edit.titleBg = newColor(0.2, 0.2, 0.2);
    edit.complete(this);

    editSelected = edit;
    editSelected.boxColor = AWL::RedColor;
    editSelected.linkColor = AWL::RedColor;
    editSelected.titleBg = AWL::RedColor;
    editSelected.complete(this);

    editParent = edit;
    editParent.boxColor = AWL::BlueColor;
    editParent.linkColor = AWL::RedColor;
    editParent.titleBg = AWL::BlueColor;
    editParent.complete(this);

    editCreate = edit;
    editCreate.boxBg = newColor(0.15, 0.15, 0.15, 0.75);
    editCreate.complete(this);
    
    guides.drawn = DrawDashed;
    guides.boxColor = newColor(0.4, 0.4, 0.4);
    guides.boxBg = AWL::ClearColor;
    guides.linkColor = AWL::ClearColor;
    guides.titleColor = AWL::ClearColor;
    guides.titleBg = AWL::ClearColor;
    guides.complete(this);

    guidesCreate = guides;
    guidesCreate.drawn = DrawDashed | DrawFilled;
    guidesCreate.boxBg = OrangeColor; // newColor(0.8, 0.4, 0.0, 0.5);  //orange
    guidesCreate.complete(this);

    guidesEdit.drawn = DrawTitle | DrawDashed;
    guidesEdit.boxColor = newColor(0., 0.6, 0); // greeen
    guidesEdit.boxBg = AWL::ClearColor;
    guidesEdit.linkColor = AWL::WhiteColor;
    guidesEdit.titleColor = AWL::ClearColor;
    guidesEdit.titleBg = AWL::ClearColor;
    guidesEdit.complete(this);

    guidesEditSelected = guidesEdit;
    guidesEditSelected.drawn = DrawTitle | DrawSolid;
    guidesEditSelected.boxColor = AWL::RedColor;
    guidesEditSelected.linkColor = AWL::RedColor;
    guidesEditSelected.complete(this);
  }
};

class TranspTheme : public Theme {
public:
  TranspTheme() {
    overlayShowBg = newColor(0.05, 0., 0.,0.1);
    overlayEditBg = newColor(0.05, 0., 0., 0.4);
    editInfoColor = AWL::WhiteColor;
    complete();
    
    show.drawn = DrawTitle;
    show.boxColor = AWL::ClearColor;
    show.boxBg = AWL::ClearColor;
    show.linkColor = AWL::GrayColor;
    show.titleColor = AWL::WhiteColor;
    show.titleBg = newColor(0.2, 0.2, 0.2);
    show.complete(this);

    showSelected = show;
    showSelected.linkColor = AWL::RedColor;
    showSelected.titleBg = AWL::RedColor;
    showSelected.complete(this);

    { auto& bt = activeBorders;
      bt.drawn = DrawSolid;
      bt.boxColor = activeBorderColor;
      bt.boxBg = AWL::ClearColor;
      bt.linkColor = AWL::ClearColor;
      bt.titleColor = activeBorderColor;
      bt.titleBg = AWL::ClearColor;
      bt.complete(this);
    }
    
    edit.drawn = DrawTitle | DrawSolid | DrawFilled;
    edit.boxColor = AWL::BlackColor;
    edit.boxBg = newColor(0.1, 0.1, 0.1, 0.6);
    edit.linkColor = AWL::WhiteColor;
    edit.titleColor = AWL::WhiteColor;
    edit.titleBg = newColor(0.2, 0.2, 0.2);
    edit.complete(this);

    editSelected = edit;
    editSelected.boxColor = AWL::RedColor;
    editSelected.linkColor = AWL::RedColor;
    editSelected.titleBg = AWL::RedColor;
    editSelected.complete(this);
    
    editParent = edit;
    editParent.boxColor = AWL::BlueColor;
    editParent.linkColor = AWL::RedColor;
    editParent.titleBg = AWL::BlueColor;
    editParent.complete(this);

    editCreate = edit;
    editCreate.boxBg = newColor(0.15, 0.15, 0.15, 0.75);
    editCreate.complete(this);
    
    guides.drawn = DrawDashed;
    guides.boxColor = newColor(0.5, 0.5, 0.5, 0.7);
    guides.boxBg = AWL::ClearColor;
    guides.linkColor = AWL::ClearColor;
    guides.titleColor = AWL::ClearColor;
    guides.titleBg = AWL::ClearColor;
    guides.complete(this);

    guidesCreate = guides;
    guidesCreate.drawn = DrawDashed | DrawFilled;
    guidesCreate.boxBg = OrangeColor;
    guidesCreate.complete(this);
    
    guidesEdit.drawn = DrawTitle | DrawDashed;
    guidesEdit.boxColor = AWL::GreenColor;
    guidesEdit.boxBg = AWL::ClearColor;
    guidesEdit.linkColor = AWL::WhiteColor;
    guidesEdit.titleColor = AWL::ClearColor;
    guidesEdit.titleBg = AWL::ClearColor;
    guidesEdit.complete(this);

    guidesEditSelected = guidesEdit;
    guidesEditSelected.drawn = DrawTitle | DrawSolid;
    guidesEditSelected.boxColor = AWL::RedColor;
    guidesEditSelected.linkColor = AWL::RedColor;
    guidesEditSelected.complete(this);
  }
};

Theme* Theme::getTheme(const std::string& name) {
  if (name == "light") {
    static Theme* theme = new LightTheme;
    return theme;
  }
  else if (name == "transparent") {
    static Theme* theme = new TranspTheme;
    return theme;
  }
  else {
    static Theme* theme = new DarkTheme;
    return theme;
  }
}
