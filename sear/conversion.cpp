#include "conversion.hpp"

#include <cerrno>
#include <iconv.h>
#include <stdexcept>
#include <vector>

namespace {
class IconvHandle {
 public:
  IconvHandle(const char *to_code, const char *from_code)
      : handle_(::iconv_open(to_code, from_code)) {}

  ~IconvHandle() {
    if (handle_ != reinterpret_cast<iconv_t>(-1)) {
      ::iconv_close(handle_);
    }
  }

  IconvHandle(const IconvHandle &)            = delete;
  IconvHandle &operator=(const IconvHandle &) = delete;

  iconv_t get() const { return handle_; }

  bool valid() const { return handle_ != reinterpret_cast<iconv_t>(-1); }

 private:
  iconv_t handle_;
};
}  // namespace

std::string convert(std::string input, std::string_view inputCodepage,
                    std::string_view outputCodepage) {
  std::string fromCode{inputCodepage};
  std::string toCode{outputCodepage};
  IconvHandle conv(toCode.c_str(), fromCode.c_str());

  if (!conv.valid()) {
    if (errno == EINVAL)
      throw std::runtime_error("not supported from " + fromCode + " to " +
                               toCode);
    else
      throw std::runtime_error("unknown error");
  }

  char *src_ptr   = input.data();
  size_t src_size = input.size();

  std::vector<char> buf(1024);
  std::string dst;

  while (0 < src_size) {
    char *dst_ptr   = buf.data();
    size_t dst_size = buf.size();
    size_t res = ::iconv(conv.get(), &src_ptr, &src_size, &dst_ptr, &dst_size);
    if (res == static_cast<size_t>(-1)) {
      if (errno == E2BIG) {
        // ignore this error
      } else {
        switch (errno) {
          case EILSEQ:
          case EINVAL:
            throw std::runtime_error("invalid multibyte chars");
          default:
            throw std::runtime_error("unknown error");
        }
      }
    }
    dst.append(buf.data(), buf.size() - dst_size);
  }

  return dst;
}

/** Converts string from specified codepage to UTF-8, defaults to IBM-1047 if nothing is specified */
std::string SEAR::toUTF8(const std::string& input, std::string_view codepage) {
  return convert(input,codepage,"UTF-8");
}

/** Converts string from UTF-8 to specified codepage, defaults to IBM-1047 if nothing is specified */
std::string SEAR::fromUTF8(const std::string& input, std::string_view codepage) {
  return convert(input,"UTF-8",codepage);
}