#ifndef KRYVO_CRYPTOGRAPHY_CRYPTOPROVIDERINTERFACE_HPP_
#define KRYVO_CRYPTOGRAPHY_CRYPTOPROVIDERINTERFACE_HPP_

#include "cryptography/CryptoState.hpp"
#include <QObject>
#include <QString>
#include <QtPlugin>

namespace Kryvo {

class CryptoProviderInterface {
 public:
  virtual ~CryptoProviderInterface() = default;

 /*!
  * \brief encrypt Encrypt a list of files
  * \param state Encryption process state
  * \param passphrase String representing the user-entered passphrase
  * \param inputFilePaths List of strings containing the file paths of the
  * files to encrypt
  * \param outputPath String containing output file path
  * \param cipher String representing name of the cipher
  * \param inputKeySize Key size in bits
  * \param modeOfOperation String representing mode of operation
  * \param compress Boolean representing compression mode
  * \param container Boolean representing container mode
  */
  virtual bool encrypt(CryptoState* state,
                       const QString& passphrase,
                       const QStringList& inputFilePaths,
                       const QString& outputPath = QString(),
                       const QString& cipher = QString("AES"),
                       const std::size_t keySize = 128,
                       const QString& modeOfOperation = QString("GCM"),
                       const bool compress = true,
                       const bool container = true) = 0;

  /*!
   * \brief decrypt Decrypt a list of files. The algorithm is determined from
   * the file header.
   * \param state Decryption process state
   * \param passphrase String representing the user-entered passphrase
   * \param inputFilePaths List of strings containing the file paths of
   * the files to decrypt
   * \param outputPath String containing output file path
   */
  virtual bool decrypt(CryptoState* state,
                       const QString& passphrase,
                       const QStringList& inputFilePaths,
                       const QString& outputPath) = 0;

  /*!
   * \brief fileProgress Emitted when the cipher operation file progress changes
   * \param path String containing path of the file to update progress on
   * \param task String containing task name
   * \param percent Integer representing the current progress as a percent
   */
  virtual void fileProgress(const QString& filePath, const QString& task,
                            const qint64 percentProgress) = 0;

  /*!
   * \brief statusMessage Emitted when a message about the current cipher
   * operation should be displayed to the user
   * \param message String containing the information message to display
   */
  virtual void statusMessage(const QString& message) = 0;

  /*!
   * \brief errorMessage Emitted when an error occurs
   * \param message String containing the error message to display
   * \param filePath String containing the file path which encountered an error
   */
  virtual void errorMessage(const QString& message,
                            const QString& filePath = QString()) = 0;

  /*!
   * \brief qObject Provide a constant cost QObject conversion
   * \return
   */
  virtual QObject* qObject() = 0;
};

} // namespace Kryvo

#define CryptoProviderInterface_iid "io.kryvo.CryptoProviderInterface"

Q_DECLARE_INTERFACE(Kryvo::CryptoProviderInterface, CryptoProviderInterface_iid)

#endif // KRYVO_CRYPTOGRAPHY_CRYPTOPROVIDERINTERFACE_HPP_
