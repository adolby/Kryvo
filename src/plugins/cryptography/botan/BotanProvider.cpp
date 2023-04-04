#include "BotanProvider.hpp"
#include "SchedulerState.hpp"
#include "Constants.hpp"
#include "FileUtility.h"
#include <QSaveFile>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QStringBuilder>
#include <QString>
#include <QHash>
#include <string>
#include <stdexcept>

namespace Kryvo {

QString algorithmString(const QString& cipher, const QString& mode,
                        const std::size_t keySize) {
  QString algo = QString(cipher % QStringLiteral("/") % mode);

  if (QStringLiteral("AES") == cipher) {
    algo = QString(cipher % QStringLiteral("-") % QString::number(keySize) %
                   QStringLiteral("/") % mode);
  }

  return algo;
}

class BotanProviderPrivate {
  Q_DISABLE_COPY(BotanProviderPrivate)
  Q_DECLARE_PUBLIC(BotanProvider)

 public:
  explicit BotanProviderPrivate(BotanProvider* bp);

  void init(SchedulerState* s);

  bool encrypt(std::size_t id, const Kryvo::EncryptFileConfig& config);

  bool decrypt(std::size_t id, const Kryvo::DecryptFileConfig& config);

  bool encryptFile(std::size_t id, const Kryvo::EncryptFileConfig& config);

  bool decryptFile(std::size_t id, const Kryvo::DecryptFileConfig& config);

  bool executeCipher(std::size_t id,
                     CryptDirection direction,
                     QFile* inFile,
                     QSaveFile* outFile,
                     Botan::Pipe* pipe);

  BotanProvider* const q_ptr{nullptr};

  SchedulerState* state{nullptr};

  static const std::string keyLabel;
  static const std::string ivLabel;
  static const std::size_t pbkdfIterations;
};

const std::string BotanProviderPrivate::keyLabel =
  std::string("user secret");

const std::string BotanProviderPrivate::ivLabel =
  std::string("initialization vector");

const std::size_t BotanProviderPrivate::pbkdfIterations = 50000;

BotanProviderPrivate::BotanProviderPrivate(BotanProvider* bp)
  : q_ptr(bp) {
}

void BotanProviderPrivate::init(SchedulerState* s) {
  state = s;
}

bool BotanProviderPrivate::encrypt(std::size_t id,
                                   const Kryvo::EncryptFileConfig& config) {
  Q_Q(BotanProvider);
  Q_ASSERT(state);

  if (!state) {
    emit q->errorMessage(Constants::messages[0], QFileInfo());
    emit q->fileFailed(id);
    return false;
  }

  if (state->isAborted() || state->isStopped(id)) {
    emit q->errorMessage(Constants::messages[3], config.inputFileInfo);
    emit q->fileFailed(id);
    return false;
  }

  bool success = false;

  try {
    success = encryptFile(id, config);
  } catch (const Botan::Stream_IO_Error&) {
    emit q->errorMessage(Constants::messages[8], config.inputFileInfo);
    emit q->fileFailed(id);
    return false;
  } catch (const Botan::Invalid_Argument&) {
    emit q->errorMessage(Constants::messages[8], config.inputFileInfo);
    emit q->fileFailed(id);
    return false;
  } catch (const Botan::Exception& e) {
    emit q->errorMessage(QStringLiteral("Error: ") % QString(e.what()),
                         config.inputFileInfo);
    emit q->fileFailed(id);
    return false;
  } catch (const std::invalid_argument&) {
    emit q->errorMessage(Constants::messages[8], config.inputFileInfo);
    emit q->fileFailed(id);
    return false;
  } catch (const std::exception& e) {
    emit q->errorMessage(QStringLiteral("Error: ") % QString(e.what()),
                         config.inputFileInfo);
    emit q->fileFailed(id);
    return false;
  }

  if (state->isAborted() || state->isStopped(id)) {
    emit q->errorMessage(Constants::messages[3], config.inputFileInfo);
    emit q->fileFailed(id);

    return false;
  }

  return success;
}

bool BotanProviderPrivate::decrypt(std::size_t id,
                                   const Kryvo::DecryptFileConfig& config) {
  Q_Q(BotanProvider);
  Q_ASSERT(state);

  if (!state) {
    emit q->errorMessage(Constants::messages[0], QFileInfo());
    emit q->fileFailed(id);
    return false;
  }

  if (state->isAborted() || state->isStopped(id)) {
    emit q->errorMessage(Constants::messages[4], config.inputFileInfo);
    emit q->fileFailed(id);
    return false;
  }

  bool success = false;

  try {
    success = decryptFile(id, config);
  } catch (const Botan::Stream_IO_Error&) {
    emit q->errorMessage(Constants::messages[7], config.inputFileInfo);
    emit q->fileFailed(id);
    return false;
  } catch (const Botan::Invalid_Argument&) {
    emit q->errorMessage(Constants::messages[7], config.inputFileInfo);
    emit q->fileFailed(id);
    return false;
  } catch (const Botan::Lookup_Error&) {
    emit q->errorMessage(Constants::messages[7], config.inputFileInfo);
    emit q->fileFailed(id);
    return false;
  } catch (const Botan::Exception& e) {
    emit q->errorMessage(QStringLiteral("Error: ") % QString(e.what()),
                         config.inputFileInfo);
    emit q->fileFailed(id);
    return false;
  } catch (const std::invalid_argument&) {
    emit q->errorMessage(Constants::messages[7], config.inputFileInfo);
    emit q->fileFailed(id);
    return false;
  } catch (const std::exception& e) {
    emit q->errorMessage(QStringLiteral("Error: ") % QString(e.what()),
                         config.inputFileInfo);
    emit q->fileFailed(id);
    return false;
  }

  if (state->isAborted() || state->isStopped(id)) {
    emit q->errorMessage(Constants::messages[4], config.inputFileInfo);
    emit q->fileFailed(id);

    return false;
  }

  return success;
}

bool BotanProviderPrivate::encryptFile(std::size_t id,
                                       const Kryvo::EncryptFileConfig& config) {
  Q_Q(BotanProvider);
  Q_ASSERT(state);

  if (!state) {
    emit q->errorMessage(Constants::messages[0], QFileInfo());
    emit q->fileFailed(id);
    return false;
  }

  if (state->isAborted() || state->isStopped(id)) {
    emit q->fileFailed(id);
    return false;
  }

  if (!config.inputFileInfo.exists() || !config.inputFileInfo.isFile() ||
      !config.inputFileInfo.isReadable()) {
    emit q->errorMessage(Constants::messages[5], config.inputFileInfo);
    emit q->fileFailed(id);
    return false;
  }

  emit q->fileProgress(id, QObject::tr("Encrypting"), 0);

  Botan::AutoSeeded_RNG rng;

  // Define a size for the PBKDF salt vector
  const std::size_t pbkdfSaltSize = 256;
  Botan::secure_vector<Botan::byte> pbkdfSalt;
  pbkdfSalt.resize(pbkdfSaltSize);

  // Create random PBKDF salt
  rng.randomize(&pbkdfSalt[0], pbkdfSalt.size());

  // Set up the key derive functions
  const std::size_t hashSize = 512;

  // PKCS5_PBKDF2 takes ownership of the new HMAC and the HMAC takes ownership
  // of the SHA3 hash function object (both via unique_ptr)
  Botan::PKCS5_PBKDF2 pbkdf(new Botan::HMAC(new Botan::SHA_3(hashSize)));

  // Create the PBKDF key
  const std::size_t pbkdfKeySize = 256;

  const std::string passphraseString = config.passphrase.toStdString();

  Botan::secure_vector<Botan::byte> pbkdfKey =
    pbkdf.derive_key(pbkdfKeySize, passphraseString, &pbkdfSalt[0],
                     pbkdfSalt.size(), pbkdfIterations).bits_of();

  // Create the key and IV
  const QByteArray kdfHash = QByteArrayLiteral("HKDF(SHA-3(512))");

  std::unique_ptr<Botan::KDF> kdf(Botan::KDF::create(kdfHash.toStdString()));

  // Set up key salt size
  const std::size_t keySaltSize = 64;
  Botan::secure_vector<Botan::byte> keySalt;
  keySalt.resize(keySaltSize);
  rng.randomize(&keySalt[0], keySalt.size());

  // Key is constrained to sizes allowed by algorithm
  const std::size_t keySizeInBytes = config.keySize / 8;
  const auto* keyLabelVector =
    reinterpret_cast<const Botan::byte*>(keyLabel.data());
  Botan::SymmetricKey key(kdf->derive_key(keySizeInBytes,
                                          pbkdfKey.data(),
                                          pbkdfKey.size(),
                                          keySalt.data(),
                                          keySalt.size(),
                                          keyLabelVector,
                                          keyLabel.size()));

  // Set up IV salt size
  const std::size_t ivSaltSize = 64;
  Botan::secure_vector<Botan::byte> ivSalt;
  ivSalt.resize(ivSaltSize);
  rng.randomize(&ivSalt[0], ivSalt.size());

  const std::size_t ivSize = 256;
  const auto* ivLabelVector =
    reinterpret_cast<const Botan::byte*>(ivLabel.data());
  Botan::InitializationVector iv(kdf->derive_key(ivSize,
                                                 pbkdfKey.data(),
                                                 pbkdfKey.size(),
                                                 ivSalt.data(),
                                                 ivSalt.size(),
                                                 ivLabelVector,
                                                 ivLabel.size()));

  QFile inFile(config.inputFileInfo.absoluteFilePath());
  const bool inFileOpen = inFile.open(QIODevice::ReadOnly);

  if (!inFileOpen) {
    emit q->errorMessage(Constants::messages[5], config.inputFileInfo);
    emit q->fileFailed(id);
    return false;
  }

  QSaveFile outFile(config.outputFileInfo.absoluteFilePath());
  const bool outFileOpen = outFile.open(QIODevice::WriteOnly);

  if (!outFileOpen) {
    outFile.cancelWriting();
    emit q->errorMessage(Constants::messages[8], config.inputFileInfo);
    emit q->fileFailed(id);
    return false;
  }

  QHash<QByteArray, QByteArray> headerData;

  headerData.insert(QByteArrayLiteral("Version"),
                    QByteArray::number(Constants::fileVersion));
  headerData.insert(QByteArrayLiteral("Cryptography provider"),
                    QByteArrayLiteral("Botan"));

  if (!config.compressionFormat.isEmpty() &&
      config.compressionFormat != QStringLiteral("None")) {
    headerData.insert(QByteArrayLiteral("Compression format"),
                      config.compressionFormat.toUtf8());
  }

  headerData.insert(QByteArrayLiteral("Cipher"), config.cipher.toUtf8());

  headerData.insert(QByteArrayLiteral("Mode"), config.modeOfOperation.toUtf8());

  headerData.insert(QByteArrayLiteral("Key size"),
                    QByteArray::number(static_cast<uint>(config.keySize)));

  headerData.insert(QByteArrayLiteral("Hash function"),
                    QByteArrayLiteral("SHA-3"));

  headerData.insert(QByteArrayLiteral("Hash output size"),
                    QByteArrayLiteral("512"));

  const std::string pbkdfSaltString =
    Botan::base64_encode(&pbkdfSalt[0], pbkdfSalt.size());

  headerData.insert(QByteArrayLiteral("PBKDF salt"),
                    QString::fromStdString(pbkdfSaltString).toUtf8());

  const std::string keySaltString =
    Botan::base64_encode(&keySalt[0], keySalt.size());

  headerData.insert(QByteArrayLiteral("Key salt"),
                    QString::fromStdString(keySaltString).toUtf8());

  const std::string ivSaltString =
    Botan::base64_encode(&ivSalt[0], ivSalt.size());

  headerData.insert(QByteArrayLiteral("IV salt"),
                    QString::fromStdString(ivSaltString).toUtf8());

  writeHeader(&outFile, headerData);

  Botan::Pipe pipe;

  const QString algorithm = algorithmString(config.cipher,
                                            config.modeOfOperation,
                                            config.keySize);

  pipe.append_filter(Botan::get_cipher(algorithm.toStdString(), key,
                                       iv, Botan::ENCRYPTION));

  const bool success = executeCipher(id, CryptDirection::Encrypt,
                                     &inFile, &outFile, &pipe);

  if (!success) {
    outFile.cancelWriting();
    emit q->errorMessage(Constants::messages[8], config.inputFileInfo);
    emit q->fileFailed(id);
    return false;
  }

  if (state->isAborted() || state->isStopped(id)) {
    outFile.cancelWriting();
    emit q->fileFailed(id);
    return false;
  }

  outFile.commit();

  // Progress: finished
  emit q->fileProgress(id, QObject::tr("Encrypted"), 100);

  // Encryption success message
  emit q->statusMessage(
    Constants::messages[1].arg(config.inputFileInfo.absoluteFilePath()));

  emit q->fileCompleted(id);

  return success;
}

bool BotanProviderPrivate::decryptFile(std::size_t id,
                                       const Kryvo::DecryptFileConfig& config) {
  Q_Q(BotanProvider);
  Q_ASSERT(state);

  if (!state) {
    emit q->errorMessage(Constants::messages[0], QFileInfo());
    emit q->fileFailed(id);
    return false;
  }

  if (state->isAborted()) {
    emit q->fileFailed(id);
    return false;
  }

  if (!config.inputFileInfo.exists() || !config.inputFileInfo.isFile() ||
      !config.inputFileInfo.isReadable()) {
    emit q->errorMessage(Constants::messages[5], config.inputFileInfo);
    emit q->fileFailed(id);
    return false;
  }

  emit q->fileProgress(id, QObject::tr("Decrypting"), 0);

  QFile inFile(config.inputFileInfo.absoluteFilePath());

  const bool inFileOpen = inFile.open(QIODevice::ReadOnly);

  if (!inFileOpen) {
    emit q->errorMessage(Constants::messages[5], config.inputFileInfo);
    emit q->fileFailed(id);
    return false;
  }

  // Seek file to after header
  const QHash<QByteArray, QByteArray> header = readHeader(&inFile);

  QSaveFile outFile(config.outputFileInfo.absoluteFilePath());

  const bool outFileOpen = outFile.open(QIODevice::WriteOnly);

  if (!outFileOpen) {
    outFile.cancelWriting();
    emit q->errorMessage(Constants::messages[7], config.inputFileInfo);
    emit q->fileFailed(id);
    return false;
  }

  const QString versionString =
    QString(header.value(QByteArrayLiteral("Version")));

  bool conversionOk = false;

// Need to check file version when file format stabilizes
//  const int fileVersion = versionString.toInt(&conversionOk);

  const QByteArray cryptoProviderByteArray =
    header.value(QByteArrayLiteral("Cryptography provider"));

  const QByteArray compressionFormatByteArray =
    header.value(QByteArrayLiteral("Compression format"));

  const QByteArray cipherByteArray =
    header.value(QByteArrayLiteral("Cipher"));

  const QByteArray modeOfOperationByteArray =
    header.value(QByteArrayLiteral("Mode"));

  const QByteArray keySizeByteArray =
    header.value(QByteArrayLiteral("Key size"));

  const QByteArray pbkdfSaltByteArray =
    header.value(QByteArrayLiteral("PBKDF salt"));

  const QByteArray keySaltByteArray =
    header.value(QByteArrayLiteral("Key salt"));

  const QByteArray ivSaltByteArray =
    header.value(QByteArrayLiteral("IV salt"));

  const QByteArray hashFunctionByteArray =
    header.value(QByteArrayLiteral("Hash function"));

  const QByteArray hashSizeByteArray =
    header.value(QByteArrayLiteral("Hash output size"));

  const std::size_t hashSize =
    static_cast<std::size_t>(hashSizeByteArray.toInt());

  // Set up the key derive functions

  // PKCS5_PBKDF2 takes ownership of the new HMAC and the HMAC takes ownership
  // of the SHA3 hash function object (via unique_ptr)
  Botan::PKCS5_PBKDF2 pbkdf(new Botan::HMAC(new Botan::SHA_3(hashSize)));

  // Create the PBKDF key
  const Botan::secure_vector<Botan::byte> pbkdfSalt =
    Botan::base64_decode(pbkdfSaltByteArray.toStdString());

  const std::size_t pbkdfKeySize = 256;

  const std::string passphraseString = config.passphrase.toStdString();

  const Botan::secure_vector<Botan::byte> pbkdfKey =
    pbkdf.derive_key(pbkdfKeySize, passphraseString, &pbkdfSalt[0],
                     pbkdfSalt.size(), pbkdfIterations).bits_of();

  // Create the key and IV
  const QByteArray kdfHash = QByteArrayLiteral("HKDF(") +
                             hashFunctionByteArray +
                             QByteArrayLiteral("(") +
                             hashSizeByteArray +
                             QByteArrayLiteral("))");

  std::unique_ptr<Botan::KDF> kdf(Botan::KDF::create(kdfHash.toStdString()));

  // Key salt
  const Botan::secure_vector<Botan::byte> keySalt =
    Botan::base64_decode(keySaltByteArray.toStdString());

  bool keySizeIntOk = false;

  const int keySizeInt = keySizeByteArray.toInt(&keySizeIntOk);

  if (!keySizeIntOk) {
    outFile.cancelWriting();
    emit q->errorMessage(Constants::messages[7], config.inputFileInfo);
    emit q->fileFailed(id);
    return false;
  }

  const std::size_t keySize = static_cast<std::size_t>(keySizeInt);

  const std::size_t keySizeInBytes = keySize / 8;

  const auto* keyLabelVector =
    reinterpret_cast<const Botan::byte*>(keyLabel.data());

  Botan::SymmetricKey key(kdf->derive_key(keySizeInBytes,
                                          pbkdfKey.data(),
                                          pbkdfKey.size(),
                                          keySalt.data(),
                                          keySalt.size(),
                                          keyLabelVector,
                                          keyLabel.size()));

  const Botan::secure_vector<Botan::byte> ivSalt =
    Botan::base64_decode(ivSaltByteArray.toStdString());
  const std::size_t ivSize = 256;
  const auto* ivLabelVector =
    reinterpret_cast<const Botan::byte*>(ivLabel.data());
  Botan::InitializationVector iv(kdf->derive_key(ivSize,
                                                 pbkdfKey.data(),
                                                 pbkdfKey.size(),
                                                 ivSalt.data(),
                                                 ivSalt.size(),
                                                 ivLabelVector,
                                                 ivLabel.size()));

  Botan::Pipe pipe;

  const QString algorithm = algorithmString(cipherByteArray,
                                            modeOfOperationByteArray, keySize);

  pipe.append_filter(Botan::get_cipher(algorithm.toStdString(),
                                       key, iv, Botan::DECRYPTION));

  const bool success = executeCipher(id, CryptDirection::Decrypt,
                                     &inFile, &outFile, &pipe);

  if (!success) {
    outFile.cancelWriting();
    emit q->errorMessage(Constants::messages[7], config.inputFileInfo);
    emit q->fileFailed(id);
    return false;
  }

  if (state->isAborted() || state->isStopped(id)) {
    outFile.cancelWriting();
    emit q->fileFailed(id);
    return false;
  }

  outFile.commit();

  // Progress: finished
  emit q->fileProgress(id, QObject::tr("Decrypted"), 100);

  // Decryption success message
  emit q->statusMessage(
    Constants::messages[2].arg(config.inputFileInfo.absoluteFilePath()));

  emit q->fileCompleted(id);

  return true;
}

bool BotanProviderPrivate::executeCipher(
  const std::size_t id, const CryptDirection direction, QFile* inFile,
  QSaveFile* outFile, Botan::Pipe* pipe) {
  Q_Q(BotanProvider);
  Q_ASSERT(state);
  Q_ASSERT(inFile);
  Q_ASSERT(outFile);
  Q_ASSERT(pipe);

  // Define a size for the buffer vector
  const std::size_t bufferSize = 4096;
  Botan::secure_vector<Botan::byte> buffer;
  buffer.resize(bufferSize);

  // Get file size for percent progress calculation
  const qint64 size = inFile->size();

  qint64 fileIndex = 0;
  qint64 percent = -1;

  pipe->start_msg();

  while (!inFile->atEnd() && !state->isAborted() && !state->isStopped(id)) {
    state->pauseWait(id);

    if (state->isAborted() || state->isStopped(id)) {
        outFile->cancelWriting();
        emit q->fileFailed(id);
        return false;
    }

    const qint64 readSize =
      inFile->read(reinterpret_cast<char*>(&buffer[0]), buffer.size());

    if (readSize < 0) {
      outFile->cancelWriting();
      emit q->errorMessage(Constants::messages[5],
                           QFileInfo(inFile->fileName()));
      emit q->fileFailed(id);
      return false;
    }

    pipe->write(&buffer[0], static_cast<std::size_t>(readSize));

    // Calculate progress in percent
    fileIndex += readSize;

    const double fractionalProgress = static_cast<double>(fileIndex) /
                                      static_cast<double>(size);

    const double percentProgress = fractionalProgress * 100.0;

    const int percentProgressInteger = static_cast<int>(percentProgress);

    if (percentProgressInteger > percent && percentProgressInteger < 100) {
      percent = percentProgressInteger;

      const QString task = CryptDirection::Encrypt == direction ?
                           QObject::tr("Encrypting") :
                           QObject::tr("Decrypting");

      emit q->fileProgress(id, task, percent);
    }

    if (inFile->atEnd()) {
      pipe->end_msg();
    }

    while (pipe->remaining() > 0) {
      const std::size_t buffered = pipe->read(&buffer[0], buffer.size());

      if (buffered < 0) {
        outFile->cancelWriting();

        if (CryptDirection::Encrypt == direction) {
          emit q->errorMessage(Constants::messages[8],
                               QFileInfo(inFile->fileName()));
        } else {
          emit q->errorMessage(Constants::messages[7],
                               QFileInfo(inFile->fileName()));
        }

        emit q->fileFailed(id);
        return false;
      }

      const qint64 writeSize =
        outFile->write(reinterpret_cast<const char*>(&buffer[0]), buffered);

      if (writeSize < 0) {
        outFile->cancelWriting();

        if (CryptDirection::Encrypt == direction) {
          emit q->errorMessage(Constants::messages[8],
                               QFileInfo(inFile->fileName()));
        } else {
          emit q->errorMessage(Constants::messages[7],
                               QFileInfo(inFile->fileName()));
        }

        emit q->fileFailed(id);
        return false;
      }
    }
  }

  return true;
}

BotanProvider::BotanProvider(QObject* parent)
  : QObject(parent),
    d_ptr(std::make_unique<BotanProviderPrivate>(this)) {
}

BotanProvider::~BotanProvider() = default;

void BotanProvider::init(SchedulerState* state) {
  Q_D(BotanProvider);

  d->init(state);
}

bool BotanProvider::encrypt(std::size_t id,
                            const Kryvo::EncryptFileConfig& config) {
  Q_D(BotanProvider);

  return d->encrypt(id, config);
}

bool BotanProvider::decrypt(std::size_t id,
                            const Kryvo::DecryptFileConfig& config) {
  Q_D(BotanProvider);

  return d->decrypt(id, config);
}

QObject* BotanProvider::qObject() {
  return this;
}

} // namespace Kryvo
