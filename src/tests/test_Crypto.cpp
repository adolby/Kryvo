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

#include "test_Crypto.hpp"
#include "cryptography/Crypto.hpp"
#include "utility/make_unique.h"
#include <QtTest/QTest>
#include <QtCore/QFile>
#include <QtCore/QThread>
#include <QtCore/QString>
#include <memory>

template <class InputIterator1, class InputIterator2>
bool equal(InputIterator1 first1, InputIterator1 last1,
            InputIterator2 first2, InputIterator2 last2)
{
  while ((first1 != last1) && (first2 != last2))
  {
    if (*first1 != *first2)
    {
      return false;
    }

    ++first1;
    ++first2;
  }

  return (first1 == last1) && (first2 == last2);
}

bool filesEqual(const QString& filePath1, const QString& filePath2)
{
  bool equivalent = false;

  QFile file1{filePath1};
  QFile file2{filePath2};

  if (file1.exists() && file2.exists())
  {
    file1.open(QFile::ReadOnly);
    uchar* fileMemory1 = file1.map(0, file1.size());

    file2.open(QFile::ReadOnly);
    uchar* fileMemory2 = file2.map(0, file2.size());

    equivalent =  equal(fileMemory1, fileMemory1 + file1.size(),
                        fileMemory2, fileMemory2 + file2.size());
  }

  return equivalent;
}

void TestCrypto::testComparatorSameFile()
{
  // Test data
  const QString fileName1 = QStringLiteral("file1.jpg");
  const QString fileName2 = QStringLiteral("file2.jpg");

  // Compare initial file to decrypted file
  auto equivalentTest = filesEqual(fileName1, fileName2);

  QVERIFY(equivalentTest);
}

void TestCrypto::testComparatorDifferentFile()
{
  // Test data
  const QString fileName1 = QStringLiteral("file1.jpg");
  const QString fileName3 = QStringLiteral("file3.jpg");

  // Compare initial file to decrypted file
  auto equivalentTest = filesEqual(fileName1, fileName3);

  QVERIFY(!equivalentTest);
}

void TestCrypto::testEncryptDecrypt_data()
{
  QTest::addColumn<QString>("passphrase");
  QTest::addColumn<QString>("algorithmName");
  QTest::addColumn<QString>("inputFileName");
  QTest::addColumn<QString>("encryptedFileName");
  QTest::addColumn<QString>("decryptedFileName");

  QTest::newRow("Small file") << "password" << "AES-128/GCM"
                                            << "test.txt" << "test.txt.enc"
                                            << "test (2).txt";

  QTest::newRow("Medium exe file") << "password2" << "AES-128/GCM"
                                   << "test.exe" << "test.exe.enc"
                                   << "test (2).exe";

  QTest::newRow("Large zip file") << "password3" << "AES-128/GCM"
                                  << "test.zip" << "test.zip.enc"
                                  << "test (2).zip";
}

void TestCrypto::testEncryptDecrypt()
{
  QFETCH(QString, passphrase);
  QFETCH(QString, algorithmName);
  QFETCH(QString, inputFileName);
  QFETCH(QString, encryptedFileName);
  QFETCH(QString, decryptedFileName);

  QStringList inputFileNames = {inputFileName};
  QStringList encryptedFileNames = {encryptedFileName};

  std::unique_ptr<Crypto> cryptography = make_unique<Crypto>();

  // Test encryption and decryption
  cryptography->encrypt(passphrase, inputFileNames, algorithmName);
  cryptography->decrypt(passphrase, encryptedFileNames);

  // Compare initial file to decrypted file
  auto equivalentTest = filesEqual(inputFileName, decryptedFileName);

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

  QVERIFY(equivalentTest);
}

void TestCrypto::testEncryptDecryptAll()
{
  // Test data
  const QString passphrase = QStringLiteral("password");
  const QString algorithmName = QStringLiteral("AES-128/GCM");

  const QStringList inputFileNames = {"test.txt",
                                      "test.exe",
                                      "test.zip"};

  const QStringList encryptedFileNames = {"test.txt.enc",
                                          "test.exe.enc",
                                          "test.zip.enc"};

  const QStringList decryptedFileNames = {"test (2).txt",
                                          "test (2).exe",
                                          "test (2).zip"};

  std::unique_ptr<Crypto> cryptography = make_unique<Crypto>();

  // Test encryption and decryption
  cryptography->encrypt(passphrase, inputFileNames, algorithmName);
  cryptography->decrypt(passphrase, encryptedFileNames);

  auto equivalentTest = false;

  const int inputFilesSize = inputFileNames.size();
  for (auto i = 0; i < inputFilesSize; ++i)
  {
    const QString inputFileName = inputFileNames[i];
    const QString decryptedFileName = decryptedFileNames[i];

    // Compare initial file to decrypted file
    equivalentTest = filesEqual(inputFileName, decryptedFileName);

    if (!equivalentTest)
    { // If these two files weren't equivalent, test failed
      break;
    }
  }

  // Clean up test files
  for (auto i = 0; i < inputFilesSize; ++i)
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

  QVERIFY(equivalentTest);
}

QTEST_MAIN(TestCrypto)
