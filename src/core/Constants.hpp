#ifndef KRYVO_CONSTANTS_HPP_
#define KRYVO_CONSTANTS_HPP_

#include <QVersionNumber>
#include <QFileInfo>
#include <QDir>
#include <QObject>
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
  static const QString kDot;
  static const QString kEncryptedFileExtension;
  static const QString kCompressedFileExtension;
  static const QString kArchiveFileExtension;
  static const QStringList kDefaultPaths;
  static const QString kDocumentsPath;

  static const QStringList messages;

  /*!
   * \brief removeExtension Attempts to return the file path string input
   * without the last extension. It's used to extract an extension to determine
   * a decrypted file path.
   * \param filePath String containing the file path
   * \param extension String representing the extension to remove from the
   * file path
   * \return String containing a file path without an extension
   */
  static QString removeExtension(const QString& filePath,
                                 const QString& extension);

  /*!
   * \brief uniqueFilePath Returns a unique file path from the input file path
   * by appending an increasing number, if necessary.
   * \param filePath String representing the file path that will be tested
   * for uniqueness
   * \return String representing a unique file path created from the input file
   * path
   */
  static QString uniqueFilePath(const QString& filePath);
};

} // namespace Kryvo

#endif // KRYVO_CONSTANTS_HPP_