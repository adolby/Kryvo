#include "src/gui/OutputFrame.hpp"
#include "src/utility/pimpl_impl.h"
#include <QLabel>
#include <QLineEdit>
#include <QHBoxLayout>

class Kryvos::OutputFrame::OutputFramePrivate {
 public:
  /*!
   * \brief OutputFramePrivate Constructs the output frame private
   * implementation.
   */
  OutputFramePrivate();

  QLineEdit* outputLineEdit;
};

Kryvos::OutputFrame::OutputFrame(QWidget* parent)
  : QFrame{parent} {
  auto outputLabel = new QLabel{tr("Output path "), this};
  outputLabel->setObjectName(QStringLiteral("text"));

  m->outputLineEdit = new QLineEdit{this};
  m->outputLineEdit->setParent(this);
  m->outputLineEdit->setObjectName(QStringLiteral("outputLineEdit"));

  auto outputLayout = new QHBoxLayout{this};
  outputLayout->addWidget(outputLabel);
  outputLayout->addWidget(m->outputLineEdit);
  outputLayout->setContentsMargins(0, 0, 0, 0);
}

Kryvos::OutputFrame::~OutputFrame() {
}

void Kryvos::OutputFrame::outputPath(const QString& path) {
  Q_ASSERT(m->outputLineEdit);

  m->outputLineEdit->setText(path);
}

QString Kryvos::OutputFrame::outputPath() const {
  Q_ASSERT(m->outputLineEdit);

  auto fileName = m->outputLineEdit->text();

  return fileName;
}

Kryvos::OutputFrame::OutputFramePrivate::OutputFramePrivate()
  : outputLineEdit{nullptr} {
}
