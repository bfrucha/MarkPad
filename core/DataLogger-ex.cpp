//
//  DataLogger.mm
//  MarkPad
//
//  (c) Eric Lecolinet - http://www.telecom-paristech.fr/~elc
//  (c) Bruno Fruchard - http://brunofruchard.com/
//  Copyright (c) 2017/18. All rights reserved.
//

#include "ccuty/ccpath.hpp"
#include "DataLogger.h"
#include "Actions.h"
using ccuty::file_exists;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Gesture Log Implementation

GestureLog::GestureLog(): _gid(std::max<int>(1, std::rand())) {}

void GestureLog::start(const MTPoint& coord, const std::string &timestamp, Shortcut *s) {
  _timestamp = timestamp;
  _startTime = std::chrono::high_resolution_clock::now();
  _trace.push_back("0 "+std::to_string(coord.x)+" "+std::to_string(coord.y)+" 0");
  
  // start_ = "<startData date='%@' x='%f' y='%f'/>\n", ts, coord.x, coord.y];
  _start = "<startShortcut/>\n";
  if(s) {
    const MTRect &rect = s->area();
    stringstream tag;
    //const string& cn = (s->commandName() == "" && s->action()) ? s->action()->name : s->commandName();
    const string& cn = s->commandName();
    tag << "<startShortcut label='" << s->name() << "' commandname='" << cn << "' x='"
    << rect.x << "' y='" << rect.y << "' width='" << rect.width << "' height='" << rect.height << "'/>\n";
    _start = tag.str();
  }
}

void GestureLog::recordMove(const MTPoint& coord, bool nmt) {
  std::chrono::duration<double> elapsed = std::chrono::high_resolution_clock::now()-_startTime;
  stringstream data;
  data << elapsed.count() << ' ' << coord.x << ' ' << coord.y << ' ' << nmt;
  _trace.push_back(std::string(data.str()));
}

void GestureLog::end(const MTPoint& coord, bool nmt, Shortcut *s) {
  std::chrono::duration<double> elapsed = std::chrono::high_resolution_clock::now()-_startTime;
  stringstream data;
  data << elapsed.count() << ' ' << coord.x << ' ' << coord.y << ' ' << nmt;
  _trace.push_back(std::string(data.str()));
  _end = "<endShortcut/>\n";
  if (s) {
    const MTRect &rect = s->area();
    stringstream tag;
    //const string& cn = (s->commandName() == "" && s->action()) ? s->action()->name : s->commandName();
    const string& cn = s->commandName();
    //cout << "command name: " << cn << endl;
    tag << "<endShortcut label='" << s->name() << "' commandname='" << cn << "' x='"
    << rect.x << "' y='" << rect.y << "' width='" << rect.width << "' height='" << rect.height << "'/>\n";
    _end = tag.str();
  }
}


std::string GestureLog::toString() {
  std::string res = "<movementData>\n";
  for (const std::string &t : _trace) res += t+'\n';
  res += "</movementData>\n";
  return
  "<gesture id='"+std::to_string(_gid)+"' timestamp='"+_timestamp+"'>\n"
  +(_start+_end+res)+"</gesture>\n";
}

std::ofstream& operator<<(std::ofstream& stream, const GestureLog& gl) {
  stream << "<gesture id='"+std::to_string(gl._gid)+"' timestamp='"+gl._timestamp+"'>\n"; // write header
  stream << gl._start+gl._end; // write shortcuts
  // write data
  stream << "<movementData>\n";
  for (const std::string& t : gl._trace) { stream << t << '\n'; }
  stream << "</movementData>\n";
  stream << "</gesture>\n";
  return stream;
}
  
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Data Logger Implementation
  
#define GESTURES_LOG_KEY "gesturesLogNb"
#define CONF_LOG_KEY "confLogNb"
  
  std::unique_ptr<DataLogger> DataLogger::instance(nullptr);
  
  DataLogger::DataLogger(const Pad &pad):
  _dirLogPath(Conf::confDir()+"data_log/"),
  _currentConfPath(_dirLogPath+"current_shortcuts.json"),
  _confLogPath(_dirLogPath+"configurations_log.xml"),
  _arcConfPath(_dirLogPath+"configurations.tar.gz"),
  _currentGuidesPath(_dirLogPath+"current_guides.json"),
  _guidesLogPath(_dirLogPath+"guides_log.xml"),
  _arcGuidesPath(_dirLogPath+"guides.tar.gz"),
  _gestLogPath(_dirLogPath+"gestures_log.xml"),
  _arcGestPath(_dirLogPath+"gestures.tar.gz"),
  _attrLogPath(_dirLogPath+"logger_attributes.txt"),
  _maxFileSize(2000000) {        // 2000000 = 2Mo max file size
  
  if (!file_exists(_dirLogPath)) { std::system(("mkdir "+_dirLogPath).c_str()); }
  
  if (!file_exists(_attrLogPath)) {
  
  auto attrFile = ofstream(_attrLogPath.c_str());
  if(attrFile.is_open()) {
  _attributes.emplace(GESTURES_LOG_KEY, "0");
  attrFile << GESTURES_LOG_KEY << ":0\n";
  _attributes.emplace(CONF_LOG_KEY, "0");
  attrFile << CONF_LOG_KEY << ":0\n";
  _attributes.emplace("padDimension", std::to_string(pad.padWidth)+","+std::to_string(pad.padHeight));
  attrFile << "padDimension:" << _attributes["padDimension"];
  attrFile.close();
  }
  } else {
  ifstream attrFile(_attrLogPath.c_str());
  if(attrFile.is_open()) {
  while(!attrFile.eof()) {
  std::string line; attrFile >> line;
  if(line == "") { continue; }
  unsigned long sep = line.find(':');
  _attributes.emplace(line.substr(0, sep), line.substr(sep+1));
  }
  }
  attrFile.close();
  }
  }
  
  DataLogger* DataLogger::obtain(const Pad &pad) {
  if (!instance) { instance = std::unique_ptr<DataLogger>(new DataLogger(pad)); }
  return instance.get();
  }
  
  
  void DataLogger::startGesture(const MTPoint &coord, Shortcut *s) {
  _currentGesture = std::shared_ptr<GestureLog>(new GestureLog);
  std::time_t now = std::time(0);
  char timestamp[64];
  std::strftime(timestamp, sizeof(timestamp), "%Y-%m-%dD%Hh%Mm%Ss%z", std::localtime(&now));
  _currentGesture->start(coord, timestamp, s);
  }
  
  void DataLogger::recordGestureMove(const MTPoint &coord, bool nmt) {
  if (_currentGesture) { _currentGesture->recordMove(coord, nmt); }
  }
  
  void DataLogger::endGesture(const MTPoint &coord, bool nmt, Shortcut *s) {
  if (!_currentGesture) {
  cerr << "DataLogger::endGesture: No currentGesture!" << endl;
  return;
  }
  _currentGesture->end(coord, nmt, s);
  _gestures.push_back(_currentGesture);
  _currentGesture.reset();
  Conf::mustSave();
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
  _attributes[CONF_LOG_KEY] = std::to_string(stoi(_attributes[CONF_LOG_KEY])+1);
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
  _attributes[CONF_LOG_KEY] = std::to_string(stoi(_attributes[CONF_LOG_KEY])+1);
  saveAttributes();
  }
  }
  }
  
  bool DataLogger::needToSaveGestures() { return _gestures.size()!=0; }
  
  void DataLogger::saveGesturesPerformed() {
  if(!_gestures.size()) { return; }
  
  bool success = false;
  if (!file_exists(_gestLogPath)) { std::system(("touch "+_gestLogPath).c_str()); }
  
  ofstream output(_gestLogPath, std::ios_base::app);
  if (output) {
  for (auto gesture : _gestures) { output << *gesture; }
  success = true;
  output.close();
  }
  else { std::cout << "could not open gestures file ("+_gestLogPath+")"; }
  
  // need to archive file if too big
  ifstream gestFile(_gestLogPath, std::ifstream::ate | std::ifstream::binary);
  if (gestFile) {
  // size in bytes of the *content* of the file (file is probably taking
  // more space on the machine)
  unsigned long size = gestFile.tellg();
  // NSLog(@"%lld", filesize);
  if (_maxFileSize < size) {
  addFileToArchive(_gestLogPath, _arcGestPath);
  _attributes[GESTURES_LOG_KEY] = std::to_string(stoi(_attributes[GESTURES_LOG_KEY])+1);
  saveAttributes();
  }
  }
  
  if (success) { _gestures.clear(); }
  }
  
  void DataLogger::addFileToArchive(const std::string &filepath, const std::string &arcPath) {
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
  for(auto it = _attributes.cbegin(); it != _attributes.cend(); it++) {
  output << it->first << ':' << it->second << '\n'; }
  output.close();
  }
  }
