#ifndef PIMPL_H_H
#define PIMPL_H_H

#include <memory>

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

#endif
