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

#include "HeaderFrame.hpp"
#include <QtWidgets/QPushButton>
#include <QtWidgets/QLabel>
#include <QtWidgets/QHBoxLayout>
#include <QtGui/QIcon>

class HeaderFrame::HeaderFramePrivate {
 public:
  explicit HeaderFramePrivate();

  QPushButton* pauseButton;
};

HeaderFrame::HeaderFrame(QWidget* parent) :
  QFrame{parent}, pimpl{new HeaderFramePrivate{}}
{
  QLabel* headerLabel = new QLabel{tr("Kryvos"), this};
  headerLabel->setObjectName("headerText");

  const QIcon pauseIcon(":/images/pauseIcon.png");
  pimpl->pauseButton = new QPushButton{pauseIcon, tr(" Pause"), this};
  pimpl->pauseButton->setObjectName("pauseButton");
  pimpl->pauseButton->setCheckable(true);
  pimpl->pauseButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

  const QIcon addFilesIcon(":/images/addFilesIcon.png");
  QPushButton* addFilesButton = new QPushButton{addFilesIcon,
                                                tr(" Add files"),
                                                this};
  addFilesButton->setObjectName("addButton");
  addFilesButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

  const QIcon clearFilesIcon(":/images/clearFilesIcon.png");
  QPushButton* clearFilesButton = new QPushButton{clearFilesIcon,
                                                  tr(" Remove all files"),
                                                  this};
  clearFilesButton->setObjectName("clearButton");
  clearFilesButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

  QHBoxLayout* headerLayout = new QHBoxLayout{this};
  headerLayout->addWidget(headerLabel);
  headerLayout->addWidget(pimpl->pauseButton);
  headerLayout->addWidget(addFilesButton);
  headerLayout->addWidget(clearFilesButton);

  connect(addFilesButton, &QPushButton::clicked,
          this, &HeaderFrame::addFiles);
  connect(clearFilesButton, &QPushButton::clicked,
          this, &HeaderFrame::removeFiles);
  connect(pimpl->pauseButton, &QPushButton::toggled,
          this, &HeaderFrame::pause);
  connect(pimpl->pauseButton, &QPushButton::toggled,
          this, &HeaderFrame::togglePauseIcon);
}

HeaderFrame::~HeaderFrame() {}

void HeaderFrame::togglePauseIcon(bool toggle)
{
  if (toggle)
  {
    const QIcon resumeIcon{":/images/resumeIcon.png"};
    pimpl->pauseButton->setIcon(resumeIcon);
    pimpl->pauseButton->setText(" Resume");
  }
  else
  {
    const QIcon pauseIcon(":/images/pauseIcon.png");
    pimpl->pauseButton->setIcon(pauseIcon);
    pimpl->pauseButton->setText(" Pause");
  }
}

HeaderFrame::HeaderFramePrivate::HeaderFramePrivate() :
  pauseButton{nullptr} {}
