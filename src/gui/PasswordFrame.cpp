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

#include "gui/PasswordFrame.hpp"
#include "utility/make_unique.h"
#include <QtWidgets/QLabel>
#include <QtWidgets/QHBoxLayout>

class PasswordFrame::PasswordFramePrivate {
 public:
  explicit PasswordFramePrivate();

  QLineEdit* passwordLineEdit;
};

PasswordFrame::PasswordFrame(QWidget* parent) :
  QFrame{parent}, pimpl{make_unique<PasswordFramePrivate>()}
{
  auto passwordLabel = new QLabel{tr("Password "), this};
  passwordLabel->setObjectName("text");

  pimpl->passwordLineEdit = new QLineEdit{this};
  pimpl->passwordLineEdit->setParent(this);
  pimpl->passwordLineEdit->setObjectName("passwordLineEdit");
  pimpl->passwordLineEdit->setEchoMode(QLineEdit::PasswordEchoOnEdit);

  auto passwordLayout = new QHBoxLayout{this};
  passwordLayout->addWidget(passwordLabel);
  passwordLayout->addWidget(pimpl->passwordLineEdit);
}

PasswordFrame::~PasswordFrame() {}

QLineEdit* PasswordFrame::passwordLineEdit()
{
  Q_ASSERT(pimpl);
  Q_ASSERT(pimpl->passwordLineEdit);

  return pimpl->passwordLineEdit;
}

PasswordFrame::PasswordFramePrivate::PasswordFramePrivate() :
  passwordLineEdit{nullptr} {}
