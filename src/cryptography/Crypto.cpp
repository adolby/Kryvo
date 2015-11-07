/**
 * Kryvos File Encryptor - Encrypts and decrypts files.
 * Copyright (C) 2014 Andrew Dolby
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Contact : andrewdolby@gmail.com
 */

#include "cryptography/Crypto.hpp"
#include "utility/make_unique.h"
#include <QtCore/QFileInfo>
#include <QtCore/QDir>
#include <QtCore/QHash>
#include <QtCore/QStringRef>
#include <QtCore/QStringBuilder>

class Crypto::CryptoPrivate {
 public:
  /*!
   * \brief CryptoPrivate Constructs the Crypto private implementation.
   */
  explicit CryptoPrivate();

  /*!
   * \brief removeExtension Attempts to return the file name string input
   * without the last extension. It's used to extract an extension to determine
   * a decrypted file name.
   * \param fileName String representing the file name.
   * \param extension String representing the extension to remove from the
   * file name.
   * \return String representing a file name without an extension.
   */
  QString removeExtension(const QString& fileName,
                          const QString& extension);

  /*!
   * \brief uniqueFileName Returns a unique file name from the input file name
   * by appending numbers, if necessary.
   * \param fileName String representing the file name that will be tested
   * for uniqueness.
   * \return String representing a unique file name created from the input file
   * name.
   */
  QString uniqueFileName(const QString& fileName);

  /*!
   * \brief resetFlags Resets the status flags, except pause, to default values.
   */
  void resetFlags();

  /*!
   * \brief abort Updates the abort status.
   * \param abort Boolean representing the abort status.
   */
  void abort(bool abort);

  /*!
   * \brief isAborted Returns the current abort status.
   * \return Boolean representing the abort status.
   */
  bool isAborted() const;

  /*!
   * \brief pause Updates the pause status.
   * \param pause Boolean representing the pause status.
   */
  void pause(bool pause);

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
  void stop(const QString& fileName, bool stop);

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
  void busy(bool busy);

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
  bool aborted;

  // The pause status, when set to false, will pause an executing cipher
  // operation. When the pause status is set to true, the cipher operation
  // that was in progress when the pause status was set will resume execution.
  bool paused;

  // The container of stopped flags, which are used to stop
  // encrypting/decrypting a file.
  QHash<QString, bool> stopped;

  // The busy status, when set to true, indicates that this class is currently
  // executing a cipher operation.
  bool busyStatus;
};

Crypto::Crypto(QObject* parent) :
  QObject{parent}, pimpl{make_unique<CryptoPrivate>()}
{
  // Initialize Botan
  Botan::LibraryInitializer init{"thread_safe=true"};
}

Crypto::~Crypto() {}

void Crypto::encrypt(const QString& passphrase,
                     const QStringList& inputFileNames,
                     const QString& algorithm)
{
  Q_ASSERT(pimpl);

  pimpl->busy(true);
  emit busyStatus(pimpl->isBusy());

  // Reset status flags
  pimpl->resetFlags();

  auto algorithmName = QString{"AES-128/GCM"};
  if (!algorithm.isEmpty())
  {
    algorithmName = algorithm;
  }

  const auto inputFileNamesSize = inputFileNames.size();
  for (auto i = 0; i < inputFileNamesSize; ++i)
  {
    const auto inputFileName = inputFileNames[i];

    try
    {
      this->encryptFile(passphrase, inputFileName, algorithmName);

      if (pimpl->isAborted())
      { // Reset abort flag
        pimpl->abort(false);
        emit errorMessage(inputFileName, pimpl->messages[2].arg(inputFileName));
        break;
      }

      if (pimpl->isStopped(inputFileName))
      {
        emit errorMessage(inputFileName, pimpl->messages[2].arg(inputFileName));
      }
    }
    catch (const Botan::Stream_IO_Error&)
    {
      emit errorMessage(inputFileName, pimpl->messages[8].arg(inputFileName));
    }
    catch (const std::exception& e)
    {
      const auto error = QString{e.what()};
      emit errorMessage(inputFileName, error);
    }
  } // End file loop

  // Reset status flags
  pimpl->resetFlags();

  pimpl->busy(false);
  emit busyStatus(pimpl->isBusy());
}

void Crypto::decrypt(const QString& passphrase,
                     const QStringList& inputFileNames)
{
  Q_ASSERT(pimpl);

  pimpl->busy(true);
  emit busyStatus(pimpl->isBusy());

  // Reset status flags
  pimpl->resetFlags();

  const auto inputFileNamesSize = inputFileNames.size();
  for (auto i = 0; i < inputFileNamesSize; ++i)
  {
    const auto inputFileName = inputFileNames[i];

    try
    {
      this->decryptFile(passphrase, inputFileName);

      if (pimpl->isAborted())
      {
        emit errorMessage(inputFileName, pimpl->messages[3].arg(inputFileName));
        break;
      }

      if (pimpl->isStopped(inputFileName))
      {
        emit errorMessage(inputFileName, pimpl->messages[3].arg(inputFileName));
      }
    }
    catch (const Botan::Decoding_Error&)
    {
      emit errorMessage(inputFileName, pimpl->messages[5].arg(inputFileName));
    }
    catch (const Botan::Integrity_Failure&)
    {
      emit errorMessage(inputFileName, pimpl->messages[5].arg(inputFileName));
    }
    catch (const std::invalid_argument&)
    {
      emit errorMessage(inputFileName, pimpl->messages[7].arg(inputFileName));
    }
    catch (const std::runtime_error&)
    {
      emit errorMessage(inputFileName, pimpl->messages[4].arg(inputFileName));
    }
    catch (const std::exception& e)
    {
      const auto error = QString{e.what()};
      emit errorMessage(inputFileName, error);
    }
  } // End file loop

  // Reset status flags
  pimpl->resetFlags();

  pimpl->busy(false);
  emit busyStatus(pimpl->isBusy());
}

void Crypto::abort()
{
  Q_ASSERT(pimpl);

  if (pimpl->isBusy())
  {
    pimpl->abort(true);
  }
}

void Crypto::pause(bool pause)
{
  Q_ASSERT(pimpl);

  pimpl->pause(pause);
}

void Crypto::stop(const QString& fileName)
{
  Q_ASSERT(pimpl);

  if (pimpl->isBusy())
  {
    pimpl->stop(fileName, true);
  }
}

void Crypto::encryptFile(const QString& passphrase,
                         const QString& inputFileName,
                         const QString& algorithmName)
{
  QFileInfo file{inputFileName};

  if (!pimpl->isAborted() && file.exists() &&
      file.isFile() && file.isReadable())
  {
    Botan::AutoSeeded_RNG range{};

    // Define a size for the master salt vector
    const auto masterSaltSize = static_cast<std::size_t>(256);
    Botan::secure_vector<Botan::byte> masterSalt;
    masterSalt.resize(masterSaltSize);

    // Create random master salt
    range.randomize(&masterSalt[0], masterSalt.size());

    // Setup the key derive functions
    const auto macSize = static_cast<std::size_t>(512);

    // PKCS5_PBKDF2 takes ownership of the new HMAC and the HMAC takes ownership
    // of the Keccak_1600 hash function object (via unique_ptr)
    Botan::PKCS5_PBKDF2 pbkdf{new Botan::HMAC{new Botan::Keccak_1600{macSize}}};
    const auto PBKDF2_ITERATIONS = 15000;

    // Create the master key
    const auto masterKeySize = static_cast<std::size_t>(256);
    Botan::secure_vector<Botan::byte> masterKey =
      pbkdf.derive_key(masterKeySize, passphrase.toStdString(), &masterSalt[0],
        masterSalt.size(), PBKDF2_ITERATIONS).bits_of();

    // Create the key and IV
    auto kdfHash = std::string{"KDF2(Keccak-1600)"};
    std::unique_ptr<Botan::KDF> kdf{Botan::get_kdf(kdfHash)};

    // Set up key salt size
    const auto keySaltSize = static_cast<std::size_t>(64);
    Botan::secure_vector<Botan::byte> keySalt;
    keySalt.resize(keySaltSize);
    range.randomize(&keySalt[0], keySalt.size());

    auto keySize = static_cast<std::size_t>(0);
    if (algorithmName.contains("128"))
    {
      keySize = static_cast<std::size_t>(16);
    }
    else if (algorithmName.contains("256"))
    {
      keySize = static_cast<std::size_t>(32);
    }

    Botan::SymmetricKey key{kdf->derive_key(keySize, masterKey, keySalt)};

    // Set up IV salt size
    const auto ivSaltSize = static_cast<std::size_t>(64);
    Botan::secure_vector<Botan::byte> ivSalt;
    ivSalt.resize(ivSaltSize);
    range.randomize(&ivSalt[0], ivSalt.size());

    const auto ivSize = static_cast<std::size_t>(256);
    Botan::InitializationVector iv{kdf->derive_key(ivSize, masterKey, ivSalt)};

    std::ifstream in{inputFileName.toStdString(), std::ios::binary};

    const auto outputFileName = QString{inputFileName % ".enc"};
    std::ofstream out{outputFileName.toStdString(), std::ios::binary};

    const auto algorithmNameStd = algorithmName.toStdString();

    out << "-------- ENCRYPTED FILE --------" << std::endl;
    out << algorithmNameStd << std::endl;
    out << Botan::base64_encode(&masterSalt[0], masterSalt.size()) << std::endl;
    out << Botan::base64_encode(&keySalt[0], keySalt.size()) << std::endl;
    out << Botan::base64_encode(&ivSalt[0], ivSalt.size()) << std::endl;

    this->executeCipher(inputFileName, algorithmNameStd, key, iv,
                        Botan::ENCRYPTION, in, out);

    if (!pimpl->isAborted() && !pimpl->isStopped(inputFileName))
    {
      // Progress: finished
      emit progress(inputFileName, 100);

      // Encryption success message
      emit statusMessage(pimpl->messages[0].arg(inputFileName));
    }
  }
}

void Crypto::decryptFile(const QString& passphrase,
                         const QString& inputFileName)
{
  QFileInfo file{inputFileName};

  if (!pimpl->isAborted() && file.exists() &&
      file.isFile() && file.isReadable())
  {
    std::ifstream in{inputFileName.toStdString(), std::ios::binary};

    // Read the salts from file
    std::string headerString, algorithmNameStd, masterSaltString,
        keySaltString, ivSaltString;

    std::getline(in, headerString);
    std::getline(in, algorithmNameStd);
    std::getline(in, masterSaltString);
    std::getline(in, keySaltString);
    std::getline(in, ivSaltString);

    if (headerString != "-------- ENCRYPTED FILE --------")
    {
      emit statusMessage(pimpl->messages[6].arg(inputFileName));
    }

    // Set up the key derive functions
    const auto macSize = static_cast<std::size_t>(512);

    // PKCS5_PBKDF2 takes ownership of the new HMAC and the HMAC takes ownership
    // of the Keccak_1600 hash function object (via unique_ptr)
    Botan::PKCS5_PBKDF2 pbkdf{new Botan::HMAC{new Botan::Keccak_1600{macSize}}};
    const auto PBKDF2_ITERATIONS = static_cast<std::size_t>(15000);

    // Create the master key
    Botan::secure_vector<Botan::byte> masterSalt =
        Botan::base64_decode(masterSaltString);
    const auto masterKeySize = static_cast<std::size_t>(256);
    Botan::secure_vector<Botan::byte> masterKey =
      pbkdf.derive_key(masterKeySize, passphrase.toStdString(), &masterSalt[0],
        masterSalt.size(), PBKDF2_ITERATIONS).bits_of();

    // Create the key and IV
    auto kdfHash = static_cast<std::string>("KDF2(Keccak-1600)");
    std::unique_ptr<Botan::KDF> kdf{Botan::get_kdf(kdfHash)};

    Botan::secure_vector<Botan::byte> keySalt =
        Botan::base64_decode(keySaltString);

    const auto keySize = static_cast<std::size_t>(16);
    Botan::SymmetricKey key{kdf->derive_key(keySize, masterKey, keySalt)};

    Botan::secure_vector<Botan::byte> ivSalt =
        Botan::base64_decode(ivSaltString);
    const auto ivSize = static_cast<std::size_t>(256);
    Botan::InitializationVector iv{kdf->derive_key(ivSize, masterKey, ivSalt)};

    // Remove the .enc extension if it's in the file name
    const auto outputFileName = pimpl->removeExtension(inputFileName, "enc");

    // Create a unique file name for the file in this directory
    auto uniqueOutputFileName = pimpl->uniqueFileName(outputFileName);

    std::ofstream out{uniqueOutputFileName.toStdString(), std::ios::binary};

    this->executeCipher(inputFileName, algorithmNameStd, key, iv,
                        Botan::DECRYPTION, in, out);

    if (!pimpl->isAborted() && !pimpl->isStopped(inputFileName))
    {
      // Progress: finished
      emit progress(inputFileName, 100);

      // Decryption success message
      emit statusMessage(pimpl->messages[1].arg(inputFileName));
    }
  }
}

void Crypto::executeCipher(const QString& inputFileName,
                           const std::string& algorithmName,
                           const Botan::SymmetricKey& key,
                           const Botan::InitializationVector& iv,
                           const Botan::Cipher_Dir cipherDirection,
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

  while (in.good() && !pimpl->isAborted() && !pimpl->isStopped(inputFileName))
  {
    if (!pimpl->isPaused())
    {
      in.read((char*)&buffer[0], buffer.size());
      const auto remainingSize = in.gcount();
      pipe.write(&buffer[0], remainingSize);

      // Calculate progress in percent
      fileIndex += remainingSize;
      const qint64 nextPercent = (fileIndex * 100) / size;

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
        out.write((const char*)&buffer[0], buffered);
      }
    }
  }

  if (in.bad() || (in.fail() && !in.eof()))
  {
    emit errorMessage(inputFileName, pimpl->messages[4].arg(inputFileName));
  }

  out.flush();
}

Crypto::CryptoPrivate::CryptoPrivate() :
  messages{tr("File %1 encrypted."),
           tr("File %1 decrypted."),
           tr("Encryption stopped. File %1 is incomplete."),
           tr("Decryption stopped. File %1 is incomplete."),
           tr("Error: File %1 couldn't be read."),
           tr("Error: Decryption failed. Wrong password entered or file %1 has"
              " changed since it was created. This could mean someone has"
              " tampered with the file. It also could mean the file has become"
              " corrupted. You'll need to encrypt the source file again."),
           tr("Error: File %1's header is not recognized."),
           tr("Error: Decryption failed. File %1's header couldn't be read."),
           tr("Error: Encryption failed. Can't encrypt file %1. Check that this"
              " file exists and that you have permission to"
              " access it and try again.")},
  aborted{false}, paused{false}, busyStatus{false}
{
  // Reserve elements to improve dictionary performance
  stopped.reserve(100);
}

QString Crypto::CryptoPrivate::removeExtension(const QString& fileName,
                                               const QString& extension)
{
  QFileInfo file{fileName};
  auto newFileName = fileName;

  if (file.suffix() == extension)
  {
    newFileName = file.absolutePath() % QDir::separator() %
                  file.completeBaseName();
  }

  return newFileName;
}

QString Crypto::CryptoPrivate::uniqueFileName(const QString& fileName)
{
  QFileInfo originalFile{fileName};
  auto uniqueFileName = fileName;

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
        uniqueFileName += "." % originalFile.completeSuffix();
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

void Crypto::CryptoPrivate::abort(bool abort)
{
  this->aborted = abort;
}

bool Crypto::CryptoPrivate::isAborted() const
{
  return this->aborted;
}

void Crypto::CryptoPrivate::pause(bool pause)
{
  this->paused = pause;
}

bool Crypto::CryptoPrivate::isPaused() const
{
  return this->paused;
}

void Crypto::CryptoPrivate::stop(const QString& fileName, bool stop)
{
  this->stopped.insert(fileName, stop);
}

bool Crypto::CryptoPrivate::isStopped(const QString& fileName) const
{
  return this->stopped.value(fileName, false);
}

void Crypto::CryptoPrivate::resetFlags()
{
  aborted = false;
  this->stopped.clear();
}

void Crypto::CryptoPrivate::busy(bool busy)
{
  this->busyStatus = busy;
}

bool Crypto::CryptoPrivate::isBusy() const
{
  return this->busyStatus;
}
