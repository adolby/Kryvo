#ifndef KRYVO_PLUGINS_CRYPTOGRAPHY_OPENSSLPROVIDER_HPP_
#define KRYVO_PLUGINS_CRYPTOGRAPHY_OPENSSLPROVIDER_HPP_

#include "cryptography/CryptoProvider.hpp"
#include "utility/pimpl.h"

#include <QFileInfo>
#include <QObject>
#include <QString>
#include <memory>

namespace Kryvo {

class OpenSslProviderPrivate;

class OpenSslProvider : public QObject,
                        public CryptoProvider {
  Q_OBJECT
  Q_DISABLE_COPY(OpenSslProvider)
  Q_PLUGIN_METADATA(IID "app.kryvo.CryptoProvider" FILE "openssl.json")
  Q_INTERFACES(Kryvo::CryptoProvider)
  DECLARE_PRIVATE(OpenSslProvider)
  std::unique_ptr<OpenSslProviderPrivate> const d_ptr;

 public:
  explicit OpenSslProvider(QObject* parent = nullptr);
  ~OpenSslProvider() override;

 signals:
  void fileCompleted(std::size_t id);

  void fileFailed(std::size_t id);

  /*!
   * \brief fileProgress Emitted when the cipher operation file progress changes
   * \param id ID representing file to update progress on
   * \param task String containing task name
   * \param percent Integer representing the current progress as a percent
   */
  void fileProgress(std::size_t id, const QString& task,
                    qint64 percentProgress) override;

  /*!
   * \brief statusMessage Emitted when a message about the current cipher
   * operation should be displayed to the user
   * \param message String containing the information message to display
   */
  void statusMessage(const QString& message) override;

  /*!
   * \brief errorMessage Emitted when an error occurs
   * \param message String containing the error message to display
   * \param fileInfo File that encountered an error
   */
  void errorMessage(const QString& message, const QFileInfo& fileInfo) override;

 public:
  void init(SchedulerState* state) override;

  /*!
  * \brief encrypt Encrypt a file
  * \param config Encrypt file config
  */
  virtual bool encrypt(const Kryvo::EncryptFileConfig& config) override;

  /*!
   * \brief decrypt Decrypt a file. The algorithm is determined from
   * the file header.
   * \param config Decrypt file config
   */
  virtual bool decrypt(const Kryvo::DecryptFileConfig& config) override;

  /*!
   * \brief qObject Provide a constant cost QObject conversion
   * \return
   */
  QObject* qObject() override;
};

} // namespace Kryvo

#endif // KRYVO_PLUGINS_CRYPTOGRAPHY_OPENSSLPROVIDER_HPP_
