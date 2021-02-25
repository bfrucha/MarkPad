//
//  DataLogger.mm
//  MarkPad
//
//  (c) Eric Lecolinet - http://www.telecom-paristech.fr/~elc
//  (c) Bruno Fruchard - http://brunofruchard.com/
//  Copyright (c) 2017/2020. All rights reserved.
//

#include "ccuty/ccpath.hpp"
#include "DataLogger.h"
#include "Actions.h"
using ccuty::file_exists;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Data Logger Implementation

#define GESTURES_LOG_KEY "gesturesLogNb"
#define CONF_LOG_KEY "confLogNb"

//std::unique_ptr<DataLogger> DataLogger::instance(nullptr);

DataLogger::DataLogger(const Pad &pad):
_dirLogPath(Conf::confDir()+"data_log/"),
_currentConfPath(_dirLogPath+"current_shortcuts.json"),
_confLogPath(_dirLogPath+"configurations_log.xml"),
_arcConfPath(_dirLogPath+"configurations.tar.gz"),
_currentGuidesPath(_dirLogPath+"current_guides.json"),
_guidesLogPath(_dirLogPath+"guides_log.xml"),
_arcGuidesPath(_dirLogPath+"guides.tar.gz"),
_gestLogPath(_dirLogPath+"gestures_log.csv"),
_arcGestPath(_dirLogPath+"gestures.tar.gz"),
_attrLogPath(_dirLogPath+"logger_attributes.txt"),
_maxFileSize(2000000) {        // 2000000 = 2Mo max file size
  
  if (!file_exists(_dirLogPath)) { std::system(("mkdir "+_dirLogPath).c_str()); }
  
  if (!file_exists(_attrLogPath)) {
    
    auto attrFile = ofstream(_attrLogPath.c_str());
    if (attrFile.is_open()) {
      attributes_.emplace(GESTURES_LOG_KEY, "0");
      attrFile << GESTURES_LOG_KEY << ":0\n";
      attributes_.emplace(CONF_LOG_KEY, "0");
      attrFile << CONF_LOG_KEY << ":0\n";
      attributes_.emplace("padDimension", std           ::to_string(pad.padWidth)+","+std::to_string(pad.padHeight));
      attrFile << "padDimension:" << attributes_["padDimension"];
      attrFile.close();
    }
  } else {
    ifstream attrFile(_attrLogPath.c_str());
    if(attrFile.is_open()) {
      while(!attrFile.eof()) {
        std::string line; attrFile >> line;
        if(line == "") { continue; }
        unsigned long sep = line.find(':');
        attributes_.emplace(line.substr(0, sep), line.substr(sep+1));
      }
    }
    attrFile.close();
  }
}

bool DataLogger::needToSaveConfiguration(const std::string &filepath) {
  char buffer[24];
  std::string result;
  
  //TODO: remove hardcoding of configuration file
  if (!file_exists(_currentConfPath)) { return true; }
  
  std::string cmd = "diff "+filepath+" "+_currentConfPath;
  if (FILE* pipe = ::popen(cmd.c_str(), "r")) {
    while (!::feof(pipe)) {
      if (::fgets(buffer, sizeof(buffer), pipe) != nullptr) { result += buffer; }
    }
    ::pclose(pipe);
  }
  else { std::cerr << "fopen() failed!" << std::endl; return false; }
  return result!="";
}


void DataLogger::saveConfigurationChanges(const std::string &confFilePath) {
  std::string cmd = "cp "+confFilePath+" "+_currentConfPath;
  system(cmd.c_str());
  
  std::time_t now = std::time(0);
  char timestamp[64];
  std::strftime(timestamp, sizeof(timestamp), "%Y-%m-%dD%Hh%Mm%Ss%z", std::localtime(&now));
  
  // open file in writing mode
  if (!file_exists(_confLogPath)) { cmd = "touch "+_confLogPath; system(cmd.c_str()); }
  
  cmd = "echo \"<configuration date='"+std::string(timestamp)+"'>\n\" >> "+_confLogPath;
  system(cmd.c_str());
  cmd = "cat "+_currentConfPath+" >> "+_confLogPath;
  system(cmd.c_str());
  cmd = "echo \"</configuration>\n\" >> "+_confLogPath;
  system(cmd.c_str());
  
  ifstream file(_confLogPath, std::ios_base::ate | std::ios_base::binary);
  if(file.is_open()) {
    unsigned long size = file.tellg();
    if(_maxFileSize < size) {
      addFileToArchive(_confLogPath, _arcConfPath);
      attributes_[CONF_LOG_KEY] = std::to_string(stoi(attributes_[CONF_LOG_KEY])+1);
      saveAttributes();
    }
    file.close();
  }
}


bool DataLogger::needToSaveGuides(const std::string &filepath) {
  char buffer[24];
  std::string result;
  //TODO: remove hardcoding of configuration file
  if (!file_exists(_currentGuidesPath)) { return true; }
  
  std::string cmd = "diff "+filepath+" "+_currentGuidesPath;
  
  if (FILE* pipe = ::popen(cmd.c_str(), "r")) {
    while (!::feof(pipe)) {
      if (::fgets(buffer, sizeof(buffer), pipe) != nullptr) { result += buffer; }
    }
    ::pclose(pipe);
  }
  else { std::cout << "fopen() failed!" << std::endl; return false; }
  return result!="";
}


void DataLogger::saveGuidesChanges(const std::string &confFilePath) {
  std::string cmd = "cp "+confFilePath+" "+_currentGuidesPath;
  system(cmd.c_str());
  
  std::time_t now = std::time(0);
  char timestamp[64];
  std::strftime(timestamp, sizeof(timestamp), "%Y-%m-%dD%Hh%Mm%Ss%z", std::localtime(&now));
  
  // open file in writing mode
  if (!file_exists(_guidesLogPath)) { cmd = "touch "+_guidesLogPath; system(cmd.c_str()); }
  
  cmd = "echo \"<guide date='"+std::string(timestamp)+"'>\n\" >> "+_guidesLogPath;
  system(cmd.c_str());
  cmd = "cat "+_currentGuidesPath+" >> "+_guidesLogPath;
  system(cmd.c_str());
  cmd = "echo \"</guide>\n\" >> "+_guidesLogPath;
  system(cmd.c_str());
  
  ifstream file(_guidesLogPath, std::ios_base::ate | std::ios_base::binary);
  if(file.is_open()) {
    unsigned long size = file.tellg();
    if(_maxFileSize < size) {
      addFileToArchive(_guidesLogPath, _arcGuidesPath);
      attributes_[CONF_LOG_KEY] = std::to_string(stoi(attributes_[CONF_LOG_KEY])+1);
      saveAttributes();
    }
  }
}

void DataLogger::addFileToArchive(const std::string &filepath,
                                  const std::string &arcPath) {
  const std::string &tmpDir = _dirLogPath+"tmp/";
  std::system(("mkdir "+tmpDir).c_str());

  // create temporary file to change the name
  std::time_t now = std::time(0);
  char timestamp[64];
  std::strftime(timestamp, sizeof(timestamp), "%Y-%m-%dD%Hh%Mm%Ss%z", std::localtime(&now));
  
  // copy file to save with id equal to the current time
  std::string cmd = "cp "+filepath+" "+(tmpDir+timestamp); system(cmd.c_str());
  
  // create new archive if it does not exist
  if (!file_exists(arcPath)) {
    cmd = "tar czf "+arcPath+" -C "+tmpDir+" ."; std::system(cmd.c_str());
  }
  // save into archive
  else {
    cmd = "tar xf "+arcPath+" -C "+tmpDir; system(cmd.c_str());
    // [self execCommand:@"/usr/bin/tar" arguments:@[@"xf", arcpath, @"-C", tmpDir]];
    cmd = "tar czf "+arcPath+" -C "+tmpDir+" ."; system(cmd.c_str());
    // [self execCommand:@"/usr/bin/tar" arguments:@[@"czf", arcpath, @"-C", tmpDir, @"."]];
  }
  
  cmd = "rm -rf "+tmpDir; system(cmd.c_str());
  cmd = "rm -f "+filepath; system(cmd.c_str());
  cout << "file added to archive" << endl;
}

void DataLogger::saveAttributes() {
  ofstream output(_attrLogPath.c_str());
  if(output.is_open()) {
    for(auto it = attributes_.cbegin(); it != attributes_.cend(); it++) {
      output << it->first << ':' << it->second << '\n'; }
    output.close();
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Gesture Log Implementation

void DataLogger::startGesture(Shortcut *s, const MTPoint &coord) {
  if (!s) return;
  if (!gestout_.is_open()) {
    gestout_.open(_gestLogPath, std::ios_base::app);
    if (!gestout_) {
      std::cerr << "could not open gestures file ("+_gestLogPath+")";
    }
    else {
      gestout_.imbue(std::locale::classic());
      gestout_ .precision(3);
      gestout_ << std::fixed;
    }
  }
  startrect_ = s->area();
  startname_ = s->name();
}


void DataLogger::endGesture(Shortcut *s, const MTPoint &coord, bool menushown) {
  if (!s) return;

  Conf::mustSave();
  char day[100], hour[100];
  std::time_t now = std::time(0);
  std::strftime(day, sizeof(day), "%Y-%m-%d", std::localtime(&now));
  std::strftime(hour, sizeof(hour), "%H:%M:%S", std::localtime(&now));
  const MTRect &endrect = s->area();

  gestout_
  << "\n" << ++gestnum_ << ", "
  << day << ", " << hour << ", "
  << menushown << ", "
  << startname_ << ", "
  << s->name() << ", " << s->commandName() << ", "
  << startrect_.x << ", " << startrect_.y << ", "
  << startrect_.width << ", " << startrect_.height << ", => , "
  << endrect.x << ", " << endrect.y << ", "
  << endrect.width << ", " << endrect.height;
  gestout_.flush();
}


void DataLogger::saveGestures() {
  gestout_.flush();
  gestout_.close();
}

/*
 void DataLogger::recordGestureMove(const MTPoint &coord, bool nmt) {
   if (_currentGesture) { _currentGesture->recordMove(coord, nmt); }
 }
 void GestureLog::recordMove(const MTPoint& coord, bool nmt) {
 std::chrono::duration<double> elapsed = std::chrono::high_resolution_clock::now()-_startTime;
 stringstream data;
 data << elapsed.count() << ' ' << coord.x << ' ' << coord.y << ' ' << nmt;
 _trace.push_back(std::string(data.str()));
 }*/
