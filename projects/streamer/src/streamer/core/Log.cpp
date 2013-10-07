#if !defined(NDEBUG) 

#include <streamer/core/Log.h>
#include <algorithm>
#include <sstream>

// --------------------------------------------------

LogContext log_ctx;

// --------------------------------------------------

LogContext::LogContext() 
  :min_level(STREAMER_LOG_LEVEL_VERBOSE)
  ,state(STREAMER_LOG_STATE_NONE)
  ,loop(NULL)
{

  uv_mutex_init(&mutex);

  loop = uv_loop_new();
  if(!loop) {
    printf("error: canot create a loop handler for the log.\n");
  }

  // get the path
  char buffer[1024 * 4];
  size_t size = 1024 * 4;
  int r = uv_exepath(buffer, &size);
  std::stringstream ss(buffer);
  std::vector<std::string> parts;
  std::string line;
  while(std::getline(ss, line, '/')) {
    parts.push_back(line);
  }
  parts.pop_back();

  // reconstruct
  std::string path;
  for(std::vector<std::string>::iterator it = parts.begin(); it != parts.end(); ++it) {
#if defined(_WIN32)
    path += *it +"\\";
#else
    path += *it +"/";    
#endif    
  }
  path += "streamer.log";

  STREAMER_LOG_INIT(path.c_str(), STREAMER_LOG_LEVEL_VERBOSE);
}

LogContext::~LogContext() {
  uv_mutex_destroy(&mutex);
  
  if(ofs.is_open()) {
    ofs.close();
  }

  state = STREAMER_LOG_STATE_NONE;
}

// --------------------------------------------------

void streamer_log_verbose(int line, const char* function, const char* fmt, ...) {
  va_list args;
  va_start(args, fmt);
  streamer_log_function(STREAMER_LOG_LEVEL_VERBOSE, line, function, fmt, args);
  va_end(args);
}

void streamer_log_warning(int line, const char* function, const char* fmt, ...) {
  va_list args;
  va_start(args, fmt);
  streamer_log_function(STREAMER_LOG_LEVEL_WARNING, line, function, fmt, args);
  va_end(args);
}

void streamer_log_error(int line, const char* function, const char* fmt, ...) {
  va_list args;
  va_start(args, fmt);
  streamer_log_function(STREAMER_LOG_LEVEL_ERROR, line, function, fmt, args);
  va_end(args);
}

void streamer_log_status(int line, const char* function, const char* fmt, ...) {
  va_list args;
  va_start(args, fmt);
  streamer_log_function(STREAMER_LOG_LEVEL_STATUS, line, function, fmt, args);
  va_end(args);
}

void streamer_log_function(int level, int line, const char* function, const char* fmt, va_list args) {

  if(level < log_ctx.min_level) {
    return;
  }

  char buf[1024 * 64]; // @todo maybe a bit too much ... 
  vsprintf(buf, fmt, args);

  std::stringstream ss_tty;
  std::stringstream ss_file;

  if(level == STREAMER_LOG_LEVEL_VERBOSE) {
    ss_tty << STREAMER_ANSI_VERBOSE;
    ss_file << "[ verbose ] ";
  }
  else if(level == STREAMER_LOG_LEVEL_WARNING) {
    ss_tty << STREAMER_ANSI_WARNING;
    ss_file << "[ warning ] ";
  }
  else if(level == STREAMER_LOG_LEVEL_ERROR) {
    ss_tty << STREAMER_ANSI_ERROR;
    ss_file << "[ error   ] ";
  }
  else if(level == STREAMER_LOG_LEVEL_STATUS) {
    ss_tty << STREAMER_ANSI_STATUS;
    ss_file << "[ status  ] ";
  }
  else {
    printf("error: invalid log level used: %d!\n", level);
    ::exit(EXIT_FAILURE);
  }

  ss_file << buf << " (" << streamer_log_gettime() << ", " << function << " {" << line << "})" ;
  ss_tty << buf << "\x1b[0m";

  LogEntry entry;

  entry.tty_message = ss_tty.str();
  entry.file_message = ss_file.str();
  streamer_log_entry(entry);

}

void streamer_log_init(std::string outfile, int maxlevel) {

  if(log_ctx.state != STREAMER_LOG_STATE_NONE) {
    printf("error: cannot initialize the log because we're already initialized.\n");
    ::exit(EXIT_FAILURE);
  }

  log_ctx.ofs.open(outfile.c_str(), std::ios::out);
  if(!log_ctx.ofs.is_open()) {
    printf("error: cannot open the output log file.\n");
    ::exit(EXIT_FAILURE);
  }

  uv_tty_init(log_ctx.loop, &log_ctx.tty, 1, 0);
  uv_tty_set_mode(&log_ctx.tty, 0);
}

void streamer_log_entry(LogEntry entry) {
  uv_buf_t buf;
  uv_write_t req;

  uv_mutex_lock(&log_ctx.mutex);
  {
    buf.base = (char*)entry.tty_message.c_str();
    buf.len = entry.tty_message.size();
    uv_write(&req, (uv_stream_t*)&log_ctx.tty, &buf, 1, NULL);

    if(log_ctx.ofs.is_open()) {
      log_ctx.ofs.write((char*)entry.file_message.c_str(), entry.file_message.size());
      log_ctx.ofs.flush();
    }    
  }
  uv_run(log_ctx.loop, UV_RUN_NOWAIT);
  uv_mutex_unlock(&log_ctx.mutex);
}

std::string streamer_log_gettime() {

  std::stringstream ss_timestamp;
  ss_timestamp << (uv_hrtime()/1000000);
  std::string ts = ss_timestamp.str();

  std::string format = "%Y.%m.%d.%H.%M.%S, ";
  time_t t;
  struct tm* info;
  char buf[4096]; // must be enough..
  time(&t);
  info = localtime(&t);
  strftime(buf, 4096, format.c_str(), info);
  std::string result(buf + ts);
  return result;
}

#endif // #if !defined(NDEBUG)
