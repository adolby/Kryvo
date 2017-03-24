#include "src/cryptography/BotanCrypto.hpp"
#include "src/constants.h"
#include <QMimeDatabase>
#include <QMimeType>
#include <QDir>
#include <QFileInfo>
#include <string>
#include <stdexcept>
#include <memory>

#include <QDebug>

namespace Kryvos {

namespace Constants {
  const auto kKDFHash = std::string{"KDF2(Keccak-1600)"};
  const auto kKeyLabel = std::string{"user secret"};
  const auto kIVLabel = std::string{"initialization vector"};
}

}

Kryvos::BotanCrypto::BotanCrypto(QObject* parent)
  : QObject{parent} {
  // Initialize Botan
  const QString threadSafe = QStringLiteral("thread_safe=true");
  Botan::LibraryInitializer init{std::string{threadSafe.toUtf8().constData()}};
}

Kryvos::BotanCrypto::~BotanCrypto() {
}

void Kryvos::BotanCrypto::encrypt(State* state,
                                  Archiver* archiver,
                                  const QString& passphrase,
                                  const QStringList& inputFilePaths,
                                  const QString& outputPath,
                                  const QString& cipher,
                                  const std::size_t keySize,
                                  const QString& modeOfOperation,
                                  const bool compress,
                                  const bool container) {
  const QString algorithm = [&cipher, &keySize, &modeOfOperation] {
    auto algo = QString{cipher % QStringLiteral("/") % modeOfOperation};

    if (QStringLiteral("AES") == cipher) {
      algo = QString{cipher % QStringLiteral("-") % QString::number(keySize) %
                     QStringLiteral("/") % modeOfOperation};
    }

    return algo;
  }();

  auto outputFilePaths = QStringList{};

  const QFileInfo outputPathInfo{outputPath};

  foreach (const auto& inputFilePath, inputFilePaths) {
    const QFileInfo inputFileInfo{inputFilePath};
    const QString inFilePath = inputFileInfo.absoluteFilePath();

    try {
      const QString outPath =
        outputPath.isEmpty() ? inputFileInfo.absolutePath() :
                               outputPathInfo.absolutePath();

      // Create output path if it doesn't exist
      const QDir pathDir{outPath};
      if (!pathDir.exists()) {
        pathDir.mkpath(outPath);
      }

      const QString outFilePath =
          QString{outPath % QDir::separator() % inputFileInfo.fileName() %
                  Constants::kDot % Constants::kExtension};

      outputFilePaths << outFilePath;

      encryptFile(state, passphrase, inFilePath, outFilePath, algorithm,
                  keySize, compress);

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
    }
    catch (const Botan::Invalid_Argument&) {
      emit errorMessage(Constants::messages[7], inFilePath);
    }
    catch (const std::invalid_argument&) {
      emit errorMessage(Constants::messages[7], inFilePath);
    }
    catch (const std::exception& e) {
      const QString error = QString{e.what()};
      emit errorMessage(QStringLiteral("Error: ") % error, inFilePath);
    }
  } // End file loop

  if (container) {
    const QString& outputArchiveFileName =
      outputPathInfo.isFile() ? outputPath :
                                QString{outputFilePaths.first() %
                                        Constants::kDot %
                                        Constants::kArchiveExtension};

    const bool compressStatus = archiver->compressFiles(outputArchiveFileName,
                                                        outputFilePaths,
                                                        inputFilePaths);

    if (!compressStatus) {
      emit errorMessage(Constants::messages[8], outputArchiveFileName);
    }
  }
}

void Kryvos::BotanCrypto::decrypt(State* state,
                                  Archiver* archiver,
                                  const QString& passphrase,
                                  const QStringList& inputFilePaths,
                                  const QString& outputPath) {
  foreach (const auto& inputFilePath, inputFilePaths) {
    const QFileInfo inputFileInfo{inputFilePath};
    const QString inFilePath = inputFileInfo.absoluteFilePath();

    try {
      QMimeDatabase db{};
      const QMimeType mime = db.mimeTypeForFile(inputFileInfo);

      const QString inPath = inputFileInfo.absolutePath();
      const QString outPath = [&inPath, &outputPath] {
        auto path = QString{inPath};

        const QFileInfo outputPathInfo{outputPath};

        if (outputPathInfo.isDir()) {
          path = outputPathInfo.absolutePath();
        }

        return QString{path % QDir::separator()};
      }();

      // Create output path if it doesn't exist
      const QDir path{outPath};
      if (!path.exists()) {
        path.mkpath(outPath);
      }

      const QString inFileName = inputFileInfo.fileName();
      const QString outFilePath = QString{outPath % inFileName};

      if (mime.inherits("application/zip")) { // Container file
        const QStringList archiveFilePaths =
          archiver->extractDir(inFilePath, outPath);

        if (archiveFilePaths.isEmpty()) {
          emit errorMessage(Constants::messages[9], inFilePath);
        }

        foreach (const auto& archiveFilePath, archiveFilePaths) {
          decryptFile(state, passphrase, archiveFilePath, outFilePath);

          if (state->isAborted()) {
            state->abort(false);
            emit errorMessage(Constants::messages[3], archiveFilePath);
            break;
          }

          if (state->isStopped(archiveFilePath)) {
            emit errorMessage(Constants::messages[3], archiveFilePath);
          }
        }
      }
      else { // Non-container file
        decryptFile(state, passphrase, inFilePath, outFilePath);
      }

      if (state->isAborted()) {
        state->abort(false);
        emit errorMessage(Constants::messages[3], inFilePath);
        break;
      }

      if (state->isStopped(inFilePath)) {
        emit errorMessage(Constants::messages[3], inFilePath);
      }
    }
    catch (const Botan::Algorithm_Not_Found) {
      // No need to warn about this error
    }
    catch (const Botan::Stream_IO_Error&) {
      emit errorMessage(Constants::messages[5], inFilePath);
    }
    catch (const Botan::Decoding_Error&) {
      emit errorMessage(Constants::messages[5], inFilePath);
    }
    catch (const Botan::Integrity_Failure&) {
      emit errorMessage(Constants::messages[5], inFilePath);
    }
    catch (const Botan::Invalid_Argument&) {
      emit errorMessage(Constants::messages[6], inFilePath);
    }
    catch (const std::invalid_argument&) {
      emit errorMessage(Constants::messages[6], inFilePath);
    }
    catch (const std::runtime_error&) {
      emit errorMessage(Constants::messages[4], inFilePath);
    }
    catch (const std::exception& e) {
      const QString error = QString{e.what()};

      if (QStringLiteral("zlib inflate error -3") == error) {
        emit errorMessage(Constants::messages[5], inFilePath);
      }
      else {
        emit errorMessage(QString{QStringLiteral("Error: ") % error},
                          inFilePath);
      }
    }
  }
}

void Kryvos::BotanCrypto::encryptFile(State* state,
                                      const QString& passphrase,
                                      const QString& inputFilePath,
                                      const QString& outputFilePath,
                                      const QString& algorithmName,
                                      const std::size_t keySize,
                                      const bool compress) {
  Q_ASSERT(state);

  const QFileInfo inputFileInfo{inputFilePath};

  if (!state->isAborted() && inputFileInfo.exists() &&
      inputFileInfo.isFile() && inputFileInfo.isReadable()) {
    Botan::AutoSeeded_RNG rng{};

    // Define a size for the PBKDF salt vector
    const auto pbkdfSaltSize = static_cast<std::size_t>(256);
    Botan::secure_vector<Botan::byte> pbkdfSalt{};
    pbkdfSalt.resize(pbkdfSaltSize);

    // Create random PBKDF salt
    rng.randomize(&pbkdfSalt[0], pbkdfSalt.size());

    // Set up the key derive functions
    const auto macSize = static_cast<std::size_t>(512);

    // PKCS5_PBKDF2 takes ownership of the new HMAC and the HMAC takes ownership
    // of the Keccak_1600 hash function object (both via unique_ptr)
    Botan::PKCS5_PBKDF2 pbkdf{new Botan::HMAC{new Botan::Keccak_1600{macSize}}};
    const auto PBKDF2_ITERATIONS = static_cast<std::size_t>(15000);

    // Create the PBKDF key
    const auto pbkdfKeySize = static_cast<std::size_t>(256);
    Botan::secure_vector<Botan::byte> pbkdfKey =
      pbkdf.derive_key(pbkdfKeySize, passphrase.toUtf8().constData(),
                       &pbkdfSalt[0], pbkdfSalt.size(),
                       PBKDF2_ITERATIONS).bits_of();

    // Create the key and IV
    std::unique_ptr<Botan::KDF> kdf{Botan::KDF::create(Constants::kKDFHash)};

    // Set up key salt size
    const auto keySaltSize = static_cast<std::size_t>(64);
    Botan::secure_vector<Botan::byte> keySalt{};
    keySalt.resize(keySaltSize);
    rng.randomize(&keySalt[0], keySalt.size());

    // Key is constrained to sizes allowed by algorithm
    const auto keySizeInBytes = static_cast<std::size_t>(keySize / 8);
    const auto keyLabelVector =
      reinterpret_cast<const Botan::byte*>(Constants::kKeyLabel.data());
    Botan::SymmetricKey key{kdf->derive_key(keySizeInBytes,
                                            pbkdfKey.data(),
                                            pbkdfKey.size(),
                                            keySalt.data(),
                                            keySalt.size(),
                                            keyLabelVector,
                                            Constants::kKeyLabel.size())};

    // Set up IV salt size
    const auto ivSaltSize = static_cast<std::size_t>(64);
    Botan::secure_vector<Botan::byte> ivSalt{};
    ivSalt.resize(ivSaltSize);
    rng.randomize(&ivSalt[0], ivSalt.size());

    const auto ivSize = static_cast<std::size_t>(256);
    const auto ivLabelVector =
      reinterpret_cast<const Botan::byte*>(Constants::kIVLabel.data());
    Botan::InitializationVector iv{kdf->derive_key(ivSize,
                                                   pbkdfKey.data(),
                                                   pbkdfKey.size(),
                                                   ivSalt.data(),
                                                   ivSalt.size(),
                                                   ivLabelVector,
                                                   Constants::kIVLabel.size())};

    std::ifstream in{inputFilePath.toUtf8().constData(), std::ios::binary};
    std::ofstream out{outputFilePath.toUtf8().constData(), std::ios::binary};

    const auto algorithmNameStd =
      std::string{algorithmName.toUtf8().constData()};

    const QString headerText = tr("-------- ENCRYPTED FILE --------");

    const QString compressedText = [&compress] {
      auto text = tr("Not compressed");

      if (compress) {
        text = tr("Compressed");
      }

      return text;
    }();

    out << headerText.toUtf8().constData() << std::endl;
    out << algorithmNameStd << std::endl;
    out << keySize << std::endl;
    out << compressedText.toUtf8().constData() << std::endl;
    out << Botan::base64_encode(&pbkdfSalt[0], pbkdfSalt.size()) << std::endl;
    out << Botan::base64_encode(&keySalt[0], keySalt.size()) << std::endl;
    out << Botan::base64_encode(&ivSalt[0], ivSalt.size()) << std::endl;

    Botan::Pipe pipe{};

    if (compress) {
      pipe.append(new Botan::Compression_Filter{std::string{"zlib"},
                                                static_cast<std::size_t>(9)});
    }

    pipe.append(Botan::get_cipher(algorithmNameStd,
                                  key,
                                  iv,
                                  Botan::ENCRYPTION));

    pipe.append(new Botan::DataSink_Stream{out});

    executeCipher(state, Botan::ENCRYPTION, inputFilePath, pipe, in, out);

    if (!state->isAborted() && !state->isStopped(inputFilePath)) {
      // Progress: finished
      emit fileProgress(inputFilePath, tr("Encrypted"), 100);

      // Encryption success message
      emit statusMessage(Constants::messages[0].arg(inputFilePath));
    }
  }
  else {
    const QString error = tr("Input file doesn't exist or can't be read");
    throw std::invalid_argument{error.toUtf8().constData()};
  }
}

void Kryvos::BotanCrypto::decryptFile(State* state,
                                      const QString& passphrase,
                                      const QString& inputFilePath,
                                      const QString& outputFilePath) {
  Q_ASSERT(state);

  const QFileInfo inputFileInfo{inputFilePath};

  if (!state->isAborted() && inputFileInfo.exists() &&
      inputFileInfo.isFile() && inputFileInfo.isReadable()) {
    std::ifstream in{inputFilePath.toUtf8().constData(), std::ios::binary};

    // Read metadata from file
    std::string headerStringStd, algorithmNameStd, keySizeString,
                compressStringStd;
    std::string pbkdfSaltString, keySaltString, ivSaltString;

    std::getline(in, headerStringStd);
    std::getline(in, algorithmNameStd);
    std::getline(in, keySizeString);
    std::getline(in, compressStringStd);

    std::getline(in, pbkdfSaltString);
    std::getline(in, keySaltString);
    std::getline(in, ivSaltString);

    const QString headerString = QString{headerStringStd.c_str()};
    const QString compressString = QString{compressStringStd.c_str()};

    if (headerString != tr("-------- ENCRYPTED FILE --------")) {
      emit errorMessage(Constants::messages[6].arg(inputFilePath));
    }

    // Set up the key derive functions
    const auto macSize = static_cast<std::size_t>(512);

    // PKCS5_PBKDF2 takes ownership of the new HMAC and the HMAC takes ownership
    // of the Keccak_1600 hash function object (via unique_ptr)
    Botan::PKCS5_PBKDF2 pbkdf{new Botan::HMAC{new Botan::Keccak_1600{macSize}}};
    const auto kPBKDF2Iterations = static_cast<std::size_t>(15000);

    // Create the PBKDF key
    Botan::secure_vector<Botan::byte> pbkdfSalt =
      Botan::base64_decode(pbkdfSaltString);
    const auto pbkdfKeySize = static_cast<std::size_t>(256);
    Botan::secure_vector<Botan::byte> pbkdfKey =
      pbkdf.derive_key(pbkdfKeySize, passphrase.toUtf8().constData(),
                       &pbkdfSalt[0], pbkdfSalt.size(),
                       kPBKDF2Iterations).bits_of();

    // Create the key and IV
    std::unique_ptr<Botan::KDF> kdf{Botan::KDF::create(Constants::kKDFHash)};

    // Key salt
    Botan::secure_vector<Botan::byte> keySalt =
      Botan::base64_decode(keySaltString);
    const auto keySize =
      static_cast<std::size_t>(QString{keySizeString.c_str()}.toInt());
    const auto keySizeInBytes = static_cast<std::size_t>(keySize / 8);
    const auto keyLabelVector =
      reinterpret_cast<const Botan::byte*>(Constants::kKeyLabel.data());
    Botan::SymmetricKey key{kdf->derive_key(keySizeInBytes,
                                            pbkdfKey.data(),
                                            pbkdfKey.size(),
                                            keySalt.data(),
                                            keySalt.size(),
                                            keyLabelVector,
                                            Constants::kKeyLabel.size())};

    Botan::secure_vector<Botan::byte> ivSalt =
      Botan::base64_decode(ivSaltString);
    const auto ivSize = static_cast<std::size_t>(256);
    const auto ivLabelVector =
      reinterpret_cast<const Botan::byte*>(Constants::kIVLabel.data());
    Botan::InitializationVector iv{kdf->derive_key(ivSize,
                                                   pbkdfKey.data(),
                                                   pbkdfKey.size(),
                                                   ivSalt.data(),
                                                   ivSalt.size(),
                                                   ivLabelVector,
                                                   Constants::kIVLabel.size())};

    // Remove the .enc and .zip extensions if at the end of the file path
    const QString outFilePath =
      Constants::removeExtension(outputFilePath, Constants::kExtension);

    // Create a unique file name for the file in this directory
    const QString uniqueOutputFilePath = Constants::uniqueFilePath(outFilePath);

    std::ofstream out{uniqueOutputFilePath.toUtf8().constData(),
                      std::ios::binary};

    Botan::Pipe pipe{};

    pipe.append(Botan::get_cipher(algorithmNameStd, key, iv,
                                  Botan::DECRYPTION));

    if (tr("Compressed") == compressString) {
      const QString zlib = QStringLiteral("zlib");
      pipe.append(new Botan::Decompression_Filter{zlib.toUtf8().constData(),
                                                  static_cast<std::size_t>(9)});
    }

    pipe.append(new Botan::DataSink_Stream{out});

    executeCipher(state, Botan::DECRYPTION, inputFilePath, pipe, in, out);

    if (!state->isAborted() && !state->isStopped(inputFilePath)) {
      // Progress: finished
      emit fileProgress(inputFilePath, tr("Decrypted"), 100);

      // Decryption success message
      emit statusMessage(Constants::messages[1].arg(inputFilePath));
    }
  }
  else {
    const QString error = tr("Input file doesn't exist or can't be read");
    throw std::invalid_argument{error.toUtf8().constData()};
  }
}

void Kryvos::BotanCrypto::executeCipher(State* state,
                                        Botan::Cipher_Dir direction,
                                        const QString& inputFilePath,
                                        Botan::Pipe& pipe,
                                        std::ifstream& in,
                                        std::ofstream& out) {
  Q_ASSERT(state);

  // Define a size for the buffer vector
  const auto bufferSize = static_cast<std::size_t>(4096);
  Botan::secure_vector<Botan::byte> buffer;
  buffer.resize(bufferSize);

  // Get file size for percent progress calculation
  const QFileInfo file{inputFilePath};
  const qint64 size = file.size();
  auto fileIndex = static_cast<std::size_t>(0);
  qint64 percent = -1;

  pipe.start_msg();

  while (in.good() && !state->isAborted() &&
         !state->isStopped(inputFilePath)) {
    if (!state->isPaused()) {
      in.read(reinterpret_cast<char*>(&buffer[0]), buffer.size());
      const auto readSize = static_cast<std::size_t>(in.gcount());

      pipe.write(&buffer[0], readSize);

      // Calculate progress in percent
      fileIndex += readSize;
      const auto nextFraction = static_cast<double>(fileIndex) /
                                static_cast<double>(size);
      const auto nextPercent = static_cast<int>(nextFraction * 100);

      if (nextPercent > percent && nextPercent < 100) {
        percent = nextPercent;

        const QString task = (Botan::ENCRYPTION == direction) ?
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
  }

  out.flush();
}
