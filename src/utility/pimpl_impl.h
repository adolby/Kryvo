#ifndef PIMPL_IMPL_H
#define PIMPL_IMPL_H

#include "make_unique.h"
#include <utility>

template<typename T>
pimpl<T>::pimpl() : m{ make_unique<T>() } { }

template<typename T>
template<typename ...Args>
pimpl<T>::pimpl( Args&& ...args )
  : m{ make_unique<T>(std::forward<Args>(args)...) } { }

template<typename T>
pimpl<T>::~pimpl() { }

template<typename T>
T* pimpl<T>::operator->() { return m.get(); }

template<typename T>
const T* pimpl<T>::operator->() const { return m.get(); }

template<typename T>
T& pimpl<T>::operator*() { return *m.get(); }

#endif
