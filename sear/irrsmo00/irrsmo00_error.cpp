#include "irrsmo00_error.hpp"

#include <algorithm>

namespace SEAR {
IRRSMO00Error::IRRSMO00Error(const std::vector<std::string>& errors)
    : errors_(errors) {
  std::for_each(errors_.begin(), errors_.end(),
                [](std::string& error) { error = "irrsmo00: " + error; });
}

IRRSMO00Error::IRRSMO00Error(std::string_view error)
  : errors_({"irrsmo00: " + std::string(error)}) {}

const std::vector<std::string>& IRRSMO00Error::getErrors() const {
  return errors_;
}

}  // namespace SEAR
