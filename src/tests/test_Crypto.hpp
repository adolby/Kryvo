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

#ifndef KRYVOS_TESTS_TEST_CRYPTO_HPP_
#define KRYVOS_TESTS_TEST_CRYPTO_HPP_

#include <QtCore/QObject>

class TestCrypto : public QObject {
  Q_OBJECT

 private slots:
  void testEncryptDecrypt_data();
  void testEncryptDecrypt();
  void testEncryptDecryptAll();
};

#endif // KRYVOS_TESTS_TEST_CRYPTO_HPP_
