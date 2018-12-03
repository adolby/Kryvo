#ifndef KRYVO_CRYPTOGRAPHY_CRYPTOPROVIDERINTERFACE_HPP_
#define KRYVO_CRYPTOGRAPHY_CRYPTOPROVIDERINTERFACE_HPP_

#include "DispatcherState.hpp"
#include <QObject>
#include <QString>
#include <QtPlugin>

namespace Kryvo {

class CryptoProviderInterface {
 public:
  virtual ~CryptoProviderInterface() = default;

 /*!
  * \brief encrypt Encrypt a file
  * \param state Encryption process state
  * \param passphrase String representing the user-entered passphrase
  * \param inFilePaths String containing the file path of the file to encrypt
  * \param outputPath String containing output file path
  * \param cipher String representing name of the cipher
  * \param keySize Key size in bits
  * \param modeOfOperation String representing mode of operation
  * \param compress Boolean representing compression mode
  */
  virtual bool encrypt(DispatcherState* state,
                       const QString& passphrase,
                       const QString& inFilePath,
                       const QString& outputPath = QString(),
                       const QString& cipher = QString("AES"),
                       std::size_t keySize = 128,
                       const QString& modeOfOperation = QString("GCM"),
                       bool compress = true) = 0;

  /*!
   * \brief decrypt Decrypt a file. The algorithm is determined from
   * the file header.
   * \param state Decryption process state
   * \param passphrase String representing the user-entered passphrase
   * \param inFilePath String containing the file path of the file to decrypt
   * \param outFilePath String containing output file path
   */
  virtual bool decrypt(DispatcherState* state,
                       const QString& passphrase,
                       const QString& inFilePath,
                       const QString& outFilePath) = 0;

  /*!
   * \brief fileProgress Emitted when the cipher operation file progress changes
   * \param path String containing path of the file to update progress on
   * \param task String containing task name
   * \param percent Integer representing the current progress as a percent
   */
  virtual void fileProgress(const QString& filePath, const QString& task,
                            qint64 percentProgress) = 0;

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

#define CryptoProviderInterface_iid "app.kryvo.CryptoProviderInterface"

Q_DECLARE_INTERFACE(Kryvo::CryptoProviderInterface, CryptoProviderInterface_iid)

#endif // KRYVO_CRYPTOGRAPHY_CRYPTOPROVIDERINTERFACE_HPP_
