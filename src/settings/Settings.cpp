/**
 * Kryvos - Encrypts and decrypts files.
 * Copyright (C) 2014, 2015 Andrew Dolby
 *
 * This file is part of Kryvos.
 *
 * Kryvos is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Kryvos is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Kryvos.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Contact : andrewdolby@gmail.com
 */

#include "settings/Settings.hpp"
#include "utility/make_unique.h"
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QSaveFile>
#include <QtCore/QFile>

class Settings::SettingsPrivate {
 public:
  SettingsPrivate();

  /*!
   * \brief importSettings Imports settings from the local settings file:
   * the last application size and position, the last directory opened by the
   * file open dialog, the last algorithm set, and the stylesheet path for
   * theming.
   */
  void importSettings();

  /*!
   * \brief exportSettings Exports settings to the local settings file: the
   * application size and position, the last directory opened by the open
   * dialog, and the stylesheet path for theming.
   */
  void exportSettings() const;

  QPoint position;
  bool maximized;
  QSize size;
  QString cipher;
  std::size_t keySize;
  QString mode;
  QString lastDirectory;
  QString styleSheetPath;
  int fileColumnWidth;
  int progressColumnWidth;
};

Settings::Settings()
  : pimpl{make_unique<SettingsPrivate>()}
{
  pimpl->importSettings();
}

Settings::~Settings()
{
  pimpl->exportSettings();
}

void Settings::position(const QPoint& position)
{
  Q_ASSERT(pimpl);

  pimpl->position = position;
}

QPoint Settings::position() const
{
  Q_ASSERT(pimpl);

  return pimpl->position;
}

void Settings::maximized(const bool maximized)
{
  Q_ASSERT(pimpl);

  pimpl->maximized = maximized;
}

bool Settings::maximized() const
{
  Q_ASSERT(pimpl);

  return pimpl->maximized;
}

void Settings::size(const QSize& size)
{
  Q_ASSERT(pimpl);

  pimpl->size = size;
}

QSize Settings::size() const
{
  Q_ASSERT(pimpl);

  return pimpl->size;
}

void Settings::cipher(const QString& cipherName)
{
  Q_ASSERT(pimpl);

  pimpl->cipher = cipherName;
}

QString Settings::cipher() const
{
  Q_ASSERT(pimpl);

  return pimpl->cipher;
}

void Settings::keySize(const std::size_t& keySize)
{
  Q_ASSERT(pimpl);

  pimpl->keySize = keySize;
}

std::size_t Settings::keySize() const
{
  Q_ASSERT(pimpl);

  return pimpl->keySize;
}

void Settings::modeOfOperation(const QString& modeOfOperation)
{
  Q_ASSERT(pimpl);

  pimpl->mode = modeOfOperation;
}

QString Settings::modeOfOperation() const
{
  Q_ASSERT(pimpl);

  return pimpl->mode;
}

void Settings::lastDirectory(const QString& directory)
{
  Q_ASSERT(pimpl);

  pimpl->lastDirectory = directory;
}

QString Settings::lastDirectory() const
{
  Q_ASSERT(pimpl);

  return pimpl->lastDirectory;
}

QString Settings::styleSheetPath() const
{
  Q_ASSERT(pimpl);

  return pimpl->styleSheetPath;
}

void Settings::fileColumnWidth(const int fileColumnWidth)
{
  Q_ASSERT(pimpl);

  pimpl->fileColumnWidth = fileColumnWidth;
}

int Settings::fileColumnWidth() const
{
  Q_ASSERT(pimpl);

  return pimpl->fileColumnWidth;
}

void Settings::progressColumnWidth(const int progressColumnWidth)
{
  Q_ASSERT(pimpl);

  pimpl->progressColumnWidth = progressColumnWidth;
}

int Settings::progressColumnWidth() const
{
  Q_ASSERT(pimpl);

  return pimpl->progressColumnWidth;
}

Settings::SettingsPrivate::SettingsPrivate()
  : maximized{false}, keySize{128}, fileColumnWidth{0},
    progressColumnWidth{0}
{}

void Settings::SettingsPrivate::importSettings()
{
  QFile settingsFile{QStringLiteral("settings.json")};
  auto fileOpen = settingsFile.open(QIODevice::ReadOnly);

  if (fileOpen)
  {
    auto settingsData = settingsFile.readAll();

    auto settingsDoc = QJsonDocument::fromJson(settingsData);
    auto settings = settingsDoc.object();

    auto positionObject = settings[QStringLiteral("position")].toObject();
    auto xObject = static_cast<QJsonValue>(positionObject[QStringLiteral("x")]);
    auto yObject = static_cast<QJsonValue>(positionObject[QStringLiteral("y")]);
    position = QPoint{xObject.toInt(200), yObject.toInt(200)};

    auto maximizedObject =
        static_cast<QJsonValue>(settings[QStringLiteral("maximized")]);
    maximized = maximizedObject.toBool(false);
    if (!maximized)
    {
      auto sizeObject = settings["size"].toObject();
      auto widthObject =
          static_cast<QJsonValue>(sizeObject[QStringLiteral("width")]);
      auto heightObject =
          static_cast<QJsonValue>(sizeObject[QStringLiteral("height")]);
      size = QSize{widthObject.toInt(800), heightObject.toInt(600)};
    }

    lastDirectory = settings[QStringLiteral("lastDirectory")].toString();

    auto cipherObject =
        static_cast<QJsonValue>(settings[QStringLiteral("cipher")]);
    cipher = cipherObject.toString(QStringLiteral("AES"));

    auto keySizeObject =
        static_cast<QJsonValue>(settings[QStringLiteral("keySize")]);
    keySize = static_cast<std::size_t>(keySizeObject.toInt(128));

    auto modeObject =
        static_cast<QJsonValue>(settings[QStringLiteral("modeOfOperation")]);
    mode = modeObject.toString(QStringLiteral("GCM"));

    auto styleObject =
        static_cast<QJsonValue>(settings[QStringLiteral("styleSheetPath")]);
    styleSheetPath = styleObject.toString(QStringLiteral("default/kryvos.qss"));

    auto fileColumnObject = static_cast<QJsonValue>
                            (settings[QStringLiteral("fileColumnWidth")]);
    fileColumnWidth = fileColumnObject.toInt();

    auto progressColumnObject = static_cast<QJsonValue>
                          (settings[QStringLiteral("progressColumnWidth")]);
    progressColumnWidth = progressColumnObject.toInt();
  }
  else
  { // Settings file couldn't be opened, so use defaults
    position = QPoint{200, 200};
    size = QSize{800, 600};
    cipher = QStringLiteral("AES");
    keySize = 128;
    mode = QStringLiteral("GCM");
    styleSheetPath = QStringLiteral("default/kryvos.qss");
    fileColumnWidth = 0;
    progressColumnWidth = 0;
  }
}

void Settings::SettingsPrivate::exportSettings() const
{
  QSaveFile settingsFile{QStringLiteral("settings.json")};
  settingsFile.setDirectWriteFallback(true);
  auto fileOpen = settingsFile.open(QIODevice::WriteOnly);

  if (fileOpen)
  {
    auto settings = QJsonObject{};

    auto positionObject = QJsonObject{};
    positionObject[QStringLiteral("x")] = position.x();
    positionObject[QStringLiteral("y")] = position.y();

    settings[QStringLiteral("position")] = positionObject;
    settings[QStringLiteral("maximized")] = maximized;

    if (!maximized)
    {
      auto sizeObject = QJsonObject{};
      sizeObject[QStringLiteral("width")] = size.width();
      sizeObject[QStringLiteral("height")] = size.height();
      settings[QStringLiteral("size")] = sizeObject;
    }

    settings[QStringLiteral("lastDirectory")] = lastDirectory;
    settings[QStringLiteral("cipher")] = cipher;
    settings[QStringLiteral("keySize")] = static_cast<int>(keySize);
    settings[QStringLiteral("modeOfOperation")] = mode;
    settings[QStringLiteral("styleSheetPath")] = styleSheetPath;
    settings[QStringLiteral("fileColumnWidth")] = fileColumnWidth;
    settings[QStringLiteral("progressColumnWidth")] = progressColumnWidth;

    auto settingsDoc = QJsonDocument{settings};
    settingsFile.write(settingsDoc.toJson());
  }

  settingsFile.commit();
}
