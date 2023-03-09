#include "ProgressFrame.hpp"
#include "ElidedLabel.hpp"
#include <QProgressBar>
#include <QLabel>
#include <QHBoxLayout>
#include <QString>

namespace Kryvo {

class ProgressFramePrivate {
  Q_DISABLE_COPY(ProgressFramePrivate)

 public:
  /*!
   * \brief MessageFramePrivate Constructs the MessageFrame private
   * implementation.
   */
  ProgressFramePrivate();

  ElidedLabel* progressTaskLabel{nullptr};
  QProgressBar* progressBar{nullptr};
};

ProgressFramePrivate::ProgressFramePrivate() = default;

ProgressFrame::ProgressFrame(QWidget* parent)
  : QFrame(parent), d_ptr(std::make_unique<ProgressFramePrivate>()) {
  Q_D(ProgressFrame);

  d->progressTaskLabel = new ElidedLabel(tr("Archive progress"), this);
  d->progressTaskLabel->setElideMode(Qt::ElideMiddle);
  d->progressTaskLabel->setObjectName(QStringLiteral("progressLabel"));

  d->progressBar = new QProgressBar(this);
  d->progressBar->setRange(0, 100);
  d->progressBar->setValue(0);

  auto progressLayout = new QHBoxLayout(this);
  progressLayout->addWidget(d->progressTaskLabel, 3);
  progressLayout->addWidget(d->progressBar, 2);
  progressLayout->setContentsMargins(5, 5, 5, 5);
}

ProgressFrame::~ProgressFrame() = default;

void ProgressFrame::updateTask(const QString& task,
                               const int percentProgress) {
  Q_D(ProgressFrame);
  Q_ASSERT(d->progressTaskLabel);
  Q_ASSERT(d->progressBar);

  if (task != d->progressTaskLabel->text()) {
    d->progressTaskLabel->setText(task);
  }

  const bool visibleStatus = (100 != percentProgress);

  this->setVisible(visibleStatus);

  d->progressBar->setValue(percentProgress);
}

} // namespace Kryvo
