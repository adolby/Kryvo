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

const std::size_t ivSize = 12;
const std::size_t saltSize = 16;
const std::size_t hashSize = 512;
const std::size_t chunkSize = 4096;
const std::size_t pbkdfIterations = 100000;

QString algorithmString(const QString& cipher, const QString& mode,
                        const std::size_t keySize) {
  QString algo = QString(cipher % QStringLiteral("/") % mode);

  if (QStringLiteral("AES") == cipher) {
    algo = QString(cipher % QStringLiteral("-") % QString::number(keySize) %
                   QStringLiteral("/") % mode);
  }

  return algo;
}

void deriveKeyAndIv(const QString& passphrase,
                    const std::size_t keySizeInBytes,
                    const Botan::secure_vector<Botan::byte>& salt,
                    const std::size_t iterations,
                    Botan::SymmetricKey& key,
                    Botan::InitializationVector& iv) {
  // PKCS5_PBKDF2 takes ownership of the new HMAC and the HMAC takes ownership
  // of the SHA3 hash function object (both via unique_ptr)
  Botan::PKCS5_PBKDF2 pbkdf(new Botan::HMAC(new Botan::SHA_3(hashSize)));

  // Create the PBKDF key
  const std::string passphraseString = passphrase.toStdString();

  Botan::secure_vector<Botan::byte> pbkdfKey =
    pbkdf.derive_key(keySizeInBytes, passphraseString, salt.data(), salt.size(),
                     iterations).bits_of();

  const QString kdfName = QStringLiteral("HKDF(SHA-3(%1))").arg(hashSize);

  std::unique_ptr<Botan::KDF> kdf(Botan::KDF::create(kdfName.toStdString()));

  // Derive the key and IV with HKDF from the PBKDF key
  key = kdf->derive_key(keySizeInBytes, pbkdfKey.data(), pbkdfKey.size(),
                        salt.data(), salt.size());

  iv = kdf->derive_key(ivSize, pbkdfKey.data(), pbkdfKey.size(),
                       salt.data(), salt.size());
}

QHash<QByteArray, QByteArray> buildHeader(
  const Kryvo::EncryptFileConfig& config,
  const Botan::secure_vector<Botan::byte>& salt) {
  QHash<QByteArray, QByteArray> headerData;

  headerData.insert(QByteArrayLiteral("Version"),
                    QByteArray::number(Constants::fileVersion));
  headerData.insert(QByteArrayLiteral("Cryptography provider"),
                    QByteArrayLiteral("Botan"));

  if (!config.encrypt.compressionFormat.isEmpty() &&
      config.encrypt.compressionFormat != QStringLiteral("None")) {
    headerData.insert(QByteArrayLiteral("Compression format"),
                      config.encrypt.compressionFormat.toUtf8());
  }

  headerData.insert(QByteArrayLiteral("Cipher"), QByteArrayLiteral("AES"));

  headerData.insert(QByteArrayLiteral("Mode"),
                    config.encrypt.modeOfOperation.toUtf8());

  headerData.insert(QByteArrayLiteral("Key size"),
                    QByteArray::number(
                      static_cast<uint>(config.encrypt.keySize)));

  const std::string saltString =
    Botan::base64_encode(salt.data(), salt.size());

  headerData.insert(QByteArrayLiteral("Salt"),
                    QString::fromStdString(saltString).toUtf8());

  return headerData;
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
};

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

  // Initialize a randomized salt vector
  Botan::secure_vector<Botan::byte> salt;
  salt.resize(saltSize);
  rng.randomize(salt.data(), salt.size());

  // Derive the key and IV from passphrase
  Botan::SymmetricKey key;
  Botan::InitializationVector iv;
  deriveKeyAndIv(config.encrypt.passphrase, config.encrypt.keySize, salt,
                 pbkdfIterations, key, iv);

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
    emit q->errorMessage(Constants::messages[8], config.inputFileInfo);
    emit q->fileFailed(id);
    return false;
  }

  const QHash<QByteArray, QByteArray> headerData = buildHeader(config, salt);
  writeHeader(&outFile, headerData);

  Botan::Pipe pipe;

  const QString algorithm = algorithmString(QStringLiteral("AES"),
                                            config.encrypt.modeOfOperation,
                                            config.encrypt.keySize);

  pipe.append_filter(Botan::get_cipher(algorithm.toStdString(), key,
                                       iv, Botan::ENCRYPTION));

  const bool success = executeCipher(id, CryptDirection::Encrypt,
                                     &inFile, &outFile, &pipe);

  if (!success) {
    emit q->errorMessage(Constants::messages[8], config.inputFileInfo);
    emit q->fileFailed(id);
    return false;
  }

  if (state->isAborted() || state->isStopped(id)) {
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
    emit q->errorMessage(Constants::messages[7], config.inputFileInfo);
    emit q->fileFailed(id);
    return false;
  }

  const QString versionString(header.value(QByteArrayLiteral("Version")));

  // Need to check file version when file format stabilizes
  //  bool conversionOk = false;
  //  const int fileVersion = versionString.toInt(&conversionOk);

  const QByteArray cryptoProviderByteArray =
    header.value(QByteArrayLiteral("Cryptography provider"));

  const QByteArray compressionFormatByteArray =
    header.value(QByteArrayLiteral("Compression format"));

  const QByteArray cipherByteArray = header.value(QByteArrayLiteral("Cipher"));

  const QByteArray modeOfOperationByteArray =
    header.value(QByteArrayLiteral("Mode"));

  const QByteArray keySizeByteArray =
    header.value(QByteArrayLiteral("Key size"));

  const QByteArray saltByteArray = header.value(QByteArrayLiteral("Salt"));

  const Botan::secure_vector<Botan::byte> salt =
    Botan::base64_decode(saltByteArray.toStdString());

  bool keySizeIntOk = false;

  const int keySizeInt = keySizeByteArray.toInt(&keySizeIntOk);

  if (!keySizeIntOk) {
    emit q->errorMessage(Constants::messages[7], config.inputFileInfo);
    emit q->fileFailed(id);
    return false;
  }

  const std::size_t keySize = static_cast<std::size_t>(keySizeInt);

  // Derive the key and IV from passphrase
  Botan::SymmetricKey key;
  Botan::InitializationVector iv;
  deriveKeyAndIv(config.decrypt.passphrase, keySize, salt, pbkdfIterations, key,
                 iv);

  Botan::Pipe pipe;

  const QString algorithm = algorithmString(cipherByteArray,
                                            modeOfOperationByteArray, keySize);

  pipe.append_filter(Botan::get_cipher(algorithm.toStdString(),
                                       key, iv, Botan::DECRYPTION));

  const bool success = executeCipher(id, CryptDirection::Decrypt,
                                     &inFile, &outFile, &pipe);

  if (!success) {
    emit q->errorMessage(Constants::messages[7], config.inputFileInfo);
    emit q->fileFailed(id);
    return false;
  }

  if (state->isAborted() || state->isStopped(id)) {
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

  // Init buffer vector
  Botan::secure_vector<Botan::byte> buffer;
  buffer.resize(chunkSize);

  // Get file size for percent progress calculation
  const qint64 size = inFile->size();

  qint64 fileIndex = 0ll;
  qint64 percent = -1ll;

  pipe->start_msg();

  while (!inFile->atEnd() && !state->isAborted() && !state->isStopped(id)) {
    state->pauseWait(id);

    if (state->isAborted() || state->isStopped(id)) {
        emit q->fileFailed(id);
        return false;
    }

    const qint64 readSize =
      inFile->read(reinterpret_cast<char*>(buffer.data()), buffer.size());

    if (readSize < 0) {
      emit q->errorMessage(Constants::messages[5],
                           QFileInfo(inFile->fileName()));
      emit q->fileFailed(id);
      return false;
    }

    pipe->write(buffer.data(), static_cast<std::size_t>(readSize));

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
      const std::size_t buffered = pipe->read(buffer.data(), buffer.size());

      if (buffered < 0) {
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
        outFile->write(reinterpret_cast<const char*>(buffer.data()), buffered);

      if (writeSize < 0) {
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

int BotanProvider::encrypt(std::size_t id,
                           const Kryvo::EncryptFileConfig& config) {
  Q_D(BotanProvider);

  const bool ret = d->encrypt(id, config);

  if (ret) {
      return 0;
  } else {
      return -1;
  }
}

int BotanProvider::decrypt(std::size_t id,
                           const Kryvo::DecryptFileConfig& config) {
  Q_D(BotanProvider);

  const bool ret = d->decrypt(id, config);

  if (ret) {
    return 0;
  } else {
    return -1;
  }
}

QObject* BotanProvider::qObject() {
  return this;
}

} // namespace Kryvo
