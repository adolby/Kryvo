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
  QFile settingsFile{"settings.json"};
  auto fileOpen = settingsFile.open(QIODevice::ReadOnly);

  if (fileOpen)
  {
    auto settingsData = settingsFile.readAll();

    auto settingsDoc = QJsonDocument::fromJson(settingsData);
    auto settings = settingsDoc.object();

    auto positionObject = settings["position"].toObject();
    auto xObject = static_cast<QJsonValue>(positionObject["x"]);
    auto yObject = static_cast<QJsonValue>(positionObject["y"]);
    position = QPoint{xObject.toInt(200), yObject.toInt(200)};

    auto maximizedObject = static_cast<QJsonValue>(settings["maximized"]);
    maximized = maximizedObject.toBool(false);
    if (!maximized)
    {
      auto sizeObject = settings["size"].toObject();
      auto widthObject = static_cast<QJsonValue>(sizeObject["width"]);
      auto heightObject = static_cast<QJsonValue>(sizeObject["height"]);
      size = QSize{widthObject.toInt(800), heightObject.toInt(600)};
    }

    lastDirectory = settings["lastDirectory"].toString();

    auto cipherObject = static_cast<QJsonValue>(settings["cipher"]);
    cipher = cipherObject.toString("AES");

    auto keySizeObject = static_cast<QJsonValue>(settings["keySize"]);
    keySize = static_cast<std::size_t>(keySizeObject.toInt(128));

    auto modeObject = static_cast<QJsonValue>(settings["modeOfOperation"]);
    mode = modeObject.toString("GCM");

    auto styleObject = static_cast<QJsonValue>(settings["styleSheetPath"]);
    styleSheetPath = styleObject.toString("default/kryvos.qss");

    auto fileColumnObject = static_cast<QJsonValue>
                            (settings["fileColumnWidth"]);
    fileColumnWidth = fileColumnObject.toInt();

    auto progressColumnObject = static_cast<QJsonValue>
                          (settings["progressColumnWidth"]);
    progressColumnWidth = progressColumnObject.toInt();
  }
  else
  { // Settings file couldn't be opened, so use defaults
    position = QPoint{200, 200};
    size = QSize{800, 600};
    cipher = "AES";
    keySize = 128;
    mode = "GCM";
    styleSheetPath = "default/kryvos.qss";
    fileColumnWidth = 0;
    progressColumnWidth = 0;
  }
}

void Settings::SettingsPrivate::exportSettings() const
{
  QSaveFile settingsFile{"settings.json"};
  settingsFile.setDirectWriteFallback(true);
  auto fileOpen = settingsFile.open(QIODevice::WriteOnly);

  if (fileOpen)
  {
    auto settings = QJsonObject{};

    auto positionObject = QJsonObject{};
    positionObject["x"] = position.x();
    positionObject["y"] = position.y();

    settings["position"] = positionObject;
    settings["maximized"] = maximized;

    if (!maximized)
    {
      auto sizeObject = QJsonObject{};
      sizeObject["width"] = size.width();
      sizeObject["height"] = size.height();
      settings["size"] = sizeObject;
    }

    settings["lastDirectory"] = lastDirectory;
    settings["cipher"] = cipher;
    settings["keySize"] = static_cast<int>(keySize);
    settings["modeOfOperation"] = mode;
    settings["styleSheetPath"] = styleSheetPath;
    settings["fileColumnWidth"] = fileColumnWidth;
    settings["progressColumnWidth"] = progressColumnWidth;

    auto settingsDoc = QJsonDocument{settings};
    settingsFile.write(settingsDoc.toJson());
  }

  settingsFile.commit();
}
