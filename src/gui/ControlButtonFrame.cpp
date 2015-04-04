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

#include "gui/ControlButtonFrame.hpp"
#include "utility/make_unique.h"
#include <QtWidgets/QPushButton>
#include <QtWidgets/QHBoxLayout>
#include <QtGui/QIcon>

class ControlButtonFrame::ControlButtonFramePrivate {
 public:
  /*!
   * \brief ControlButtonFramePrivate Constructs the ControlButtonFrame private
   * implementation.
   */
  explicit ControlButtonFramePrivate();

 public:
  QPushButton* encryptButton;
  QPushButton* decryptButton;
};

ControlButtonFrame::ControlButtonFrame(QWidget* parent)
  : QFrame{parent}, pimpl{make_unique<ControlButtonFramePrivate>()}
{
  const QSize iconSize{19, 19};

  const auto lockIcon = QIcon{":/images/lockIcon.png"};
  pimpl->encryptButton = new QPushButton{lockIcon,
                                       tr(" Encrypt"),
                                       this};
  pimpl->encryptButton->setIconSize(iconSize);
  pimpl->encryptButton->setObjectName("cryptButton");
  pimpl->encryptButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

  const auto unlockedIcon = QIcon{":/images/unlockIcon.png"};
  pimpl->decryptButton = new QPushButton{unlockedIcon,
                                       tr(" Decrypt"),
                                       this};
  pimpl->decryptButton->setIconSize(iconSize);
  pimpl->decryptButton->setObjectName("cryptButton");
  pimpl->decryptButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

  auto buttonLayout = new QHBoxLayout{this};
  buttonLayout->addWidget(pimpl->encryptButton);
  buttonLayout->addWidget(pimpl->decryptButton);

  connect(pimpl->encryptButton, &QPushButton::clicked,
          this, &ControlButtonFrame::encryptFiles);
  connect(pimpl->decryptButton, &QPushButton::clicked,
          this, &ControlButtonFrame::decryptFiles);
}

ControlButtonFrame::~ControlButtonFrame() {}

void ControlButtonFrame::encryptFiles()
{
  emit processFiles(true);
}

void ControlButtonFrame::decryptFiles()
{
  emit processFiles(false);
}

void ControlButtonFrame::setIconSize(const QSize& iconSize)
{
  Q_ASSERT(pimpl->encryptButton);
  Q_ASSERT(pimpl->decryptButton);

  pimpl->encryptButton->setIconSize(iconSize);
  pimpl->decryptButton->setIconSize(iconSize);
}

ControlButtonFrame::ControlButtonFramePrivate::ControlButtonFramePrivate()
  : encryptButton{nullptr}, decryptButton{nullptr}
{}
