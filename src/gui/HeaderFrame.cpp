#include "gui/HeaderFrame.hpp"
#include "gui/FluidLayout.hpp"
#include "utility/pimpl_impl.h"
#include <QPushButton>
#include <QLabel>
#include <QHBoxLayout>
#include <QIcon>

class HeaderFrame::HeaderFramePrivate {
 public:
  /*!
   * \brief HeaderFramePrivate Constructs the HeaderFrame private
   * implementation.
   */
  HeaderFramePrivate();

  QPushButton* pauseButton;
  QPushButton* addFilesButton;
  QPushButton* clearFilesButton;
  QPushButton* settingsButton;

  QSize iconSize;
};

HeaderFrame::HeaderFrame(QWidget* parent)
  : QFrame{parent}
{
  auto buttonFrame = new QFrame{this};

  const auto pauseIcon = QIcon{QStringLiteral(":/images/pauseIcon.png")};
  m->pauseButton = new QPushButton{pauseIcon, tr(" Pause"), this};
  m->pauseButton->setObjectName(QStringLiteral("pauseButton"));
  m->pauseButton->setCheckable(true);
  m->pauseButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

  const auto addFilesIcon = QIcon{QStringLiteral(":/images/addFilesIcon.png")};
  m->addFilesButton = new QPushButton{addFilesIcon, tr(" Add files"), this};
  m->addFilesButton->setObjectName(QStringLiteral("addButton"));
  m->addFilesButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

  const auto clearFilesIcon =
        QIcon{QStringLiteral(":/images/clearFilesIcon.png")};
  m->clearFilesButton = new QPushButton{clearFilesIcon,
                                        tr(" Remove all files"),
                                        this};
  m->clearFilesButton->setObjectName(QStringLiteral("clearButton"));
  m->clearFilesButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

  const auto settingsIcon = QIcon{QStringLiteral(":/images/gearIcon.png")};
  m->settingsButton = new QPushButton{settingsIcon, tr(" Settings"), this};
  m->settingsButton->setObjectName(QStringLiteral("settingsButton"));
  m->settingsButton->setSizePolicy(QSizePolicy::Fixed,
                                   QSizePolicy::Fixed);

  auto buttonLayout = new FluidLayout{buttonFrame};
  buttonLayout->addWidget(m->pauseButton);
  buttonLayout->addWidget(m->addFilesButton);
  buttonLayout->addWidget(m->clearFilesButton);
  buttonLayout->addWidget(m->settingsButton);

  auto headerLayout = new QHBoxLayout{this};
  headerLayout->addWidget(buttonFrame, 1);
  headerLayout->setContentsMargins(10, 0, 0, 0);

  connect(m->pauseButton, &QPushButton::toggled,
          this, &HeaderFrame::pause);
  connect(m->pauseButton, &QPushButton::toggled,
          this, &HeaderFrame::togglePauseIcon);
  connect(m->addFilesButton, &QPushButton::clicked,
          this, &HeaderFrame::addFiles);
  connect(m->clearFilesButton, &QPushButton::clicked,
          this, &HeaderFrame::removeFiles);
  connect(m->settingsButton, &QPushButton::clicked,
          this, &HeaderFrame::switchFrame);
}

HeaderFrame::~HeaderFrame() {}

void HeaderFrame::togglePauseIcon(const bool toggle)
{
  Q_ASSERT(m->pauseButton);

  if (toggle)
  {
    const auto resumeIcon = QIcon{QStringLiteral(":/images/resumeIcon.png")};
    m->pauseButton->setIcon(resumeIcon);
    m->pauseButton->setText(tr(" Resume"));
  }
  else
  {
    const auto pauseIcon = QIcon{QStringLiteral(":/images/pauseIcon.png")};
    m->pauseButton->setIcon(pauseIcon);
    m->pauseButton->setText(tr(" Pause"));
  }
}

void HeaderFrame::setIconSize(const QSize& iconSize)
{
  Q_ASSERT(m->pauseButton);
  Q_ASSERT(m->addFilesButton);
  Q_ASSERT(m->clearFilesButton);

  m->iconSize = iconSize;

  m->pauseButton->setIconSize(m->iconSize);
  m->addFilesButton->setIconSize(m->iconSize);
  m->clearFilesButton->setIconSize(m->iconSize);
}

HeaderFrame::HeaderFramePrivate::HeaderFramePrivate()
  : pauseButton{nullptr}, addFilesButton{nullptr}, clearFilesButton{nullptr}
{}
