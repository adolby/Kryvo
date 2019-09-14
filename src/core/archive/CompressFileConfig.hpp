#ifndef KRYVO_ARCHIVE_COMPRESSFILECONFIG_HPP_
#define KRYVO_ARCHIVE_COMPRESSFILECONFIG_HPP_

#include <QFileInfo>

namespace Kryvo {

struct CompressFileConfig {
  QFileInfo inputFileInfo;
  QFileInfo outputFileInfo;
};

}

#endif // KRYVO_ARCHIVE_COMPRESSFILECONFIG_HPP_
