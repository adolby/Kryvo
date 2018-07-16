#include "gui/HeaderFrame.hpp"
#include "gui/flowlayout.h"
#include <QPushButton>
#include <QLabel>
#include <QHBoxLayout>
#include <QIcon>

class Kryvo::HeaderFramePrivate {
  Q_DISABLE_COPY(HeaderFramePrivate)

 public:
  /*!
   * \brief HeaderFramePrivate Constructs the HeaderFrame private
   * implementation
   */
  HeaderFramePrivate();

  QPushButton* pauseButton;
  QPushButton* addFilesButton;
  QPushButton* clearFilesButton;
  QPushButton* settingsButton;

  QSize iconSize;
};

Kryvo::HeaderFrame::HeaderFrame(QWidget* parent)
  : QFrame(parent), d_ptr(std::make_unique<HeaderFramePrivate>()) {
  Q_D(HeaderFrame);

  auto headerImageLabel = new QLabel(this);
  headerImageLabel->setPixmap(QPixmap(QStringLiteral(":/images/kryvo.png")));
  headerImageLabel->setObjectName(QStringLiteral("headerImageLabel"));

  auto buttonFrame = new QFrame(this);
  buttonFrame->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

  const QIcon pauseIcon(QStringLiteral(":/images/pauseIcon.png"));
  d->pauseButton = new QPushButton(pauseIcon, tr(" Pause"), buttonFrame);
  d->pauseButton->setObjectName(QStringLiteral("pauseButton"));
  d->pauseButton->setCheckable(true);
  d->pauseButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

  const QIcon addFilesIcon(QStringLiteral(":/images/addFilesIcon.png"));
  d->addFilesButton = new QPushButton(addFilesIcon,
                                      tr(" Add files"),
                                      buttonFrame);
  d->addFilesButton->setObjectName(QStringLiteral("addButton"));
  d->addFilesButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

  const QIcon clearFilesIcon{QStringLiteral(":/images/clearFilesIcon.png")};
  d->clearFilesButton = new QPushButton(clearFilesIcon,
                                        tr(" Remove all files"),
                                        buttonFrame);
  d->clearFilesButton->setObjectName(QStringLiteral("clearButton"));
  d->clearFilesButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

  const QIcon settingsIcon{QStringLiteral(":/images/gearIcon.png")};
  d->settingsButton = new QPushButton(settingsIcon,
                                      tr(" Settings"),
                                      buttonFrame);
  d->settingsButton->setObjectName(QStringLiteral("settingsButton"));
  d->settingsButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

  auto buttonLayout = new FlowLayout(this, 2);
  buttonLayout->addWidget(headerImageLabel);
  buttonLayout->addWidget(d->pauseButton);
  buttonLayout->addWidget(d->addFilesButton);
  buttonLayout->addWidget(d->clearFilesButton);
  buttonLayout->addWidget(d->settingsButton);

  connect(d->pauseButton, &QPushButton::toggled,
          this, &HeaderFrame::pause);
  connect(d->pauseButton, &QPushButton::toggled,
          this, &HeaderFrame::togglePauseIcon);
  connect(d->addFilesButton, &QPushButton::clicked,
          this, &HeaderFrame::addFiles);
  connect(d->clearFilesButton, &QPushButton::clicked,
          this, &HeaderFrame::removeFiles);
  connect(d->settingsButton, &QPushButton::clicked,
          this, &HeaderFrame::switchFrame);
}

Kryvo::HeaderFrame::~HeaderFrame() {
}

void Kryvo::HeaderFrame::setIconSize(const QSize& iconSize) {
  Q_D(HeaderFrame);
  Q_ASSERT(d->pauseButton);
  Q_ASSERT(d->addFilesButton);
  Q_ASSERT(d->clearFilesButton);

  d->iconSize = iconSize;

  d->pauseButton->setIconSize(d->iconSize);
  d->addFilesButton->setIconSize(d->iconSize);
  d->clearFilesButton->setIconSize(d->iconSize);
}

void Kryvo::HeaderFrame::togglePauseIcon(const bool toggle) {
  Q_D(HeaderFrame);
  Q_ASSERT(d->pauseButton);

  if (toggle) {
    d->pauseButton->setIcon(QIcon(QStringLiteral(":/images/resumeIcon.png")));
    d->pauseButton->setText(tr(" Resume"));
  }
  else {
    d->pauseButton->setIcon(QIcon(QStringLiteral(":/images/pauseIcon.png")));
    d->pauseButton->setText(tr(" Pause"));
  }
}

Kryvo::HeaderFramePrivate::HeaderFramePrivate()
  : pauseButton(nullptr), addFilesButton(nullptr), clearFilesButton(nullptr),
    settingsButton(nullptr) {
}
