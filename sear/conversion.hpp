#ifndef __SEAR_CONVERSION_H_
#define __SEAR_CONVERSION_H_

#include <string>
#include <string_view>

namespace SEAR {
std::string toUTF8(std::string_view input,
                   std::string_view codepage = "IBM-1047");
std::string fromUTF8(std::string_view input,
                     std::string_view codepage = "IBM-1047");
}
#endif