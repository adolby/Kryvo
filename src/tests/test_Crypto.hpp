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
  /*!
   * \brief testComparatorSameFile Compares two files using the file comparator
   * functions private to the TestCrypto class.
   */
  void testComparatorSameFile();

  /*!
   * \brief testComparatorDifferentFile Compares two files using the file
   * comparator functions private to the TestCrypto class.
   */
  void testComparatorDifferentFile();

  /*!
   * \brief testEncryptDecrypt_data Sends a passphrase, algorithm
   * name, input file name, and expected encrypted and decrypted file names to
   * the testEncryptDecrypt method for testing.
   */
  void testEncryptDecrypt_data();

  /*!
   * \brief testEncryptDecrypt Crypto's encryption operation randomly
   * salts the key and IV. This means that encryption and decryption will need
   * to be tested together. Tests the encryption/decryption process with single
   * test files as input.
   */
  void testEncryptDecrypt();

  /*!
   * \brief testEncryptDecryptAll Crypto's encryption operation randomly
   * salts the key and IV. This means that encryption and decryption will need
   * to be tested together. Tests the encryption and decryption process with
   * multiple test files as input.
   */
  void testEncryptDecryptAll();
};

#endif // KRYVOS_TESTS_TEST_CRYPTO_HPP_
