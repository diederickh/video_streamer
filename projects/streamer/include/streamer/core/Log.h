/*

---------------------------------------------------------------------------------
      
                                               oooo              
                                               `888              
                oooo d8b  .ooooo.  oooo    ooo  888  oooo  oooo  
                `888""8P d88' `88b  `88b..8P'   888  `888  `888  
                 888     888   888    Y888'     888   888   888  
                 888     888   888  .o8"'88b    888   888   888  
                d888b    `Y8bod8P' o88'   888o o888o  `V88V"V8P' 
                                                                 
                                                  www.roxlu.com
                                             www.apollomedia.nl  
                                          www.twitter.com/roxlu
              
---------------------------------------------------------------------------------


 */
#ifndef ROXLU_STREAMER_LOG_H
#define ROXLU_STREAMER_LOG_H

#if !defined(NDEBUG) 

extern "C" {
#  include <uv.h> 
}

#include <string>
#include <fstream>
#include <vector>

#if defined(_WIN32)
#  define STREAMER_ANSI_VERBOSE "\x1b[32;1m"
#  define STREAMER_ANSI_WARNING "\x1b[35;1m"
#  define STREAMER_ANSI_ERROR "\x1b[31;1m"
#  define STREAMER_ANSI_STATUS "\x1b[33;1m"
#else 
#  define STREAMER_ANSI_VERBOSE "\033[1;32m"  // 32 = green, 36 = blueish, 1b[41m <-- red background
#  define STREAMER_ANSI_WARNING "\033[1;35m"
#  define STREAMER_ANSI_ERROR "\033[1;31m"
#  define STREAMER_ANSI_STATUS "\033[1;33m"
#endif

#define STREAMER_LOG_LEVEL_VERBOSE 0
#define STREAMER_LOG_LEVEL_WARNING 1
#define STREAMER_LOG_LEVEL_ERROR 2
#define STREAMER_LOG_LEVEL_STATUS 3

#define STREAMER_LOG_STATE_NONE 0
#define STREAMER_LOG_STATE_INITIALIZED 1

#define STREAMER_LOG_INIT(file, minlevel) { streamer_log_init(file, minlevel); } 


#if(_MSC_VER)
#  define STREAMER_VERBOSE(fmt, ...) { rx_verbose(__LINE__, __FUNCSIG__, fmt, ##__VA_ARGS__); } 
#  define STREAMER_WARNING(fmt, ...) { rx_warning(__LINE__, __FUNCSIG__, fmt, ##__VA_ARGS__); } 
#  define STREAMER_ERROR(fmt, ...) { rx_error(__LINE__, __FUNCSIG__, fmt, ##__VA_ARGS__); } 
#  define STREAMER_STATUS(fmt, ...) { rx_error(__LINE__, __FUNCSIG__, fmt, ##__VA_ARGS__); } 
#else
#  define STREAMER_VERBOSE(fmt, ...) { streamer_log_verbose(__LINE__, __PRETTY_FUNCTION__, fmt, ##__VA_ARGS__); } 
#  define STREAMER_WARNING(fmt, ...) { streamer_log_warning(__LINE__, __PRETTY_FUNCTION__, fmt, ##__VA_ARGS__); } 
#  define STREAMER_ERROR(fmt, ...)   { streamer_log_error(__LINE__, __PRETTY_FUNCTION__, fmt, ##__VA_ARGS__); } 
#  define STREAMER_STATUS(fmt, ...)   { streamer_log_status(__LINE__, __PRETTY_FUNCTION__, fmt, ##__VA_ARGS__); } 
#endif

// --------------------------------------------------

struct LogEntry {
  std::string tty_message;
  std::string file_message;
  int level;
};

// --------------------------------------------------
  
struct LogContext {
  LogContext();
  ~LogContext();
  uv_mutex_t mutex;
  uv_loop_t* loop;
  uv_tty_t tty;
  int min_level;
  int state;
  std::ofstream ofs;
};

// --------------------------------------------------
std::string streamer_log_gettime();
void streamer_log_verbose(int line, const char* function, const char* fmt, ...);
void streamer_log_warning(int line, const char* function, const char* fmt, ...);
void streamer_log_error(int line, const char* function, const char* fmt, ...);
void streamer_log_status(int line, const char* function, const char* fmt, ...);
void streamer_log_function(int level, int line, const char* function, const char* fmt, va_list args);
void streamer_log_entry(LogEntry entry);
void streamer_log_init(std::string outfile, int minLevel);
// --------------------------------------------------

extern LogContext log_ctx; 

#else  // #if !defined(NDEBUG) 
#  define STREAMER_VERBOSE(fmt, ...) { }
#  define STREAMER_WARNING(fmt, ...) { }
#  define STREAMER_ERROR(fmt, ...) { }
#  define STREAMER_STATUS(fmt, ...) { } 
#endif // #if !defined(NDEBUG) 

#endif // file 
