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

#include "gui/HeaderFrame.hpp"
#include "gui/FluidLayout.hpp"
#include "utility/pimpl_impl.h"
#include "utility/make_unique.h"
#include <QPushButton>
#include <QLabel>
#include <QHBoxLayout>
#include <QIcon>

class HeaderFrame::HeaderFramePrivate {
 public:
  /*!
   * \brief HeaderFramePrivate Constructs the HeaderFrame private
   * implementation.
   */
  HeaderFramePrivate();

  QPushButton* pauseButton;
  QPushButton* addFilesButton;
  QPushButton* clearFilesButton;
  QPushButton* settingsButton;

  QSize iconSize;
};

HeaderFrame::HeaderFrame(QWidget* parent)
  : QFrame{parent}
{
  auto headerLabel = new QLabel{tr("Kryvos"), this};
  headerLabel->setObjectName(QStringLiteral("headerText"));

  auto buttonFrame = new QFrame{this};
  buttonFrame->setObjectName(QStringLiteral("coloredFrame"));

  const auto pauseIcon = QIcon{QStringLiteral(":/images/pauseIcon.png")};
  m->pauseButton = new QPushButton{pauseIcon, tr(" Pause"), this};
  m->pauseButton->setObjectName(QStringLiteral("pauseButton"));
  m->pauseButton->setCheckable(true);
  m->pauseButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

  const auto addFilesIcon = QIcon{QStringLiteral(":/images/addFilesIcon.png")};
  m->addFilesButton = new QPushButton{addFilesIcon,
                                      tr(" Add files"),
                                      this};
  m->addFilesButton->setObjectName(QStringLiteral("addButton"));
  m->addFilesButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

  const auto clearFilesIcon =
        QIcon{QStringLiteral(":/images/clearFilesIcon.png")};
  m->clearFilesButton = new QPushButton{clearFilesIcon,
                                        tr(" Remove all files"),
                                        this};
  m->clearFilesButton->setObjectName(QStringLiteral("clearButton"));
  m->clearFilesButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

  const auto settingsIcon = QIcon{QStringLiteral(":/images/gearIcon.png")};
  m->settingsButton = new QPushButton{settingsIcon,
                                      tr(" Settings"),
                                      this};
  m->settingsButton->setObjectName(QStringLiteral("settingsButton"));
  m->settingsButton->setSizePolicy(QSizePolicy::Fixed,
                                   QSizePolicy::Fixed);

  auto buttonLayout = new FluidLayout{buttonFrame};
  buttonLayout->addWidget(m->pauseButton);
  buttonLayout->addWidget(m->addFilesButton);
  buttonLayout->addWidget(m->clearFilesButton);
  buttonLayout->addWidget(m->settingsButton);

  auto headerLayout = new QHBoxLayout{this};
  headerLayout->addWidget(headerLabel, 0);
  headerLayout->addSpacing(20);
  headerLayout->addWidget(buttonFrame, 1);
  headerLayout->setContentsMargins(10, 0, 0, 0);

  connect(m->pauseButton, &QPushButton::toggled,
          this, &HeaderFrame::pause);
  connect(m->pauseButton, &QPushButton::toggled,
          this, &HeaderFrame::togglePauseIcon);
  connect(m->addFilesButton, &QPushButton::clicked,
          this, &HeaderFrame::addFiles);
  connect(m->clearFilesButton, &QPushButton::clicked,
          this, &HeaderFrame::removeFiles);
  connect(m->settingsButton, &QPushButton::clicked,
          this, &HeaderFrame::switchFrame);
}

HeaderFrame::~HeaderFrame() {}

void HeaderFrame::togglePauseIcon(const bool toggle)
{
  Q_ASSERT(m->pauseButton);

  if (toggle)
  {
    const auto resumeIcon = QIcon{QStringLiteral(":/images/resumeIcon.png")};
    m->pauseButton->setIcon(resumeIcon);
    m->pauseButton->setText(tr(" Resume"));
  }
  else
  {
    const auto pauseIcon = QIcon{QStringLiteral(":/images/pauseIcon.png")};
    m->pauseButton->setIcon(pauseIcon);
    m->pauseButton->setText(tr(" Pause"));
  }
}

void HeaderFrame::setIconSize(const QSize& iconSize)
{
  Q_ASSERT(m->pauseButton);
  Q_ASSERT(m->addFilesButton);
  Q_ASSERT(m->clearFilesButton);

  m->iconSize = iconSize;

  m->pauseButton->setIconSize(m->iconSize);
  m->addFilesButton->setIconSize(m->iconSize);
  m->clearFilesButton->setIconSize(m->iconSize);
}

HeaderFrame::HeaderFramePrivate::HeaderFramePrivate()
  : pauseButton{nullptr}, addFilesButton{nullptr}, clearFilesButton{nullptr}
{}
