#include "cryptography/Crypto.hpp"
#include "SchedulerState.hpp"
#include "PluginLoader.hpp"
#include "Plugin.hpp"
#include "FileUtility.h"
#include "FileOperations.hpp"
#include "doctest.h"
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
                     const QFileInfo& inputFileInfo,
                     const QFileInfo& encryptedFileInfo,
                     const QFileInfo& decryptedFileInfo);

  QString cryptoProvider;
  QString compressionFormat;
  QString passphrase;
  QString cipher;
  std::size_t keySize = 0;
  QString modeOfOperation;
  QFileInfo inputFileInfo;
  QFileInfo encryptedFileInfo;
  QFileInfo decryptedFileInfo;
};

EncryptionTestData::EncryptionTestData() = default;

EncryptionTestData::EncryptionTestData(const QString& cryptoProvider,
                                       const QString& compressionFormat,
                                       const QString& passphrase,
                                       const QString& cipher,
                                       std::size_t keySize,
                                       const QString& modeOfOperation,
                                       const QFileInfo& inputFileInfo,
                                       const QFileInfo& encryptedFileInfo,
                                       const QFileInfo& decryptedFileInfo)
  : cryptoProvider(cryptoProvider), compressionFormat(compressionFormat),
    passphrase(passphrase), cipher(cipher), keySize(keySize),
    modeOfOperation(modeOfOperation), inputFileInfo(inputFileInfo),
    encryptedFileInfo(encryptedFileInfo), decryptedFileInfo(decryptedFileInfo) {
}

/*!
 * testEncryptDecrypt Crypto's encryption operation randomly salts the key and
 * IV. This means that encryption and decryption will need to be tested
 * together. Tests the encryption/decryption process with various file types as
 * input.
 */
SCENARIO("testEncryptDecrypt") {
  INFO("Test encryption and decryption on various file types");

  std::vector<EncryptionTestData> testDataVector;

  // Text file without compression
  testDataVector.emplace_back(QStringLiteral("Botan"), QStringLiteral("gzip"),
                              QStringLiteral("password"),
                              QStringLiteral("AES"), 128, QStringLiteral("GCM"),
                              QFileInfo(QStringLiteral("test1.txt")),
                              QFileInfo(QStringLiteral("test1.txt.enc")),
                              QFileInfo(QStringLiteral("test1 (2).txt")));

  testDataVector.emplace_back(QStringLiteral("OpenSsl"), QStringLiteral("gzip"),
                              QStringLiteral("password"),
                              QStringLiteral("AES"), 128, QStringLiteral("GCM"),
                              QFileInfo(QStringLiteral("test1.txt")),
                              QFileInfo(QStringLiteral("test1.txt.enc")),
                              QFileInfo(QStringLiteral("test1 (2).txt")));

  // Executable file without compression
  testDataVector.emplace_back(QStringLiteral("Botan"), QStringLiteral("gzip"),
                              QStringLiteral("password2"),
                              QStringLiteral("AES"), 256, QStringLiteral("GCM"),
                              QFileInfo(QStringLiteral("test2.exe")),
                              QFileInfo(QStringLiteral("test2.exe.enc")),
                              QFileInfo(QStringLiteral("test2 (2).exe")));

  testDataVector.emplace_back(QStringLiteral("OpenSsl"), QStringLiteral("gzip"),
                              QStringLiteral("password"),
                              QStringLiteral("AES"), 256, QStringLiteral("GCM"),
                              QFileInfo(QStringLiteral("test2.exe")),
                              QFileInfo(QStringLiteral("test2.exe.enc")),
                              QFileInfo(QStringLiteral("test2 (2).exe")));

  // Zip file without compression
  testDataVector.emplace_back(QStringLiteral("Botan"), QStringLiteral("gzip"),
                              QStringLiteral("password3"),
                              QStringLiteral("AES"), 256, QStringLiteral("GCM"),
                              QFileInfo(QStringLiteral("test3.zip")),
                              QFileInfo(QStringLiteral("test3.zip.enc")),
                              QFileInfo(QStringLiteral("test3 (2).zip")));

  testDataVector.emplace_back(QStringLiteral("OpenSsl"), QStringLiteral("gzip"),
                              QStringLiteral("password3"),
                              QStringLiteral("AES"), 256, QStringLiteral("GCM"),
                              QFileInfo(QStringLiteral("test3.zip")),
                              QFileInfo(QStringLiteral("test3.zip.enc")),
                              QFileInfo(QStringLiteral("test3 (2).zip")));

  Kryvo::SchedulerState state;

  Kryvo::PluginLoader pluginLoader;
  pluginLoader.loadPlugins();

  const QHash<QString, Kryvo::Plugin> providers =
    pluginLoader.cryptoProviders();

  Kryvo::Crypto cryptographer(&state);

  cryptographer.updateProviders(providers);

  for (const EncryptionTestData& etd : testDataVector) {
    const QString inputFilePath = etd.inputFileInfo.absoluteFilePath();

    GIVEN("A test file") {
      const QString filePathMessage =
        QStringLiteral("File name: %1").arg(inputFilePath);

      INFO(filePathMessage.toStdString());

      const QString msgMissing = QStringLiteral("Test file %1 is missing.");

      if (!etd.inputFileInfo.exists()) {
        FAIL(msgMissing.arg(inputFilePath).toStdString());
      }

      WHEN("Encrypting and decrypting a file") {
        const std::size_t id = 0;

        // Test encryption and decryption
        Kryvo::EncryptFileConfig encryptFileConfig;
        encryptFileConfig.encrypt.provider = etd.cryptoProvider;
        encryptFileConfig.encrypt.compressionFormat = etd.compressionFormat;
        encryptFileConfig.encrypt.passphrase = etd.passphrase;
        encryptFileConfig.encrypt.keySize =
          static_cast<std::size_t>(etd.keySize);
        encryptFileConfig.encrypt.modeOfOperation = etd.modeOfOperation;
        encryptFileConfig.inputFileInfo = etd.inputFileInfo;
        encryptFileConfig.outputFileInfo = etd.encryptedFileInfo;

        const int encrypted = cryptographer.encryptFile(id, encryptFileConfig);

        QFile inFile(etd.encryptedFileInfo.absoluteFilePath());

        const bool inFileOpen = inFile.open(QIODevice::ReadOnly);

        if (!inFileOpen) {
          FAIL(
            msgMissing.arg(
              etd.encryptedFileInfo.absoluteFilePath()).toStdString());
        }

        const QHash<QByteArray, QByteArray> headers =
          Kryvo::readHeader(&inFile);

        if (!headers.contains(QByteArrayLiteral("Version"))) {
          const QString headerError = QStringLiteral("Header error in %1");
          FAIL(
            headerError.arg(
              etd.encryptedFileInfo.absoluteFilePath()).toStdString());
        }

        inFile.close();

        Kryvo::DecryptFileConfig decryptFileConfig;
        decryptFileConfig.decrypt.provider = etd.cryptoProvider;
        decryptFileConfig.decrypt.passphrase = etd.passphrase;
        decryptFileConfig.inputFileInfo = etd.encryptedFileInfo;
        decryptFileConfig.outputFileInfo = etd.decryptedFileInfo;

        const bool decrypted = cryptographer.decryptFile(id, decryptFileConfig);

        // Compare initial file with decrypted file
        const bool equivalentTest =
          FileOperations::filesEqual(etd.inputFileInfo.absoluteFilePath(),
                                     etd.decryptedFileInfo.absoluteFilePath());

        // Clean up test files
        QFile encryptedFile(etd.encryptedFileInfo.absoluteFilePath());

        if (encryptedFile.exists()) {
          encryptedFile.remove();
        }

        QFile decryptedFile(etd.decryptedFileInfo.absoluteFilePath());

        if (decryptedFile.exists()) {
          decryptedFile.remove();
        }

        THEN("Decrypted file matches plaintext file") {
          REQUIRE(encrypted == 1);
          REQUIRE(decrypted == 1);
          REQUIRE(equivalentTest);
        }
      }
    }
  }
}
