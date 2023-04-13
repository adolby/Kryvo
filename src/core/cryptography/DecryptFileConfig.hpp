#ifndef KRYVO_CRYPTOGRAPHY_DECRYPTFILECONFIG_HPP_
#define KRYVO_CRYPTOGRAPHY_DECRYPTFILECONFIG_HPP_

#include "DecryptConfig.hpp"
#include <QFileInfo>

namespace Kryvo {

struct DecryptFileConfig {
  Kryvo::DecryptConfig decrypt;
  QFileInfo inputFileInfo;
  QFileInfo outputFileInfo;
};

} // namespace Kryvo

#endif // KRYVO_CRYPTOGRAPHY_DECRYPTFILECONFIG_HPP_
