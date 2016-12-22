#include "src/gui/MessageFrame.hpp"
#include "src/utility/pimpl_impl.h"
#include <QToolButton>
#include <QLabel>
#include <QHBoxLayout>
#include <QAction>
#include <QIcon>
#include <QString>
#include <QVector>
#include <iterator>

class Kryvos::MessageFrame::MessageFramePrivate {
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

Kryvos::MessageFrame::MessageFrame(QWidget* parent)
  : QFrame{parent} {
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
  messageLayout->setContentsMargins(6, 5, 6, 5);
  messageLayout->setSpacing(8);
}

Kryvos::MessageFrame::~MessageFrame() {
}

void Kryvos::MessageFrame::appendText(const QString& message) {
  Q_ASSERT(m->messageLabel);

  m->messages.append(message);
  m->it = std::prev(m->messages.constEnd(), 1);
  m->messageLabel->setText(*m->it);
}

void Kryvos::MessageFrame::pageLeft() {
  Q_ASSERT(m->messageLabel);

  if (m->it != m->messages.constBegin()) {
    m->it = std::prev(m->it, 1);
    m->messageLabel->setText(*m->it);
  }
}

void Kryvos::MessageFrame::pageRight()
{
  Q_ASSERT(m->messageLabel);

  const auto end = m->messages.constEnd();

  if (m->it != std::prev(end, 1) && m->it != end) {
    m->it = std::next(m->it, 1);
    m->messageLabel->setText(*m->it);
  }
}

Kryvos::MessageFrame::MessageFramePrivate::MessageFramePrivate()
  : messageLabel{nullptr} {
}
