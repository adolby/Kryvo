/**
 * Kryvos - Encrypts and decrypts files.
 * Copyright (C) 2014, 2015 Andrew Dolby
 *
 * This file is part of Kryvos.
 *
 * Kryvos is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Kryvos is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Kryvos.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Contact : andrewdolby@gmail.com
 */

#ifndef KRYVOS_UTILITY_PIMPL_IMPL_H_
#define KRYVOS_UTILITY_PIMPL_IMPL_H_

#include <memory>
#include <utility>

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
