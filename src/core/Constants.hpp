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
  static const QVersionNumber version;
  static constexpr int fileVersion = 1;
  static const QString dot;
  static const QString encryptedFileExtension;
  static const QString compressedFileExtension;
  static const QString archiveFileExtension;
  static const QStringList defaultPaths;
  static const QString documentsPath;
  static const QStringList messages;
  static constexpr std::size_t maxConfigFileSize = 100000000; // 100 MB
};

} // namespace Kryvo

#endif // KRYVO_CONSTANTS_HPP_
