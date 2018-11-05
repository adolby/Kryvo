#include "FileOperations.h"
#include "cryptography/Crypto.hpp"
#include "catch.hpp"
#include <QFile>
#include <QStringList>
#include <QString>
#include <memory>

class EncryptionTestData {
 public:
  EncryptionTestData(const QString& passphrase, const QString& cipher,
                     std::size_t keySize, const QString& modeOfOperation,
                     bool compress, const QStringList& inputFileName,
                     const QString& encryptedFileName,
                     const QString& decryptedFileName);

  QString passphrase;
  QString cipher;
  std::size_t keySize = 0;
  QString modeOfOperation;
  bool compress = false;
  QStringList inputFileNames;
  QStringList encryptedFileNames;
  QStringList decryptedFileNames;
};

EncryptionTestData::EncryptionTestData(const QString& passphrase,
                                       const QString& cipher,
                                       std::size_t keySize,
                                       const QString& modeOfOperation,
                                       bool compress,
                                       const QStringList& inputFileNames,
                                       const QString& encryptedFileName,
                                       const QString& decryptedFileName)
  : passphrase(passphrase), cipher(cipher), keySize(keySize),
    modeOfOperation(modeOfOperation), compress(compress),
    inputFileNames(inputFileNames), encryptedFileNames(encryptedFileNames),
    decryptedFileNames(decryptedFileNames) {
}

/*!
 * testEncryptDecrypt Crypto's encryption operation randomly salts the key and
 * IV. This means that encryption and decryption will need to be tested
 * together. Tests the encryption/decryption process with single test files as
 * input.
 */
SCENARIO("Test encryption and decryption on various files with single file as "
         "input", "[testEncryptDecrypt]") {
  std::vector<EncryptionTestData> testDataVector;

  // Text file with compression
  testDataVector.emplace_back(QStringLiteral("password"),
                              QStringLiteral("AES"), 128, QStringLiteral("GCM"),
                              true, QStringList{QStringLiteral("test.txt")},
                              QStringList{QStringLiteral("test.txt.enc")},
                              QStringList{QStringLiteral("test (2).txt")});

  // Executable file with compression
  testDataVector.emplace_back(QStringLiteral("password2"),
                              QStringLiteral("AES"), 128, QStringLiteral("GCM"),
                              true, QStringList{QStringLiteral("test.exe")},
                              QStringList{QStringLiteral("test.exe.enc")},
                              QStringList{QStringLiteral("test (2).exe")});

  // Zip file with compression
  testDataVector.emplace_back(QStringLiteral("password3"),
                              QStringLiteral("AES"), 128, QStringLiteral("GCM"),
                              true, QStringList{QStringLiteral("test.zip")},
                              QStringList{QStringLiteral("test.zip.enc")},
                              QStringList{QStringLiteral("test (2).zip")});

  // Text file without compression
  testDataVector.emplace_back(QStringLiteral("password"),
                              QStringLiteral("AES"), 128, QStringLiteral("GCM"),
                              false, QStringList{QStringLiteral("test.txt")},
                              QStringList{QStringLiteral("test.txt.enc")},
                              QStringList{QStringLiteral("test (2).txt")});

  // Executable file without compression
  testDataVector.emplace_back(QStringLiteral("password2"),
                              QStringLiteral("AES"), 128, QStringLiteral("GCM"),
                              false, QStringList{QStringLiteral("test.exe")},
                              QStringList{QStringLiteral("test.exe.enc")},
                              QStringList{QStringLiteral("test (2).exe")});

  // Zip file without compression
  testDataVector.emplace_back(QStringLiteral("password3"),
                              QStringLiteral("AES"), 128, QStringLiteral("GCM"),
                              false, QStringList{QStringLiteral("test.zip")},
                              QStringList{QStringLiteral("test.zip.enc")},
                              QStringList{QStringLiteral("test (2).zip")});

  for (const EncryptionTestData& etd : testDataVector) {
    GIVEN("A test file") {
      const QFile inputFile(etd.inputFileNames.at(0));

      REQUIRE(inputFile.exists());

      WHEN("Encrypting and decrypting") {
        Kryvo::Crypto cryptography;

        QString outputPath;

        // Test encryption and decryption
        cryptography.encrypt(etd.passphrase, etd.inputFileNames, outputPath, etd.cipher,
                             static_cast<std::size_t>(etd.keySize),
                             etd.modeOfOperation, etd.compress, false);

        cryptography.decrypt(etd.passphrase, etd.encryptedFileNames);

        // Compare initial file to decrypted file
        const bool equivalentTest =
          FileOperations::filesEqual(etd.inputFileNames.at(0),
                                     etd.decryptedFileNames.at(0));

        // Clean up test files
        QFile encryptedFile(etd.encryptedFileNames.at(0));

        if (encryptedFile.exists()) {
          encryptedFile.remove();
        }

        QFile decryptedFile(etd.decryptedFileNames.at(0));

        if (decryptedFile.exists()) {
          decryptedFile.remove();
        }

        THEN ("decrypted file matches plaintext file") {
          REQUIRE(equivalentTest);
        }
      }
    }
  }
}

/*!
 * testEncryptDecryptMultiple Crypto's encryption operation randomly salts the
 * key and IV. This means that encryption and decryption will need to be tested
 * together. Tests the encryption and decryption process with multiple test
 * files as input.
 */
SCENARIO("Test encryption and decryption on various file types with multiple "
         "files as input", "[testEncryptDecryptMultiple]") {
  GIVEN("Test files") {
    std::vector<EncryptionTestData> testDataVector;

    QString message;

    QStringList inputFileNames{QStringLiteral("test.txt"),
                               QStringLiteral("test.exe"),
                               QStringLiteral("test.zip")};

    for (const QString& inputFileName : inputFileNames) {
      const QFile inputFile(inputFileName);

      REQUIRE(!inputFile.exists());
    }

    testDataVector.emplace_back(QStringLiteral("password"),
                                QStringLiteral("AES"), 128, QStringLiteral("GCM"),
                                true, inputFileNames,
                                QStringList{QStringLiteral("test.txt.enc"),
                                            QStringLiteral("test.exe.enc"),
                                            QStringLiteral("test.zip.enc")},
                                QStringList{QStringLiteral("test (2).txt"),
                                            QStringLiteral("test (2).exe"),
                                            QStringLiteral("test (2).zip")});

    WHEN("Encrypting and decrypting files") {
      const EncryptionTestData& etd = testDataVector.at(0);

      Kryvo::Crypto cryptography;

      QString outputPath;

      // Test encryption and decryption
      cryptography.encrypt(etd.passphrase, etd.inputFileNames, outputPath,
                           etd.cipher, etd.keySize, etd.modeOfOperation,
                           etd.compress, false);

      cryptography.decrypt(etd.passphrase, etd.encryptedFileNames);

      bool equivalentTest = false;

      const int inputFilesSize = inputFileNames.size();
      for (int i = 0; i < inputFilesSize; ++i) {
        const QString& inputFileName = inputFileNames.at(i);
        const QString& decryptedFileName = etd.decryptedFileNames.at(i);

        // Compare initial file to decrypted file
        equivalentTest =
          FileOperations::filesEqual(inputFileName, decryptedFileName);

        if (!equivalentTest) {
          // If these two files aren't equivalent then the test failed
          break;
        }
      }

      // Clean up test files
      for (int i = 0; i < inputFilesSize; ++i) {
        const QString& encryptedFileName = etd.encryptedFileNames.at(i);
        QFile encryptedFile(encryptedFileName);

        if (encryptedFile.exists()) {
          encryptedFile.remove();
        }

        const QString& decryptedFileName = etd.decryptedFileNames.at(i);
        QFile decryptedFile(decryptedFileName);

        if (decryptedFile.exists()) {
          decryptedFile.remove();
        }
      }

      THEN("all decrypted files match plaintext files") {
        REQUIRE(equivalentTest);
      }
    }
  }
}
