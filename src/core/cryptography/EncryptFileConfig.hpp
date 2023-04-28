#ifndef KRYVO_CRYPTOGRAPHY_ENCRYPTFILECONFIG_HPP_
#define KRYVO_CRYPTOGRAPHY_ENCRYPTFILECONFIG_HPP_

#include "EncryptConfig.hpp"
#include <QFileInfo>
#include <QString>

namespace Kryvo {

struct EncryptFileConfig {
  Kryvo::EncryptConfig encrypt;
  QFileInfo inputFileInfo;
  QFileInfo outputFileInfo;
};

} // namespace Kryvo

#endif // KRYVO_CRYPTOGRAPHY_ENCRYPTFILECONFIG_HPP_
