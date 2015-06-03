#include "klogger.h"

Logger::Logger() {
}

Logger::~Logger() {
    if (_file.is_open()) {
        _file.close();
    }
}

void Logger::Init(const std::string& filename, int level, uint32_t rotate_num, long size_limit, const std::string& log_dir) {
#ifdef CHECK_LOG
    check_log_file.open("/tmp/check_log");
    char* current_dir = get_current_dir_name();
    check_log_file << "current dir = " << current_dir << std::endl;
#endif
    // other settings
    _level = level;
    _rotate_num = rotate_num;
    _size_limit = size_limit;

    
    if(log_dir.empty()) {
        _path_dir = get_current_dir_name();
        _path_dir = _path_dir + "/../log/";
    } else {
        _path_dir = log_dir;
    }
    
    uint32_t position = filename.find_last_of("/");
    _log_file_name = filename.substr(position + 1);
    _pure_name = _log_file_name;
    _pure_name.erase(_pure_name.length() - 4, 4);

    initListFile();
}

void Logger::initListFile() {
    struct dirent *ent;
    _dir = opendir(_path_dir.c_str());
    if (_dir != NULL) {
        while ((ent = readdir(_dir)) != NULL) {
            std::string file_name = ent->d_name;
            size_t name_pos = file_name.find(_pure_name);
            if (name_pos != std::string::npos) {
                name_pos += _pure_name.size();
                size_t tail_pos = file_name.rfind(".log");
                if (tail_pos != std::string::npos) {
                    if (tail_pos - name_pos > 0) {
                        int number = atoi(file_name.substr(name_pos, tail_pos - name_pos).c_str());
                        _list_file.push_front(number);
                    } else if (tail_pos - name_pos == 0) {
                        _list_file.push_front(0);
                    }
                }
            }
        }
    }

    _list_file.sort();
    _list_file.reverse();

    closedir(_dir);
    if (_list_file.empty()) {
        openLogFile();
        _list_file.push_front(0);
    } else {
        rotateFile();
        openLogFile();
    }

}

Logger& Logger::Instance() {
    static Logger instance_;
    return instance_;
}

void Logger::Log(int level, const char* sourcefilename, int line, const char* msg, ...) {
    char date[1024] = {0};
    if (level >= _level) {
        //todo: check file limit size, if exceeds, do rotation.
        long log_file_size = _file.tellp();
        if (log_file_size >= _size_limit) {
            rotateFile();
            openLogFile();
        }

        char dest[1024 * 64]; // this is used to store log, if you want to log a big one, plz increase it
        va_list argptr;
        va_start(argptr, msg);
        vsnprintf(dest, 1024*64,  msg, argptr);
        va_end(argptr);
        
        //check level, if higher, then log the msg ( set time of msg arrival, human readable).
        time_t curTime = time(0);
        struct tm tm_time = *localtime((const time_t*) (&curTime));

        sprintf(date, "%4.4d-%2.2d-%2.2d %2.2d:%2.2d:%2.2d", tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday, tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec);
        try {
            _file << date << ":" << Level(level) << ": " << sourcefilename << ":" << line << ":" << dest << std::endl;
        } catch (std::exception& e) {
            check_log_file << date << "cannot write to file" << std::endl;
        }
    }
}

std::string Logger::Level(int level) {
    if (level == INFO) return "INFO ";
    if (level == DEBUG) return "DEBUG";
    if (level == WARNING) return "WARN ";
    if (level == ERROR) return "ERROR";
    if (level == FATAL) return "FATAL";
    return "INFO";
}

void Logger::openLogFile() {
    if (!_file.is_open()) {
        _file.exceptions(std::ofstream::failbit | std::ofstream::badbit);
        try {
            _file.open((_path_dir + _log_file_name).c_str());
        } catch (std::ios_base::failure e) {
            char date[1024] = {0};
            time_t curTime = time(0);
            struct tm tm_time = *localtime((const time_t*) (&curTime));
            sprintf(date, "%4.4d-%2.2d-%2.2d %2.2d:%2.2d:%2.2d", tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday, tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec);
#ifdef CHECK_LOG            
            check_log_file << "***************" << date << ":cannot open file " << (_path_dir + _log_file_name).c_str() << "**************" << std::endl;
            check_log_file << e.what() << std::endl;
#endif
        }
    }

}

void Logger::rotateFile() {
#ifdef CHECK_LOG
    char date[1024] = {0};
    time_t curTime = time(0);
    struct tm tm_time = *localtime((const time_t*) (&curTime));
    sprintf(date, "%4.4d-%2.2d-%2.2d %2.2d:%2.2d:%2.2d", tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday, tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec);
    check_log_file << "---------------" << date << ":in rotate file function ------------------" << std::endl;
    uint32_t t = _list_file.size();
    if (t >= _rotate_num) {
        check_log_file << "full room ==> must delete the last file" << std::endl;
    } else {
        check_log_file << "just shift file" << std::endl;
    }
#endif

    if (_file.is_open()) {
        try {
            _file.close();
        } catch (std::ios_base::failure e) {
#ifdef CHECK_LOG
            check_log_file << "***************" << "cannot close file " << "**************" << std::endl;
            check_log_file << e.what() << std::endl;
#endif
        }
    }

    uint32_t temp = _list_file.size();
    if (temp >= _rotate_num) {
        //remove latest file
        char str[10];
        sprintf(str, "%d", _list_file.front());
        std::string num = str;

        int ret = std::remove((_path_dir + _pure_name + num + ".log").c_str());
        if (ret == 0) {//success
            _list_file.pop_front();
#ifdef CHECK_LOG
            check_log_file << "remove file:" << (_path_dir + _pure_name + num + ".log").c_str() << " successfully" << std::endl;
#endif
        } else {
#ifdef CHECK_LOG
            check_log_file << "cannot remove file:" << (_path_dir + _pure_name + num + ".log").c_str() << std::endl;
#endif
            perror("Remove Error Is");
        }
    }
    for (std::list<int>::iterator iter = _list_file.begin(); iter != _list_file.end(); iter++) {
        char str[10];
        sprintf(str, "%d", (*iter));
        std::string old_num = ((*iter) == 0 ? "" : str);

        (*iter)++;

        char str2[10];
        sprintf(str2, "%d", (*iter));
        std::string new_num = str2;

        int ret = rename((_path_dir + _pure_name + old_num + ".log").c_str(), (_path_dir + _pure_name + new_num + ".log").c_str());
        if (ret == 0) {
#ifdef CHECK_LOG
            check_log_file << "successfully rename file:" << (_path_dir + _pure_name + old_num + ".log").c_str() << " to " <<
                    (_path_dir + _pure_name + new_num + ".log").c_str() << std::endl;
#endif
        } else {
#ifdef CHECK_LOG
            check_log_file << "cannot rename file:" << (_path_dir + _pure_name + old_num + ".log").c_str() << " to " <<
                    (_path_dir + _pure_name + new_num + ".log").c_str() << std::endl;
#endif      
            perror("Rename Error Is");
        }
    }
    //push new file to list
    _list_file.push_back(0);
}