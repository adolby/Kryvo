#ifndef KRYVO_SETTINGS_SETTINGS_HPP_
#define KRYVO_SETTINGS_SETTINGS_HPP_

#include <QObject>
#include "Plugin.hpp"
#include "utility/pimpl.h"
#include <QPoint>
#include <QSize>
#include <QString>
#include <memory>

namespace Kryvo {

class SettingsPrivate;

class Settings : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY(Settings)
  DECLARE_PRIVATE(Settings)
  std::unique_ptr<SettingsPrivate> const d_ptr;

 public:
  /*!
   * \brief Settings Constructs an instance of the settings management class
   */
  explicit Settings(QObject* parent = nullptr);

  /*!
   * \brief ~Settings Destroys an instance of the settings management class
   */
  virtual ~Settings();

  /*!
   * \brief position Sets the main window position
   * \param position Upper left corner position of the main window
   */
  void position(const QPoint& position);

  /*!
   * \brief position Returns the main window's position
   * \return Upper left corner position of the main window
   */
  QPoint position() const;

  /*!
   * \brief maximized Sets the maximized state
   * \param maximized Boolean representing maximized state
   */
  void maximized(bool maximized);

  /*!
   * \brief maximized Returns the main window's maximized state
   * \return Boolean representing maximized state
   */
  bool maximized() const;

  /*!
   * \brief size Sets the main window size
   * \param size Size of the main window
   */
  void size(const QSize& size);

  /*!
   * \brief size Returns the main window's last size
   * \return Size of the main window
   */
  QSize size() const;

  void cryptoProvider(const QString& provider);

  QString cryptoProvider() const;

  /*!
   * \brief compressionFormat Sets the compression format
   * \param format String representing the compression format
   */
  void compressionFormat(const QString& format);

  /*!
   * \brief compressionFormat Returns the compression format
   * \return String representing the compression format
   */
  QString compressionFormat() const;

  /*!
   * \brief cipher Sets the cipher for later storage
   * \param cipherName String containing the cipher name
   */
  void cipher(const QString& cipherName);

  /*!
   * \brief cipher Returns the name of the last cipher
   * \return String containing the cipher name
   */
  QString cipher() const;

  /*!
   * \brief keySize Sets the key size
   * \param keySize Key size in bits
   */
  void keySize(std::size_t keySize);

  /*!
   * \brief keySize Returns the key size
   * \return Key size in bits
   */
  std::size_t keySize() const;

  /*!
   * \brief modeOfOperation Sets the mode of operation
   * \param modeOfOperation String containing the mode of operation
   */
  void modeOfOperation(const QString& modeOfOperation);

  /*!
   * \brief modeOfOperation Returns the mode of operation
   * \return String containing the mode of operation
   */
  QString modeOfOperation() const;

  /*!
   * \brief removeIntermediateFiles Sets the remove intermediate files
   * preference
   * \param removeIntermediate Boolean indicating whether to remove intermediate
   * files
   */
  void removeIntermediateFiles(bool removeIntermediate);

  /*!
   * \brief removeIntermediateFiles Returns the remove intermediate files
   * preference
   * \return Boolean indicating whether to remove intermediate files
   */
  bool removeIntermediateFiles() const;

  /*!
   * \brief containerMode Sets the container mode
   * \param container Boolean representing the container mode
   */
  void containerMode(bool container);

  /*!
   * \brief containerMode Returns the container mode
   * \return Boolean representing the container mode
   */
  bool containerMode() const;

  /*!
   * \brief outputPath Sets the output path
   * \param path String containing the output path
   */
  void outputPath(const QString& path);

  /*!
   * \brief outputPath Returns the name of the output path
   * \return String containing the output path
   */
  QString outputPath() const;

  /*!
   * \brief inputPath Sets the input path
   * \param path String containing the path
   */
  void inputPath(const QString& path);

  /*!
   * \brief inputPath Returns the name of the input path
   * \return String containing the path
   */
  QString inputPath() const;

  /*!
   * \brief styleSheetPath Returns the path to the application stylesheet
   * \return String containing the stylesheet path
   */
  QString styleSheetPath() const;

 signals:
  void settingsImported();
  void positionChanged(const QPoint& position);
  void maximizedChanged(bool maximized);
  void sizeChanged(const QSize& size);
  void cryptoProviderChanged(const QString& provider);
  void compressionFormatChanged(const QString& format);
  void cipherChanged(const QString& cipher);
  void keySizeChanged(std::size_t keySize);
  void modeOfOperationChanged(const QString& mode);
  void removeIntermediateFilesChanged(bool removeIntermediate);
  void containerModeChanged(bool container);
  void outputPathChanged(const QString& path);
  void inputPathChanged(const QString& path);

 public slots:
  void cryptoProvidersChanged(const QHash<QString, Plugin>& providers);
};

} // namespace Kryvo

#endif // KRYVO_SETTINGS_SETTINGS_HPP_
