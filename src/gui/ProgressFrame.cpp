#include "src/gui/ProgressFrame.hpp"
#include "src/gui/ElidedLabel.hpp"
#include "src/utility/pimpl_impl.h"
#include <QProgressBar>
#include <QLabel>
#include <QHBoxLayout>
#include <QString>

#include <QDebug>

class Kryvos::ProgressFrame::ProgressFramePrivate {
 public:
  /*!
   * \brief MessageFramePrivate Constructs the MessageFrame private
   * implementation.
   */
  ProgressFramePrivate();

  ElidedLabel* progressTaskLabel;
  QProgressBar* progressBar;
};

Kryvos::ProgressFrame::ProgressFrame(QWidget* parent)
  : QFrame{parent} {
  m->progressTaskLabel = new ElidedLabel{tr("Archive progress"), this};
  m->progressTaskLabel->setElideMode(Qt::ElideMiddle);
  m->progressTaskLabel->setObjectName(QStringLiteral("progressLabel"));

  m->progressBar = new QProgressBar{this};
  m->progressBar->setRange(0, 100);
  m->progressBar->setValue(0);

  auto progressLayout = new QHBoxLayout{this};
  progressLayout->addWidget(m->progressTaskLabel, 3);
  progressLayout->addWidget(m->progressBar, 2);
  progressLayout->setContentsMargins(5, 5, 5, 5);
}

Kryvos::ProgressFrame::~ProgressFrame() {
}

void Kryvos::ProgressFrame::updateTask(const QString& task,
                                       const int percentProgress) {
  Q_ASSERT(m->progressTaskLabel);
  Q_ASSERT(m->progressBar);

  if (task != m->progressTaskLabel->text()) {
    m->progressTaskLabel->setText(task);
  }

  const bool visibleStatus = (100 == percentProgress) ? false : true;

  this->setVisible(visibleStatus);

  m->progressBar->setValue(percentProgress);
}

Kryvos::ProgressFrame::ProgressFramePrivate::ProgressFramePrivate()
  : progressTaskLabel{nullptr}, progressBar{nullptr} {
}
