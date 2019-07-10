#include "OutputFrame.hpp"
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QHBoxLayout>

class Kryvo::OutputFramePrivate {
  Q_DISABLE_COPY(OutputFramePrivate)

 public:
  OutputFramePrivate();

  QLineEdit* outputLineEdit{nullptr};
  QPushButton* outputPushButton{nullptr};
};

Kryvo::OutputFramePrivate::OutputFramePrivate() = default;

Kryvo::OutputFrame::OutputFrame(QWidget* parent)
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
