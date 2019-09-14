#ifndef KRYVO_CRYPTOGRAPHY_ENCRYPTFILECONFIG_HPP_
#define KRYVO_CRYPTOGRAPHY_ENCRYPTFILECONFIG_HPP_

#include <QFileInfo>
#include <QString>

namespace Kryvo {

struct EncryptFileConfig {
  QString provider;
  QString compressionFormat;
  QString passphrase;
  QFileInfo inputFileInfo;
  QFileInfo outputFileInfo;
  QString cipher;
  std::size_t keySize = 0;
  QString modeOfOperation;
};

}

#endif // KRYVO_CRYPTOGRAPHY_ENCRYPTFILECONFIG_HPP_
