#ifndef __IRRSMO00_ERROR_H_
#define __IRRSMO00_ERROR_H_

#include <exception>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

namespace SEAR {
class IRRSMO00Error : public std::exception {
 private:
  std::vector<std::string> errors_;

 public:
  explicit IRRSMO00Error(const std::vector<std::string>& errors);
  explicit IRRSMO00Error(std::string_view error);
  const std::vector<std::string>& getErrors() const;
};
}  // namespace SEAR

#endif
