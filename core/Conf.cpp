//
//  Conf.cpp: configuration file
//  MarkPad Project
//
//  (c) Eric Lecolinet - http://www.telecom-paristech.fr/~elc
//  (c) Bruno Fruchard - http://brunofruchard.com/
//  Copyright (c) 2017/18. All rights reserved.
// 

#include <cstdio>
#include <iostream>
#include <locale>
#include <clocale>
#include <vector>
#include <fstream>
#include <sstream>
#include "ccuty/ccstring.hpp"
#include "jsonserial/jsonserial.hpp"
#include "jsonserial/map.hpp"
#include "jsonserial/list.hpp"
#include "jsonserial/vector.hpp"
#include "Conf.h"
#include "GUI.h"
#include "MarkPad.h"
#include "Pad.h"
#include "Actions.h"
#include "Services.h"
#include "DataLogger.h"
using namespace ccuty;
 
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

class ConfImpl : public JsonClasses {
  friend class Conf;
  friend class CConf;
  static std::istringstream iss;
  static std::ostringstream oss;

public:
  static ConfImpl impl;
  
  static void readAction(Shortcut& s, JsonSerial&, const string& val) {
    if (val.empty()) return;
    else if (val[0]=='!') {
      Actions::instance.setShortcutActionFromConfFile(s, "!", val.substr(1));
    }
    else {
      string keyword, args;
      strsplit(keyword, args, val, strdelim().spaces());
      Actions::instance.setShortcutActionFromConfFile(s, keyword, args);
    }
  }
  
  static void writeAction(const Shortcut& s, JsonSerial& js) {
    const Action* a = s.action();
    if (a && !s.submenu_) {
      if (!a->isHotkey() || s.modifiers() == 0) {
        js.writeMember("!" + s.commandName() + " " + s.arg());
      }
      else {
        string mod_string;
        Services::modifiersToModString(s.modifiers(), mod_string);
        js.writeMember("!" + s.commandName() + " " + mod_string + s.arg());
      }
    }
  }
  
  static void readModes(Shortcut& s, JsonSerial& js, const string& val) {
    if (!val.empty()) {
      if (val == "DontStartFromBorder") s.touchFromBorder_ = s.touchOpenMenu_ = false;
      else if (val == "DontOpenMenu") s.touchOpenMenu_ = false;
    }
  }
  
  static void writeModes(const Shortcut& s, JsonSerial& js) {
    if (!s.touchOpenMenu_ || !s.touchFromBorder_) {
      std::string val;
      if (!s.touchFromBorder_) val = "DontStartFromBorder";
      else if (!s.touchOpenMenu_) val = "DontOpenMenu";
      js.writeMember(val);
    }
  }
  
  static void readFeedback(Shortcut& s, JsonSerial& js, const string& val) {
    if (!val.empty()) js.readMember(s.feedback_, val);
  }
  
  static void writeFeedback(const Shortcut& s, JsonSerial& js) {
    if (s.feedback_) js.writeMember(*s.feedback_);
  }
  
  static void readSubmenu(Shortcut& s, JsonSerial& js, const string& val) {
    if (!val.empty()) js.readMember(s.submenu_, val);
  }
  
  static void writeSubmenu(const Shortcut& s, JsonSerial& js) {
    if (s.submenu_) js.writeMember(s.submenu_);
  }
  
  static void readArea(Shortcut& s, JsonSerial&, const string& val) {
    iss.clear();
    iss.str(val);
    iss >> s.area_.x >> s.area_.y >> s.area_.width >> s.area_.height;
  }
  
  static void readNameArea(Shortcut& s, JsonSerial&, const string& val) {
    iss.clear();
    iss.str(val);
    iss >> s.namearea_.x >> s.namearea_.y;
  }
  
  static void writeArea(const Shortcut& s, JsonSerial& js) {
    oss.clear();
    oss.str("");
    oss << s.area_.x <<' '<< s.area_.y <<' '<< s.area_.width <<' '<< s.area_.height;
    js.writeMember(oss.str());
  }
  
  static void writeNameArea(const Shortcut& s, JsonSerial& js) {
    oss.clear();
    oss.str("");
    oss << (s.namearea_.x - s.area_.x) <<' '<< (s.namearea_.y - s.area_.y);
    js.writeMember(oss.str());
  }
  
  static void completeShortcut(Shortcut& s) {
    s.namearea_.x += s.area_.x;
    s.namearea_.y += s.area_.y;
    if (s.submenu_) s.submenu_->opener_ = &s;
  }
  
  // - - - - - -
  
  ConfImpl() {
    /* test: changes the locale to fr_FR.UTF-8 to see if still working properly:

    std::locale::global(std::locale("fr_FR.UTF-8"));
    locale test;
    cout << "Locale: "<< test.name() << endl;
    std::cout.imbue(test);
    std::cout << 1000.01 << "\n\n";
    */
    
    // iss and oss must use the C locale otherwise 1000.01 would be written/read 1000,01
    iss.imbue(std::locale::classic());
    oss.imbue(std::locale::classic());
    oss .precision(4);
    oss << std::fixed;
    
    defclass<Conf>("Conf", nullptr)   // this class cannot be instancied directly
    .member("fileVersion", &Conf::fileVersion)
    
    .member("developerMode", &Conf::developerMode)
    .member("allowAccessibility", &Conf::allowAccessibility)
    .member("logData", &Conf::logData)
    .member("showHotkey", &Conf::showHotkey)
    .member("editHotkey", &Conf::editHotkey)
    .member("theme", &Conf::theme)
    .member("mediaHost", &Conf::mediaHost)

    .member("activeBorders", &Conf::activeBorders)
    .member("menuDelay", &Conf::menuDelay)
    .member("minMovement", &Conf::minMovement)
    .member("minTouchSize", &Conf::minTouchSize)
    .member("maxTouchSize", &Conf::maxTouchSize)

    .member("showBgImage", &Conf::showBgImage)
    .member("bgImage", &Conf::bgImage)
    .member("showFeedback", &Conf::showFeedback)
    .member("feedback", &Conf::feedback)
    .member("showFingers", &Conf::showFingers)
    .member("showGrid", &Conf::showGrid)

    .member("mainMenu", &Conf::mainMenu_);

    defclass<CConf>("CConf")
    .extends<Conf>();   // derives from CConf
    
    defclass<ShortcutMenu>("ShortcutMenu")
    .member("shortcuts", &ShortcutMenu::shortcuts_)
    .postread([](ShortcutMenu& menu) {   // done after reading children
      for (auto s : menu.shortcuts()) s->setParentMenu(&menu);
    });

    defclass<Shortcut>("Shortcut")
    .member("name", &Shortcut::name_)
    .member("modes", readModes, writeModes)
    .member("action", readAction, writeAction)
    .member("feedback", readFeedback, writeFeedback)
    .member("area", readArea, writeArea)
    .member("nameArea", readNameArea, writeNameArea)
    .member("submenu", readSubmenu, writeSubmenu)
    .postread(completeShortcut);
    
    defclass<Conf::Feedback>("Feedback")
    .member("delay", &Conf::Feedback::delay)
    .member("font", &Conf::Feedback::font)
    .member("color", &Conf::Feedback::color);
    
    defclass<Conf::ActiveBorder>("ActiveBorder")
    .member("left", &Conf::ActiveBorder::left)
    .member("right", &Conf::ActiveBorder::right)
    .member("top", &Conf::ActiveBorder::top)
    .member("bottom", &Conf::ActiveBorder::bottom);
  }
};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// order matters! (because oss and iss are static variables)
std::istringstream ConfImpl::iss;
std::ostringstream ConfImpl::oss;
ConfImpl ConfImpl::impl;

CConf Conf::instance;  // Conf singleton.
const Conf& Conf::k = CConf::instance;  // read-only access to Conf singleton.

const std::string& Conf::confDir() {return instance.confdir;}
const std::string& Conf::resourceDir() {return instance.resdir;}
const std::string& Conf::execPath() {return instance.execpath;}

std::string Conf::imageDir() {return instance.resdir + "images/";}
std::string Conf::scriptDir() {return instance.resdir + "scripts/";}
std::string Conf::bufferDir() {return instance.confdir + "buffers/";}
std::string Conf::shortcutFile() {return instance.confdir + "Shortcuts.json";}
std::string Conf::guideFile() {return instance.confdir + "Guides.json";}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

static bool copyFile(const std::string& from, const std::string& to) {
  // note: <filesystem> provides standard functions in C++17
  std::ifstream src(from);
  std::ofstream dst(to);
  if (!src || !dst) return false;
  char c;
  while (src.get(c)) dst << c;
  return true;
}

static bool fileExits(const std::string& path) {
  return std::ifstream(path).good();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void CConf::read() {
  ostringstream errors;
  bool fatal = false;
  std::string confFile = Conf::shortcutFile();
  std::string guideFile = Conf::guideFile();
  //setlocale(LC_NUMERIC, "C");  // (should not be needed now)

  JsonSerial js(ConfImpl::impl, [&](const JsonError& e){
    e.print(errors); errors<<"\n";
    if (e.fatal) fatal = true;
  });
  
  if (!js.read(*this, confFile)) {  // in case of an error
    if (fatal) {
      GUI::alert("Error while reading configuration","In file: "
                 +confFile+"\n" + errors.str());
      copyFile(confFile, confFile+"-bug");
    }
    else {
      MarkPad::warning("while reading configuration\nIn file: "
                        + confFile+"\n"+errors.str());
    }
    mustSave();
  }

  // compatibility
  for (ShortcutMenu* m : CConf::instance.menus_) {
    if (m->isMainMenu()) mainMenu_ = m;
  }
  
  errors.clear();
  
  // read guide file
  if (!js.read(MarkPad::instance.guide_, guideFile)) {
    GUI::alert("Error while reading Guides","In file: "+guideFile+"\n" + errors.str(), "OK");
  }
  
  if (MarkPad::instance.guide_) {
    // guide must be a main menu otherwise we'll face incoherencies
    MarkPad::instance.guide_->setMainMenu();
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void Conf::mustSave() {
  instance.changed_ = true;
}

void Conf::saveIfNeeded() {
  if (instance.changed_) {
    instance.changed_ = false;
    instance.write();
  }
  if (Conf::k.logData) {
    MarkPad::instance.currentPad()->dataLogger().saveGestures();
  }
}

void Conf::write() {
  std::string confFile = Conf::shortcutFile();
  std::string guideFile = Conf::guideFile();
  
  if (!previousConfSaved_) {  // save previous configuration file first
    previousConfSaved_ = true;
    ifstream f(confFile);
    if (f) {
      copyFile(confFile, confFile+"~");
      f.close();
    }
  }
  
  ofstream fout(confFile);
  if (!fout) {
    // alert() will appear below the overlay if the editor is opened!
    MarkPad::instance.edit(MarkPad::EditNone);
    GUI::alert("Can't save configuration","Can't open file: "+confFile);
    return;
  }
  
  fout.precision(4); // 4 digits after decimal for floats
  fout << std::fixed;
  fout << "// MarkPad Configuration File\n"
  << "// Must be located in ~/Library/MarkPad/\n" << endl;
  
  ostringstream errors;
  JsonSerial js(ConfImpl::impl, [&errors](const JsonError& e){e.print(errors); errors<<"\n";});
  
  if (!js.write(*this, fout)) {
    MarkPad::instance.edit(MarkPad::EditNone);
    GUI::alert("Could not save configuration","Error in file: "+confFile +"\n" + errors.str());
  }
  
  // save Guides.json if it was edited
  if (MarkPad::instance.guideChanged_) {
    errors.clear();    
    if (!js.write(MarkPad::instance.guide(), guideFile)) {
      MarkPad::instance.edit(MarkPad::EditNone);
      GUI::alert("Guides could not be saved","Error in file: "+guideFile+"\n" + errors.str());
    }
  }
  
  // saving all data if needed
  if (Conf::k.logData) {
    auto& logger = MarkPad::instance.currentPad()->dataLogger();
    if (logger.needToSaveConfiguration(confFile)) logger.saveConfigurationChanges(confFile);
    if (logger.needToSaveGuides(guideFile)) logger.saveGuidesChanges(guideFile);
    /*if (logger.needToSaveGestures())*/ logger.saveGestures();
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// creates the directory that contains the configuration file.
// If dir does not exist, create dir and copy config files.
void Conf::init(const std::string& execpath_)
{
  execpath = execpath_;
  confdir = Services::getUserConfDir();
  resdir = Services::getResourceDir();
  
  if (!fileExits(resdir)) {
    // note: can't call alert() at this stage because AWL is not yet init
    MarkPad::warning("Application bundle could not be found");
  }
  else {
    if (!fileExits(imageDir()))
      MarkPad::warning("Images could not be found in Application bundle");

    if (!fileExits(scriptDir()))
      MarkPad::warning("Scripts could not be found in Application bundle");
  }

  // create conf dir if needed
  if (!fileExits(confdir)) {
    cout << "Creating MarkPad directory: " << confdir << endl;
    system("mkdir " + confdir);
    system("mkdir " + bufferDir());
  }

  if (!fileExits(confdir+"Shortcuts.json")) {
    if (!copyFile(resdir+"Shortcuts.json", confdir+"Shortcuts.json"))
      MarkPad::warning("Shortcuts.json could not be copied");
  }

  if (!fileExits(confdir+"Guides.json")) {
    if (!copyFile(resdir+"Guides.json", confdir+"Guides.json"))
      MarkPad::warning("Guides.json could not be copied");
  }

  if (!fileExits(confdir+"bgimage.png")) {
    if (!copyFile(resdir+"bgimage.png", confdir+"bgimage.png"))
      MarkPad::warning("bgimage.png could not be copied");
  }

  if (!fileExits(confdir+"/configs")) {
    system("cp -r " + resdir+"configs " + confdir);
  }

}
