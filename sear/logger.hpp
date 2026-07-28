#ifndef __SEAR_LOGGER_H_
#define __SEAR_LOGGER_H_

#include <iostream>
#include <string>
#include <string_view>

namespace SEAR {
class Logger {
 private:
  bool debug_;
  std::string ansi_bright_red_    = "\033[91m";
  std::string ansi_bright_green_  = "\033[92m";
  std::string ansi_bright_yellow_ = "\033[93m";
  std::string ansi_reset_         = "\033[0m";
  explicit Logger();

 public:
  explicit Logger(Logger const&) = delete;
  void operator=(Logger const&)  = delete;
  static Logger& getInstance();
  void setDebug(bool debug);
  void debug(std::string_view message, std::string_view body = "") const;
  void debugAllocate(const void* ptr, int rmode, int byte_count) const;
  void debugFree(const void* ptr) const;
  void hexDump(const char* p_buffer, int length) const;
};
}  // namespace SEAR

#endif
