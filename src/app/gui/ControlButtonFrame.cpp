#include "gui/ControlButtonFrame.hpp"
#include <QPushButton>
#include <QHBoxLayout>
#include <QIcon>

class Kryvo::ControlButtonFramePrivate {
  Q_DISABLE_COPY(ControlButtonFramePrivate)

 public:
  /*!
   * \brief ControlButtonFramePrivate Constructs the ControlButtonFrame private
   * implementation.
   */
  ControlButtonFramePrivate();

  QPushButton* encryptButton{nullptr};
  QPushButton* decryptButton{nullptr};
};

Kryvo::ControlButtonFramePrivate::ControlButtonFramePrivate() = default;

Kryvo::ControlButtonFrame::ControlButtonFrame(QWidget* parent)
  : QFrame(parent), d_ptr(std::make_unique<ControlButtonFramePrivate>()) {
  Q_D(ControlButtonFrame);

  const QSize iconSize(19, 19);

  const QIcon lockIcon(QStringLiteral(":/images/lockIcon.png"));
  d->encryptButton = new QPushButton(lockIcon, tr(" Encrypt"), this);
  d->encryptButton->setIconSize(iconSize);
  d->encryptButton->setObjectName(QStringLiteral("cryptButton"));
  d->encryptButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

  const QIcon unlockedIcon(QStringLiteral(":/images/unlockIcon.png"));
  d->decryptButton = new QPushButton(unlockedIcon, tr(" Decrypt"), this);
  d->decryptButton->setIconSize(iconSize);
  d->decryptButton->setObjectName(QStringLiteral("cryptButton"));
  d->decryptButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

  auto buttonLayout = new QHBoxLayout(this);
  buttonLayout->addWidget(d->encryptButton);
  buttonLayout->addWidget(d->decryptButton);
  buttonLayout->setContentsMargins(0, 0, 0, 0);

  connect(d->encryptButton, &QPushButton::clicked,
          this, &ControlButtonFrame::encryptFiles);
  connect(d->decryptButton, &QPushButton::clicked,
          this, &ControlButtonFrame::decryptFiles);
}

Kryvo::ControlButtonFrame::~ControlButtonFrame() = default;

void Kryvo::ControlButtonFrame::setIconSize(const QSize& iconSize) {
  Q_D(ControlButtonFrame);
  Q_ASSERT(d->encryptButton);
  Q_ASSERT(d->decryptButton);

  d->encryptButton->setIconSize(iconSize);
  d->decryptButton->setIconSize(iconSize);
}

void Kryvo::ControlButtonFrame::encryptFiles() {
  emit processFiles(true);
}

void Kryvo::ControlButtonFrame::decryptFiles() {
  emit processFiles(false);
}
