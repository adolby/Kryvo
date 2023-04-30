#include "OpenSslProvider.hpp"
#include "SchedulerState.hpp"
#include "Constants.hpp"
#include "FileUtility.h"
#include <QSaveFile>
#include <QFile>
#include <QFileInfo>
#include <QIODevice>
#include <QDir>
#include <QStringBuilder>
#include <QString>
#include <QHash>
#include <QMap>
#include <openssl/evp.h>
#include <openssl/kdf.h>
#include <openssl/ossl_typ.h>
#include <openssl/core_names.h>
#include <openssl/rand.h>
#include <openssl/aes.h>
#include <openssl/err.h>

namespace Kryvo {

const std::size_t ivSize = 12; // 96-bit IV
const std::size_t tagSize = 16; // 128-bit tag
const std::size_t saltSize = 16; // 128-bit salt
const std::size_t chunkSize = 4096;
const std::size_t pbkdfIterations = 100000;

int pbkdf2(const QByteArray& password, const QByteArray& salt,
           const std::size_t iterations, QByteArray& key) {
  const int rc =
    PKCS5_PBKDF2_HMAC(password.constData(), password.size(),
                      (const unsigned char*)salt.constData(), salt.size(),
                      iterations, EVP_sha3_512(), key.size(),
                      (unsigned char*)key.data());

  return rc;
}

int deriveKeyAndIv(const QByteArray& passphrase,
                   const std::size_t keySizeInBytes, const QByteArray& salt,
                   const std::size_t iterations, QByteArray& key,
                   QByteArray& iv) {
  QByteArray stretchedKey(keySizeInBytes, Qt::Uninitialized);
  int rc = pbkdf2(passphrase, salt, iterations, stretchedKey);

  if (rc <= 0) {
    return rc;
  }

  EVP_KDF* kdf = EVP_KDF_fetch(NULL, "HKDF", NULL);

  if (!kdf) {
    return -1;
  }

  EVP_KDF_CTX* kctx = EVP_KDF_CTX_new(kdf);
  EVP_KDF_free(kdf);

  if (!kctx) {
    return -1;
  }

  OSSL_PARAM params[4];

  params[0] = OSSL_PARAM_construct_utf8_string(OSSL_KDF_PARAM_DIGEST,
                                               const_cast<char*>(SN_sha3_512),
                                               strlen(SN_sha3_512));
  params[1] = OSSL_PARAM_construct_octet_string(
                OSSL_KDF_PARAM_KEY, const_cast<char*>(stretchedKey.data()),
                stretchedKey.size());
  params[2] = OSSL_PARAM_construct_octet_string(OSSL_KDF_PARAM_SALT,
                                                const_cast<char*>(salt.data()),
                                                salt.size());
  params[3] = OSSL_PARAM_construct_end();

  rc = EVP_KDF_CTX_set_params(kctx, params);

  if (rc <= 0) {
    EVP_KDF_CTX_free(kctx);
    return rc;
  }

  rc = EVP_KDF_derive(kctx, reinterpret_cast<unsigned char*>(key.data()),
                      key.size(), params);

  if (rc <= 0) {
    EVP_KDF_CTX_free(kctx);
    return rc;
  }

  rc = EVP_KDF_derive(kctx, reinterpret_cast<unsigned char*>(iv.data()),
                      iv.size(), params);

  EVP_KDF_CTX_free(kctx);

  return rc;
}

int initCipherOperation(EVP_CIPHER_CTX* ctx, const EVP_CIPHER* cipher,
                        const QByteArray& key, const QByteArray& iv,
                        Kryvo::CryptDirection direction) {
  const int cipherDirection = Kryvo::CryptDirection::Encrypt == direction ?
                              1 :
                              0;

  // Initialize the cipher operation
  int rc = EVP_CipherInit_ex(ctx, cipher, nullptr, nullptr, nullptr,
                             cipherDirection);

  if (rc <= 0) {
    return rc;
  }

  // Set IV size
  rc = EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, ivSize, nullptr);

  if (rc <= 0) {
    return rc;
  }

  // Init key and IV
  rc =
    EVP_CipherInit_ex(ctx, nullptr, nullptr,
                      reinterpret_cast<const unsigned char*>(key.constData()),
                      reinterpret_cast<const unsigned char*>(iv.constData()),
                      cipherDirection);

  return rc;
}

QMap<QByteArray, QByteArray> buildHeader(
  const Kryvo::EncryptFileConfig& config, const QByteArray& salt) {
  QMap<QByteArray, QByteArray> headerData;

  headerData.insert(QByteArrayLiteral("Version"),
                    QByteArray::number(Constants::fileVersion));
  headerData.insert(QByteArrayLiteral("Cryptography provider"),
                    QByteArrayLiteral("OpenSsl"));

  const QString compressionFormat = config.encrypt.compressionFormat;

  if (!compressionFormat.isEmpty() &&
      compressionFormat != QStringLiteral("None")) {
    headerData.insert(QByteArrayLiteral("Compression format"),
                      compressionFormat.toUtf8());
  }

  headerData.insert(QByteArrayLiteral("Cipher"), QByteArrayLiteral("AES"));

  headerData.insert(QByteArrayLiteral("Mode"),
                    config.encrypt.modeOfOperation.toUtf8());

  headerData.insert(QByteArrayLiteral("Key size"),
                    QByteArray::number(
                      static_cast<uint>(config.encrypt.keySize)));

  headerData.insert(QByteArrayLiteral("Salt"), salt.toBase64());

  return headerData;
}

class OpenSslProviderPrivate {
  Q_DISABLE_COPY(OpenSslProviderPrivate)
  Q_DECLARE_PUBLIC(OpenSslProvider)

 public:
  OpenSslProviderPrivate(OpenSslProvider* op);

  void init(SchedulerState* s);
  int encrypt(std::size_t id, const Kryvo::EncryptFileConfig& config);
  int decrypt(std::size_t id, const Kryvo::DecryptFileConfig& config);
  int executeCipher(std::size_t id,
                    EVP_CIPHER_CTX* ctx,
                    const EVP_CIPHER* cipher,
                    const QFileInfo& inputFileInfo,
                    QIODevice* inFile,
                    QIODevice* outFile,
                    Kryvo::CryptDirection direction);

  OpenSslProvider* const q_ptr{nullptr};

  SchedulerState* state{nullptr};
};

OpenSslProviderPrivate::OpenSslProviderPrivate(OpenSslProvider* os)
  : q_ptr(os) {
}

void OpenSslProviderPrivate::init(SchedulerState* s) {
  state = s;
}

int OpenSslProviderPrivate::encrypt(std::size_t id,
                                    const Kryvo::EncryptFileConfig& config) {
  Q_Q(OpenSslProvider);
  Q_ASSERT(state);

  if (!state) {
    emit q->errorMessage(Constants::messages[0], QFileInfo());
    emit q->fileFailed(id);
    return -1;
  }

  if (state->isCancelled() || state->isStopped(id)) {
    emit q->errorMessage(Constants::messages[3], config.inputFileInfo);
    emit q->fileFailed(id);
    return -1;
  }

  if (!config.inputFileInfo.exists() || !config.inputFileInfo.isFile() ||
      !config.inputFileInfo.isReadable()) {
    emit q->errorMessage(Constants::messages[5], config.inputFileInfo);
    emit q->fileFailed(id);
    return -1;
  }

  emit q->fileProgress(id, QObject::tr("Encrypting"), 0);

  QFile inFile(config.inputFileInfo.absoluteFilePath());
  const bool inFileOpen = inFile.open(QIODevice::ReadOnly);

  if (!inFileOpen) {
    emit q->errorMessage(Constants::messages[5], config.inputFileInfo);
    emit q->fileFailed(id);
    return -1;
  }

  QSaveFile outFile(config.outputFileInfo.absoluteFilePath());
  const bool outFileOpen = outFile.open(QIODevice::WriteOnly);

  if (!outFileOpen) {
    emit q->errorMessage(Constants::messages[8], config.inputFileInfo);
    emit q->fileFailed(id);
    return -1;
  }

  QByteArray salt(saltSize, Qt::Uninitialized);
  RAND_bytes((unsigned char*)salt.data(), salt.size());

  const QMap<QByteArray, QByteArray> headerData = buildHeader(config, salt);
  writeHeader(&outFile, headerData);

  const QByteArray passphrase = config.encrypt.passphrase.toUtf8();

  const std::size_t keySizeInBytes = config.encrypt.keySize / 8;

  QByteArray key(keySizeInBytes, Qt::Uninitialized);
  QByteArray iv(ivSize, Qt::Uninitialized);
  int rc = deriveKeyAndIv(passphrase, keySizeInBytes, salt, pbkdfIterations,
                          key, iv);

  if (rc <= 0) {
    emit q->errorMessage(Constants::messages[0], config.inputFileInfo);
    emit q->fileFailed(id);
    return rc;
  }

  EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();

  if (!ctx) {
    emit q->errorMessage(Constants::messages[0], config.inputFileInfo);
    emit q->fileFailed(id);
    return -1;
  }

  const EVP_CIPHER* cipher = (config.encrypt.keySize == 128) ?
                             EVP_aes_128_gcm() :
                             EVP_aes_256_gcm();

  // Initialize the encryption operation
  rc = initCipherOperation(ctx, cipher, key, iv,
                           Kryvo::CryptDirection::Encrypt);

  if (rc <= 0) {
    emit q->errorMessage(Constants::messages[0], config.inputFileInfo);
    emit q->fileFailed(id);
    EVP_CIPHER_CTX_free(ctx);
    return rc;
  }

  rc = executeCipher(id, ctx, cipher, config.inputFileInfo, &inFile, &outFile,
                     Kryvo::CryptDirection::Encrypt);

  if (rc <= 0) {
    EVP_CIPHER_CTX_free(ctx);
    return rc;
  }

  QByteArray outBuffer(chunkSize + EVP_CIPHER_get_block_size(cipher),
                       Qt::Uninitialized);
  int outSize = 0;

  // Encrypt final block and finalize (doesn't actually encrypt data for GCM)
  rc = EVP_EncryptFinal_ex(ctx,
                           reinterpret_cast<unsigned char*>(outBuffer.data()),
                           &outSize);

  if (rc <= 0) {
    emit q->errorMessage(Constants::messages[8], config.inputFileInfo);
    emit q->fileFailed(id);
    EVP_CIPHER_CTX_free(ctx);
    return rc;
  }

  QByteArray tag(tagSize, Qt::Uninitialized);

  // Retrieve authentication tag
  rc = EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, tag.size(), tag.data());

  if (rc <= 0) {
    emit q->errorMessage(Constants::messages[8], config.inputFileInfo);
    emit q->fileFailed(id);
    EVP_CIPHER_CTX_free(ctx);
    return rc;
  }

  outFile.write(tag);

  EVP_CIPHER_CTX_free(ctx);

  if (rc <= 0) {
    emit q->errorMessage(Constants::messages[8], config.inputFileInfo);
    emit q->fileFailed(id);
    return rc;
  }

  if (state->isCancelled() || state->isStopped(id)) {
    emit q->errorMessage(Constants::messages[3], config.inputFileInfo);
    emit q->fileFailed(id);
    return -1;
  }

  outFile.commit();

  // Progress: finished
  emit q->fileProgress(id, QObject::tr("Encrypted"), 100);

  // Encryption success message
  emit q->statusMessage(
    Constants::messages[1].arg(config.inputFileInfo.absoluteFilePath()));

  emit q->fileCompleted(id);

  return rc;
}

int OpenSslProviderPrivate::decrypt(std::size_t id,
                                    const Kryvo::DecryptFileConfig& config) {
  Q_Q(OpenSslProvider);
  Q_ASSERT(state);

  if (!state) {
    emit q->errorMessage(Constants::messages[0], QFileInfo());
    emit q->fileFailed(id);
    return -1;
  }

  if (state->isCancelled() || state->isStopped(id)) {
    emit q->errorMessage(Constants::messages[4], config.inputFileInfo);
    emit q->fileFailed(id);
    return -1;
  }

  if (!config.inputFileInfo.exists() || !config.inputFileInfo.isFile() ||
      !config.inputFileInfo.isReadable()) {
    emit q->errorMessage(Constants::messages[5], config.inputFileInfo);
    emit q->fileFailed(id);
    return -1;
  }

  emit q->fileProgress(id, QObject::tr("Decrypting"), 0);

  QFile inFile(config.inputFileInfo.absoluteFilePath());

  const bool inFileOpen = inFile.open(QIODevice::ReadOnly);

  if (!inFileOpen) {
    emit q->errorMessage(Constants::messages[5], config.inputFileInfo);
    emit q->fileFailed(id);
    return -1;
  }

  // Seek file to after header
  const QHash<QByteArray, QByteArray> header = readHeader(&inFile);

  QSaveFile outFile(config.outputFileInfo.absoluteFilePath());

  const bool outFileOpen = outFile.open(QIODevice::WriteOnly);

  if (!outFileOpen) {
    emit q->errorMessage(Constants::messages[7], config.inputFileInfo);
    emit q->fileFailed(id);
    return -1;
  }

  const QString versionString(header.value(QByteArrayLiteral("Version")));

  // Need to check file version when file format stabilizes
  //  bool conversionOk = false;
  //  const int fileVersion = versionString.toInt(&conversionOk);

  const QByteArray cryptoProvider =
    header.value(QByteArrayLiteral("Cryptography provider"));

  const QByteArray compressionFormat =
    header.value(QByteArrayLiteral("Compression format"));

  const QByteArray cipherName = header.value(QByteArrayLiteral("Cipher"));

  const QByteArray modeOfOperation = header.value(QByteArrayLiteral("Mode"));

  const QByteArray keySizeByteArray =
    header.value(QByteArrayLiteral("Key size"));

  bool keySizeIntOk = false;

  const int keySizeInt = keySizeByteArray.toInt(&keySizeIntOk);

  if (!keySizeIntOk) {
    emit q->errorMessage(Constants::messages[7], config.inputFileInfo);
    emit q->fileFailed(id);
    return false;
  }

  const std::size_t keySize = static_cast<std::size_t>(keySizeInt);
  const std::size_t keySizeInBytes = keySize / 8;

  const QByteArray salt =
    QByteArray::fromBase64(header.value(QByteArrayLiteral("Salt")));

  const QByteArray passphrase = config.decrypt.passphrase.toUtf8();

  QByteArray key(keySizeInBytes, Qt::Uninitialized);
  QByteArray iv(ivSize, Qt::Uninitialized);
  int rc = deriveKeyAndIv(passphrase, keySizeInBytes, salt, pbkdfIterations,
                          key, iv);

  if (rc <= 0) {
    emit q->errorMessage(Constants::messages[0], config.inputFileInfo);
    emit q->fileFailed(id);
    return rc;
  }

  EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();

  if (!ctx) {
    emit q->errorMessage(Constants::messages[0], config.inputFileInfo);
    emit q->fileFailed(id);
    return -1;
  }

  const EVP_CIPHER* cipher = (keySize == 128) ?
                             EVP_aes_128_gcm() :
                             EVP_aes_256_gcm();

  // Initialize the decryption operation
  rc = initCipherOperation(ctx, cipher, key, iv,
                           Kryvo::CryptDirection::Decrypt);

  if (rc <= 0) {
    emit q->errorMessage(Constants::messages[0], config.inputFileInfo);
    emit q->fileFailed(id);
    EVP_CIPHER_CTX_free(ctx);
    return rc;
  }

  rc = executeCipher(id, ctx, cipher, config.inputFileInfo, &inFile, &outFile,
                     Kryvo::CryptDirection::Encrypt);

  if (rc <= 0) {
    EVP_CIPHER_CTX_free(ctx);
    return rc;
  }

  // Read tag from file
  QByteArray tag(tagSize, Qt::Uninitialized);
  const qint64 tagBytesRead = inFile.read(tag.data(), tagSize);

  if (tagBytesRead < tagSize) { // Tag too short, can't authenticate
    emit q->errorMessage(Constants::messages[6], config.inputFileInfo);
    emit q->fileFailed(id);
    EVP_CIPHER_CTX_free(ctx);
  }

  // Set authentication tag obtained from encrypted file
  rc = EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, tag.size(), tag.data());

  if (rc <= 0) {
    emit q->errorMessage(Constants::messages[0], config.inputFileInfo);
    emit q->fileFailed(id);
    EVP_CIPHER_CTX_free(ctx);
    return rc;
  }

  QByteArray outBuffer(chunkSize + EVP_CIPHER_get_block_size(cipher),
                       Qt::Uninitialized);
  int outSize = 0;

  // Decrypt last block and finalize (doesn't decrypt with GCM). Computes a tag
  // and checks it against the authentication tag that was computed during
  // encryption. A non-positive return code indicates the verification failed.
  rc = EVP_DecryptFinal_ex(ctx,
                           reinterpret_cast<unsigned char*>(outBuffer.data()),
                           &outSize);

  if (rc <= 0) { // Authentication failed
    emit q->errorMessage(Constants::messages[6], config.inputFileInfo);
    emit q->fileFailed(id);
    EVP_CIPHER_CTX_free(ctx);
    return rc;
  }

  EVP_CIPHER_CTX_free(ctx);

  if (state->isCancelled() || state->isStopped(id)) {
    emit q->errorMessage(Constants::messages[4], config.inputFileInfo);
    emit q->fileFailed(id);
    return -1;
  }

  outFile.commit();

  // Progress: finished
  emit q->fileProgress(id, QObject::tr("Decrypted"), 100);

  // Decryption success message
  emit q->statusMessage(
    Constants::messages[2].arg(config.inputFileInfo.absoluteFilePath()));

  emit q->fileCompleted(id);

  return rc;
}

int OpenSslProviderPrivate::executeCipher(std::size_t id,
                                          EVP_CIPHER_CTX* ctx,
                                          const EVP_CIPHER* cipher,
                                          const QFileInfo& inputFileInfo,
                                          QIODevice* inFile,
                                          QIODevice* outFile,
                                          Kryvo::CryptDirection direction) {
  Q_Q(OpenSslProvider);

  QByteArray inBuffer(chunkSize, Qt::Uninitialized);

  QByteArray outBuffer(chunkSize + EVP_CIPHER_get_block_size(cipher),
                       Qt::Uninitialized);
  int outSize = 0;

  const qint64 fileEnd = Kryvo::CryptDirection::Encrypt == direction ?
                         inFile->size() :
                         inFile->size() - tagSize;
  qint64 fileIndex = inFile->pos(); // input data start position
  qint64 percent = -1ll;

  const QString encryptDecryptString =
    Kryvo::CryptDirection::Encrypt == direction ?
    QObject::tr("Encrypting") :
    QObject::tr("Decrypting");

  while (fileIndex < fileEnd) {
    if (state->isCancelled() || state->isStopped(id)) {
      emit q->errorMessage(Constants::messages[4], inputFileInfo);
      emit q->fileFailed(id);
      return -1;
    }

    state->pauseWait(id);

    if (state->isCancelled() || state->isStopped(id)) {
      emit q->errorMessage(Constants::messages[4], inputFileInfo);
      emit q->fileFailed(id);
      return -1;
    }

    const qint64 remainingBytes = fileEnd - fileIndex;

    qint64 inBufferMaxSize = inBuffer.size();

    if (remainingBytes < chunkSize) { // Last partial input chunk size
      inBufferMaxSize = remainingBytes;
    }

    const qint64 bytesRead = inFile->read(inBuffer.data(), inBufferMaxSize);

    if (bytesRead < 0) {
      emit q->errorMessage(Constants::messages[5], inputFileInfo);
      emit q->fileFailed(id);
      return -1;
    }

    fileIndex += bytesRead;

    // Encrypt or decrypt a block
    int rc =
      EVP_CipherUpdate(
        ctx, reinterpret_cast<unsigned char*>(outBuffer.data()), &outSize,
        reinterpret_cast<const unsigned char*>(inBuffer.constData()),
        bytesRead);

    if (rc <= 0) {
      emit q->errorMessage(Constants::messages[6], inputFileInfo);
      emit q->fileFailed(id);
      return rc;
    }

    outFile->write(outBuffer, outSize);

    // Calculate progress in percent
    const double fractionalProgress = static_cast<double>(fileIndex) /
                                      static_cast<double>(fileEnd);

    const double percentProgress = fractionalProgress * 100.0;

    const int percentProgressInteger = static_cast<int>(percentProgress);

    if (percentProgressInteger > percent && percentProgressInteger < 100) {
      percent = percentProgressInteger;
      emit q->fileProgress(id, encryptDecryptString, percent);
    }
  }

  return 1;
}

OpenSslProvider::OpenSslProvider(QObject* parent)
  : QObject(parent), d_ptr(std::make_unique<OpenSslProviderPrivate>(this)) {
}

OpenSslProvider::~OpenSslProvider() = default;

void OpenSslProvider::init(SchedulerState* state) {
  Q_D(OpenSslProvider);

  d->init(state);
}

int OpenSslProvider::encrypt(std::size_t id,
                             const Kryvo::EncryptFileConfig& config) {
  Q_D(OpenSslProvider);

  return d->encrypt(id, config);
}

int OpenSslProvider::decrypt(std::size_t id,
                             const Kryvo::DecryptFileConfig& config) {
  Q_D(OpenSslProvider);

  return d->decrypt(id, config);
}

QObject* OpenSslProvider::qObject() {
  return this;
}

} // namespace Kryvo
