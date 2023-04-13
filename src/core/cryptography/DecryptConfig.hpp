#ifndef KRYVO_CRYPTOGRAPHY_DECRYPTCONFIG_HPP_
#define KRYVO_CRYPTOGRAPHY_DECRYPTCONFIG_HPP_

#include <QString>

namespace Kryvo {

struct DecryptConfig {
  QString provider;
  QString passphrase;
  bool removeIntermediateFiles = true;
};

} // namespace Kryvo

#endif // KRYVO_CRYPTOGRAPHY_DECRYPTCONFIG_HPP_
