#ifndef KRYVO_CRYPTOGRAPHY_CRYPTO_HPP_
#define KRYVO_CRYPTOGRAPHY_CRYPTO_HPP_

#include "Pipe.hpp"
#include "SchedulerState.hpp"
#include "cryptography/CryptoProvider.hpp"
#include "Plugin.hpp"
#include "utility/pimpl.h"
#include <QFileInfo>
#include <QObject>
#include <QString>
#include <memory>

namespace Kryvo {

class CryptoPrivate;

class Crypto : public Pipe {
  Q_OBJECT
  Q_DISABLE_COPY(Crypto)
  DECLARE_PRIVATE(Crypto)
  std::unique_ptr<CryptoPrivate> const d_ptr;

 public:
  /*!
   * \brief Crypto Constructs the Crypto class
   * \param parent
   */
  explicit Crypto(SchedulerState* state, QObject* parent = nullptr);

  ~Crypto() override;

  /*!
   * \brief encryptFile Executed when a signal is received for encryption
   * \param id
   * \param config Encrypt file config
   */
  int encryptFile(std::size_t id, const Kryvo::EncryptFileConfig& config);

  /*!
   * \brief decryptFile Executed when a signal is received for decryption
   * \param id
   * \param config Decrypt file config
   */
  int decryptFile(std::size_t id, const Kryvo::DecryptFileConfig& config);

 public slots:
  void updateProviders(const QHash<QString, Plugin>& loadedProviders);

  /*!
   * \brief encrypt Executed when a signal is received for encryption
   * \param confgi Encrypt file config
   */
  void encrypt(std::size_t id, const Kryvo::EncryptFileConfig& config);

  /*!
   * \brief decrypt Executed when a signal is received for decryption
   * \param config Decrypt file config
   */
  void decrypt(std::size_t id, const Kryvo::DecryptFileConfig& config);
};

} // namespace Kryvo

#endif // KRYVO_CRYPTOGRAPHY_CRYPTO_HPP_
