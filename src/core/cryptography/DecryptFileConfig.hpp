#ifndef KRYVO_CRYPTOGRAPHY_DECRYPTFILECONFIG_HPP_
#define KRYVO_CRYPTOGRAPHY_DECRYPTFILECONFIG_HPP_

#include <QFileInfo>
#include <QString>

namespace Kryvo {

struct DecryptFileConfig {
  QString provider;
  QString passphrase;
  QFileInfo inputFileInfo;
  QFileInfo outputFileInfo;
};

}

#endif // KRYVO_CRYPTOGRAPHY_DECRYPTFILECONFIG_HPP_
