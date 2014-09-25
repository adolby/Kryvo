/**
 * Kryvos File Encryptor - Encrypts and decrypts files.
 * Copyright (C) 2014 Andrew Dolby
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Contact : andrewdolby@gmail.com
 */

#include "test_Crypto.hpp"
#include "cryptography/Crypto.hpp"
#include "utility/make_unique.h"
#include <QtTest/QtTest>
#include <QtCore/QThread>
#include <QtCore/QString>
#include <algorithm>
#include <iterator>
#include <string>
#include <fstream>
#include <memory>

/*!
 * \brief TestCrypto::testEncryptDecrypt_data Sends a passphrase, algorithm
 * name, input file name, and expected encrypted and decrypted file names to the
 * testEncryptDecrypt method for testing.
 */
void TestCrypto::testEncryptDecrypt_data()
{
  QTest::addColumn<QString>("passphrase");
  QTest::addColumn<QString>("algorithmName");
  QTest::addColumn<QString>("inputFileName");
  QTest::addColumn<QString>("encryptedFileName");
  QTest::addColumn<QString>("decryptedFileName");

  QTest::newRow("Small file, no extension") << "password" << "AES-128/GCM"
                                            << "test" << "test.enc"
                                            << "test (2)";

  QTest::newRow("Medium exe file") << "password2" << "AES-128/GCM"
                                   << "test.exe" << "test.exe.enc"
                                   << "test (2).exe";

  QTest::newRow("Large zip file") << "password3" << "AES-128/GCM"
                                  << "test.zip" << "test.zip.enc"
                                  << "test (2).zip";
}

/*!
 * \brief TestCrypto::testEncryptDecrypt Crypto's encryption operation randomly
 * salts the key and IV. This means that encryption and decryption will need to
 * be tested together. Tests the encryption/decryption process with single test
 * files as input.
 */
void TestCrypto::testEncryptDecrypt()
{
  QFETCH(QString, passphrase);
  QFETCH(QString, algorithmName);
  QFETCH(QString, inputFileName);
  QFETCH(QString, encryptedFileName);
  QFETCH(QString, decryptedFileName);

  QStringList inputFileNames = {inputFileName};
  QStringList encryptedFileNames = {encryptedFileName};

  std::unique_ptr<Crypto> cryptography{new Crypto};

  // Test encryption and decryption
  cryptography->encrypt(passphrase, inputFileNames, algorithmName);
  cryptography->decrypt(passphrase, encryptedFileNames);

  bool equivalentTest = true;
  bool completedTest = true;

  // Compare initial file to decrypted file
  std::ifstream inFile{inputFileName.toStdString().c_str(), std::ios::binary};
  std::ifstream outFile{decryptedFileName.toStdString().c_str(),
                        std::ios::binary};

  std::istreambuf_iterator<char> begin1(inFile);
  std::istreambuf_iterator<char> begin2(outFile);
  std::istreambuf_iterator<char> end;

  while (begin1 != end && begin2 != end)
  { // Check files are same char by char
    if (*begin1 != *begin2)
    {
      equivalentTest = false;
      break;
    }

    ++begin1;
    ++begin2;
  }

  completedTest = (begin1 == end) && (begin2 == end);

  // Close streams
  inFile.close();
  outFile.close();

  // Clean up test files
  QFile encryptedFile{encryptedFileName};

  if (encryptedFile.exists())
  {
    encryptedFile.remove();
  }

  QFile decryptedFile{decryptedFileName};

  if (decryptedFile.exists())
  {
    decryptedFile.remove();
  }

  QVERIFY(equivalentTest && completedTest);
}

/*!
 * \brief TestCrypto::testEncryptDecrypt Crypto's encryption operation randomly
 * salts the key and IV. This means that encryption and decryption will need to
 * be tested together. Tests the encryption and decryption process with multiple
 * test files as input.
 */
void TestCrypto::testEncryptDecryptAll()
{
  // Test data
  const QString passphrase = "password";
  const QString algorithmName = "AES-128/GCM";

  const QStringList inputFileNames = {"test",
                                      "test.exe",
                                      "test.zip"};

  const QStringList encryptedFileNames = {"test.enc",
                                          "test.exe.enc",
                                          "test.zip.enc"};

  const QStringList decryptedFileNames = {"test (2)",
                                          "test (2).exe",
                                          "test (2).zip"};

  std::unique_ptr<Crypto> cryptography = make_unique<Crypto>();

  // Test encryption and decryption
  cryptography->encrypt(passphrase, inputFileNames, algorithmName);
  cryptography->decrypt(passphrase, encryptedFileNames);

  bool equivalentTest = true;
  bool completedTest = true;

  const int inputFilesSize = inputFileNames.size();

  for (int i = 0; i < inputFilesSize; ++i)
  { // Compare initial file to decrypted file
    const QString inputFileName = inputFileNames[i];
    const QString decryptedFileName = decryptedFileNames[i];

    std::ifstream inFile{inputFileName.toStdString().c_str(), std::ios::binary};
    std::ifstream outFile{decryptedFileName.toStdString().c_str(),
                          std::ios::binary};

    std::istreambuf_iterator<char> begin1(inFile);
    std::istreambuf_iterator<char> begin2(outFile);
    std::istreambuf_iterator<char> end;

    while (begin1 != end && begin2 != end)
    { // Check files are same char by char
      if (*begin1 != *begin2)
      {
        equivalentTest = false;
        break;
      }

      ++begin1;
      ++begin2;
    }

    completedTest = (begin1 == end) && (begin2 == end);

    if (!equivalentTest || !completedTest)
    { // If these two files weren't equivalent or completed, test failed
      break;
    }
  }

  // Clean up test files
  for (int i = 0; i < inputFilesSize; ++i)
  {
    const QString encryptedFileName = encryptedFileNames[i];
    QFile encryptedFile{encryptedFileName};

    if (encryptedFile.exists())
    {
      encryptedFile.remove();
    }

    const QString decryptedFileName = decryptedFileNames[i];
    QFile decryptedFile{decryptedFileName};

    if (decryptedFile.exists())
    {
      decryptedFile.remove();
    }
  }

  QVERIFY(equivalentTest && completedTest);
}

QTEST_MAIN(TestCrypto)
