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

#include "gui/HeaderFrame.hpp"
#include "utility/flowlayout.h"
#include "utility/make_unique.h"
#include <QtWidgets/QPushButton>
#include <QtWidgets/QLabel>
#include <QtWidgets/QHBoxLayout>
#include <QtGui/QIcon>

class HeaderFrame::HeaderFramePrivate {
 public:
  /*!
   * \brief HeaderFramePrivate Constructs the HeaderFrame private
   * implementation.
   */
  explicit HeaderFramePrivate();

  QPushButton* pauseButton;
  QPushButton* addFilesButton;
  QPushButton* clearFilesButton;
  QPushButton* settingsButton;

  QSize iconSize;
};

HeaderFrame::HeaderFrame(QWidget* parent) :
  QFrame{parent}, pimpl{make_unique<HeaderFramePrivate>()}
{
  auto headerLabel = new QLabel{tr("Kryvos"), this};
  headerLabel->setObjectName("headerText");

  auto buttonFrame = new QFrame{this};

  const auto pauseIcon = QIcon{":/images/pauseIcon.png"};
  pimpl->pauseButton = new QPushButton{pauseIcon, tr(" Pause"), this};
  pimpl->pauseButton->setObjectName("pauseButton");
  pimpl->pauseButton->setCheckable(true);
  pimpl->pauseButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

  const auto addFilesIcon = QIcon{":/images/addFilesIcon.png"};
  pimpl->addFilesButton = new QPushButton{addFilesIcon,
                                          tr(" Add files"),
                                          this};
  pimpl->addFilesButton->setObjectName("addButton");
  pimpl->addFilesButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

  const auto clearFilesIcon = QIcon{":/images/clearFilesIcon.png"};
  pimpl->clearFilesButton = new QPushButton{clearFilesIcon,
                                            tr(" Remove all files"),
                                            this};
  pimpl->clearFilesButton->setObjectName("clearButton");
  pimpl->clearFilesButton->setSizePolicy(QSizePolicy::Fixed,
                                         QSizePolicy::Fixed);

  const auto settingsIcon = QIcon{":/images/gearIcon.png"};
  pimpl->settingsButton = new QPushButton{settingsIcon,
                                          tr(" Settings"),
                                          this};
  pimpl->settingsButton->setObjectName("settingsButton");
  pimpl->settingsButton->setSizePolicy(QSizePolicy::Fixed,
                                       QSizePolicy::Fixed);

  auto buttonLayout = new FlowLayout{buttonFrame};
  buttonLayout->addWidget(pimpl->pauseButton);
  buttonLayout->addWidget(pimpl->addFilesButton);
  buttonLayout->addWidget(pimpl->clearFilesButton);
  buttonLayout->addWidget(pimpl->settingsButton);

  auto headerLayout = new QHBoxLayout{this};
  headerLayout->addWidget(headerLabel, 0);
  headerLayout->addWidget(buttonFrame, 1);
  headerLayout->setContentsMargins(10, 0, 0, 0);

  connect(pimpl->pauseButton, &QPushButton::toggled,
          this, &HeaderFrame::pause);
  connect(pimpl->pauseButton, &QPushButton::toggled,
          this, &HeaderFrame::togglePauseIcon);
  connect(pimpl->addFilesButton, &QPushButton::clicked,
          this, &HeaderFrame::addFiles);
  connect(pimpl->clearFilesButton, &QPushButton::clicked,
          this, &HeaderFrame::removeFiles);
  connect(pimpl->settingsButton, &QPushButton::clicked,
          this, &HeaderFrame::switchFrame);
}

HeaderFrame::~HeaderFrame() {}

void HeaderFrame::togglePauseIcon(bool toggle)
{
  Q_ASSERT(pimpl);
  Q_ASSERT(pimpl->pauseButton);

  if (toggle)
  {
    const auto resumeIcon = QIcon{":/images/resumeIcon.png"};
    pimpl->pauseButton->setIcon(resumeIcon);
    pimpl->pauseButton->setText(" Resume");
  }
  else
  {
    const auto pauseIcon = QIcon{":/images/pauseIcon.png"};
    pimpl->pauseButton->setIcon(pauseIcon);
    pimpl->pauseButton->setText(" Pause");
  }
}

void HeaderFrame::setIconSize(const QSize& iconSize)
{
  Q_ASSERT(pimpl);
  Q_ASSERT(pimpl->pauseButton);
  Q_ASSERT(pimpl->addFilesButton);
  Q_ASSERT(pimpl->clearFilesButton);

  pimpl->iconSize = iconSize;

  pimpl->pauseButton->setIconSize(pimpl->iconSize);
  pimpl->addFilesButton->setIconSize(pimpl->iconSize);
  pimpl->clearFilesButton->setIconSize(pimpl->iconSize);
}

HeaderFrame::HeaderFramePrivate::HeaderFramePrivate() :
  pauseButton{nullptr}, addFilesButton{nullptr}, clearFilesButton{nullptr} {}
