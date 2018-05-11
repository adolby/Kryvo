#include "gui/MessageFrame.hpp"
#include <QToolButton>
#include <QLabel>
#include <QHBoxLayout>
#include <QAction>
#include <QIcon>
#include <QString>
#include <QVector>
#include <iterator>

class Kryvo::MessageFramePrivate {
  Q_DISABLE_COPY(MessageFramePrivate)

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

Kryvo::MessageFrame::MessageFrame(QWidget* parent)
  : QFrame{parent}, d_ptr{std::make_unique<MessageFramePrivate>()} {
  Q_D(MessageFrame);

  auto leftAction = new QAction{this};
  connect(leftAction, &QAction::triggered, this, &MessageFrame::pageLeft);
  const QIcon leftIcon{QStringLiteral(":/images/leftArrowIcon.png")};
  leftAction->setIcon(leftIcon);
  auto leftButton = new QToolButton{this};
  leftButton->setObjectName(QStringLiteral("messageNavButton"));
  leftButton->setDefaultAction(leftAction);

  d->messageLabel = new QLabel{this};
  d->messageLabel->setObjectName(QStringLiteral("message"));
  d->messageLabel->setWordWrap(true);

  auto rightAction = new QAction{this};
  connect(rightAction, &QAction::triggered, this, &MessageFrame::pageRight);
  const QIcon rightIcon{QStringLiteral(":/images/rightArrowIcon.png")};
  rightAction->setIcon(rightIcon);
  auto rightButton = new QToolButton{this};
  rightButton->setObjectName(QStringLiteral("messageNavButton"));
  rightButton->setDefaultAction(rightAction);

  auto messageLayout = new QHBoxLayout{this};
  messageLayout->addWidget(leftButton, 1);
  messageLayout->addWidget(d->messageLabel, 20);
  messageLayout->addWidget(rightButton, 1);
  messageLayout->setContentsMargins(6, 5, 6, 5);
  messageLayout->setSpacing(8);
}

Kryvo::MessageFrame::~MessageFrame() {
}

void Kryvo::MessageFrame::appendText(const QString& message) {
  Q_D(MessageFrame);
  Q_ASSERT(d->messageLabel);

  d->messages.append(message);
  d->it = std::prev(d->messages.constEnd(), 1);
  d->messageLabel->setText(*d->it);
}

void Kryvo::MessageFrame::pageLeft() {
  Q_D(MessageFrame);
  Q_ASSERT(d->messageLabel);

  if (d->it != d->messages.constBegin()) {
    d->it = std::prev(d->it, 1);
    d->messageLabel->setText(*d->it);
  }
}

void Kryvo::MessageFrame::pageRight()
{
  Q_D(MessageFrame);
  Q_ASSERT(d->messageLabel);

  const auto end = d->messages.constEnd();

  if (d->it != std::prev(end, 1) && d->it != end) {
    d->it = std::next(d->it, 1);
    d->messageLabel->setText(*d->it);
  }
}

Kryvo::MessageFramePrivate::MessageFramePrivate()
  : messageLabel{nullptr} {
}
