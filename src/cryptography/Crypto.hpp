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

#ifndef KRYVOS_CRYPTOGRAPHY_CRYPTO_HPP_
#define KRYVOS_CRYPTOGRAPHY_CRYPTO_HPP_

#include "botan/botan.h"
#include <QtCore/QObject>
#include <QtCore/QFileInfo>
#include <QtCore/QStringList>
#include <QtCore/QString>
#include <fstream>
#include <string>
#include <memory>

/*!
 * \brief Crypto class
 */
class Crypto : public QObject
{
  Q_OBJECT

 public:
  /*!
   * \brief Crypto Constructs the Crypto class. Initializes Botan.
   * \param parent
   */
  explicit Crypto(QObject* parent = nullptr);

  /*!
   * \brief ~Crypto Destroys the Crypto class.
   */
  virtual ~Crypto();

 signals:
  /*!
   * \brief progress Emitted when the cipher operation progress changes.
   * \param index Integer representing the index of the current file being
   * encrypted or decrypted.
   * \param percent Integer representing the current percent.
   */
  void progress(const QString& path, qint64 percent);

  /*!
   * \brief statusMessage Emitted when a message about the current cipher
   * operation should be displayed to the user.
   * \param message String containing the information message to display.
   */
  void statusMessage(const QString& message);

  /*!
   * \brief errorMessage Emitted when an error occurs.
   * \param index Integer representing the index of the current file being
   * encrypted or decrypted.
   * \param message String containing the error message to display.
   */
  void errorMessage(const QString& path, const QString& message);

  /*!
   * \brief busyStatus Emitted when a cipher operation starts and ends.
   * \param busyStatus Boolean representing the busy status.
   */
  void busyStatus(bool busyStatus);

 public slots:
  /*!
   * \brief encrypt Executed when a signal is received for encryption with a
   * passphrase, a list of input file names, and the algorithm name.
   * \param passphrase String representing the user entered passphrase.
   * \param inputFileNames List of strings representing the file paths of the
   * files to encrypt.
   * \param algorithmName String representing the name of the algorithm to use
   * for encryption.
   */
  void encrypt(const QString& passphrase,
               const QStringList& inputFileNames,
               const QString& algorithm = "AES-128/GCM");

  /*!
   * \brief decrypt Executed when a signal is received for decryption with a
   * passphrase and a list of input file names. The algorithm is determined from
   * the file header.
   * \param passphrase String representing the user entered passphrase.
   * \param inputFileNames List of strings representing the file paths of
   * the files to decrypt.
   */
  void decrypt(const QString& passphrase,
               const QStringList& inputFileNames);

  /*!
   * \brief abort Executed when a signal is received to set the abort status.
   * The abort status, when set, will stop the execution of the current cipher
   * operation and prevent further cipher operations from starting until it is
   * reset to false. The current cipher operation is abandoned and cannot be
   * continued.
   */
  void abort();

  /*!
   * \brief pause Executed when a signal is received to set or clear the pause
   * status (via the state of the boolean parameter pause). The pause status, if
   * set to true, will pause the execution of the current cipher operation until
   * it is reset to false. When the pause status is reset to false, the cipher
   * operation that was in progress when the pause was signaled will resume
   * execution.
   * \param pause Boolean representing the pause state.
   */
  void pause(bool pause);

  /*!
   * \brief stop Executed when a signal is received to set the stop status for
   * the file name input parameter. The stop status, if set, will skip the input
   * file name in the encrypt/decrypt process.
   * \param fileName String representing a file name.
   */
  void stop(const QString& fileName);

 private:
  /*!
   * \brief encryptFile Encrypts a single file with the input passphrase and
   * algorithm name.
   * \param passphrase String representing the user entered passphrase.
   * \param inputFileName String representing the file path of the file to
   * encrypt.
   * \param algorithmName String representing the name of the algorithm to use
   * for encryption.
   */
  void encryptFile(const QString& passphrase,
                   const QString& inputFileName,
                   const QString& algorithmName);

  /*!
   * \brief decryptFile Decrypts a single file with the input passphrase and
   * algorithm name.
   * \param passphrase String representing the user entered passphrase.
   * \param inputFileName String representing the file path of the file to
   * decrypt.
   */
  void decryptFile(const QString& passphrase,
                   const QString& inputFileName);

  /*!
   * \brief executeCipher Executes a cipher on a file with the a key,
   * initialization vector, and cipher direction.
   * \param inputFileName String representing the file path of the file to
   * encrypt/decrypt.
   * \param algorithmName String representing the name of the algorithm to
   * use for encryption/decryption.
   * \param key Cipher key.
   * \param iv Cipher initialization vector.
   * \param cipherDirection Cipher direction. Valid values are:
   * Botan::ENCRYPTION and Botan::DECRYPTION
   * \param in Input file stream.
   * \param out Output file stream.
   */
  void executeCipher(const QString& inputFileName,
                     const std::string& algorithmName,
                     const Botan::SymmetricKey& key,
                     const Botan::InitializationVector& iv,
                     const Botan::Cipher_Dir cipherDirection,
                     std::ifstream& in,
                     std::ofstream& out);

 private:
  class CryptoPrivate;
  std::unique_ptr<CryptoPrivate> pimpl;
};

#endif // KRYVOS_CRYPTOGRAPHY_CRYPTO_HPP_
