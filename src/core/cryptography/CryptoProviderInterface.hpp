#ifndef KRYVO_CRYPTOGRAPHY_CRYPTOPROVIDERINTERFACE_HPP_
#define KRYVO_CRYPTOGRAPHY_CRYPTOPROVIDERINTERFACE_HPP_

#include "DispatcherState.hpp"
#include <QObject>
#include <QFileInfo>
#include <QString>
#include <QtPlugin>

namespace Kryvo {

class CryptoProviderInterface {
 public:
  virtual ~CryptoProviderInterface() = default;

  virtual void fileCompleted(std::size_t id) = 0;

  virtual void fileFailed(std::size_t id) = 0;

 /*!
  * \brief fileProgress Emitted when the cipher operation file progress changes
  * \param id ID representing file to update progress on
  * \param task String containing task name
  * \param percent Integer representing the current progress as a percent
  */
  virtual void fileProgress(std::size_t id, const QString& task,
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
  * \param fileInfo File that encountered an error
  */
  virtual void errorMessage(const QString& message,
                            const QFileInfo& fileInfo) = 0;

  virtual void init(DispatcherState* state) = 0;

  /*!
  * \brief encrypt Encrypt a file
  * \param id ID representing file to encrypt
  * \param passphrase String representing the user-entered passphrase
  * \param inputFileInfo File to encrypt
  * \param outputFileInfo Encrypted file
  * \param cipher String representing name of the cipher
  * \param keySize Key size in bits
  * \param modeOfOperation String representing mode of operation
  */
  virtual bool encrypt(std::size_t id,
                       const QString& compressionFormat,
                       const QString& passphrase,
                       const QFileInfo& inputFileInfo,
                       const QFileInfo& outputFileInfo,
                       const QString& cipher,
                       std::size_t keySize,
                       const QString& modeOfOperation) = 0;

  /*!
   * \brief decrypt Decrypt a file. The algorithm is determined from
   * the file header.
   * \param id ID representing file to decrypt
   * \param passphrase String representing the user-entered passphrase
   * \param inputFileInfo File to decrypt
   * \param outputFileInfo Decrypted file
   */
  virtual bool decrypt(std::size_t id,
                       const QString& passphrase,
                       const QFileInfo& inputFileInfo,
                       const QFileInfo& outputFileInfo,
                       const QByteArray& algorithmNameByteArray,
                       const QByteArray& keySizeByteArray,
                       const QByteArray& pbkdfSaltByteArray,
                       const QByteArray& keySaltByteArray,
                       const QByteArray& ivSaltByteArray) = 0;

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
