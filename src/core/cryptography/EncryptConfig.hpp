#ifndef KRYVO_CRYPTOGRAPHY_ENCRYPTCONFIG_HPP_
#define KRYVO_CRYPTOGRAPHY_ENCRYPTCONFIG_HPP_

#include <QString>

namespace Kryvo {

struct EncryptConfig {
  QString provider;
  QString compressionFormat;
  QString passphrase;
  QString cipher;
  std::size_t keySize = 0;
  QString modeOfOperation;
  bool removeIntermediateFiles = true;
};

} // namespace Kryvo

#endif // KRYVO_CRYPTOGRAPHY_ENCRYPTCONFIG_HPP_
