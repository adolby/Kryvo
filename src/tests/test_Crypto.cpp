#include "cryptography/Crypto.hpp"
#include "PluginLoader.hpp"
#include "FileUtility.h"
#include "FileOperations.hpp"
#include "catch.hpp"
#include <QFileInfo>
#include <QFile>
#include <QStringList>
#include <QString>
#include <vector>

class EncryptionTestData {
 public:
  EncryptionTestData();

  EncryptionTestData(const QString& cryptoProvider,
                     const QString& compressionFormat,
                     const QString& passphrase, const QString& cipher,
                     std::size_t keySize, const QString& modeOfOperation,
                     const QFileInfo& inputFilePath,
                     const QFileInfo& encryptedFilePath,
                     const QFileInfo& decryptedFilePath);

  QString cryptoProvider;
  QString compressionFormat;
  QString passphrase;
  QString cipher;
  std::size_t keySize = 0;
  QString modeOfOperation;
  QFileInfo inputFilePath;
  QFileInfo encryptedFilePath;
  QFileInfo decryptedFilePath;
};

EncryptionTestData::EncryptionTestData() = default;

EncryptionTestData::EncryptionTestData(const QString& cryptoProvider,
                                       const QString& compressionFormat,
                                       const QString& passphrase,
                                       const QString& cipher,
                                       std::size_t keySize,
                                       const QString& modeOfOperation,
                                       const QFileInfo& inputFilePath,
                                       const QFileInfo& encryptedFilePath,
                                       const QFileInfo& decryptedFilePath)
  : cryptoProvider(cryptoProvider), compressionFormat(compressionFormat),
    passphrase(passphrase), cipher(cipher), keySize(keySize),
    modeOfOperation(modeOfOperation), inputFilePath(inputFilePath),
    encryptedFilePath(encryptedFilePath), decryptedFilePath(decryptedFilePath) {
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
  testDataVector.emplace_back(QStringLiteral("Botan"), QStringLiteral("gzip"),
                              QStringLiteral("password"),
                              QStringLiteral("AES"), 128, QStringLiteral("GCM"),
                              QFileInfo(QStringLiteral("test1.txt")),
                              QFileInfo(QStringLiteral("test1.txt.enc")),
                              QFileInfo(QStringLiteral("test1 (2).txt")));

  // Executable file without compression
  testDataVector.emplace_back(QStringLiteral("Botan"), QStringLiteral("gzip"),
                              QStringLiteral("password2"),
                              QStringLiteral("AES"), 128, QStringLiteral("GCM"),
                              QFileInfo(QStringLiteral("test2.exe")),
                              QFileInfo(QStringLiteral("test2.exe.enc")),
                              QFileInfo(QStringLiteral("test2 (2).exe")));

  // Zip file without compression
  testDataVector.emplace_back(QStringLiteral("Botan"), QStringLiteral("gzip"),
                              QStringLiteral("password3"),
                              QStringLiteral("AES"), 128, QStringLiteral("GCM"),
                              QFileInfo(QStringLiteral("test3.zip")),
                              QFileInfo(QStringLiteral("test3.zip.enc")),
                              QFileInfo(QStringLiteral("test3 (2).zip")));

  Kryvo::DispatcherState state;

  Kryvo::PluginLoader pluginLoader;
  pluginLoader.loadPlugins();

  QHash<QString, QObject*> providers = pluginLoader.cryptoProviders();

  Kryvo::Crypto cryptographer(&state);

  cryptographer.updateProviders(providers);

  for (const EncryptionTestData& etd : testDataVector) {
    const QString& inputFilePath = etd.inputFilePath.absoluteFilePath();

    GIVEN("Test file: " + inputFilePath.toStdString()) {
      const QFileInfo inputFileInfo(etd.inputFilePath);

      const QString& msgMissing = QStringLiteral("Test file %1 is missing.");

      if (!inputFileInfo.exists()) {
        FAIL(msgMissing.arg(inputFilePath).toStdString());
      }

      WHEN("Encrypting and decrypting file: " + inputFilePath.toStdString()) {
        const std::size_t id = 0;

        // Test encryption and decryption
        const bool encrypted =
          cryptographer.encrypt(id, etd.cryptoProvider, etd.compressionFormat,
                                etd.passphrase, etd.inputFilePath,
                                etd.encryptedFilePath,
                                etd.cipher,
                                static_cast<std::size_t>(etd.keySize),
                                etd.modeOfOperation);

        REQUIRE(encrypted);

        QFile inFile(etd.encryptedFilePath.absoluteFilePath());

        const bool inFileOpen = inFile.open(QIODevice::ReadOnly);

        if (!inFileOpen) {
          FAIL(msgMissing.arg(
            etd.encryptedFilePath.absoluteFilePath()).toStdString());
        }

        const QHash<QByteArray, QByteArray>& headers =
          Kryvo::readHeader(&inFile);

        if (!headers.contains(QByteArrayLiteral("Version"))) {
          const QString& headerError = QStringLiteral("Header error in %1");
          FAIL(headerError.arg(
            etd.encryptedFilePath.absoluteFilePath()).toStdString());
        }

        const QString& versionString =
          headers.value(QByteArrayLiteral("Version"));

        bool conversionOk = false;

        const int fileVersion = versionString.toInt(&conversionOk);

        const QByteArray& cryptoProvider =
          headers.value(QByteArrayLiteral("Cryptography provider"));

        const QByteArray& compressionFormat =
          headers.value(QByteArrayLiteral("Compression format"));

        const QByteArray& algorithmName =
          headers.value(QByteArrayLiteral("Algorithm name"));

        const QByteArray& keySize =
          headers.value(QByteArrayLiteral("Key size"));

        const QByteArray& pbkdfSalt =
          headers.value(QByteArrayLiteral("PBKDF salt"));
        const QByteArray& keySalt =
          headers.value(QByteArrayLiteral("Key salt"));
        const QByteArray& ivSalt = headers.value(QByteArrayLiteral("IV salt"));

        inFile.close();

        const bool decrypted =
          cryptographer.decrypt(id, etd.cryptoProvider, etd.passphrase,
                                etd.encryptedFilePath, etd.decryptedFilePath,
                                algorithmName, keySize, pbkdfSalt, keySalt,
                                ivSalt);

        REQUIRE(decrypted);

        // Compare initial file with decrypted file
        const bool equivalentTest =
          FileOperations::filesEqual(etd.inputFilePath.absoluteFilePath(),
                                     etd.decryptedFilePath.absoluteFilePath());

        // Clean up test files
        QFile encryptedFile(etd.encryptedFilePath.absoluteFilePath());

        if (encryptedFile.exists()) {
          encryptedFile.remove();
        }

        QFile decryptedFile(etd.decryptedFilePath.absoluteFilePath());

        if (decryptedFile.exists()) {
          decryptedFile.remove();
        }

        THEN("Decrypted file matches plaintext file: " +
             inputFilePath.toStdString()) {
          REQUIRE(equivalentTest);
        }
      }
    }
  }
}
