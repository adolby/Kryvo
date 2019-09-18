#ifndef KRYVO_CONSTANTS_HPP_
#define KRYVO_CONSTANTS_HPP_

#include <QVersionNumber>
#include <QStringList>
#include <QString>

namespace Kryvo {
Q_NAMESPACE

enum class CryptDirection {
  Encrypt,
  Decrypt
}; Q_ENUM_NS(CryptDirection)

class Constants {
 public:
  static const QVersionNumber kVersion;
  static constexpr int kFileVersion = 1;
  static const QString kDot;
  static const QString kEncryptedFileExtension;
  static const QString kCompressedFileExtension;
  static const QString kArchiveFileExtension;
  static const QStringList kDefaultPaths;
  static const QString kDocumentsPath;
  static const QStringList kMessages;
};

} // namespace Kryvo

#endif // KRYVO_CONSTANTS_HPP_
