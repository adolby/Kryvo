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
  * \brief fileProgress Emitted when the cipher operation file progress changes
  * \param id ID representing file to update progress on
  * \param task String containing task name
  * \param percent Integer representing the current progress as a percent
  */
  virtual void fileProgress(int id, const QString& task,
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
  virtual void errorMessage(const QString& message, const QString& filePath) = 0;

  virtual void init(DispatcherState* state) = 0;

 /*!
  * \brief encrypt Encrypt a file
  * \param passphrase String representing the user-entered passphrase
  * \param inFilePaths String containing the file path of the file to encrypt
  * \param outputPath String containing output file path
  * \param cipher String representing name of the cipher
  * \param keySize Key size in bits
  * \param modeOfOperation String representing mode of operation
  * \param compress Boolean representing compression mode
  */
  virtual bool encrypt(int id,
                       const QString& passphrase,
                       const QString& inFilePath,
                       const QString& outputPath,
                       const QString& cipher,
                       std::size_t keySize,
                       const QString& modeOfOperation,
                       bool compress) = 0;

  /*!
   * \brief decrypt Decrypt a file. The algorithm is determined from
   * the file header.
   * \param passphrase String representing the user-entered passphrase
   * \param inFilePath String containing the file path of the file to decrypt
   * \param outFilePath String containing output file path
   */
  virtual bool decrypt(int id,
                       const QString& passphrase,
                       const QString& inFilePath,
                       const QString& outFilePath,
                       const QString& algorithmNameString,
                       const QString& keySizeString,
                       const QString& pbkdfSaltString,
                       const QString& keySaltString,
                       const QString& ivSaltString) = 0;

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
