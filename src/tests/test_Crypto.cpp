#include "cryptography/Crypto.hpp"
#include "FileOperations.hpp"
#include "catch.hpp"
#include <QFileInfo>
#include <QFile>
#include <QStringList>
#include <QString>
#include <vector>

class EncryptionTestData {
 public:
  EncryptionTestData(const QString& passphrase, const QString& cipher,
                     std::size_t keySize, const QString& modeOfOperation,
                     bool compress, const QString& inputFilePath,
                     const QString& encryptedFilePath,
                     const QString& decryptedFilePath);

  QString passphrase;
  QString cipher;
  std::size_t keySize = 0;
  QString modeOfOperation;
  bool compress = false;
  QString inputFilePath;
  QString encryptedFilePath;
  QString decryptedFilePath;
};

EncryptionTestData::EncryptionTestData(const QString& passphrase,
                                       const QString& cipher,
                                       std::size_t keySize,
                                       const QString& modeOfOperation,
                                       bool compress,
                                       const QString& inputFilePath,
                                       const QString& encryptedFilePath,
                                       const QString& decryptedFilePath)
  : passphrase(passphrase), cipher(cipher), keySize(keySize),
    modeOfOperation(modeOfOperation), compress(compress),
    inputFilePath(inputFilePath), encryptedFilePath(encryptedFilePath),
    decryptedFilePath(decryptedFilePath) {
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
                              false, QStringLiteral("test1.txt"),
                              QStringLiteral("test1.txt.enc"),
                              QStringLiteral("test1 (2).txt"));

  // Executable file without compression
  testDataVector.emplace_back(QStringLiteral("password2"),
                              QStringLiteral("AES"), 128, QStringLiteral("GCM"),
                              false, QStringLiteral("test2.exe"),
                              QStringLiteral("test2.exe.enc"),
                              QStringLiteral("test2 (2).exe"));

  // Zip file without compression
  testDataVector.emplace_back(QStringLiteral("password3"),
                              QStringLiteral("AES"), 128, QStringLiteral("GCM"),
                              false, QStringLiteral("test3.zip"),
                              QStringLiteral("test3.zip.enc"),
                              QStringLiteral("test3 (2).zip"));

  Kryvo::DispatcherState state;
  Kryvo::Crypto cryptographer(&state);

  for (const EncryptionTestData& etd : testDataVector) {
    GIVEN("Test file: " + etd.inputFilePath.toStdString()) {
      const QFileInfo inputFileInfo(etd.inputFilePath);

      const QString& msgMissing = QStringLiteral("Test file %1 is missing.");

      if (!inputFileInfo.exists()) {
        FAIL(msgMissing.arg(etd.inputFilePath).toStdString());
      }

      WHEN("Encrypting and decrypting file: " +
           etd.inputFilePath.toStdString()) {
        const std::size_t id = 0;

        // Test encryption and decryption
        const bool encrypted =
          cryptographer.encrypt(id, etd.passphrase, etd.inputFilePath,
                                etd.encryptedFilePath, etd.cipher,
                                static_cast<std::size_t>(etd.keySize),
                                etd.modeOfOperation, etd.compress);

        Q_ASSERT(encrypted);

        QFile inFile(etd.encryptedFilePath);

        const bool inFileOpen = inFile.open(QIODevice::ReadOnly);

        if (!inFileOpen) {
          FAIL(msgMissing.arg(etd.encryptedFilePath).toStdString());
        }

        // Read metadata from file

        // Read line but skip \n
        auto readLine = [](QFile* file) {
          if (file) {
            QByteArray line = file->readLine();
            return line.replace(QByteArrayLiteral("\n"), QByteArrayLiteral(""));
          }

          return QByteArray();
        };

        const QByteArray& headerString = readLine(&inFile);

        if (headerString !=
            QByteArrayLiteral("-------- ENCRYPTED FILE --------")) {
          const QString& headerError = QStringLiteral("Header error in %1");

          FAIL(headerError.arg(etd.encryptedFilePath).toStdString());
        }

        const QString& algorithmNameString = readLine(&inFile);
        const QString& keySizeString = readLine(&inFile);
        const QString& compressString = readLine(&inFile);

        Q_UNUSED(compressString);

        const QString& pbkdfSaltString = readLine(&inFile);
        const QString& keySaltString = readLine(&inFile);
        const QString& ivSaltString = readLine(&inFile);

        const QByteArray& footerString = readLine(&inFile);

        if (footerString !=
            QByteArrayLiteral("---------------------------------")) {
          const QString& headerError = QStringLiteral("Footer error in %1");

          FAIL(headerError.arg(etd.encryptedFilePath).toStdString());
        }

        inFile.close();

        const bool decrypted =
          cryptographer.decrypt(id, etd.passphrase, etd.encryptedFilePath,
                                etd.decryptedFilePath, algorithmNameString,
                                keySizeString, pbkdfSaltString, keySaltString,
                                ivSaltString);

        Q_ASSERT(decrypted);

        // Compare initial file with decrypted file
        const bool equivalentTest =
          FileOperations::filesEqual(etd.inputFilePath, etd.decryptedFilePath);

        // Clean up test files
        QFile encryptedFile(etd.encryptedFilePath);

        if (encryptedFile.exists()) {
          encryptedFile.remove();
        }

        QFile decryptedFile(etd.decryptedFilePath);

        if (decryptedFile.exists()) {
          decryptedFile.remove();
        }

        THEN("Decrypted file matches plaintext file: " +
             etd.inputFilePath.toStdString()) {
          REQUIRE(equivalentTest);
        }
      }
    }
  }
}
