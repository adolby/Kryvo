#ifndef KRYVOS_UTILITY_PIMPL_IMPL_H_
#define KRYVOS_UTILITY_PIMPL_IMPL_H_

#include <memory>
#include <utility>

using namespace Kryvos;

template<typename T>
pimpl<T>::pimpl() : m{ std::make_unique<T>() } { }

template<typename T>
template<typename ...Args>
pimpl<T>::pimpl( Args&& ...args )
  : m{ std::make_unique<T>(std::forward<Args>(args)...) } { }

template<typename T>
pimpl<T>::~pimpl() { }

template<typename T>
T* pimpl<T>::operator->() { return m.get(); }

template<typename T>
const T* pimpl<T>::operator->() const { return m.get(); }

template<typename T>
T& pimpl<T>::operator*() { return *m.get(); }

#endif // KRYVOS_UTILITY_PIMPL_IMPL_H_
