//
//  DataLogger.h
//  MarkPad
//
//  (c) Eric Lecolinet - http://www.telecom-paristech.fr/~elc
//  (c) Bruno Fruchard - http://brunofruchard.com/
//  Copyright (c) 2017/18. All rights reserved.
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


class GestureLog {
private:
  int _gid;
  std::string _timestamp;
  std::chrono::time_point<std::chrono::high_resolution_clock> _startTime;
  std::vector<std::string> _trace; // information is saved like this: time x y
  std::string _start, _end;
  
public:
  GestureLog();
  void start(const MTPoint&, const std::string&, Shortcut*);
  void recordMove(const MTPoint&, bool);
  void end(const MTPoint&, bool, Shortcut*);
  std::string toString();
  friend std::ofstream& operator<<(std::ofstream&, const GestureLog&);
};


class DataLogger {
private:
  DataLogger(const Pad&);
  
  // all file paths to record data
  const std::string _dirLogPath, _currentConfPath, _confLogPath, _arcConfPath,
                    _currentGuidesPath, _guidesLogPath, _arcGuidesPath,
                    _gestLogPath, _arcGestPath, _attrLogPath;
  const unsigned int _maxFileSize;
  
  std::shared_ptr<GestureLog> _currentGesture;
  std::vector<std::shared_ptr<GestureLog>> _gestures;
  std::map<std::string, std::string> _attributes; // logger attributes (file saved and pad dimension)
  
  void addFileToArchive(const std::string&, const std::string&);
  
public:
  static std::unique_ptr<DataLogger> instance;
  
  static DataLogger* obtain(const Pad&);
  void recordGestureMove(const MTPoint&, bool);
  void startGesture(const MTPoint&, Shortcut*);
  void endGesture(const MTPoint&, bool, Shortcut*);
  bool needToSaveConfiguration(const std::string&);
  void saveConfigurationChanges(const std::string&);
  bool needToSaveGuides(const std::string&);
  void saveGuidesChanges(const std::string&);
  bool needToSaveGestures();
  void saveGesturesPerformed();
  void saveAttributes();
};

#endif
