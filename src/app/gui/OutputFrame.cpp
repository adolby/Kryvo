#include "gui/OutputFrame.hpp"
#include <QLabel>
#include <QLineEdit>
#include <QHBoxLayout>

class Kryvo::OutputFramePrivate {
  Q_DISABLE_COPY(OutputFramePrivate)

 public:
  QLineEdit* outputLineEdit{nullptr};
};

Kryvo::OutputFrame::OutputFrame(QWidget* parent)
  : QFrame(parent), d_ptr(std::make_unique<OutputFramePrivate>()) {
  Q_D(OutputFrame);

  auto outputLabel = new QLabel(tr("Output path "), this);
  outputLabel->setObjectName(QStringLiteral("text"));

  d->outputLineEdit = new QLineEdit(this);
  d->outputLineEdit->setObjectName(QStringLiteral("outputLineEdit"));

  auto outputLayout = new QHBoxLayout(this);
  outputLayout->addWidget(outputLabel);
  outputLayout->addWidget(d->outputLineEdit);
  outputLayout->setContentsMargins(0, 0, 0, 0);
}

Kryvo::OutputFrame::~OutputFrame() = default;

void Kryvo::OutputFrame::outputPath(const QString& path) {
  Q_D(OutputFrame);
  Q_ASSERT(d->outputLineEdit);

  d->outputLineEdit->setText(path);
}

QString Kryvo::OutputFrame::outputPath() const {
  Q_D(const OutputFrame);
  Q_ASSERT(d->outputLineEdit);

  return d->outputLineEdit->text();
}
