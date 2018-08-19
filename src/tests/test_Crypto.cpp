#include "test_Crypto.hpp"
#include "FileOperations.h"
#include "cryptography/Crypto.hpp"
#include <QTest>
#include <QFile>
#include <QThread>
#include <QString>
#include <memory>

void TestCrypto::testComparatorSameFile() {
  // Test data
  const QString& fileName1 = QStringLiteral("file1.png");
  const QString& fileName2 = QStringLiteral("file2.png");

  const QFile file1(fileName1);
  const QFile file2(fileName2);

  if (!file1.exists() || !file2.exists()) {
    const QString& msg = QStringLiteral("Test file %1 is missing. ");

    QString message;

    if (!file1.exists()) {
      message += msg.arg(fileName1);
    }

    if (!file2.exists()) {
      message += msg.arg(fileName2);
    }

    const auto* messages = reinterpret_cast<const char*>(message.constData());

    QSKIP(messages);
  }

  const bool equivalentTest = FileOperations::filesEqual(fileName1, fileName2);

  QVERIFY(equivalentTest);
}

void TestCrypto::testComparatorDifferentFile() {
  // Test data
  const QString& fileName1 = QStringLiteral("file1.png");
  const QString& fileName2 = QStringLiteral("file3.png");

  const QFile file1(fileName1);
  const QFile file2(fileName2);

  if (!file1.exists() || !file2.exists()) {
    QString message;
    const QString& msg = QStringLiteral("Test file %1 is missing. ");

    if (!file1.exists()) {
      message += msg.arg(fileName1);
    }

    if (!file2.exists()) {
      message += msg.arg(fileName2);
    }

    const auto* messages = reinterpret_cast<const char*>(message.constData());

    QSKIP(messages);
  }

  const bool equivalentTest = FileOperations::filesEqual(fileName1, fileName2);

  QVERIFY(!equivalentTest);
}

void TestCrypto::testEncryptDecrypt_data() {
  QTest::addColumn<QString>("passphrase");
  QTest::addColumn<QString>("cipher");
  QTest::addColumn<int>("keySize");
  QTest::addColumn<QString>("modeOfOperation");
  QTest::addColumn<bool>("compress");
  QTest::addColumn<QString>("inputFileName");
  QTest::addColumn<QString>("encryptedFileName");
  QTest::addColumn<QString>("decryptedFileName");

  QTest::newRow("Text file with compression")
    << "password" << "AES" << 128 << "GCM" << true << "test.txt"
    << "test.txt.enc" << "test (2).txt";

  QTest::newRow("Executable file with compression")
    << "password2" << "AES" << 128 << "GCM" << true << "test.exe"
    << "test.exe.enc" << "test (2).exe";

  QTest::newRow("Zip file with compression")
    << "password3" << "AES" << 128 << "GCM" << true << "test.zip"
    << "test.zip.enc" << "test (2).zip";

  QTest::newRow("Text file without compression")
    << "password" << "AES" << 128 << "GCM" << true << "test.txt"
    << "test.txt.enc" << "test (2).txt";

  QTest::newRow("Executable file without compression")
    << "password2" << "AES" << 128 << "GCM" << true << "test.exe"
    << "test.exe.enc" << "test (2).exe";

  QTest::newRow("Zip file without compression")
    << "password3" << "AES" << 128 << "GCM" << true << "test.zip"
    << "test.zip.enc" << "test (2).zip";
}

void TestCrypto::testEncryptDecrypt() {
  QFETCH(QString, passphrase);
  QFETCH(QString, cipher);
  QFETCH(int, keySize);
  QFETCH(QString, modeOfOperation);
  QFETCH(bool, compress);
  QFETCH(QString, inputFileName);
  QFETCH(QString, encryptedFileName);
  QFETCH(QString, decryptedFileName);

  const QFile inputFile(inputFileName);

  if (!inputFile.exists()) {
    const QString& msg = QStringLiteral("Test file %1 is missing.");
    const QString& message = msg.arg(inputFileName);
    const auto* messageString =
        reinterpret_cast<const char*>(message.constData());

    QSKIP(messageString);
  }

  QStringList inputFileNames;
  inputFileNames.append(inputFileName);

  QStringList encryptedFileNames;
  encryptedFileNames.append(encryptedFileName);

  Kryvo::Crypto cryptography;

  QString outputPath;

  // Test encryption and decryption
  cryptography.encrypt(passphrase, inputFileNames, outputPath, cipher,
                       static_cast<std::size_t>(keySize), modeOfOperation,
                       compress, false);
  cryptography.decrypt(passphrase, encryptedFileNames);

  // Compare initial file to decrypted file
  const bool equivalentTest =
        FileOperations::filesEqual(inputFileName, decryptedFileName);

  // Clean up test files
  QFile encryptedFile(encryptedFileName);

  if (encryptedFile.exists()) {
    encryptedFile.remove();
  }

  QFile decryptedFile(decryptedFileName);

  if (decryptedFile.exists()) {
    decryptedFile.remove();
  }

  QVERIFY(equivalentTest);
}

void TestCrypto::testEncryptDecryptAll() {
  // Test data
  const QString& passphrase = QStringLiteral("password");
  const QString& cipher = QStringLiteral("AES");
  const std::size_t keySize = 128;
  const QString& modeOfOperation = QStringLiteral("GCM");
  const bool compress = true;

  const QStringList& inputFileNames = {QStringLiteral("test.txt"),
                                       QStringLiteral("test.exe"),
                                       QStringLiteral("test.zip")};

  bool skip = false;
  QString message;

  for (const QString& inputFileName : inputFileNames) {
    const QFile inputFile(inputFileName);

    if (!inputFile.exists()) {
      message +=
        QStringLiteral("\nTest file %1 is missing.").arg(inputFileName);
      skip = true;
    }
  }

  if (skip) {
    const auto* msg = reinterpret_cast<const char*>(message.constData());

    QSKIP(msg);
  }

  const QStringList& encryptedFileNames = {QStringLiteral("test.txt.enc"),
                                           QStringLiteral("test.exe.enc"),
                                           QStringLiteral("test.zip.enc")};

  const QStringList& decryptedFileNames = {QStringLiteral("test (2).txt"),
                                           QStringLiteral("test (2).exe"),
                                           QStringLiteral("test (2).zip")};

  Kryvo::Crypto cryptography;

  QString outputPath;

  // Test encryption and decryption
  cryptography.encrypt(passphrase, inputFileNames, outputPath, cipher, keySize,
                       modeOfOperation, compress, false);
  cryptography.decrypt(passphrase, encryptedFileNames);

  bool equivalentTest = false;

  const int inputFilesSize = inputFileNames.size();
  for (int i = 0; i < inputFilesSize; ++i) {
    const QString& inputFileName = inputFileNames[i];
    const QString& decryptedFileName = decryptedFileNames[i];

    // Compare initial file to decrypted file
    equivalentTest =
          FileOperations::filesEqual(inputFileName, decryptedFileName);

    if (!equivalentTest) { // If these two files weren't equivalent, test failed
      break;
    }
  }

  // Clean up test files
  for (int i = 0; i < inputFilesSize; ++i) {
    const QString& encryptedFileName = encryptedFileNames[i];
    QFile encryptedFile(encryptedFileName);

    if (encryptedFile.exists()) {
      encryptedFile.remove();
    }

    const QString& decryptedFileName = decryptedFileNames[i];
    QFile decryptedFile(decryptedFileName);

    if (decryptedFile.exists()) {
      decryptedFile.remove();
    }
  }

  QVERIFY(equivalentTest);
}

QTEST_GUILESS_MAIN(TestCrypto)
