#include "src/gui/ControlButtonFrame.hpp"
#include "src/utility/pimpl_impl.h"
#include <QPushButton>
#include <QHBoxLayout>
#include <QIcon>

class Kryvos::ControlButtonFrame::ControlButtonFramePrivate {
 public:
  /*!
   * \brief ControlButtonFramePrivate Constructs the ControlButtonFrame private
   * implementation.
   */
  ControlButtonFramePrivate();

  QPushButton* encryptButton;
  QPushButton* decryptButton;
};

Kryvos::ControlButtonFrame::ControlButtonFrame(QWidget* parent)
  : QFrame{parent} {
  const auto iconSize = QSize{19, 19};

  const auto lockIcon = QIcon{QStringLiteral(":/images/lockIcon.png")};
  m->encryptButton = new QPushButton{lockIcon, tr(" Encrypt"), this};
  m->encryptButton->setIconSize(iconSize);
  m->encryptButton->setObjectName(QStringLiteral("cryptButton"));
  m->encryptButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

  const auto unlockedIcon = QIcon{QStringLiteral(":/images/unlockIcon.png")};
  m->decryptButton = new QPushButton{unlockedIcon, tr(" Decrypt"), this};
  m->decryptButton->setIconSize(iconSize);
  m->decryptButton->setObjectName(QStringLiteral("cryptButton"));
  m->decryptButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

  auto buttonLayout = new QHBoxLayout{this};
  buttonLayout->addWidget(m->encryptButton);
  buttonLayout->addWidget(m->decryptButton);
  buttonLayout->setContentsMargins(0, 0, 0, 0);

  connect(m->encryptButton, &QPushButton::clicked,
          this, &ControlButtonFrame::encryptFiles);
  connect(m->decryptButton, &QPushButton::clicked,
          this, &ControlButtonFrame::decryptFiles);
}

Kryvos::ControlButtonFrame::~ControlButtonFrame() {
}

void Kryvos::ControlButtonFrame::setIconSize(const QSize& iconSize) {
  Q_ASSERT(m->encryptButton);
  Q_ASSERT(m->decryptButton);

  m->encryptButton->setIconSize(iconSize);
  m->decryptButton->setIconSize(iconSize);
}

void Kryvos::ControlButtonFrame::encryptFiles() {
  emit processFiles(true);
}

void Kryvos::ControlButtonFrame::decryptFiles() {
  emit processFiles(false);
}

Kryvos::ControlButtonFrame::ControlButtonFramePrivate::
ControlButtonFramePrivate()
  : encryptButton{nullptr}, decryptButton{nullptr} {
}
