//
//  DataLogger.h
//  MarkPad
//
//  (c) Eric Lecolinet - http://www.telecom-paristech.fr/~elc
//  (c) Bruno Fruchard - http://brunofruchard.com/
//  Copyright (c) 2017/2020. All rights reserved.
//
//  Logger used to record interactions form users. This logs data while the user is customizing
//  gestures, but also when a gesture is being performed. Data logging can be disabled in the settings menu.
//

#ifndef MarkPad_DataLogger
#define MarkPad_DataLogger

#include <string>
#include <array>
#include <vector>
#include <map>
#include <cstdio>
#include <iostream>
#include <memory>
#include <fstream>
#include <ctime>
#include <vector>
#include <chrono>
#include <algorithm>
#include <cstdlib>
#include <sstream>

#include "Conf.h"
#include "MarkPad.h"
#include "Pad.h"
#include "Shortcut.h"
#include "MTouch.h"

class DataLogger {
public:
  DataLogger(const Pad&);
  void startGesture(Shortcut*, const MTPoint&);
  void endGesture(Shortcut*, const MTPoint&, bool menushown);
  bool needToSaveConfiguration(const std::string&);
  void saveConfigurationChanges(const std::string&);
  bool needToSaveGuides(const std::string&);
  void saveGuidesChanges(const std::string&);
  void saveAttributes();
  void addFileToArchive(const std::string&, const std::string&);
  void saveGestures();

private:
  // all file paths to record data
  const std::string _dirLogPath, _currentConfPath, _confLogPath, _arcConfPath,
  _currentGuidesPath, _guidesLogPath, _arcGuidesPath,
  _gestLogPath, _arcGestPath, _attrLogPath;
  const unsigned int _maxFileSize;
  // logger attributes (file saved and pad dimension)
  std::map<std::string, std::string> attributes_;
  // for storing gestures:
  std::ofstream gestout_;
  unsigned long gestnum_{};
  std::string startname_;
  MTRect startrect_{};
};

#endif
