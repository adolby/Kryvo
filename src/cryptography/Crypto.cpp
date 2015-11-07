#include "cryptography/Crypto.hpp"
#include "utility/pimpl_impl.h"
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

  Settings* settings;

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

Crypto::Crypto(Settings* settings, QObject* parent)
  : QObject{parent}
{
  m->settings = settings;

  // Initialize Botan
  Botan::LibraryInitializer init{std::string{"thread_safe=true"}};
}

Crypto::~Crypto() {}

void Crypto::encrypt(const QString& passphrase,
                     const QStringList& inputFileNames,
                     const QString& algorithm,
                     const std::size_t& inputKeySize)
{
  m->busy(true);
  emit busyStatus(m->isBusy());

  m->resetFlags();

  QString algorithmName;
  auto keySize = std::size_t{};
  if (!algorithm.isEmpty())
  {
    algorithmName = algorithm;
    keySize = inputKeySize;
  }
  else
  {
    algorithmName = QStringLiteral("AES-128/GCM");
    keySize = static_cast<std::size_t>(128);
  }

  for (const auto& inputFileName : inputFileNames)
  {
    try
    {
      this->encryptFile(passphrase, inputFileName, algorithmName, keySize);

      if (m->isAborted())
      { // Reset abort flag
        m->abort(false);
        emit errorMessage(inputFileName, m->messages[2].arg(inputFileName));
        break;
      }

      if (m->isStopped(inputFileName))
      {
        emit errorMessage(inputFileName, m->messages[2].arg(inputFileName));
      }
    }
    catch (const Botan::Stream_IO_Error&)
    {
      emit errorMessage(inputFileName, m->messages[8].arg(inputFileName));
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

  m->resetFlags();

  for (const auto& inputFileName : inputFileNames)
  {
    try
    {
      this->decryptFile(passphrase, inputFileName);

      if (m->isAborted())
      {
        emit errorMessage(inputFileName, m->messages[3].arg(inputFileName));
        break;
      }

      if (m->isStopped(inputFileName))
      {
        emit errorMessage(inputFileName, m->messages[3].arg(inputFileName));
      }
    }
    catch (const Botan::Decoding_Error&)
    {
      emit errorMessage(inputFileName, m->messages[5].arg(inputFileName));
    }
    catch (const Botan::Integrity_Failure&)
    {
      emit errorMessage(inputFileName, m->messages[5].arg(inputFileName));
    }
    catch (const std::invalid_argument&)
    {
      emit errorMessage(inputFileName, m->messages[7].arg(inputFileName));
    }
    catch (const std::runtime_error&)
    {
      emit errorMessage(inputFileName, m->messages[4].arg(inputFileName));
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
                         const std::size_t& keySize)
{
  QFileInfo fileInfo{inputFileName};

  if (!m->isAborted() && fileInfo.exists() &&
      fileInfo.isFile() && fileInfo.isReadable())
  {
    Botan::AutoSeeded_RNG range{};

    // Define a size for the PBKDF salt vector
    const auto pbkdfSaltSize = static_cast<std::size_t>(256);
    Botan::secure_vector<Botan::byte> pbkdfSalt;
    pbkdfSalt.resize(pbkdfSaltSize);

    // Create random PBKDF salt
    range.randomize(&pbkdfSalt[0], pbkdfSalt.size());

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
    std::unique_ptr<Botan::KDF> kdf{Botan::get_kdf(kdfHash)};

    // Set up key salt size
    const auto keySaltSize = static_cast<std::size_t>(64);
    Botan::secure_vector<Botan::byte> keySalt;
    keySalt.resize(keySaltSize);
    range.randomize(&keySalt[0], keySalt.size());

    // Key is constrained to sizes allowed by algorithm
    const auto keySizeInBytes = keySize / 8;
    Botan::SymmetricKey key{kdf->derive_key(keySizeInBytes,
                                            pbkdfKey,
                                            keySalt)};

    // Set up IV salt size
    const auto ivSaltSize = static_cast<std::size_t>(64);
    Botan::secure_vector<Botan::byte> ivSalt;
    ivSalt.resize(ivSaltSize);
    range.randomize(&ivSalt[0], ivSalt.size());

    const auto ivSize = static_cast<std::size_t>(256);
    Botan::InitializationVector iv{kdf->derive_key(ivSize, pbkdfKey, ivSalt)};

    std::ifstream in{inputFileName.toStdString(), std::ios::binary};

    const QString outputFileName = inputFileName % QStringLiteral(".enc");
    std::ofstream out{outputFileName.toStdString(), std::ios::binary};

    const auto algorithmNameStd = algorithmName.toStdString();

    out << std::string{"-------- ENCRYPTED FILE --------"} << std::endl;
    out << algorithmNameStd << std::endl;
    out << keySize << std::endl;
    out << Botan::base64_encode(&pbkdfSalt[0], pbkdfSalt.size()) << std::endl;
    out << Botan::base64_encode(&keySalt[0], keySalt.size()) << std::endl;
    out << Botan::base64_encode(&ivSalt[0], ivSalt.size()) << std::endl;

    this->executeCipher(inputFileName, algorithmNameStd, key, iv,
                        Botan::ENCRYPTION, in, out);

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
    std::string headerString, algorithmNameStd, keySizeString, pbkdfSaltString,
        keySaltString, ivSaltString;

    std::getline(in, headerString);
    std::getline(in, algorithmNameStd);
    std::getline(in, keySizeString);
    std::getline(in, pbkdfSaltString);
    std::getline(in, keySaltString);
    std::getline(in, ivSaltString);

    if (headerString != std::string{"-------- ENCRYPTED FILE --------"})
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
    std::unique_ptr<Botan::KDF> kdf{Botan::get_kdf(kdfHash)};

    // Key salt
    Botan::secure_vector<Botan::byte> keySalt =
        Botan::base64_decode(keySaltString);

    const auto keySize = static_cast<std::size_t>
                         (std::stoll(keySizeString.c_str()));
    const auto keySizeInBytes = keySize / 8;
    Botan::SymmetricKey key{kdf->derive_key(keySizeInBytes, pbkdfKey, keySalt)};

    Botan::secure_vector<Botan::byte> ivSalt =
        Botan::base64_decode(ivSaltString);
    const auto ivSize = static_cast<std::size_t>(256);
    Botan::InitializationVector iv{kdf->derive_key(ivSize, pbkdfKey, ivSalt)};

    // Remove the .enc extension if it's in the file name
    const auto outputFileName = removeExtension(inputFileName,
                                                QStringLiteral("enc"));

    // Create a unique file name for the file in this directory
    auto uniqueOutputFileName = uniqueFileName(outputFileName);

    std::ofstream out{uniqueOutputFileName.toStdString(), std::ios::binary};

    this->executeCipher(inputFileName, algorithmNameStd, key, iv,
                        Botan::DECRYPTION, in, out);

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
                           const std::string& algorithmName,
                           const Botan::SymmetricKey& key,
                           const Botan::InitializationVector& iv,
                           const Botan::Cipher_Dir& cipherDirection,
                           std::ifstream& in,
                           std::ofstream& out)
{
  Botan::Pipe pipe{Botan::get_cipher(algorithmName, key, iv, cipherDirection),
                   new Botan::DataSink_Stream{out}};

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
    emit errorMessage(inputFileName, m->messages[4].arg(inputFileName));
  }

  out.flush();
}

Crypto::CryptoPrivate::CryptoPrivate()
  : settings{nullptr},
    messages{tr("File %1 encrypted."),
             tr("File %1 decrypted."),
             tr("Encryption stopped. File %1 is incomplete."),
             tr("Decryption stopped. File %1 is incomplete."),
             tr("Error: File %1 couldn't be read."),
             tr("Error: Decryption failed. Wrong password entered or file %1 "
                "has changed since it was created. This could mean someone "
                "has tampered with the file. It also could mean the file has "
                "become corrupted. You'll need to encrypt the source file "
                "again."),
             tr("Error: File %1's header is not recognized."),
             tr("Error: Decryption failed. File %1's header couldn't be "
                "read."),
             tr("Error: Encryption failed. Can't encrypt file %1. Check that "
                "this file exists and that you have permission to access it "
                "and try again.")},
    aborted{false}, paused{false}, busyStatus{false}
{
  // Reserve elements to improve dictionary performance
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
