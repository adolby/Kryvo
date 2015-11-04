#ifndef KRYVOS_UTILITY_MAKE_UNIQUE_H_
#define KRYVOS_UTILITY_MAKE_UNIQUE_H_

#include <memory>

template<typename T, typename... Args>
std::unique_ptr<T> make_unique(Args&&... args)
{
  return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

#endif // KRYVOS_UTILITY_MAKE_UNIQUE_H_
