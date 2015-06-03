/* 
 * File:   klogger.h
 * Author: baodn
 * 
 * Created on April 5, 2013, 9:56 AM
 */

#ifndef K_LOGGER_H_
#define K_LOGGER_H_

#include <iostream>
#include <fstream>
#include <dirent.h>
#include <list>
#include <stdint.h>
#include <stdlib.h>

#define INIT_LOG(filename, level, rotate_num, size_limit, log_dir) \
{ Logger::Instance().Init(filename, level, rotate_num, size_limit, log_dir);}
// make use of__FILE__, __LINE__
#define DBG_LOG(level,fmt, ...) \
{  Logger::Instance().Log(level,__FILE__, __LINE__,fmt, ##__VA_ARGS__); }
#define DBG_LOG_PROTO(level, message) \
{  std::string __output__;\
   google::protobuf::TextFormat::PrintToString((message),&__output__); \
   if(__output__.size() < 16*1024)\
   Logger::Instance().Log(level,__FILE__, __LINE__,"%s", __output__.c_str()); }
   
enum {
    DEBUG = 0,
    INFO = 1,
    WARNING = 2,
    ERROR = 3,
    FATAL = 4,
};

class Logger {
public:
    ~Logger();
    void Init(const std::string& filename, int level, uint32_t rotate_num, long size_limit, const std::string& log_dir);
    static Logger &Instance();
    void Log(int level, const char* sourcefilename, int line, const char* msg, ...);
private:
    Logger();
    int _level;
    uint32_t _rotate_num;
    long _size_limit;
    std::ofstream _file;
    DIR* _dir;
    std::string _log_file_name;
    std::string _path_dir;
    std::string _pure_name;

    std::list<int> _list_file;

    void initListFile();
    void openLogFile();
    void rotateFile();
    std::string Level(int level);
    
#ifdef CHECK_LOG //use to print error information
    std::ofstream check_log_file;
#endif
};

#endif // K_LOGGER_H_
