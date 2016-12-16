#ifndef KRYVOS_SETTINGS_SETTINGS_HPP_
#define KRYVOS_SETTINGS_SETTINGS_HPP_

#include "src/utility/pimpl.h"
#include <QPoint>
#include <QSize>
#include <QString>

/*!
 * \brief The Settings class keeps settings data for the application.
 */
class Settings {
 public:
  /*!
   * \brief Settings Constructs an instance of the settings management class
   */
  Settings();

  /*!
   * \brief ~Settings Destroys an instance of the settings management class
   */
  virtual ~Settings();

  /*!
   * \brief position Sets the main window position
   * \param position Point at position of the main window
   */
  void position(const QPoint& position);

  /*!
   * \brief position Returns the main window's position
   * \return Point at position of the main window
   */
  QPoint position() const;

  /*!
   * \brief maximized Sets the maximized state
   * \param maximized Boolean representing maximized state
   */
  void maximized(const bool maximized);

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
   * \return Size of main window
   */
  QSize size() const;

  /*!
   * \brief cipher Sets the cipher for later storage
   * \param cipherName String containing the cipher name
   */
  void cipher(const QString& cipherName);

  /*!
   * \brief lastAlgorithm Returns the name of the last cipher
   * \return String containing the cipher name
   */
  QString cipher() const;

  /*!
   * \brief keySize Sets the key size
   * \param keySize Key size in bits
   */
  void keySize(const std::size_t& keySize);

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
   * \brief compressionMode Sets the compression mode
   * \param compress Boolean representing the compression mode
   */
  void compressionMode(const bool compress);

  /*!
   * \brief compressionMode Returns the compression mode
   * \return Boolean representing the compression mode
   */
  bool compressionMode() const;

  /*!
   * \brief containerMode Sets the container mode
   * \param container Boolean representing the container mode
   */
  void containerMode(const bool container);

  /*!
   * \brief containerMode Returns the container mode
   * \return Boolean representing the container mode
   */
  bool containerMode() const;

  /*!
   * \brief outputPath Sets the output path
   * \param fileName String containing the output path
   */
  void outputPath(const QString& path);

  /*!
   * \brief outputPath Returns the name of the output path
   * \return String containing the output path
   */
  QString outputPath() const;

  /*!
   * \brief lastDirectory Sets the last opened directory
   * \param algorithm String containing the directory path
   */
  void lastDirectory(const QString& directory);

  /*!
   * \brief lastDirectory Returns the name of the last opened directory
   * \return String containing the directory path
   */
  QString lastDirectory() const;

  /*!
   * \brief styleSheetPath Returns the path to the application stylesheet
   * \return String containing the stylesheet path
   */
  QString styleSheetPath() const;

 private:
  class SettingsPrivate;
  pimpl<SettingsPrivate> m;
};

#endif // KRYVOS_SETTINGS_SETTINGS_HPP_
