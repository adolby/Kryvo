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

#include "gui/MessageFrame.hpp"
#include "utility/make_unique.h"
#include <QtWidgets/QPlainTextEdit>
#include <QtWidgets/QHBoxLayout>

class MessageFrame::MessageFramePrivate {
 public:
  /*!
   * \brief MessageFramePrivate Constructs the MessageFrame private
   * implementation.
   */
  explicit MessageFramePrivate();

  QPlainTextEdit* messageTextEdit;
};

MessageFrame::MessageFrame(QWidget* parent) :
  QFrame{parent}, pimpl{make_unique<MessageFramePrivate>()}
{
  pimpl->messageTextEdit = new QPlainTextEdit{tr("To add files, click the add"
                                                 " files button or drag and"
                                                 " drop files."), this};
  pimpl->messageTextEdit->setObjectName("message");
  pimpl->messageTextEdit->setReadOnly(true);
  pimpl->messageTextEdit->setTextInteractionFlags(Qt::NoTextInteraction);
  pimpl->messageTextEdit->viewport()->setCursor(Qt::ArrowCursor);
  pimpl->messageTextEdit->setSizePolicy(QSizePolicy::Expanding,
                                        QSizePolicy::Preferred);

  auto messageLayout = new QHBoxLayout{this};
  messageLayout->addWidget(pimpl->messageTextEdit);
  messageLayout->setContentsMargins(2, 2, 2, 2);
  messageLayout->setSpacing(0);
}

MessageFrame::~MessageFrame() {}

void MessageFrame::appendPlainText(const QString& message)
{
  Q_ASSERT(pimpl);
  Q_ASSERT(pimpl->messageTextEdit);

  pimpl->messageTextEdit->appendPlainText(message);
}

MessageFrame::MessageFramePrivate::MessageFramePrivate() :
  messageTextEdit{nullptr}
{}