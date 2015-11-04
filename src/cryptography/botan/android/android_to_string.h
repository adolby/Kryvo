#ifndef KRYVOS_CRYPTOGRAPHY_ANDROID_TO_STRING_H_
#define KRYVOS_CRYPTOGRAPHY_ANDROID_TO_STRING_H_

#include <sstream>
#include <string>

namespace std {

template <typename T>
std::string to_string(T value)
{
  std::ostringstream os;
  os << value;
  return os.str();
}

}

#endif // KRYVOS_CRYPTOGRAPHY_ANDROID_TO_STRING_H_
