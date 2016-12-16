#include "src/gui/OutputFrame.hpp"
#include "src/utility/pimpl_impl.h"
#include <QLabel>
#include <QLineEdit>
#include <QHBoxLayout>

class OutputFrame::OutputFramePrivate {
 public:
  /*!
   * \brief OutputFramePrivate Constructs the output frame private
   * implementation.
   */
  OutputFramePrivate();

  QLineEdit* outputLineEdit;
};

OutputFrame::OutputFrame(QWidget* parent)
  : QFrame{parent}
{
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

OutputFrame::~OutputFrame() {}

void OutputFrame::outputPath(const QString& path)
{
  Q_ASSERT(m->outputLineEdit);

  m->outputLineEdit->setText(path);
}

QString OutputFrame::outputPath() const
{
  Q_ASSERT(m->outputLineEdit);

  auto fileName = m->outputLineEdit->text();

  return fileName;
}

OutputFrame::OutputFramePrivate::OutputFramePrivate()
  : outputLineEdit{nullptr} {}
