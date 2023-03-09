#include "OutputFrame.hpp"
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QHBoxLayout>

namespace Kryvo {

class OutputFramePrivate {
  Q_DISABLE_COPY(OutputFramePrivate)

 public:
  OutputFramePrivate();

  QLineEdit* outputLineEdit{nullptr};
  QPushButton* outputPushButton{nullptr};
};

OutputFramePrivate::OutputFramePrivate() = default;

OutputFrame::OutputFrame(QWidget* parent)
  : QFrame(parent), d_ptr(std::make_unique<OutputFramePrivate>()) {
  Q_D(OutputFrame);

  auto outputLabel = new QLabel(tr("Output path "), this);
  outputLabel->setObjectName(QStringLiteral("text"));

  d->outputLineEdit = new QLineEdit(this);
  d->outputLineEdit->setObjectName(QStringLiteral("outputLineEdit"));

  d->outputPushButton = new QPushButton(this);
  d->outputPushButton->setText(QStringLiteral("Select Folder"));
  d->outputPushButton->setObjectName(QStringLiteral("outputButton"));
  connect(d->outputPushButton, &QPushButton::clicked,
          this, &OutputFrame::selectOutputDir);

  auto outputLayout = new QHBoxLayout(this);
  outputLayout->addWidget(outputLabel);
  outputLayout->addWidget(d->outputLineEdit);
  outputLayout->addWidget(d->outputPushButton);
  outputLayout->setContentsMargins(0, 0, 0, 0);
}

OutputFrame::~OutputFrame() = default;

void OutputFrame::outputPath(const QString& path) {
  Q_D(OutputFrame);
  Q_ASSERT(d->outputLineEdit);

  d->outputLineEdit->setText(path);
}

QString OutputFrame::outputPath() const {
  Q_D(const OutputFrame);
  Q_ASSERT(d->outputLineEdit);

  return d->outputLineEdit->text();
}

} // namespace Kryvo
