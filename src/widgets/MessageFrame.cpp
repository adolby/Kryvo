#include "MessageFrame.hpp"
#include <QToolButton>
#include <QLabel>
#include <QHBoxLayout>
#include <QAction>
#include <QIcon>
#include <QString>
#include <iterator>
#include <vector>

class Kryvo::MessageFramePrivate {
  Q_DISABLE_COPY(MessageFramePrivate)

 public:
  MessageFramePrivate();

  std::vector<QString> messages;
  std::vector<QString>::const_iterator messageIterator;
  QLabel* messageLabel{nullptr};
};

Kryvo::MessageFramePrivate::MessageFramePrivate() = default;

Kryvo::MessageFrame::MessageFrame(QWidget* parent)
  : QFrame(parent), d_ptr(std::make_unique<MessageFramePrivate>()) {
  Q_D(MessageFrame);

  auto leftAction = new QAction(this);
  connect(leftAction, &QAction::triggered, this, &MessageFrame::pageLeft);
  leftAction->setIcon(QIcon(QStringLiteral(":/images/leftArrowIcon.png")));
  auto leftButton = new QToolButton(this);
  leftButton->setObjectName(QStringLiteral("messageNavButton"));
  leftButton->setDefaultAction(leftAction);

  d->messageLabel = new QLabel(this);
  d->messageLabel->setObjectName(QStringLiteral("message"));
  d->messageLabel->setWordWrap(true);

  auto rightAction = new QAction(this);
  connect(rightAction, &QAction::triggered, this, &MessageFrame::pageRight);
  rightAction->setIcon(QIcon(QStringLiteral(":/images/rightArrowIcon.png")));
  auto rightButton = new QToolButton(this);
  rightButton->setObjectName(QStringLiteral("messageNavButton"));
  rightButton->setDefaultAction(rightAction);

  auto messageLayout = new QHBoxLayout(this);
  messageLayout->addWidget(leftButton, 1);
  messageLayout->addWidget(d->messageLabel, 20);
  messageLayout->addWidget(rightButton, 1);
  messageLayout->setContentsMargins(6, 5, 6, 5);
  messageLayout->setSpacing(8);
}

Kryvo::MessageFrame::~MessageFrame() = default;

void Kryvo::MessageFrame::appendMessage(const QString& message) {
  Q_D(MessageFrame);
  Q_ASSERT(d->messageLabel);

  d->messages.push_back(message);
  d->messageIterator = std::prev(d->messages.end(), 1);
  d->messageLabel->setText(*d->messageIterator);
}

void Kryvo::MessageFrame::pageLeft() {
  Q_D(MessageFrame);
  Q_ASSERT(d->messageLabel);

  if (d->messageIterator != d->messages.begin()) {
    d->messageIterator = std::prev(d->messageIterator, 1);
    d->messageLabel->setText(*d->messageIterator);
  }
}

void Kryvo::MessageFrame::pageRight() {
  Q_D(MessageFrame);
  Q_ASSERT(d->messageLabel);

  const auto end = d->messages.end();

  if (d->messageIterator != std::prev(end, 1) && d->messageIterator != end) {
    d->messageIterator = std::next(d->messageIterator, 1);
    d->messageLabel->setText(*d->messageIterator);
  }
}
