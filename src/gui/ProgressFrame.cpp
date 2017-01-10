#include "src/gui/ProgressFrame.hpp"
#include "src/utility/pimpl_impl.h"
#include <QProgressBar>
#include <QLabel>
#include <QHBoxLayout>
#include <QString>

class Kryvos::ProgressFrame::ProgressFramePrivate {
 public:
  /*!
   * \brief MessageFramePrivate Constructs the MessageFrame private
   * implementation.
   */
  ProgressFramePrivate();

  QLabel* progressTaskLabel;
  QProgressBar* progressBar;
};

Kryvos::ProgressFrame::ProgressFrame(QWidget* parent)
  : QFrame{parent} {
  m->progressTaskLabel = new QLabel{this};
  m->progressTaskLabel->setObjectName(QStringLiteral("progressLabel"));
  m->progressTaskLabel->setText(QStringLiteral("Creating archive"));

  m->progressBar = new QProgressBar{this};
  m->progressBar->setRange(0, 100);
  m->progressBar->setValue(0);

  auto progressLayout = new QHBoxLayout{this};
  progressLayout->addWidget(m->progressTaskLabel, 1);
  progressLayout->addWidget(m->progressBar, 3);
  progressLayout->setContentsMargins(20, 5, 20, 5);
}

Kryvos::ProgressFrame::~ProgressFrame() {
}

void Kryvos::ProgressFrame::updateTask(const QString& task,
                                       const int percentProgress) {
  Q_ASSERT(m->progressTaskLabel);
  Q_ASSERT(m->progressBar);

  const auto visibleStatus = (100 == percentProgress) ? false : true;

  this->setVisible(visibleStatus);

  if (task != m->progressTaskLabel->text()) {
    m->progressTaskLabel->setText(task);
  }

  m->progressBar->setValue(percentProgress);
}

Kryvos::ProgressFrame::ProgressFramePrivate::ProgressFramePrivate()
  : progressTaskLabel{nullptr}, progressBar{nullptr} {
}
