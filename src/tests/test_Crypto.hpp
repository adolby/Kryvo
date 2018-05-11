#ifndef KRYVO_TESTS_TEST_CRYPTO_HPP_
#define KRYVO_TESTS_TEST_CRYPTO_HPP_

#include <QObject>

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

#endif // KRYVO_TESTS_TEST_CRYPTO_HPP_
