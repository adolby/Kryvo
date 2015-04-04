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

#include "gui/PasswordFrame.hpp"
#include "utility/make_unique.h"
#include <QtWidgets/QLabel>
#include <QtWidgets/QHBoxLayout>

class PasswordFrame::PasswordFramePrivate {
 public:
  /*!
   * \brief PasswordFramePrivate Constructs the password frame private
   * implementation.
   */
  explicit PasswordFramePrivate();

  QLineEdit* passwordLineEdit;
};

PasswordFrame::PasswordFrame(QWidget* parent)
  : QFrame{parent}, pimpl{make_unique<PasswordFramePrivate>()}
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

  connect(pimpl->passwordLineEdit, &QLineEdit::editingFinished,
          this, &PasswordFrame::editingFinished);
}

PasswordFrame::~PasswordFrame() {}

QString PasswordFrame::password() const
{
  Q_ASSERT(pimpl);
  Q_ASSERT(pimpl->passwordLineEdit);

  auto password = pimpl->passwordLineEdit->text();

  return password;
}

PasswordFrame::PasswordFramePrivate::PasswordFramePrivate()
  : passwordLineEdit{nullptr} {}
