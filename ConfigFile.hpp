#ifndef _CONFIG_FILE_H_
#define _CONFIG_FILE_H_
#include <iostream>
#include <string>
#include <sstream>
#include <map>
#include <fstream>
#include <typeinfo>
#include "klogger.h"


class ConfigFile
{
public:
   ConfigFile()
   {
   }
   ConfigFile(const std::string &fName)
   {
      this->fName = fName;
      ExtractKeys();
   }
   void Init(const std::string &fName)
   {
      //todo
      this->fName = fName;
      ExtractKeys();
   }
   ~ConfigFile()
   {
   }
   bool keyExists(const std::string &key) const
   {
      return contents.find(key) != contents.end();
   }

   template <typename ValueType>
   ValueType getValueOfKey(const std::string &key, ValueType const &defaultValue = ValueType()) const
   {
      if (!keyExists(key))
         return defaultValue;

      return string_to_T<ValueType>(contents.find(key)->second, defaultValue);
   }
   void printAllValues()
   {
      for (std::map<std::string, std::string>::iterator it = contents.begin(); 
                it != contents.end(); it++)
         std::cout << it->first << "=" << it->second << std::endl;
   }
   static ConfigFile &Instance()
   {
      static ConfigFile instance_;
      return instance_;
   }
private:
  template <typename T>
  static std::string T_to_string(T const &val) 
  {
     std::ostringstream ostr;
     ostr << val;
     return ostr.str();
  }
  template <typename T>
  static T string_to_T(std::string const &val, const T & defval) 
  {
     std::istringstream istr(val);
     T returnVal;
     if (!(istr >> returnVal))
     {
     	DBG_LOG(WARN,"CFG: Not a valid " + val 
                            + (std::string)typeid(T).name() 
                            + " received!\n");
        return defval;
     }
     return returnVal;
   }

   void removeComment(std::string &line) const
   {
      if (line.find('#') != line.npos)
      line.erase(line.find('#'));
   }

   bool onlyWhitespace(const std::string &line) const
   {
      return (line.find_first_not_of(' ') == line.npos);
   }
   bool validLine(const std::string &line) const
   {
      std::string temp = line;
      temp.erase(0, temp.find_first_not_of("\t "));
      if (temp[0] == '=')
         return false;

      for (size_t i = temp.find('=') + 1; i < temp.length(); i++)
	if (temp[i] != ' ')
           return true;

      return false;
   }

   void extractKey(std::string &key, size_t const &sepPos, const std::string &line) const
   {
      key = line.substr(0, sepPos);
      if (key.find('\t') != line.npos || key.find(' ') != line.npos)
         key.erase(key.find_first_of("\t "));
   }
   void extractValue(std::string &value, size_t const &sepPos, const std::string &line) const
   {
      value = line.substr(sepPos + 1);
      value.erase(0, value.find_first_not_of("\t "));
      value.erase(value.find_last_not_of("\t ") + 1);
   }
   void extractContents(const std::string &line) 
   {
      std::string temp = line;
      temp.erase(0, temp.find_first_not_of("\t "));
      size_t sepPos = temp.find('=');

      std::string key, value;
      extractKey(key, sepPos, temp);
      extractValue(value, sepPos, temp);

      if (!keyExists(key))
         contents.insert(std::pair<std::string, std::string>(key, value));
      else
         DBG_LOG(WARN,"CFG: Can only have unique key names!\n");
   }

   void parseLine(const std::string &line, size_t const lineNo)
   {
      if (line.find('=') == line.npos)
         DBG_LOG(ERROR,"CFG: Couldn't find separator on line: " + T_to_string(lineNo) + "\n");

      if (!validLine(line))
         DBG_LOG(ERROR,"CFG: Bad format for line: " + T_to_string(lineNo) + "\n");

      extractContents(line);
   }

   void ExtractKeys()
   {
      std::ifstream file;
      file.open(fName.c_str());
      if (!file)
         DBG_LOG(ERROR,"CFG: File " + fName + " couldn't be found!\n");

      std::string line;
      size_t lineNo = 0;
      while (std::getline(file, line))
      {
         lineNo++;
         std::string temp = line;

         if (temp.empty())
         continue;

         removeComment(temp);
         if (onlyWhitespace(temp))
         continue;

         parseLine(temp, lineNo);
      }

      file.close();
   }
private:
   std::map<std::string, std::string> contents;
   std::string fName;
};
#endif //_CONFIG_FILE_H_
