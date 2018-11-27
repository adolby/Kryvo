#include "BotanProvider.hpp"
#include "Constants.hpp"
#include <QSaveFile>
#include <QMimeDatabase>
#include <QMimeType>
#include <QFileInfo>
#include <QDir>
#include <string>
#include <stdexcept>

class Kryvo::BotanProviderPrivate {
  Q_DISABLE_COPY(BotanProviderPrivate)

 public:
  BotanProviderPrivate();

  QMimeDatabase db;

  const std::string kKDFHash = std::string("KDF2(Keccak-1600)");
  const std::string kKeyLabel = std::string("user secret");
  const std::string kIVLabel = std::string("initialization vector");
  const std::size_t kPbkdfIterations = 15000;
};

Kryvo::BotanProviderPrivate::BotanProviderPrivate() = default;

Kryvo::BotanProvider::BotanProvider(QObject* parent)
  : QObject(parent), d_ptr(std::make_unique<BotanProviderPrivate>()) {
}

Kryvo::BotanProvider::~BotanProvider() = default;

bool Kryvo::BotanProvider::encrypt(CryptoState* state,
                                   const QString& passphrase,
                                   const QStringList& inputFilePaths,
                                   const QString& outputPath,
                                   const QString& cipher,
                                   const std::size_t keySize,
                                   const QString& modeOfOperation,
                                   const bool compress) {
  if (!state) {
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

  const QDir outputDir(outputPath);

  // Create output path if it doesn't exist
  if (!outputDir.exists()) {
    outputDir.mkpath(outputPath);
  }

  bool success = true;

  for (const QString& inputFilePath : inputFilePaths) {
    const QFileInfo inputFileInfo(inputFilePath);
    const QString& inFilePath = inputFileInfo.absoluteFilePath();

    const QString& outPath = outputDir.exists() ?
                             outputDir.absolutePath() :
                             inputFileInfo.absolutePath();

    const QString& outFilePath =
      QString(outPath % QStringLiteral("/") % inputFileInfo.fileName() %
              Kryvo::Constants::kDot %
              Kryvo::Constants::kEncryptedFileExtension);

    try {
      const bool encryptFileSuccess =
        encryptFile(state, passphrase, inFilePath, outFilePath, algorithm,
                    keySize, compress);

      success = success && encryptFileSuccess;
    }
    catch (const Botan::Stream_IO_Error&) {
      emit errorMessage(Kryvo::Constants::messages[8], inFilePath);
      return false;
    }
    catch (const Botan::Invalid_Argument&) {
      emit errorMessage(Kryvo::Constants::messages[8], inFilePath);
      return false;
    }
    catch (const Botan::Exception& e) {
      emit errorMessage(QStringLiteral("Error: ") % QString(e.what()),
                        inFilePath);
      return false;
    }
    catch (const std::invalid_argument&) {
      emit errorMessage(Kryvo::Constants::messages[8], inFilePath);
      return false;
    }
    catch (const std::exception& e) {
      emit errorMessage(QStringLiteral("Error: ") % QString(e.what()),
                        inFilePath);
      return false;
    }

    if (state->isAborted() || state->isStopped(inFilePath)) {
      emit errorMessage(Kryvo::Constants::messages[3], inFilePath);

      if (state->isAborted()) {
        state->abort(false);
        break;
      }
    }
  } // End file loop

  return success;
}

bool Kryvo::BotanProvider::decrypt(CryptoState* state,
                                   const QString& passphrase,
                                   const QStringList& inputFilePaths,
                                   const QString& outputPath) {
  Q_D(BotanProvider);

  if (!state) {
    return false;
  }

  const QDir outputDir(outputPath);

  // Create output path if it doesn't exist
  if (!outputDir.exists()) {
    outputDir.mkpath(outputPath);
  }

  bool success = true;

  for (const QString& inputFilePath : inputFilePaths) {
    const QFileInfo inputFileInfo(inputFilePath);
    const QString& outPath = outputDir.exists() ?
                             outputDir.absolutePath() :
                             inputFileInfo.absolutePath();

    const QString& inFilePath = inputFileInfo.absoluteFilePath();

    const QString& outFilePath = QString(outPath % QStringLiteral("/") %
                                 inputFileInfo.fileName());

    try {
      const bool decryptFileSuccess =
        decryptFile(state, passphrase, inFilePath, outFilePath);

      success = success && decryptFileSuccess;
    }
    catch (const Botan::Stream_IO_Error&) {
      emit errorMessage(Constants::messages[7], inFilePath);
      return false;
    }
    catch (const Botan::Invalid_Argument&) {
      emit errorMessage(Constants::messages[7], inFilePath);
      return false;
    }
    catch (const Botan::Lookup_Error&) {
      emit errorMessage(Constants::messages[7], inFilePath);
      return false;
    }
    catch (const Botan::Exception& e) {
      emit errorMessage(QStringLiteral("Error: ") % QString(e.what()),
                        inFilePath);
      return false;
    }
    catch (const std::invalid_argument&) {
      emit errorMessage(Constants::messages[7], inFilePath);
      return false;
    }
    catch (const std::exception& e) {
      emit errorMessage(QStringLiteral("Error: ") % QString(e.what()),
                        inFilePath);
      return false;
    }

    if (state->isAborted()) {
      state->abort(false);
      emit statusMessage(Constants::messages[4].arg(inFilePath));
      break;
    }

    if (state->isStopped(inFilePath)) {
      emit statusMessage(Constants::messages[4].arg(inFilePath));
    }
  } // End file loop

  return success;
}

bool Kryvo::BotanProvider::encryptFile(CryptoState* state,
                                       const QString& passphrase,
                                       const QString& inputFilePath,
                                       const QString& outputFilePath,
                                       const QString& algorithmName,
                                       const std::size_t keySize,
                                       const bool compress) {
  Q_D(BotanProvider);
  Q_ASSERT(state);

  if (!state) {
    emit errorMessage(Constants::messages[0]);
    return false;
  }

  if (state->isAborted()) {
    return false;
  }

  const QFileInfo inputFileInfo(inputFilePath);

  if (!inputFileInfo.exists() || !inputFileInfo.isFile() ||
      !inputFileInfo.isReadable()) {
    emit errorMessage(Constants::messages[5], inputFilePath);
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
                     pbkdfSalt.size(), d->kPbkdfIterations).bits_of();

  // Create the key and IV
  std::unique_ptr<Botan::KDF> kdf(Botan::KDF::create(d->kKDFHash));

  // Set up key salt size
  const std::size_t keySaltSize = 64;
  Botan::secure_vector<Botan::byte> keySalt;
  keySalt.resize(keySaltSize);
  rng.randomize(&keySalt[0], keySalt.size());

  // Key is constrained to sizes allowed by algorithm
  const std::size_t keySizeInBytes = keySize / 8;
  const auto* keyLabelVector =
    reinterpret_cast<const Botan::byte*>(d->kKeyLabel.data());
  Botan::SymmetricKey key(kdf->derive_key(keySizeInBytes,
                                          pbkdfKey.data(),
                                          pbkdfKey.size(),
                                          keySalt.data(),
                                          keySalt.size(),
                                          keyLabelVector,
                                          d->kKeyLabel.size()));

  // Set up IV salt size
  const std::size_t ivSaltSize = 64;
  Botan::secure_vector<Botan::byte> ivSalt;
  ivSalt.resize(ivSaltSize);
  rng.randomize(&ivSalt[0], ivSalt.size());

  const std::size_t ivSize = 256;
  const auto* ivLabelVector =
    reinterpret_cast<const Botan::byte*>(d->kIVLabel.data());
  Botan::InitializationVector iv(kdf->derive_key(ivSize,
                                                 pbkdfKey.data(),
                                                 pbkdfKey.size(),
                                                 ivSalt.data(),
                                                 ivSalt.size(),
                                                 ivLabelVector,
                                                 d->kIVLabel.size()));

  QFile inFile(inputFilePath);
  const bool inFileOpen = inFile.open(QIODevice::ReadOnly);

  if (!inFileOpen) {
    emit errorMessage(Constants::messages[5], inputFilePath);
    return false;
  }

  QSaveFile outFile(outputFilePath);
  const bool outFileOpen = outFile.open(QIODevice::WriteOnly);

  if (!outFileOpen) {
    emit errorMessage(Constants::messages[8], inputFilePath);
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
      text = QByteArrayLiteral("Compressed");
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

  const QByteArray& footerText = QByteArrayLiteral("---------------------------------");

  outFile.write(footerText);
  outFile.write(newLine);

  Botan::Pipe pipe;

  pipe.append_filter(Botan::get_cipher(algorithmName.toStdString(), key, iv,
                                       Botan::ENCRYPTION));

  const bool success =
    executeCipher(state, Botan::ENCRYPTION, &inFile, &outFile, &pipe);

  if (!success) {
    emit errorMessage(Constants::messages[8], inputFilePath);
    return false;
  }

  if (!state->isAborted() && !state->isStopped(inputFilePath)) {
    // Progress: finished
    emit fileProgress(inputFilePath, tr("Encrypted"), 100);

    // Encryption success message
    emit statusMessage(Constants::messages[1].arg(inputFilePath));
  }

  outFile.commit();

  return true;
}

bool Kryvo::BotanProvider::decryptFile(CryptoState* state,
                                       const QString& passphrase,
                                       const QString& inputFilePath,
                                       const QString& outputFilePath) {
  Q_D(BotanProvider);
  Q_ASSERT(state);

  if (!state) {
    emit errorMessage(Constants::messages[0]);
    return false;
  }

  if (state->isAborted()) {
    return false;
  }

  const QFileInfo inputFileInfo(inputFilePath);

  if (!inputFileInfo.exists() || !inputFileInfo.isFile() ||
      !inputFileInfo.isReadable()) {
    emit errorMessage(Constants::messages[5], inputFilePath);
    return false;
  }

  QFile inFile(inputFilePath);

  const bool inFileOpen = inFile.open(QIODevice::ReadOnly);

  if (!inFileOpen) {
    emit errorMessage(Constants::messages[5], inputFilePath);
    return false;
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

  if (headerString != QByteArrayLiteral("-------- ENCRYPTED FILE --------")) {
    emit errorMessage(Constants::messages[7], inputFilePath);
    return false;
  }

  const QString& algorithmNameString = readLine(&inFile);
  const QString& keySizeString = readLine(&inFile);
  const QString& compressString = readLine(&inFile);

  const QString& pbkdfSaltString = readLine(&inFile);
  const QString& keySaltString = readLine(&inFile);
  const QString& ivSaltString = readLine(&inFile);

  const QByteArray& footerString = readLine(&inFile);

  if (footerString != QByteArrayLiteral("---------------------------------")) {
    emit errorMessage(Constants::messages[7], inputFilePath);
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
  std::unique_ptr<Botan::KDF> kdf(Botan::KDF::create(d->kKDFHash));

  // Key salt
  const Botan::secure_vector<Botan::byte>& keySalt =
    Botan::base64_decode(keySaltString.toStdString());

  bool keySizeIntOk = false;

  const std::size_t keySize =
      static_cast<std::size_t>(keySizeString.toInt(&keySizeIntOk));

  if (!keySizeIntOk) {
    emit errorMessage(Constants::messages[7], inputFilePath);
    return false;
  }

  const std::size_t keySizeInBytes = keySize / 8;

  const auto* keyLabelVector =
    reinterpret_cast<const Botan::byte*>(d->kKeyLabel.data());

  Botan::SymmetricKey key(kdf->derive_key(keySizeInBytes,
                                          pbkdfKey.data(),
                                          pbkdfKey.size(),
                                          keySalt.data(),
                                          keySalt.size(),
                                          keyLabelVector,
                                          d->kKeyLabel.size()));

  const Botan::secure_vector<Botan::byte>& ivSalt =
    Botan::base64_decode(ivSaltString.toStdString());
  const std::size_t ivSize = 256;
  const auto* ivLabelVector =
    reinterpret_cast<const Botan::byte*>(d->kIVLabel.data());
  Botan::InitializationVector iv(kdf->derive_key(ivSize,
                                                 pbkdfKey.data(),
                                                 pbkdfKey.size(),
                                                 ivSalt.data(),
                                                 ivSalt.size(),
                                                 ivLabelVector,
                                                 d->kIVLabel.size()));

  // Remove the .enc extensions if at the end of the file path
  const QString& outFilePath =
    Constants::removeExtension(outputFilePath,
                               Constants::kEncryptedFileExtension);

  // Create a unique file name for the file in this directory
  const QString& uniqueOutputFilePath = Constants::uniqueFilePath(outFilePath);

  QSaveFile outFile(uniqueOutputFilePath);

  const bool outFileOpen = outFile.open(QIODevice::WriteOnly);

  if (!outFileOpen) {
    emit errorMessage(Constants::messages[7], inputFilePath);
    return false;
  }

  Botan::Pipe pipe;

  pipe.append_filter(Botan::get_cipher(algorithmNameString.toStdString(), key,
                                       iv, Botan::DECRYPTION));

  const bool success = executeCipher(state, Botan::DECRYPTION, &inFile,
                                     &outFile, &pipe);

  if (!success) {
    emit errorMessage(Constants::messages[7], inputFilePath);
    return false;
  }

  if (!state->isAborted() && !state->isStopped(inputFilePath)) {
    // Progress: finished
    emit fileProgress(inputFilePath, tr("Decrypted"), 100);

    // Decryption success message
    emit statusMessage(Constants::messages[2].arg(inputFilePath));
  }

  outFile.commit();

  return true;
}

bool Kryvo::BotanProvider::executeCipher(CryptoState* state,
                                         const Botan::Cipher_Dir direction,
                                         QFile* inFile, QSaveFile* outFile,
                                         Botan::Pipe* pipe) {
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

  while (!inFile->atEnd() && !state->isAborted() &&
         !state->isStopped(inFile->fileName())) {
    if (!state->isPaused()) {
      const qint64 readSize =
        inFile->read(reinterpret_cast<char*>(&buffer[0]), buffer.size());

      if (readSize < 0) {
        emit errorMessage(Constants::messages[5], inFile->fileName());
        return false;
      }

      pipe->write(&buffer[0], static_cast<std::size_t>(readSize));

      // Calculate progress in percent
      fileIndex += readSize;
      const double nextFraction = static_cast<double>(fileIndex) /
                                  static_cast<double>(size);
      const int nextPercent = static_cast<int>(nextFraction * 100);

      if (nextPercent > percent && nextPercent < 100) {
        percent = nextPercent;

        const QString& task = Botan::ENCRYPTION == direction ?
                              tr("Encrypting") :
                              tr("Decrypting");

        emit fileProgress(inFile->fileName(), task, percent);
      }

      if (inFile->atEnd()) {
        pipe->end_msg();
      }

      while (pipe->remaining() > 0) {
        const std::size_t buffered = pipe->read(&buffer[0], buffer.size());

        if (buffered < 0) {
          if (Botan::ENCRYPTION == direction) {
            emit errorMessage(Constants::messages[8], inFile->fileName());
          } else {
            emit errorMessage(Constants::messages[7], inFile->fileName());
          }
        }

        const qint64 writeSize =
          outFile->write(reinterpret_cast<const char*>(&buffer[0]), buffered);

        if (writeSize < 0) {
          if (Botan::ENCRYPTION == direction) {
            emit errorMessage(Constants::messages[8], inFile->fileName());
          } else {
            emit errorMessage(Constants::messages[7], inFile->fileName());
          }
        }
      }
    }
  }

  return true;
}

QObject* Kryvo::BotanProvider::qObject() {
  return this;
}
