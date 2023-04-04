#ifndef KRYVO_ARCHIVE_DECOMPRESSFILECONFIG_HPP_
#define KRYVO_ARCHIVE_DECOMPRESSFILECONFIG_HPP_

#include <QFileInfo>

namespace Kryvo {

struct DecompressFileConfig {
  QFileInfo inputFileInfo;
  QFileInfo outputFileInfo;
};

}

#endif // KRYVO_ARCHIVE_DECOMPRESSFILECONFIG_HPP_
