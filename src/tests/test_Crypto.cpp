#include "FileOperations.hpp"
#include "cryptography/Crypto.hpp"
#include "catch.hpp"
#include <QFileInfo>
#include <QFile>
#include <QStringList>
#include <QString>

class EncryptionTestData {
 public:
  EncryptionTestData(const QString& passphrase, const QString& cipher,
                     std::size_t keySize, const QString& modeOfOperation,
                     bool compress, const QStringList& inputFileNames,
                     const QStringList& encryptedFileNames,
                     const QStringList& decryptedFileNames);

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
                                       const QStringList& encryptedFileNames,
                                       const QStringList& decryptedFileNames)
  : passphrase(passphrase), cipher(cipher), keySize(keySize),
    modeOfOperation(modeOfOperation), compress(compress),
    inputFileNames(inputFileNames), encryptedFileNames(encryptedFileNames),
    decryptedFileNames(decryptedFileNames) {
}

/*!
 * testEncryptDecrypt Crypto's encryption operation randomly salts the key and
 * IV. This means that encryption and decryption will need to be tested
 * together. Tests the encryption/decryption process with various file types as
 * input.
 */
SCENARIO("Test encryption and decryption on various file types",
         "[testEncryptDecrypt]") {
  std::vector<EncryptionTestData> testDataVector;

  // Text file without compression
  testDataVector.emplace_back(QStringLiteral("password"),
                              QStringLiteral("AES"), 128, QStringLiteral("GCM"),
                              false, QStringList{QStringLiteral("test1.txt")},
                              QStringList{QStringLiteral("test1.txt.enc")},
                              QStringList{QStringLiteral("test1 (2).txt")});

  // Executable file without compression
  testDataVector.emplace_back(QStringLiteral("password2"),
                              QStringLiteral("AES"), 128, QStringLiteral("GCM"),
                              false, QStringList{QStringLiteral("test2.exe")},
                              QStringList{QStringLiteral("test2.exe.enc")},
                              QStringList{QStringLiteral("test2 (2).exe")});

  // Zip file without compression
  testDataVector.emplace_back(QStringLiteral("password3"),
                              QStringLiteral("AES"), 128, QStringLiteral("GCM"),
                              false, QStringList{QStringLiteral("test3.zip")},
                              QStringList{QStringLiteral("test3.zip.enc")},
                              QStringList{QStringLiteral("test3 (2).zip")});

  Kryvo::Crypto cryptography;

  for (const EncryptionTestData& etd : testDataVector) {
    GIVEN("Test file: " + etd.inputFileNames.at(0).toStdString()) {
      const QFileInfo inputFileInfo(etd.inputFileNames.at(0));

      const QString& msgTemplate = QStringLiteral("Test file %1 is missing.");

      if (!inputFileInfo.exists()) {
        FAIL(msgTemplate.arg(etd.inputFileNames.at(0)).toStdString());
      }

      WHEN("Encrypting and decrypting: " +
           etd.inputFileNames.at(0).toStdString()) {
        // Test encryption and decryption
        const bool encryptSuccess =
          cryptography.encryptFiles(etd.passphrase, etd.inputFileNames,
                                    QString(), etd.cipher,
                                    static_cast<std::size_t>(etd.keySize),
                                    etd.modeOfOperation, etd.compress);

        REQUIRE(encryptSuccess);

        const bool decryptSuccess =
          cryptography.decryptFiles(etd.passphrase, etd.encryptedFileNames);

        REQUIRE(decryptSuccess);

        // Compare initial file with decrypted file
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

        THEN("Decrypted file matches plaintext file: " +
             etd.inputFileNames.at(0).toStdString()) {
          REQUIRE(equivalentTest);
        }
      }
    }
  }
}
