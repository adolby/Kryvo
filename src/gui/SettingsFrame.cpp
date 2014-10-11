/**
 * Kryvos File Encryptor - Encrypts and decrypts files.
 * Copyright (C) 2014 Andrew Dolby
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Contact : andrewdolby@gmail.com
 */

#include "gui/SettingsFrame.hpp"
#include "gui/SlideSwitch.hpp"
#include "utility/flowlayout.h"
#include "utility/make_unique.h"
#include <QtWidgets/QComboBox>
#include <QtWidgets/QLabel>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QAction>
#include <QtGui/QIcon>

class SettingsFrame::SettingsFramePrivate {
 public:
  explicit SettingsFramePrivate();

  QComboBox* cipherComboBox;
  QComboBox* keySizeComboBox;
};

SettingsFrame::SettingsFrame(QWidget* parent) :
  QFrame{parent}, pimpl{make_unique<SettingsFramePrivate>()}
{
  //auto slideSwitch = new SlideSwitch{this};
  //slideSwitch->setBackgroundPixmap(":/images/sliderBackground.png");
  //slideSwitch->setKnobPixmaps(":/images/sliderKnobEnable.png",
  //                            ":/images/sliderKnobDisable.png");

  auto headerFrame = new QFrame{this};

  auto headerLabel = new QLabel{tr("Settings"), headerFrame};
  headerLabel->setObjectName("headerText");

  auto headerLayout = new QHBoxLayout{headerFrame};
  headerLayout->addWidget(headerLabel);

  auto contentFrame = new QFrame{this};
  contentFrame->setObjectName("settingsSubFrame");

  auto cipherFrame = new QFrame{contentFrame};
  cipherFrame->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
  auto cipherLabel = new QLabel{"Cipher: ", cipherFrame};
  cipherLabel->setObjectName("text");
  pimpl->cipherComboBox = new QComboBox{cipherFrame};
  pimpl->cipherComboBox->setObjectName("settingsComboBox");
  pimpl->cipherComboBox->addItem("AES");
  auto cipherLayout = new QHBoxLayout{cipherFrame};
  cipherLayout->addWidget(cipherLabel);
  cipherLayout->addWidget(pimpl->cipherComboBox);

//  auto keySizeFrame = new QFrame{contentFrame};
//  keySizeFrame->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
//  auto keySizeLabel = new QLabel{"Key size: ", keySizeFrame};
//  keySizeLabel->setObjectName("text");
//  pimpl->keySizeComboBox = new QComboBox{keySizeFrame};
//  pimpl->keySizeComboBox->setObjectName("settingsComboBox");
//  auto keySizeLayout = new QHBoxLayout{keySizeFrame};
//  keySizeLayout->addWidget(keySizeLabel);
//  keySizeLayout->addWidget(pimpl->keySizeComboBox);

  auto contentLayout = new QHBoxLayout{contentFrame};
  contentLayout->addWidget(cipherFrame);
//  contentLayout->addWidget(keySizeFrame);

  auto layout = new QVBoxLayout{this};
  layout->addWidget(headerFrame, 0);
  layout->addWidget(contentFrame, 1);

  // Return to previous GUI state
  auto returnAction = new QAction{this};
  returnAction->setShortcut(Qt::Key_Escape);
  connect(returnAction, &QAction::triggered,
          this, &SettingsFrame::switchFrame);
  this->addAction(returnAction);
}

SettingsFrame::~SettingsFrame() {}

SettingsFrame::SettingsFramePrivate::SettingsFramePrivate()
{}
