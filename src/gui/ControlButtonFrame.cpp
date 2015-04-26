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
#include "utility/pimpl_impl.h"
#include <QtWidgets/QPushButton>
#include <QtWidgets/QHBoxLayout>
#include <QtGui/QIcon>

class ControlButtonFrame::ControlButtonFramePrivate {
 public:
  /*!
   * \brief ControlButtonFramePrivate Constructs the ControlButtonFrame private
   * implementation.
   */
  ControlButtonFramePrivate();

  QPushButton* encryptButton;
  QPushButton* decryptButton;
};

ControlButtonFrame::ControlButtonFrame(QWidget* parent)
  : QFrame{parent}
{
  const QSize iconSize{19, 19};

  const auto lockIcon = QIcon{QStringLiteral(":/images/lockIcon.png")};
  m->encryptButton = new QPushButton{lockIcon,
                                         tr(" Encrypt"),
                                         this};
  m->encryptButton->setIconSize(iconSize);
  m->encryptButton->setObjectName(QStringLiteral("cryptButton"));
  m->encryptButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

  const auto unlockedIcon = QIcon{QStringLiteral(":/images/unlockIcon.png")};
  m->decryptButton = new QPushButton{unlockedIcon,
                                     tr(" Decrypt"),
                                     this};
  m->decryptButton->setIconSize(iconSize);
  m->decryptButton->setObjectName(QStringLiteral("cryptButton"));
  m->decryptButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

  auto buttonLayout = new QHBoxLayout{this};
  buttonLayout->addWidget(m->encryptButton);
  buttonLayout->addWidget(m->decryptButton);

  connect(m->encryptButton, &QPushButton::clicked,
          this, &ControlButtonFrame::encryptFiles);
  connect(m->decryptButton, &QPushButton::clicked,
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
  Q_ASSERT(m->encryptButton);
  Q_ASSERT(m->decryptButton);

  m->encryptButton->setIconSize(iconSize);
  m->decryptButton->setIconSize(iconSize);
}

ControlButtonFrame::ControlButtonFramePrivate::ControlButtonFramePrivate()
  : encryptButton{nullptr}, decryptButton{nullptr}
{}
