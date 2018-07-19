#include "gui/PasswordFrame.hpp"
#include <QLabel>
#include <QLineEdit>
#include <QHBoxLayout>

class Kryvo::PasswordFramePrivate {
  Q_DISABLE_COPY(PasswordFramePrivate)

 public:
  QLineEdit* passwordLineEdit{nullptr};
};

Kryvo::PasswordFrame::PasswordFrame(QWidget* parent)
  : QFrame(parent), d_ptr(std::make_unique<PasswordFramePrivate>()) {
  Q_D(PasswordFrame);

  auto passwordLabel = new QLabel(tr("Password "), this);
  passwordLabel->setObjectName(QStringLiteral("text"));

  d->passwordLineEdit = new QLineEdit(this);
  d->passwordLineEdit->setObjectName(QStringLiteral("passwordLineEdit"));
  d->passwordLineEdit->setEchoMode(QLineEdit::PasswordEchoOnEdit);

  auto passwordLayout = new QHBoxLayout(this);
  passwordLayout->addWidget(passwordLabel);
  passwordLayout->addWidget(d->passwordLineEdit);
  passwordLayout->setContentsMargins(0, 0, 0, 0);
}

Kryvo::PasswordFrame::~PasswordFrame() = default;

QString Kryvo::PasswordFrame::password() const {
  Q_D(const PasswordFrame);
  Q_ASSERT(d->passwordLineEdit);

  return d->passwordLineEdit->text();
}
