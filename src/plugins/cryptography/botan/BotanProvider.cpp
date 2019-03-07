#include "BotanProvider.hpp"
#include "DispatcherState.hpp"
#include "Constants.hpp"
#include <QSaveFile>
#include <QFileInfo>
#include <QDir>
#include <string>
#include <stdexcept>

class Kryvo::BotanProviderPrivate {
  Q_DISABLE_COPY(BotanProviderPrivate)
  Q_DECLARE_PUBLIC(BotanProvider)

 public:
  BotanProviderPrivate(BotanProvider* bp);

  void init(DispatcherState* ds);

  bool encrypt(std::size_t id,
               const QString& passphrase,
               const QString& inFilePath,
               const QString& outFilePath,
               const QString& cipher,
               std::size_t keySize,
               const QString& modeOfOperation,
               bool compress);

  bool decrypt(std::size_t id,
               const QString& passphrase,
               const QString& inFilePath,
               const QString& outFilePath,
               const QString& algorithmNameString,
               const QString& keySizeString,
               const QString& pbkdfSaltString,
               const QString& keySaltString,
               const QString& ivSaltString);

  bool encryptFile(std::size_t id,
                   const QString& passphrase,
                   const QString& inputFilePath,
                   const QString& outputFilePath,
                   const QString& algorithmName,
                   std::size_t keySize,
                   bool compress);

  bool decryptFile(std::size_t id,
                   const QString& passphrase,
                   const QString& inputFilePath,
                   const QString& outputFilePath,
                   const QString& algorithmNameString,
                   const QString& keySizeString,
                   const QString& pbkdfSaltString,
                   const QString& keySaltString,
                   const QString& ivSaltString);

  bool executeCipher(std::size_t id, const Botan::Cipher_Dir direction,
                     QFile* inFile, QSaveFile* outFile, Botan::Pipe* pipe);

  BotanProvider* const q_ptr{nullptr};

  DispatcherState* state{nullptr};

  const std::string kKDFHash{"KDF2(Keccak-1600)"};
  const std::string kKeyLabel{"user secret"};
  const std::string kIVLabel{"initialization vector"};
  const std::size_t kPbkdfIterations{15000};
};

Kryvo::BotanProviderPrivate::BotanProviderPrivate(BotanProvider* bp)
  : q_ptr(bp) {
}

void Kryvo::BotanProviderPrivate::init(DispatcherState* ds) {
  state = ds;
}

bool Kryvo::BotanProviderPrivate::encrypt(const std::size_t id,
                                          const QString& passphrase,
                                          const QString& inFilePath,
                                          const QString& outFilePath,
                                          const QString& cipher,
                                          const std::size_t keySize,
                                          const QString& modeOfOperation,
                                          const bool compress) {
  Q_Q(BotanProvider);
  Q_ASSERT(state);

  if (!state) {
    emit q->errorMessage(Constants::messages[0], QString(""));
    emit q->fileFailed(id);
    return false;
  }

  if (state->isAborted() || state->isStopped(id)) {
    emit q->errorMessage(Kryvo::Constants::messages[3], inFilePath);
    emit q->fileFailed(id);

    return false;
  }

  const QString& algorithm = [&cipher, &keySize, &modeOfOperation]() {
    QString algo = QString(cipher % QStringLiteral("/") % modeOfOperation);

    if (QStringLiteral("AES") == cipher) {
      algo = QString(cipher % QStringLiteral("-") % QString::number(keySize) %
                     QStringLiteral("/") % modeOfOperation);
    }

    return algo;
  }();

  bool success = false;

  try {
    success = encryptFile(id, passphrase, inFilePath, outFilePath, algorithm,
                          keySize, compress);
  }
  catch (const Botan::Stream_IO_Error&) {
    emit q->errorMessage(Kryvo::Constants::messages[8], inFilePath);
    emit q->fileFailed(id);
    return false;
  }
  catch (const Botan::Invalid_Argument&) {
    emit q->errorMessage(Kryvo::Constants::messages[8], inFilePath);
    emit q->fileFailed(id);
    return false;
  }
  catch (const Botan::Exception& e) {
    emit q->errorMessage(QStringLiteral("Error: ") % QString(e.what()),
                         inFilePath);
    emit q->fileFailed(id);
    return false;
  }
  catch (const std::invalid_argument&) {
    emit q->errorMessage(Kryvo::Constants::messages[8], inFilePath);
    emit q->fileFailed(id);
    return false;
  }
  catch (const std::exception& e) {
    emit q->errorMessage(QStringLiteral("Error: ") % QString(e.what()),
                         inFilePath);
    emit q->fileFailed(id);
    return false;
  }

  if (state->isAborted() || state->isStopped(id)) {
    emit q->errorMessage(Kryvo::Constants::messages[3], inFilePath);
    emit q->fileFailed(id);

    return false;
  }

  return success;
}

bool Kryvo::BotanProviderPrivate::decrypt(const std::size_t id,
                                          const QString& passphrase,
                                          const QString& inFilePath,
                                          const QString& outFilePath,
                                          const QString& algorithmNameString,
                                          const QString& keySizeString,
                                          const QString& pbkdfSaltString,
                                          const QString& keySaltString,
                                          const QString& ivSaltString) {
  Q_Q(BotanProvider);
  Q_ASSERT(state);

  if (!state) {
    emit q->errorMessage(Constants::messages[0], QString(""));
    emit q->fileFailed(id);
    return false;
  }

  if (state->isAborted() || state->isStopped(id)) {
    emit q->errorMessage(Kryvo::Constants::messages[4], inFilePath);
    emit q->fileFailed(id);

    return false;
  }

  bool success = false;

  try {
    success = decryptFile(id, passphrase, inFilePath, outFilePath,
                          algorithmNameString, keySizeString,
                          pbkdfSaltString, keySaltString, ivSaltString);
  }
  catch (const Botan::Stream_IO_Error&) {
    emit q->errorMessage(Constants::messages[7], inFilePath);
    emit q->fileFailed(id);
    return false;
  }
  catch (const Botan::Invalid_Argument&) {
    emit q->errorMessage(Constants::messages[7], inFilePath);
    emit q->fileFailed(id);
    return false;
  }
  catch (const Botan::Lookup_Error&) {
    emit q->errorMessage(Constants::messages[7], inFilePath);
    emit q->fileFailed(id);
    return false;
  }
  catch (const Botan::Exception& e) {
    emit q->errorMessage(QStringLiteral("Error: ") % QString(e.what()),
                         inFilePath);
    emit q->fileFailed(id);
    return false;
  }
  catch (const std::invalid_argument&) {
    emit q->errorMessage(Constants::messages[7], inFilePath);
    emit q->fileFailed(id);
    return false;
  }
  catch (const std::exception& e) {
    emit q->errorMessage(QStringLiteral("Error: ") % QString(e.what()),
                         inFilePath);
    emit q->fileFailed(id);
    return false;
  }

  if (state->isAborted() || state->isStopped(id)) {
    emit q->errorMessage(Kryvo::Constants::messages[4], inFilePath);
    emit q->fileFailed(id);

    return false;
  }

  return success;
}

bool Kryvo::BotanProviderPrivate::encryptFile(const std::size_t id,
                                              const QString& passphrase,
                                              const QString& inputFilePath,
                                              const QString& outputFilePath,
                                              const QString& algorithmName,
                                              const std::size_t keySize,
                                              const bool compress) {
  Q_Q(BotanProvider);
  Q_ASSERT(state);

  if (!state) {
    emit q->errorMessage(Constants::messages[0], QString(""));
    emit q->fileFailed(id);
    return false;
  }

  if (state->isAborted() || state->isStopped(id)) {
    emit q->fileFailed(id);
    return false;
  }

  const QFileInfo inputFileInfo(inputFilePath);

  if (!inputFileInfo.exists() || !inputFileInfo.isFile() ||
      !inputFileInfo.isReadable()) {
    emit q->errorMessage(Constants::messages[5], inputFilePath);
    emit q->fileFailed(id);
    return false;
  }

  Botan::AutoSeeded_RNG rng;

  // Define a size for the PBKDF salt vector
  const std::size_t pbkdfSaltSize = 256;
  Botan::secure_vector<Botan::byte> pbkdfSalt;
  pbkdfSalt.resize(pbkdfSaltSize);

  // Create random PBKDF salt
  rng.randomize(&pbkdfSalt[0], pbkdfSalt.size());

  // Set up the key derive functions
  const std::size_t macSize = 512;

  // PKCS5_PBKDF2 takes ownership of the new HMAC and the HMAC takes ownership
  // of the Keccak_1600 hash function object (both via unique_ptr)
  Botan::PKCS5_PBKDF2 pbkdf(new Botan::HMAC(new Botan::Keccak_1600(macSize)));

  // Create the PBKDF key
  const std::size_t pbkdfKeySize = 256;

  const std::string& passphraseString = passphrase.toStdString();

  Botan::secure_vector<Botan::byte> pbkdfKey =
    pbkdf.derive_key(pbkdfKeySize, passphraseString, &pbkdfSalt[0],
                     pbkdfSalt.size(), kPbkdfIterations).bits_of();

  // Create the key and IV
  std::unique_ptr<Botan::KDF> kdf(Botan::KDF::create(kKDFHash));

  // Set up key salt size
  const std::size_t keySaltSize = 64;
  Botan::secure_vector<Botan::byte> keySalt;
  keySalt.resize(keySaltSize);
  rng.randomize(&keySalt[0], keySalt.size());

  // Key is constrained to sizes allowed by algorithm
  const std::size_t keySizeInBytes = keySize / 8;
  const auto* keyLabelVector =
    reinterpret_cast<const Botan::byte*>(kKeyLabel.data());
  Botan::SymmetricKey key(kdf->derive_key(keySizeInBytes,
                                          pbkdfKey.data(),
                                          pbkdfKey.size(),
                                          keySalt.data(),
                                          keySalt.size(),
                                          keyLabelVector,
                                          kKeyLabel.size()));

  // Set up IV salt size
  const std::size_t ivSaltSize = 64;
  Botan::secure_vector<Botan::byte> ivSalt;
  ivSalt.resize(ivSaltSize);
  rng.randomize(&ivSalt[0], ivSalt.size());

  const std::size_t ivSize = 256;
  const auto* ivLabelVector =
    reinterpret_cast<const Botan::byte*>(kIVLabel.data());
  Botan::InitializationVector iv(kdf->derive_key(ivSize,
                                                 pbkdfKey.data(),
                                                 pbkdfKey.size(),
                                                 ivSalt.data(),
                                                 ivSalt.size(),
                                                 ivLabelVector,
                                                 kIVLabel.size()));

  QFile inFile(inputFilePath);
  const bool inFileOpen = inFile.open(QIODevice::ReadOnly);

  if (!inFileOpen) {
    emit q->errorMessage(Constants::messages[5], inputFilePath);
    emit q->fileFailed(id);
    return false;
  }

  QSaveFile outFile(outputFilePath);
  const bool outFileOpen = outFile.open(QIODevice::WriteOnly);

  if (!outFileOpen) {
    outFile.cancelWriting();
    emit q->errorMessage(Constants::messages[8], inputFilePath);
    emit q->fileFailed(id);
    return false;
  }

  const QByteArray& headerText =
    QByteArrayLiteral("-------- ENCRYPTED FILE --------");

  const QByteArray& newLine = QByteArrayLiteral("\n");

  outFile.write(headerText);
  outFile.write(newLine);
  outFile.write(algorithmName.toUtf8());
  outFile.write(newLine);
  outFile.write(QString::number(keySize).toUtf8());
  outFile.write(newLine);

  const QByteArray& compressedText = [&compress]() {
    QByteArray text = QByteArrayLiteral("Not compressed");

    if (compress) {
      text = QByteArrayLiteral("Gzip Compressed");
    }

    return text;
  }();

  outFile.write(compressedText);
  outFile.write(newLine);

  const std::string& pbkdfSaltString =
    Botan::base64_encode(&pbkdfSalt[0], pbkdfSalt.size());

  outFile.write(QByteArray::fromStdString(pbkdfSaltString));
  outFile.write(newLine);

  const std::string& keySaltString =
    Botan::base64_encode(&keySalt[0], keySalt.size());

  outFile.write(QByteArray::fromStdString(keySaltString));
  outFile.write(newLine);

  const std::string& ivSaltString =
    Botan::base64_encode(&ivSalt[0], ivSalt.size());

  outFile.write(QByteArray::fromStdString(ivSaltString));
  outFile.write(newLine);

  const QByteArray& footerText =
    QByteArrayLiteral("---------------------------------");

  outFile.write(footerText);
  outFile.write(newLine);

  Botan::Pipe pipe;

  pipe.append_filter(Botan::get_cipher(algorithmName.toStdString(), key, iv,
                                       Botan::ENCRYPTION));

  const bool success = executeCipher(id, Botan::ENCRYPTION, &inFile, &outFile,
                                     &pipe);

  if (!success) {
    outFile.cancelWriting();
    emit q->errorMessage(Constants::messages[8], inputFilePath);
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
  emit q->statusMessage(Constants::messages[1].arg(inputFilePath));

  emit q->fileCompleted(id);

  return success;
}

bool Kryvo::BotanProviderPrivate::decryptFile(const std::size_t id,
                                              const QString& passphrase,
                                              const QString& inputFilePath,
                                              const QString& outputFilePath,
                                              const QString& algorithmNameString,
                                              const QString& keySizeString,
                                              const QString& pbkdfSaltString,
                                              const QString& keySaltString,
                                              const QString& ivSaltString) {
  Q_Q(BotanProvider);
  Q_ASSERT(state);

  if (!state) {
    emit q->errorMessage(Constants::messages[0], QString(""));
    emit q->fileFailed(id);
    return false;
  }

  if (state->isAborted()) {
    emit q->fileFailed(id);
    return false;
  }

  const QFileInfo inputFileInfo(inputFilePath);

  if (!inputFileInfo.exists() || !inputFileInfo.isFile() ||
      !inputFileInfo.isReadable()) {
    emit q->errorMessage(Constants::messages[5], inputFilePath);
    emit q->fileFailed(id);
    return false;
  }

  QFile inFile(inputFilePath);

  const bool inFileOpen = inFile.open(QIODevice::ReadOnly);

  if (!inFileOpen) {
    emit q->errorMessage(Constants::messages[5], inputFilePath);
    emit q->fileFailed(id);
    return false;
  }

  for (int i = 0; i < 8; ++i) { // Skip header as it was already read
    inFile.readLine();
  }

  QSaveFile outFile(outputFilePath);

  const bool outFileOpen = outFile.open(QIODevice::WriteOnly);

  if (!outFileOpen) {
    outFile.cancelWriting();
    emit q->errorMessage(Constants::messages[7], inputFilePath);
    emit q->fileFailed(id);
    return false;
  }

  // Set up the key derive functions
  const std::size_t macSize = 512;

  // PKCS5_PBKDF2 takes ownership of the new HMAC and the HMAC takes ownership
  // of the Keccak_1600 hash function object (via unique_ptr)
  Botan::PKCS5_PBKDF2 pbkdf(new Botan::HMAC(new Botan::Keccak_1600(macSize)));
  const std::size_t kPBKDF2Iterations = 15000;

  // Create the PBKDF key
  const Botan::secure_vector<Botan::byte>& pbkdfSalt =
    Botan::base64_decode(pbkdfSaltString.toStdString());

  const std::size_t pbkdfKeySize = 256;

  const std::string& passphraseString = passphrase.toStdString();

  const Botan::secure_vector<Botan::byte>& pbkdfKey =
    pbkdf.derive_key(pbkdfKeySize, passphraseString, &pbkdfSalt[0],
                     pbkdfSalt.size(), kPBKDF2Iterations).bits_of();

  // Create the key and IV
  std::unique_ptr<Botan::KDF> kdf(Botan::KDF::create(kKDFHash));

  // Key salt
  const Botan::secure_vector<Botan::byte>& keySalt =
    Botan::base64_decode(keySaltString.toStdString());

  bool keySizeIntOk = false;

  const int keySizeInt = keySizeString.toInt(&keySizeIntOk);

  if (!keySizeIntOk) {
    outFile.cancelWriting();
    emit q->errorMessage(Constants::messages[7], inputFilePath);
    emit q->fileFailed(id);
    return false;
  }

  const std::size_t keySize = static_cast<std::size_t>(keySizeInt);

  const std::size_t keySizeInBytes = keySize / 8;

  const auto* keyLabelVector =
    reinterpret_cast<const Botan::byte*>(kKeyLabel.data());

  Botan::SymmetricKey key(kdf->derive_key(keySizeInBytes,
                                          pbkdfKey.data(),
                                          pbkdfKey.size(),
                                          keySalt.data(),
                                          keySalt.size(),
                                          keyLabelVector,
                                          kKeyLabel.size()));

  const Botan::secure_vector<Botan::byte>& ivSalt =
    Botan::base64_decode(ivSaltString.toStdString());
  const std::size_t ivSize = 256;
  const auto* ivLabelVector =
    reinterpret_cast<const Botan::byte*>(kIVLabel.data());
  Botan::InitializationVector iv(kdf->derive_key(ivSize,
                                                 pbkdfKey.data(),
                                                 pbkdfKey.size(),
                                                 ivSalt.data(),
                                                 ivSalt.size(),
                                                 ivLabelVector,
                                                 kIVLabel.size()));

  Botan::Pipe pipe;

  pipe.append_filter(Botan::get_cipher(algorithmNameString.toStdString(), key,
                                       iv, Botan::DECRYPTION));

  const bool success = executeCipher(id, Botan::DECRYPTION, &inFile, &outFile,
                                     &pipe);

  if (!success) {
    outFile.cancelWriting();
    emit q->errorMessage(Constants::messages[7], inputFilePath);
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
  emit q->statusMessage(Constants::messages[2].arg(inputFilePath));

  emit q->fileCompleted(id);

  return true;
}

bool Kryvo::BotanProviderPrivate::executeCipher(
  const std::size_t id, const Botan::Cipher_Dir direction, QFile* inFile,
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
    while (state->isPaused()) {
      // Wait while paused
    }

    const qint64 readSize =
      inFile->read(reinterpret_cast<char*>(&buffer[0]), buffer.size());

    if (readSize < 0) {
      outFile->cancelWriting();
      emit q->errorMessage(Constants::messages[5], inFile->fileName());
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

      const QString& task = Botan::ENCRYPTION == direction ?
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
        if (Botan::ENCRYPTION == direction) {
          outFile->cancelWriting();
          emit q->errorMessage(Constants::messages[8], inFile->fileName());
          emit q->fileFailed(id);
          return false;
        } else {
          outFile->cancelWriting();
          emit q->errorMessage(Constants::messages[7], inFile->fileName());
          emit q->fileFailed(id);
          return false;
        }
      }

      const qint64 writeSize =
        outFile->write(reinterpret_cast<const char*>(&buffer[0]), buffered);

      if (writeSize < 0) {
        if (Botan::ENCRYPTION == direction) {
          outFile->cancelWriting();
          emit q->errorMessage(Constants::messages[8], inFile->fileName());
          emit q->fileFailed(id);
          return false;
        } else {
          outFile->cancelWriting();
          emit q->errorMessage(Constants::messages[7], inFile->fileName());
          emit q->fileFailed(id);
          return false;
        }
      }
    }
  }

  return true;
}

Kryvo::BotanProvider::BotanProvider(QObject* parent)
  : QObject(parent),
    d_ptr(std::make_unique<BotanProviderPrivate>(this)) {
}

Kryvo::BotanProvider::~BotanProvider() = default;

void Kryvo::BotanProvider::init(DispatcherState* state) {
  Q_D(BotanProvider);

  d->init(state);
}

bool Kryvo::BotanProvider::encrypt(const std::size_t id,
                                   const QString& passphrase,
                                   const QString& inFilePath,
                                   const QString& outFilePath,
                                   const QString& cipher,
                                   const std::size_t keySize,
                                   const QString& modeOfOperation,
                                   const bool compress) {
  Q_D(BotanProvider);

  return d->encrypt(id, passphrase, inFilePath, outFilePath, cipher, keySize,
                    modeOfOperation, compress);
}

bool  Kryvo::BotanProvider::decrypt(const std::size_t id,
                                    const QString& passphrase,
                                    const QString& inFilePath,
                                    const QString& outFilePath,
                                    const QString& algorithmNameString,
                                    const QString& keySizeString,
                                    const QString& pbkdfSaltString,
                                    const QString& keySaltString,
                                    const QString& ivSaltString) {
  Q_D(BotanProvider);

  return d->decrypt(id, passphrase, inFilePath, outFilePath,
                    algorithmNameString, keySizeString, pbkdfSaltString,
                    keySaltString, ivSaltString);
}

QObject* Kryvo::BotanProvider::qObject() {
  return this;
}
