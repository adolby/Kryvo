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

#include "gui/MessageFrame.hpp"
#include "utility/pimpl_impl.h"
#include <QToolButton>
#include <QLabel>
#include <QHBoxLayout>
#include <QAction>
#include <QIcon>
#include <QString>
#include <QVector>
#include <iterator>

class MessageFrame::MessageFramePrivate {
 public:
  /*!
   * \brief MessageFramePrivate Constructs the MessageFrame private
   * implementation.
   */
  MessageFramePrivate();

  QVector<QString> messages;
  QVector<QString>::ConstIterator it;
  QLabel* messageLabel;
};

MessageFrame::MessageFrame(QWidget* parent)
  : QFrame{parent}
{
  auto leftAction = new QAction{this};
  connect(leftAction, &QAction::triggered, this, &MessageFrame::pageLeft);
  const auto leftIcon = QIcon{QStringLiteral(":/images/leftArrowIcon.png")};
  leftAction->setIcon(leftIcon);
  auto leftButton = new QToolButton{this};
  leftButton->setObjectName(QStringLiteral("messageNavButton"));
  leftButton->setDefaultAction(leftAction);

  m->messageLabel = new QLabel{this};
  m->messageLabel->setObjectName(QStringLiteral("message"));
  m->messageLabel->setWordWrap(true);

  auto rightAction = new QAction{this};
  connect(rightAction, &QAction::triggered, this, &MessageFrame::pageRight);
  const auto rightIcon = QIcon{QStringLiteral(":/images/rightArrowIcon.png")};
  rightAction->setIcon(rightIcon);
  auto rightButton = new QToolButton{this};
  rightButton->setObjectName(QStringLiteral("messageNavButton"));
  rightButton->setDefaultAction(rightAction);

  auto messageLayout = new QHBoxLayout{this};
  messageLayout->addWidget(leftButton, 1);
  messageLayout->addWidget(m->messageLabel, 20);
  messageLayout->addWidget(rightButton, 1);
  messageLayout->setContentsMargins(10, 2, 10, 2);
  messageLayout->setSpacing(5);
}

MessageFrame::~MessageFrame() {}

void MessageFrame::appendText(const QString& message)
{
  Q_ASSERT(m->messageLabel);

  m->messages.append(message);
  m->it = std::prev(m->messages.constEnd(), 1);
  m->messageLabel->setText(*m->it);
}

void MessageFrame::pageLeft()
{
  Q_ASSERT(m->messageLabel);

  if (m->it != m->messages.constBegin())
  {
    m->it = std::prev(m->it, 1);
    m->messageLabel->setText(*m->it);
  }
}

void MessageFrame::pageRight()
{
  Q_ASSERT(m->messageLabel);

  const auto end = m->messages.constEnd();

  if (m->it != std::prev(end, 1) && m->it != end)
  {
    m->it = std::next(m->it, 1);
    m->messageLabel->setText(*m->it);
  }
}

MessageFrame::MessageFramePrivate::MessageFramePrivate()
  : messageLabel{nullptr}
{}
