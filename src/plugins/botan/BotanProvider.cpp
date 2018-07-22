#include "BotanProvider.hpp"
#include "constants.h"
#include <QMimeDatabase>
#include <QMimeType>
#include <QFileInfo>
#include <QDir>
#include <string>
#include <stdexcept>

#include <QDebug>

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
  // Initialize Botan
  Botan::LibraryInitializer init(std::string("thread_safe=true"));
}

Kryvo::BotanProvider::~BotanProvider() = default;

bool Kryvo::BotanProvider::encrypt(CryptoState* state,
                                   const QString& passphrase,
                                   const QStringList& inputFilePaths,
                                   const QString& outputPath,
                                   const QString& cipher,
                                   const std::size_t keySize,
                                   const QString& modeOfOperation,
                                   const bool compress, const bool container) {
  if (!state) {
    return false;
  }

  const QString& algorithm = [&cipher, &keySize, &modeOfOperation]() {
    QString algo = QString(cipher % QDir::separator() % modeOfOperation);

    if (QStringLiteral("AES") == cipher) {
      algo = QString(cipher % QStringLiteral("-") % QString::number(keySize) %
                     QDir::separator() % modeOfOperation);
    }

    return algo;
  }();

  const QDir outputDir(outputPath);

  // Create output path if it doesn't exist
  if (!outputDir.exists()) {
    outputDir.mkpath(outputPath);
  }

  QStringList outputFilePaths;

  for (const QString& inputFilePath : inputFilePaths) {
    const QFileInfo inputFileInfo(inputFilePath);
    const QString& inFilePath = inputFileInfo.absoluteFilePath();

    const QString& outPath = outputDir.exists() ?
                             outputDir.absolutePath() :
                             inputFileInfo.absolutePath();

    const QString& outFilePath = QString(outPath % QDir::separator() %
                                         inputFileInfo.fileName() %
                                         Constants::kDot %
                                         Constants::kExtension);

    outputFilePaths << outFilePath;

    try {
      encryptFile(state, passphrase, inFilePath, outFilePath, algorithm,
                  keySize, compress);

      qDebug() << inputFilePath;

      if (state->isAborted() || state->isStopped(inFilePath)) {
        emit errorMessage(Constants::messages[2], inFilePath);

        if (state->isAborted()) {
          state->abort(false);
          break;
        }
      }
    }
    catch (const Botan::Stream_IO_Error&) {
      emit errorMessage(Constants::messages[7], inFilePath);
      qDebug() << "BOTAN STREAM ERROR";
      return false;
    }
    catch (const Botan::Invalid_Argument&) {
      emit errorMessage(Constants::messages[7], inFilePath);
      qDebug() << "BOTAN INVALID ARGUMENT";
      return false;
    }
    catch (const std::invalid_argument&) {
      emit errorMessage(Constants::messages[7], inFilePath);
      qDebug() << "STD INVALID ARGUMENT";
      return false;
    }
    catch (const std::exception& e) {
      const QString error(e.what());
      emit errorMessage(QStringLiteral("Error: ") % error, inFilePath);
      qDebug() << "ERROROROROR";
      return false;
    }
  } // End file loop

// TODO: Fix container
//  if (container) {
//    const QString& outputArchiveFileName =
//      outputPathInfo.isFile() ?
//      outputPath :
//      QString(outputFilePaths.first() % Constants::kDot %
//              Constants::kArchiveExtension);

//    const bool compressStatus =
//        archiver->compressFiles(outputArchiveFileName, outputFilePaths,
//                                inputFilePaths);

//    if (!compressStatus) {
//      emit errorMessage(Constants::messages[8], outputArchiveFileName);
//      return false;
//    }
//  }

  return true;
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

  for (const QString& inputFilePath : inputFilePaths) {
    const QFileInfo inputFileInfo(inputFilePath);
    const QMimeType& mime = d->db.mimeTypeForFile(inputFileInfo);

    const QString& inPath = inputFileInfo.absolutePath();

    const QString& outPath = outputDir.exists() ?
                             outputDir.absolutePath() :
                             inputFileInfo.absolutePath();

    const QString& inFilePath = inputFileInfo.absoluteFilePath();

    const QString& outFilePath = QString(outPath % QDir::separator() %
                                         inputFileInfo.fileName());

// TODO: Fix container
//    if (mime.inherits(QStringLiteral("application/zip"))) { // Container file
//      emit extract(state, passphrase, inFilePath, outPath);
//    }
//    else { // Non-container file
      decryptFile(state, passphrase, inFilePath, outFilePath);
//    }

    if (state->isAborted()) {
      state->abort(false);

      // TODO: Change to status message
      emit errorMessage(Constants::messages[3], inFilePath);
      break;
    }

    if (state->isStopped(inFilePath)) {
      // TODO: Change to status message
      emit errorMessage(Constants::messages[3], inFilePath);
    }
  }

  return true;
}

bool Kryvo::BotanProvider::encryptFile(CryptoState* state,
                                       const QString& passphrase,
                                       const QString& inputFilePath,
                                       const QString& outputFilePath,
                                       const QString& algorithmName,
                                       const std::size_t keySize,
                                       const bool compress) {
  Q_D(BotanProvider);

  if (!state) {
    return false;
  }

  qDebug() << inputFilePath;

  const QFileInfo inputFileInfo(inputFilePath);

  if (!state || state->isAborted() || !inputFileInfo.exists() ||
      !inputFileInfo.isFile() || !inputFileInfo.isReadable()) {
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
    pbkdf.derive_key(pbkdfKeySize, passphraseString,
                     &pbkdfSalt[0], pbkdfSalt.size(),
                     d->kPbkdfIterations).bits_of();

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
  const std::string& inputFilePathString = inputFilePath.toStdString();

  std::ifstream in(inputFilePathString, std::ios::binary);

  const std::string& outputFilePathString = outputFilePath.toStdString();
  std::ofstream out(outputFilePathString, std::ios::binary);

  const std::string& algorithmNameStd = algorithmName.toStdString();

  const QString& headerText = tr("-------- ENCRYPTED FILE --------");

  const QString& compressedText = [&compress]() {
    QString text = tr("Not compressed");

    if (compress) {
      text = tr("Compressed");
    }

    return text;
  }();

  const std::string& headerTextString = headerText.toStdString();
  const std::string& compressedTextString = compressedText.toStdString();

  const std::string newLine("\n");

  out << headerTextString << newLine;
  out << algorithmNameStd << newLine;
  out << keySize << newLine;
  out << compressedTextString << newLine;
  out << Botan::base64_encode(&pbkdfSalt[0], pbkdfSalt.size()) << newLine;
  out << Botan::base64_encode(&keySalt[0], keySalt.size()) << newLine;
  out << Botan::base64_encode(&ivSalt[0], ivSalt.size()) << newLine;

  Botan::Pipe pipe;

  if (compress) {
    pipe.append(new Botan::Compression_Filter(std::string("zlib"),
                                              static_cast<std::size_t>(9)));
  }

  pipe.append(Botan::get_cipher(algorithmNameStd, key, iv, Botan::ENCRYPTION));

  pipe.append(new Botan::DataSink_Stream(out));

  const bool success =
    executeCipher(state, Botan::ENCRYPTION, inputFilePath, pipe, in, out);

  if (!success) {
    return false;
  }

  if (!state->isAborted() && !state->isStopped(inputFilePath)) {
    // Progress: finished
    emit fileProgress(inputFilePath, tr("Encrypted"), 100);

    // Encryption success message
    emit statusMessage(Constants::messages[0].arg(inputFilePath));
  }

  return true;
}

bool Kryvo::BotanProvider::decryptFile(CryptoState* state,
                                       const QString& passphrase,
                                       const QString& inputFilePath,
                                       const QString& outputFilePath) {
  Q_D(BotanProvider);

  Q_ASSERT(state);

  const QFileInfo inputFileInfo(inputFilePath);

  if (!state || state->isAborted() || !inputFileInfo.exists() ||
      !inputFileInfo.isFile() || !inputFileInfo.isReadable()) {
    emit errorMessage(Constants::messages[4], inputFilePath);
    return false;
  }

  const std::string& inputFilePathString = inputFilePath.toStdString();

  std::ifstream in(inputFilePathString, std::ios::binary);

  // Read metadata from file
  std::string headerStringStd, algorithmNameStd, keySizeStringStd,
    compressStringStd;

  std::string pbkdfSaltString, keySaltString, ivSaltString;

  std::getline(in, headerStringStd);
  std::getline(in, algorithmNameStd);
  std::getline(in, keySizeStringStd);
  std::getline(in, compressStringStd);

  std::getline(in, pbkdfSaltString);
  std::getline(in, keySaltString);
  std::getline(in, ivSaltString);

  const QString& headerString = QString::fromStdString(headerStringStd);
  const QString& compressString = QString::fromStdString(compressStringStd);

  if (headerString != tr("-------- ENCRYPTED FILE --------")) {
    emit errorMessage(Constants::messages[6].arg(inputFilePath));
  }

  // Set up the key derive functions
  const std::size_t macSize = 512;

  // PKCS5_PBKDF2 takes ownership of the new HMAC and the HMAC takes ownership
  // of the Keccak_1600 hash function object (via unique_ptr)
  Botan::PKCS5_PBKDF2 pbkdf(new Botan::HMAC(new Botan::Keccak_1600(macSize)));
  const std::size_t kPBKDF2Iterations = 15000;

  // Create the PBKDF key
  const Botan::secure_vector<Botan::byte>& pbkdfSalt =
    Botan::base64_decode(pbkdfSaltString);

  const std::size_t pbkdfKeySize = 256;

  const std::string& passphraseString = passphrase.toStdString();

  const Botan::secure_vector<Botan::byte>& pbkdfKey =
    pbkdf.derive_key(pbkdfKeySize, passphraseString, &pbkdfSalt[0],
                     pbkdfSalt.size(), kPBKDF2Iterations).bits_of();

  // Create the key and IV
  std::unique_ptr<Botan::KDF> kdf(Botan::KDF::create(d->kKDFHash));

  // Key salt
  const Botan::secure_vector<Botan::byte>& keySalt =
    Botan::base64_decode(keySaltString);

  const QString& keySizeString = QString::fromStdString(keySizeStringStd);
  const auto keySize = static_cast<std::size_t>(keySizeString.toInt());

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
    Botan::base64_decode(ivSaltString);
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

  // Remove the .enc and .zip extensions if at the end of the file path
  const QString& outFilePath =
    Constants::removeExtension(outputFilePath, Constants::kExtension);

  // Create a unique file name for the file in this directory
  const QString& uniqueOutputFilePath = Constants::uniqueFilePath(outFilePath);
  const std::string& uniqueOutputFilePathString =
    uniqueOutputFilePath.toStdString();

  std::ofstream out(uniqueOutputFilePathString, std::ios::binary);

  Botan::Pipe pipe;

  pipe.append(Botan::get_cipher(algorithmNameStd, key, iv, Botan::DECRYPTION));

  if (tr("Compressed") == compressString) {
    pipe.append(new Botan::Decompression_Filter(std::string("lzma"),
                                                static_cast<std::size_t>(9)));
  }

  pipe.append(new Botan::DataSink_Stream(out));

  const bool success = executeCipher(state, Botan::DECRYPTION, inputFilePath,
                                     pipe, in, out);

  if (!success) {
    return false;
  }

  if (!state->isAborted() && !state->isStopped(inputFilePath)) {
    // Progress: finished
    emit fileProgress(inputFilePath, tr("Decrypted"), 100);

    // Decryption success message
    emit statusMessage(Constants::messages[1].arg(inputFilePath));
  }

  return true;
}

bool Kryvo::BotanProvider::executeCipher(CryptoState* state,
                                         Botan::Cipher_Dir direction,
                                         const QString& inputFilePath,
                                         Botan::Pipe& pipe,
                                         std::ifstream& in,
                                         std::ofstream& out) {
  Q_ASSERT(state);

  // Define a size for the buffer vector
  const std::size_t bufferSize = 4096;
  Botan::secure_vector<Botan::byte> buffer;
  buffer.resize(bufferSize);

  // Get file size for percent progress calculation
  const QFileInfo file(inputFilePath);
  const qint64 size = file.size();
  std::size_t fileIndex = 0;
  qint64 percent = -1;

  pipe.start_msg();

  while (in.good() && !state->isAborted() &&
         !state->isStopped(inputFilePath)) {
    if (!state->isPaused()) {
      in.read(reinterpret_cast<char*>(&buffer[0]), buffer.size());
      const std::size_t readSize = in.gcount();

      pipe.write(&buffer[0], readSize);

      // Calculate progress in percent
      fileIndex += readSize;
      const double nextFraction = static_cast<double>(fileIndex) /
                                  static_cast<double>(size);
      const auto nextPercent = static_cast<int>(nextFraction * 100);

      if (nextPercent > percent && nextPercent < 100) {
        percent = nextPercent;

        const QString task = Botan::ENCRYPTION == direction ?
                             tr("Encrypting") :
                             tr("Decrypting");

        emit fileProgress(inputFilePath, task, percent);
      }

      if (in.eof()) {
        pipe.end_msg();
      }

      while (pipe.remaining() > 0) {
        const std::size_t buffered = pipe.read(&buffer[0], buffer.size());
        out.write(reinterpret_cast<const char*>(&buffer[0]), buffered);
      }
    }
  }

  if (in.bad() || (in.fail() && !in.eof())) {
    emit errorMessage(Constants::messages[4], inputFilePath);
    return false;
  }

  out.flush();

  return true;
}

QObject* Kryvo::BotanProvider::qObject() {
  return this;
}
