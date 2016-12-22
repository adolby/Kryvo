#ifndef KRYVOS_UTILITY_PIMPL_H_
#define KRYVOS_UTILITY_PIMPL_H_

#include <memory>

namespace Kryvos {

template<typename T>
class pimpl {
 public:
  pimpl();
  template<typename ...Args> pimpl(Args&& ...);
  ~pimpl();

  T* operator->();
  const T* operator->() const;
  T& operator*();

 private:
  std::unique_ptr<T> m;
};

}

#endif // KRYVOS_UTILITY_PIMPL_H_
