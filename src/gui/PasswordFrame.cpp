#include "src/gui/PasswordFrame.hpp"
#include "src/utility/pimpl_impl.h"
#include <QLabel>
#include <QLineEdit>
#include <QHBoxLayout>

class Kryvos::PasswordFrame::PasswordFramePrivate {
 public:
  /*!
   * \brief PasswordFramePrivate Constructs the password frame private
   * implementation.
   */
  PasswordFramePrivate();

  QLineEdit* passwordLineEdit;
};

Kryvos::PasswordFrame::PasswordFrame(QWidget* parent)
  : QFrame{parent} {
  auto passwordLabel = new QLabel{tr("Password "), this};
  passwordLabel->setObjectName(QStringLiteral("text"));

  m->passwordLineEdit = new QLineEdit{this};
  m->passwordLineEdit->setParent(this);
  m->passwordLineEdit->setObjectName(QStringLiteral("passwordLineEdit"));
  m->passwordLineEdit->setEchoMode(QLineEdit::PasswordEchoOnEdit);

  auto passwordLayout = new QHBoxLayout{this};
  passwordLayout->addWidget(passwordLabel);
  passwordLayout->addWidget(m->passwordLineEdit);
  passwordLayout->setContentsMargins(0, 0, 0, 0);
}

Kryvos::PasswordFrame::~PasswordFrame() {
}

QString Kryvos::PasswordFrame::password() const {
  Q_ASSERT(m->passwordLineEdit);

  const auto& password = m->passwordLineEdit->text();

  return password;
}

Kryvos::PasswordFrame::PasswordFramePrivate::PasswordFramePrivate()
  : passwordLineEdit{nullptr} {
}
