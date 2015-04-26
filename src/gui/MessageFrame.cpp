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
#include <QtWidgets/QScroller>
#include <QtWidgets/QPlainTextEdit>
#include <QtWidgets/QHBoxLayout>

class MessageFrame::MessageFramePrivate {
 public:
  /*!
   * \brief MessageFramePrivate Constructs the MessageFrame private
   * implementation.
   */
  MessageFramePrivate();

  QPlainTextEdit* messageTextEdit;
};

MessageFrame::MessageFrame(QWidget* parent)
  : QFrame{parent}
{
  m->messageTextEdit = new QPlainTextEdit{tr("To add files, click the add "
                                             "files button or drag and "
                                             "drop files."), this};
  m->messageTextEdit->setObjectName(QStringLiteral("message"));
  m->messageTextEdit->setMaximumBlockCount(10);
  m->messageTextEdit->setReadOnly(true);
  m->messageTextEdit->setTextInteractionFlags(Qt::NoTextInteraction);
  m->messageTextEdit->viewport()->setCursor(Qt::ArrowCursor);
  m->messageTextEdit->setSizePolicy(QSizePolicy::Expanding,
                                        QSizePolicy::Preferred);

  // Attach a scroller to the message text edit
  QScroller::grabGesture(m->messageTextEdit, QScroller::TouchGesture);

  // Disable overshoot; it makes interacting with small widgets harder
  QScroller* scroller = QScroller::scroller(m->messageTextEdit);

  QScrollerProperties properties = scroller->scrollerProperties();

  QVariant overshootPolicy = QVariant::fromValue
                              <QScrollerProperties::OvershootPolicy>
                              (QScrollerProperties::OvershootAlwaysOff);

  properties.setScrollMetric(QScrollerProperties::HorizontalOvershootPolicy,
                             overshootPolicy);
  properties.setScrollMetric(QScrollerProperties::VerticalOvershootPolicy,
                             overshootPolicy);

  scroller->setScrollerProperties(properties);

  auto messageLayout = new QHBoxLayout{this};
  messageLayout->addWidget(m->messageTextEdit);
  messageLayout->setContentsMargins(2, 2, 2, 2);
  messageLayout->setSpacing(0);
}

MessageFrame::~MessageFrame() {}

void MessageFrame::appendPlainText(const QString& message)
{
  Q_ASSERT(m->messageTextEdit);

  m->messageTextEdit->appendPlainText(message);
}

void MessageFrame::setText(const QString& startText)
{
  Q_ASSERT(m->messageTextEdit);

  m->messageTextEdit->setPlainText(startText);
}

MessageFrame::MessageFramePrivate::MessageFramePrivate()
  : messageTextEdit{nullptr}
{}
