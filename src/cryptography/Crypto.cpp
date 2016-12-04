#include "src/cryptography/Crypto.hpp"
#include "src/utility/pimpl_impl.h"
#include <QFileInfo>
#include <QDir>
#include <QHash>
#include <QStringRef>
#include <QStringBuilder>
#include <string>
#include <atomic>

#if defined(Q_OS_ANDROID)
namespace std {

int64_t stoll(const string& str)
{
  return strtoll(str.c_str(), nullptr, 10);
}

}
#endif

/*!
 * \brief removeExtension Attempts to return the file name string input
 * without the last extension. It's used to extract an extension to determine
 * a decrypted file name.
 * \param fileName String representing the file name.
 * \param extension String representing the extension to remove from the
 * file name.
 * \return String representing a file name without an extension.
 */
QString removeExtension(const QString& fileName, const QString& extension)
{
  QFileInfo file{fileName};
  QString newFileName = fileName;

  if (file.suffix() == extension)
  {
    newFileName = file.absolutePath() % QDir::separator() %
                  file.completeBaseName();
  }

  return newFileName;
}

/*!
 * \brief uniqueFileName Returns a unique file name from the input file name
 * by appending numbers, if necessary.
 * \param fileName String representing the file name that will be tested
 * for uniqueness.
 * \return String representing a unique file name created from the input file
 * name.
 */
QString uniqueFileName(const QString& fileName)
{
  QFileInfo originalFile{fileName};
  QString uniqueFileName = fileName;

  auto foundUniqueFileName = false;
  auto i = 0;

  while (!foundUniqueFileName && i < 100000)
  {
    QFileInfo uniqueFile{uniqueFileName};

    if (uniqueFile.exists() && uniqueFile.isFile())
    { // Write number of copies before file extension
      uniqueFileName = originalFile.absolutePath() % QDir::separator() %
                       originalFile.baseName() % QString{" (%1)"}.arg(i + 2);

      if (!originalFile.completeSuffix().isEmpty())
      { // Add the file extension if there is one
        uniqueFileName += QStringLiteral(".") % originalFile.completeSuffix();
      }

      ++i;
    }
    else
    {
      foundUniqueFileName = true;
    }
  }

  return uniqueFileName;
}

class Crypto::CryptoPrivate {
 public:
  /*!
   * \brief CryptoPrivate Constructs the Crypto private implementation.
   */
  CryptoPrivate();

  /*!
   * \brief resetFlags Resets the status flags, except pause, to default values.
   */
  void resetFlags();

  /*!
   * \brief abort Updates the abort status.
   * \param abort Boolean representing the abort status.
   */
  void abort(const bool abort);

  /*!
   * \brief isAborted Returns the current abort status.
   * \return Boolean representing the abort status.
   */
  bool isAborted() const;

  /*!
   * \brief pause Updates the pause status.
   * \param pause Boolean representing the pause status.
   */
  void pause(const bool pause);

  /*!
   * \brief isPaused Returns the current pause status.
   * \return Boolean representing the pause status.
   */
  bool isPaused() const;

  /*!
   * \brief stop Updates a stop status in the stop status container. A stop
   * status corresponds with a filename and is used to decide whether to stop
   * encrypting/decrypting a file.
   * \param fileName String containing the filename to set to stopped.
   * \param stop Boolean representing the stop status for the file represented
   * by fileName.
   */
  void stop(const QString& fileName, const bool stop);

  /*!
   * \brief isStopped Returns a stop status in the stop status container. A stop
   * status corresponds with a filename and is used to decide whether to stop
   * encrypting/decrypting a file.
   * \param fileName String containing the file name to check the stop status
   * for.
   * \return Boolean Boolean representing the stop status for the file
   * represented by fileName.
   */
  bool isStopped(const QString& fileName) const;

  /*!
   * \brief busy Updates the busy status.
   * \param busy Boolean representing the busy status.
   */
  void busy(const bool busy);

  /*!
   * \brief isBusy Returns the busy status.
   * \return Boolean representing the busy status.
   */
  bool isBusy() const;

  // The list of status messages that can be displayed to the user
  const QStringList messages;

 private:
  // The abort status, when set to true, will stop an executing cryptopgraphic
  // operation and prevent new cipher operations from starting until it is reset
  // to false.
  std::atomic<bool> aborted;

  // The pause status, when set to false, will pause an executing cipher
  // operation. When the pause status is set to true, the cipher operation
  // that was in progress when the pause status was set will resume execution.
  std::atomic<bool> paused;

  // The container of stopped flags, which are used to stop
  // encrypting/decrypting a file.
  QHash<QString, bool> stopped;

  // The busy status, when set to true, indicates that this class is currently
  // executing a cipher operation.
  std::atomic<bool> busyStatus;
};

Crypto::Crypto(QObject* parent)
  : QObject{parent}
{
  // Initialize Botan
  Botan::LibraryInitializer init{std::string{"thread_safe=true"}};
}

Crypto::~Crypto() {}

void Crypto::encrypt(const QString& passphrase,
                     const QStringList& inputFileNames,
                     const QString& cipher,
                     const std::size_t& inputKeySize,
                     const QString& modeOfOperation,
                     const bool compress)
{
  m->busy(true);
  emit busyStatus(m->isBusy());

  auto keySize = std::size_t{128};

  if (inputKeySize > 0)
  {
    keySize = inputKeySize;
  }

  auto algorithm = QStringLiteral("AES-128/GCM");

  if (QStringLiteral("AES") == cipher)
  {
    algorithm = cipher % QStringLiteral("-") % QString::number(keySize) %
                QStringLiteral("/") % modeOfOperation;
  }
  else
  {
    algorithm = cipher % QStringLiteral("/") % modeOfOperation;
  }

  for (const auto& inputFileName : inputFileNames)
  {
    try
    {
      encryptFile(passphrase, inputFileName, algorithm, keySize, compress);

      if (m->isAborted() || m->isStopped(inputFileName))
      {
        emit errorMessage(inputFileName, m->messages[2]);

        if (m->isAborted())
        {
          m->abort(false);
          break;
        }
      }
    }
    catch (const Botan::Stream_IO_Error&)
    {
      emit errorMessage(inputFileName, m->messages[7]);
    }
    catch (const std::exception& e)
    {
      const auto error = QString{e.what()};
      emit errorMessage(inputFileName, error);
    }
  } // End file loop

  m->resetFlags();

  m->busy(false);
  emit busyStatus(m->isBusy());
}

void Crypto::decrypt(const QString& passphrase,
                     const QStringList& inputFileNames)
{
  m->busy(true);
  emit busyStatus(m->isBusy());

  for (const auto& inputFileName : inputFileNames)
  {
    try
    {
      decryptFile(passphrase, inputFileName);

      if (m->isAborted())
      { // Reset abort flag
        m->abort(false);
        emit errorMessage(inputFileName, m->messages[3]);
        break;
      }

      if (m->isStopped(inputFileName))
      {
        emit errorMessage(inputFileName, m->messages[3]);
      }
    }
    catch (const Botan::Decoding_Error&)
    {
      emit errorMessage(inputFileName, m->messages[5]);
    }
    catch (const Botan::Integrity_Failure&)
    {
      emit errorMessage(inputFileName, m->messages[5]);
    }
    catch (const Botan::Invalid_Argument&)
    {
      emit errorMessage(inputFileName, m->messages[6]);
    }
    catch (const std::invalid_argument&)
    {
      emit errorMessage(inputFileName, m->messages[6]);
    }
    catch (const std::runtime_error&)
    {
      emit errorMessage(inputFileName, m->messages[4]);
    }
    catch (const std::exception& e)
    {
      const auto error = QString{e.what()};

      if ("zlib inflate error -3" == error)
      {
        emit errorMessage(inputFileName, m->messages[5]);
      }
      else
      {
        emit errorMessage(inputFileName, error);
      }
    }
  } // End file loop

  m->resetFlags();

  m->busy(false);
  emit busyStatus(m->isBusy());
}

void Crypto::abort()
{
  if (m->isBusy())
  {
    m->abort(true);
  }
}

void Crypto::pause(bool pause)
{
  m->pause(pause);
}

void Crypto::stop(const QString& fileName)
{
  if (m->isBusy())
  {
    m->stop(fileName, true);
  }
}

void Crypto::encryptFile(const QString& passphrase,
                         const QString& inputFileName,
                         const QString& algorithmName,
                         const std::size_t& keySize,
                         const bool compress)
{
  QFileInfo fileInfo{inputFileName};

  if (!m->isAborted() && fileInfo.exists() &&
      fileInfo.isFile() && fileInfo.isReadable())
  {
    Botan::AutoSeeded_RNG rng{};

    // Define a size for the PBKDF salt vector
    const auto pbkdfSaltSize = static_cast<std::size_t>(256);
    Botan::secure_vector<Botan::byte> pbkdfSalt;
    pbkdfSalt.resize(pbkdfSaltSize);

    // Create random PBKDF salt
    rng.randomize(&pbkdfSalt[0], pbkdfSalt.size());

    // Set up the key derive functions
    const auto macSize = static_cast<std::size_t>(512);

    // PKCS5_PBKDF2 takes ownership of the new HMAC and the HMAC takes ownership
    // of the Keccak_1600 hash function object (both via unique_ptr)
    Botan::PKCS5_PBKDF2 pbkdf{new Botan::HMAC{new Botan::Keccak_1600{macSize}}};
    const auto PBKDF2_ITERATIONS = 15000;

    // Create the PBKDF key
    const auto pbkdfKeySize = static_cast<std::size_t>(256);
    Botan::secure_vector<Botan::byte> pbkdfKey =
      pbkdf.derive_key(pbkdfKeySize, passphrase.toStdString(), &pbkdfSalt[0],
                       pbkdfSalt.size(), PBKDF2_ITERATIONS).bits_of();

    // Create the key and IV
    const auto kdfHash = std::string{"KDF2(Keccak-1600)"};
    std::unique_ptr<Botan::KDF> kdf{Botan::KDF::create(kdfHash)};

    // Set up key salt size
    const auto keySaltSize = static_cast<std::size_t>(64);
    Botan::secure_vector<Botan::byte> keySalt;
    keySalt.resize(keySaltSize);
    rng.randomize(&keySalt[0], keySalt.size());

    // Key is constrained to sizes allowed by algorithm
    const auto keySizeInBytes = keySize / 8;
    const std::string keyLabel = "user secret";
    const auto keyLabelVector =
      reinterpret_cast<const Botan::byte*>(keyLabel.data());
    Botan::SymmetricKey key{kdf->derive_key(keySizeInBytes,
                                            pbkdfKey.data(),
                                            pbkdfKey.size(),
                                            keySalt.data(),
                                            keySalt.size(),
                                            keyLabelVector,
                                            keyLabel.size())};

    // Set up IV salt size
    const auto ivSaltSize = static_cast<std::size_t>(64);
    Botan::secure_vector<Botan::byte> ivSalt;
    ivSalt.resize(ivSaltSize);
    rng.randomize(&ivSalt[0], ivSalt.size());

    const auto ivSize = static_cast<std::size_t>(256);
    const std::string ivLabel = "initialization vector";
    const auto ivLabelVector =
      reinterpret_cast<const Botan::byte*>(ivLabel.data());
    Botan::InitializationVector iv{kdf->derive_key(ivSize,
                                                   pbkdfKey.data(),
                                                   pbkdfKey.size(),
                                                   ivSalt.data(),
                                                   ivSalt.size(),
                                                   ivLabelVector,
                                                   ivLabel.size())};

    std::ifstream in{inputFileName.toStdString(), std::ios::binary};

    const QString outputFileName = inputFileName % QStringLiteral(".enc");
    std::ofstream out{outputFileName.toStdString(), std::ios::binary};

    const auto algorithmNameStd = algorithmName.toStdString();

    auto headerText = tr("-------- ENCRYPTED FILE --------");

    auto compressedText = tr("Not compressed");

    if (compress)
    {
      compressedText = tr("Compressed");
    }

    out << headerText.toStdString() << std::endl;
    out << algorithmNameStd << std::endl;
    out << keySize << std::endl;
    out << compressedText.toStdString() << std::endl;
    out << Botan::base64_encode(&pbkdfSalt[0], pbkdfSalt.size()) << std::endl;
    out << Botan::base64_encode(&keySalt[0], keySalt.size()) << std::endl;
    out << Botan::base64_encode(&ivSalt[0], ivSalt.size()) << std::endl;

    Botan::Pipe pipe{};

    if (compress)
    {
      pipe.append(new Botan::Compression_Filter{std::string{"zlib"},
                  static_cast<std::size_t>(9)});
    }

    pipe.append(Botan::get_cipher(algorithmNameStd,
                                  key,
                                  iv,
                                  Botan::ENCRYPTION));

    pipe.append(new Botan::DataSink_Stream{out});

    executeCipher(inputFileName, pipe, in, out);

    if (!m->isAborted() && !m->isStopped(inputFileName))
    {
      // Progress: finished
      emit progress(inputFileName, 100);

      // Encryption success message
      emit statusMessage(m->messages[0].arg(inputFileName));
    }
  }
}

void Crypto::decryptFile(const QString& passphrase,
                         const QString& inputFileName)
{
  const QFileInfo fileInfo{inputFileName};

  if (!m->isAborted() && fileInfo.exists() &&
      fileInfo.isFile() && fileInfo.isReadable())
  {
    std::ifstream in{inputFileName.toStdString(), std::ios::binary};

    // Read the salts from file
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

    auto headerString = QString{headerStringStd.c_str()};
    auto compressString = QString{compressStringStd.c_str()};

    if (headerString != tr("-------- ENCRYPTED FILE --------"))
    {
      emit statusMessage(m->messages[6].arg(inputFileName));
    }

    // Set up the key derive functions
    const auto macSize = static_cast<std::size_t>(512);

    // PKCS5_PBKDF2 takes ownership of the new HMAC and the HMAC takes ownership
    // of the Keccak_1600 hash function object (via unique_ptr)
    Botan::PKCS5_PBKDF2 pbkdf{new Botan::HMAC{new Botan::Keccak_1600{macSize}}};
    const auto PBKDF2_ITERATIONS = static_cast<std::size_t>(15000);

    // Create the PBKDF key
    Botan::secure_vector<Botan::byte> pbkdfSalt =
      Botan::base64_decode(pbkdfSaltString);
    const auto pbkdfKeySize = static_cast<std::size_t>(256);
    Botan::secure_vector<Botan::byte> pbkdfKey =
      pbkdf.derive_key(pbkdfKeySize, passphrase.toStdString(), &pbkdfSalt[0],
                       pbkdfSalt.size(), PBKDF2_ITERATIONS).bits_of();

    // Create the key and IV
    const auto kdfHash = std::string{"KDF2(Keccak-1600)"};
    std::unique_ptr<Botan::KDF> kdf{Botan::KDF::create(kdfHash)};

    // Key salt
    Botan::secure_vector<Botan::byte> keySalt =
        Botan::base64_decode(keySaltString);
    const auto keySize =
        static_cast<std::size_t> (std::stoll(keySizeString.c_str()));
    const auto keySizeInBytes = keySize / 8;
    const std::string keyLabel = "user secret";
    const auto keyLabelVector =
        reinterpret_cast<const Botan::byte*>(keyLabel.data());
    Botan::SymmetricKey key{kdf->derive_key(keySizeInBytes,
                                            pbkdfKey.data(),
                                            pbkdfKey.size(),
                                            keySalt.data(),
                                            keySalt.size(),
                                            keyLabelVector,
                                            keyLabel.size())};

    Botan::secure_vector<Botan::byte> ivSalt =
      Botan::base64_decode(ivSaltString);
    const auto ivSize = static_cast<std::size_t>(256);
    const std::string ivLabel = "initialization vector";
    const auto ivLabelVector =
        reinterpret_cast<const Botan::byte*>(ivLabel.data());
    Botan::InitializationVector iv{kdf->derive_key(ivSize,
                                                   pbkdfKey.data(),
                                                   pbkdfKey.size(),
                                                   ivSalt.data(),
                                                   ivSalt.size(),
                                                   ivLabelVector,
                                                   ivLabel.size())};

    // Remove the .enc extension if it's in the file name
    const auto outputFileName = removeExtension(inputFileName,
                                                QStringLiteral("enc"));

    // Create a unique file name for the file in this directory
    auto uniqueOutputFileName = uniqueFileName(outputFileName);

    std::ofstream out{uniqueOutputFileName.toStdString(), std::ios::binary};

    Botan::Pipe pipe{};

    pipe.append(Botan::get_cipher(algorithmNameStd,
                                  key,
                                  iv,
                                  Botan::DECRYPTION));

    if (tr("Compressed") == compressString)
    {
      pipe.append(new Botan::Decompression_Filter{std::string{"zlib"},
                  static_cast<std::size_t>(9)});
    }

    pipe.append(new Botan::DataSink_Stream{out});

    executeCipher(inputFileName, pipe, in, out);

    if (!m->isAborted() && !m->isStopped(inputFileName))
    {
      // Progress: finished
      emit progress(inputFileName, 100);

      // Decryption success message
      emit statusMessage(m->messages[1].arg(inputFileName));
    }
  }
}

void Crypto::executeCipher(const QString& inputFileName,
                           Botan::Pipe& pipe,
                           std::ifstream& in,
                           std::ofstream& out)
{
  // Define a size for the buffer vector
  const auto bufferSize = static_cast<std::size_t>(4096);
  Botan::secure_vector<Botan::byte> buffer;
  buffer.resize(bufferSize);

  // Get file size for percent progress calculation
  QFileInfo file{inputFileName};
  const qint64 size = file.size();
  std::size_t fileIndex = 0;
  qint64 percent = -1;

  pipe.start_msg();

  while (in.good() && !m->isAborted() && !m->isStopped(inputFileName))
  {
    if (!m->isPaused())
    {
      in.read(reinterpret_cast<char*>(&buffer[0]), buffer.size());
      const auto remainingSize = in.gcount();
      pipe.write(&buffer[0], remainingSize);

      // Calculate progress in percent
      fileIndex += remainingSize;
      const auto nextFraction = static_cast<double>(fileIndex) /
                                static_cast<double>(size);
      const qint64 nextPercent = static_cast<qint64>(nextFraction * 100);

      if (nextPercent > percent && nextPercent < 100)
      {
        percent = nextPercent;
        emit progress(inputFileName, percent);
      }

      if (in.eof())
      {
        pipe.end_msg();
      }

      while (pipe.remaining() > 0)
      {
        const auto buffered = pipe.read(&buffer[0], buffer.size());
        out.write(reinterpret_cast<const char*>(&buffer[0]), buffered);
      }
    }
  }

  if (in.bad() || (in.fail() && !in.eof()))
  {
    emit errorMessage(inputFileName, m->messages[4]);
  }

  out.flush();
}

Crypto::CryptoPrivate::CryptoPrivate()
  : messages{tr("File %1 encrypted."),
             tr("File %1 decrypted."),
             tr("Encryption stopped. File %1 is incomplete."),
             tr("Decryption stopped. File %1 is incomplete."),
             tr("Error: Can't read file %1."),
             tr("Error: Can't decrypt file %1. Wrong password entered or the "
                "file has been corrupted."),
             tr("Error: Can't decrypt file %1. Is it an encrypted file?"),
             tr("Error: Can't encrypt file %1. Check that this file exists and "
                "that you have permission to access it and try again.")},
    aborted{false}, paused{false}, busyStatus{false}
{
  // Reserve a small number of elements to improve dictionary performance
  stopped.reserve(100);
}

void Crypto::CryptoPrivate::abort(const bool abort)
{
  aborted = abort;
}

bool Crypto::CryptoPrivate::isAborted() const
{
  return aborted;
}

void Crypto::CryptoPrivate::pause(const bool pause)
{
  paused = pause;
}

bool Crypto::CryptoPrivate::isPaused() const
{
  return paused;
}

void Crypto::CryptoPrivate::stop(const QString& fileName, const bool stop)
{
  stopped.insert(fileName, stop);
}

bool Crypto::CryptoPrivate::isStopped(const QString& fileName) const
{
  return stopped.value(fileName, false);
}

void Crypto::CryptoPrivate::resetFlags()
{
  aborted = false;
  stopped.clear();
}

void Crypto::CryptoPrivate::busy(const bool busy)
{
  busyStatus = busy;
}

bool Crypto::CryptoPrivate::isBusy() const
{
  return busyStatus;
}
