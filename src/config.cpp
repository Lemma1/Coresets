#include "config.h"

Chameleon::Chameleon(std::string const& value) 
{
    value_=value;
}

#include <iostream>
#include <sstream>
#include <fstream>

Chameleon::Chameleon(const char* c) 
{
    value_=c;
}

Chameleon::Chameleon(double d) 
{
    std::stringstream s;
    s<<d;
    value_=s.str();
}

Chameleon::Chameleon(Chameleon const& other) 
{
    value_=other.value_;
}

Chameleon& Chameleon::operator=(Chameleon const& other) 
{
    value_=other.value_;
    return *this;
}

Chameleon& Chameleon::operator=(double i) 
{
    std::stringstream s;
    s << i;
    value_ = s.str();
    return *this;
}

Chameleon& Chameleon::operator=(std::string const& s) 
{
    value_=s;
    return *this;
}

Chameleon::operator std::string() const 
{
    return value_;
}

Chameleon::operator double() const 
{
    return atof(value_.c_str());
}

std::string trim(std::string const& source, char const* delims = " \t\r\n") {
  std::string result(source);
  std::string::size_type index = result.find_last_not_of(delims);
  if(index != std::string::npos)
    result.erase(++index);

  index = result.find_first_not_of(delims);
  if(index != std::string::npos)
    result.erase(0, index);
  else
    result.erase();
  return result;
}

ConfigFile::ConfigFile(std::string const& configFile) {
  std::ifstream file(configFile.c_str());

  std::string line;
  std::string name;
  std::string value;
  std::string inSection;
  int posEqual;
  while (std::getline(file,line)) {

    if (! line.length()) continue;

    if (line[0] == '#') continue;
    if (line[0] == ';') continue;

    if (line[0] == '[') {
      inSection=trim(line.substr(1,line.find(']')-1));
      continue;
    }

    posEqual=line.find('=');
    name  = trim(line.substr(0,posEqual));
    value = trim(line.substr(posEqual+1));

    content_[inSection+'/'+name]=Chameleon(value);
  }
}

Chameleon const& ConfigFile::Value(std::string const& section, std::string const& entry) const {

  std::map<std::string,Chameleon>::const_iterator ci = content_.find(section + '/' + entry);

  if (ci == content_.end()) throw "does not exist";

  return ci->second;
}

Chameleon const& ConfigFile::Value(std::string const& section, std::string const& entry, double value) {
  try {
    return Value(section, entry);
  } catch(const char *) {
    return content_.insert(std::make_pair(section+'/'+entry, Chameleon(value))).first->second;
  }
}

Chameleon const& ConfigFile::Value(std::string const& section, std::string const& entry, std::string const& value) {
  try {
    return Value(section, entry);
  } catch(const char *) {
    return content_.insert(std::make_pair(section+'/'+entry, Chameleon(value))).first->second;
  }
}

ConfReader::ConfReader (std::string filename, std::string confKey) {
  m_configFile = new ConfigFile(filename);
  m_confKey = confKey;
}

ConfReader::~ConfReader () {
  if (!m_configFile) {
    delete m_configFile;
  }
}

int ConfReader::get_int (std::string key) {
  int val = m_configFile->Value(m_confKey, key);
  return val;
}

double ConfReader::get_double(std::string key)
{
  double val = m_configFile->Value(m_confKey, key);
  return val;
}

std::string ConfReader::get_string(std::string key)
{
  std::string val = m_configFile->Value(m_confKey, key);
  return val;
}