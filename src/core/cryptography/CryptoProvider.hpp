#ifndef KRYVO_CRYPTOGRAPHY_CRYPTOPROVIDER_HPP_
#define KRYVO_CRYPTOGRAPHY_CRYPTOPROVIDER_HPP_

#include "cryptography/EncryptFileConfig.hpp"
#include "cryptography/DecryptFileConfig.hpp"
#include "SchedulerState.hpp"
#include <QObject>
#include <QFileInfo>
#include <QString>
#include <QtPlugin>

namespace Kryvo {

class CryptoProvider {
 public:
  virtual ~CryptoProvider() = default;

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

  virtual void init(SchedulerState* state) = 0;

  /*!
  * \brief encrypt Encrypt a file
  * \param config Encrypt file config
  */
  virtual bool encrypt(std::size_t id,
                       const Kryvo::EncryptFileConfig& config) = 0;

  /*!
   * \brief decrypt Decrypt a file. The algorithm is determined from
   * the file header.
   * \param config Decrypt file config
   */
  virtual bool decrypt(std::size_t id,
                       const Kryvo::DecryptFileConfig& config) = 0;

  /*!
   * \brief qObject Provide a constant cost QObject conversion
   * \return
   */
  virtual QObject* qObject() = 0;
};

} // namespace Kryvo

#define CryptoProvider_iid "app.kryvo.CryptoProvider"

Q_DECLARE_INTERFACE(Kryvo::CryptoProvider, CryptoProvider_iid)

#endif // KRYVO_CRYPTOGRAPHY_CRYPTOPROVIDER_HPP_
