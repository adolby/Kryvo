#include "src/gui/SettingsFrame.hpp"
#include "src/gui/SlideSwitch.hpp"
#include "src/gui/FluidLayout.hpp"
#include "src/utility/pimpl_impl.h"
#include <QCheckBox>
#include <QComboBox>
#include <QGroupBox>
#include <QPushButton>
#include <QLabel>
#include <QHBoxLayout>
#include <QAction>
#include <QToolTip>
#include <QIcon>
#include <QFontMetrics>
#include <QPainter>
#include <QPropertyAnimation>
#include <QStringRef>
#include <QStringBuilder>

class SettingsFrame::SettingsFramePrivate {
 public:
  /*!
   * \brief SettingsFramePrivate Constructs the settings frame private
   * implementation.
   */
  SettingsFramePrivate();

  QString splitToolTip(const QString& text, const int width) const;

  // Cryptography settings
  QComboBox* cipherComboBox;
  QComboBox* keySizeComboBox;
  QComboBox* modeComboBox;

  // File settings
  QCheckBox* compressionCheckBox;

  int toolTipWidth;
};

SettingsFrame::SettingsFrame(const QString& cipher,
                             const std::size_t& keySize,
                             const QString& mode,
                             QWidget* parent)
  : QFrame{parent}
{
  // TODO: Fix this slide switch or remove it.
  //auto slideSwitch = new SlideSwitch{this};
  //slideSwitch->setBackgroundPixmap(":/images/sliderBackground.png");
  //slideSwitch->setKnobPixmaps(QStringLiteral(":/images/sliderKnobEnable.png"),
  //                            QStringLiteral(":/images/sliderKnobDisable.png"));

  auto headerFrame = new QFrame{this};
  headerFrame->setObjectName(QStringLiteral("headerFrame"));
  headerFrame->setContentsMargins(0, 0, 0, 0);
  headerFrame->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

  auto gearImageLabel = new QLabel{headerFrame};
  gearImageLabel->setPixmap(QPixmap{QStringLiteral(":/images/gearIcon.png")});

  const auto backIcon = QIcon{QStringLiteral(":/images/backIcon.png")};
  auto backButton = new QPushButton{backIcon, tr(" Back"), headerFrame};
  backButton->setObjectName(QStringLiteral("backButton"));
  backButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

  auto headerLayout = new QHBoxLayout{headerFrame};
  headerLayout->addWidget(gearImageLabel);
  headerLayout->addStretch(10);
  headerLayout->addWidget(backButton);
  headerLayout->addStretch(1);
  headerLayout->setContentsMargins(0, 0, 0, 0);
  headerLayout->setSpacing(0);

  auto contentFrame = new QFrame{this};
  contentFrame->setContentsMargins(0, 0, 0, 0);

  // Cryptography settings
  auto cryptoSettingsFrame = new QFrame{contentFrame};
  cryptoSettingsFrame->setObjectName(QStringLiteral("settingsSubFrame"));

  auto cryptoSettingsLabel = new QLabel{tr("Cryptography Settings"),
                                        cryptoSettingsFrame};
  cryptoSettingsLabel->setObjectName(QStringLiteral("text"));

  auto cipherFrame = new QFrame{cryptoSettingsFrame};
  cipherFrame->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

  auto cipherLabel = new QLabel{tr("Cipher: "), cipherFrame};
  cipherLabel->setObjectName(QStringLiteral("text"));

  m->cipherComboBox = new QComboBox{cipherFrame};
  m->cipherComboBox->setObjectName(QStringLiteral("settingsComboBox"));
  m->cipherComboBox->addItem(tr("AES"));
  m->cipherComboBox->addItem(tr("Serpent"));
  m->cipherComboBox->setCurrentText(cipher);

  auto cipherLayout = new QHBoxLayout{cipherFrame};
  cipherLayout->addWidget(cipherLabel);
  cipherLayout->addWidget(m->cipherComboBox);

  auto keySizeFrame = new QFrame{cryptoSettingsFrame};
  keySizeFrame->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

  const auto keySizeToolTip = QString{tr("The cipher key size is the number of "
                                         "bits in the key that is created from "
                                         "your password via a secure hash "
                                         "function. A larger key size does not "
                                         "necessarily yield a more secure "
                                         "encrypted output. Key sizes of 128, "
                                         "192, and 256 are all currently "
                                         "considered to be secure key sizes.")};
  const auto keySizeSplitToolTip = m->splitToolTip(keySizeToolTip,
                                                   m->toolTipWidth);

  auto keySizeLabel = new QLabel{tr("Key size (bits): "), keySizeFrame};
  keySizeLabel->setObjectName(QStringLiteral("text"));
  keySizeLabel->setToolTip(keySizeSplitToolTip);

  m->keySizeComboBox = new QComboBox{keySizeFrame};
  m->keySizeComboBox->setObjectName(QStringLiteral("settingsComboBox"));
  m->keySizeComboBox->addItem(tr("128"));
  m->keySizeComboBox->addItem(tr("192"));
  m->keySizeComboBox->addItem(tr("256"));
  m->keySizeComboBox->setCurrentText(QString::number(keySize));
  m->keySizeComboBox->setToolTip(keySizeSplitToolTip);

  auto keySizeLayout = new QHBoxLayout{keySizeFrame};
  keySizeLayout->addWidget(keySizeLabel);
  keySizeLayout->addWidget(m->keySizeComboBox);

  auto modeFrame = new QFrame{cryptoSettingsFrame};
  modeFrame->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

  const auto modeToolTip = QString{tr("The mode of operation is the algorithm "
                                      "that repeatedly applies the cipher's "
                                      "single-block operation to securely "
                                      "transform data. GCM and EAX are both "
                                      "currently considered to be secure modes "
                                      "of operation.")};
  const auto modeSplitToolTip = m->splitToolTip(modeToolTip,
                                                m->toolTipWidth);

  auto modeLabel = new QLabel{tr("Mode of operation: "), modeFrame};
  modeLabel->setObjectName(QStringLiteral("text"));
  modeLabel->setToolTip(modeSplitToolTip);

  m->modeComboBox = new QComboBox{modeFrame};
  m->modeComboBox->setObjectName(QStringLiteral("settingsComboBox"));
  m->modeComboBox->addItem(tr("GCM"));
  m->modeComboBox->addItem(tr("EAX"));
  m->modeComboBox->setCurrentText(mode);
  m->modeComboBox->setToolTip(modeSplitToolTip);

  auto modeLayout = new QHBoxLayout{modeFrame};
  modeLayout->addWidget(modeLabel);
  modeLayout->addWidget(m->modeComboBox);

  auto cryptoSettingsLayout = new QVBoxLayout{cryptoSettingsFrame};
  cryptoSettingsLayout->addWidget(cryptoSettingsLabel);
  cryptoSettingsLayout->addWidget(cipherFrame);
  cryptoSettingsLayout->addWidget(keySizeFrame);
  cryptoSettingsLayout->addWidget(modeFrame);

  // File settings
  auto fileSettingsFrame = new QFrame{contentFrame};
  fileSettingsFrame->setObjectName(QStringLiteral("settingsSubFrame"));

  auto fileSettingsLabel = new QLabel{tr("File Settings"),
                                        cryptoSettingsFrame};
  fileSettingsLabel->setObjectName(QStringLiteral("text"));

  auto compressionFrame = new QFrame{fileSettingsFrame};
  compressionFrame->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

  m->compressionCheckBox = new QCheckBox{"Compress files before encryption",
                                         compressionFrame};
  m->compressionCheckBox->setObjectName("settingsCheckBox");
  m->compressionCheckBox->setChecked(true);

  auto compressionLayout = new QHBoxLayout{compressionFrame};
  compressionLayout->addWidget(m->compressionCheckBox);

  auto fileSettingsLayout = new QVBoxLayout{fileSettingsFrame};
  fileSettingsLayout->addWidget(fileSettingsLabel);
  fileSettingsLayout->addWidget(compressionFrame);

  auto contentLayout = new QVBoxLayout{contentFrame};
  contentLayout->addWidget(cryptoSettingsFrame);
  contentLayout->addStretch();
  contentLayout->addWidget(fileSettingsFrame);
  contentLayout->addStretch();
  contentLayout->setSpacing(0);

  auto centerFrame = new QFrame{this};
  centerFrame->setContentsMargins(0, 0, 0, 0);

  auto centerLayout = new QHBoxLayout{centerFrame};
  centerLayout->addWidget(contentFrame);
  centerLayout->addStretch();
  centerLayout->setContentsMargins(0, 0, 0, 0);
  centerLayout->setSpacing(0);

  auto settingsLayout = new QVBoxLayout{this};
  settingsLayout->addWidget(headerFrame, 0);
  settingsLayout->addWidget(centerFrame, 1);
  settingsLayout->setSpacing(0);

  // Capture function pointer to specific QComboBox signal overload
  void (QComboBox::*indexChangedSignal)(const QString&) =
      &QComboBox::currentIndexChanged;

  // Connect cipher combo box change signal to change cipher slot
  connect(m->cipherComboBox, indexChangedSignal,
          this, &SettingsFrame::changeCipher);

  // Connect key size combo box change signal to change key size slot
  connect(m->keySizeComboBox, indexChangedSignal,
          this, &SettingsFrame::changeKeySize);

  // Connect mode combo box change signal to change mode of operation slot
  connect(m->modeComboBox, indexChangedSignal,
          this, &SettingsFrame::changeModeOfOperation);

  connect(m->compressionCheckBox, &QCheckBox::stateChanged,
          this, &SettingsFrame::changeCompressionMode);

  // Return to previous GUI state
  auto returnAction = new QAction{this};
  returnAction->setShortcut(Qt::Key_Escape);
  connect(returnAction, &QAction::triggered, this, &SettingsFrame::switchFrame);

  // Connect back button to return action
  connect(backButton, &QPushButton::clicked, returnAction, &QAction::trigger);

  this->addAction(returnAction);
}

SettingsFrame::~SettingsFrame() {}

void SettingsFrame::changeCipher()
{
  Q_ASSERT(m->cipherComboBox);

  emit updateCipher(m->cipherComboBox->currentText());
}

void SettingsFrame::changeKeySize()
{
  Q_ASSERT(m->keySizeComboBox);

  const auto keySizeString = m->keySizeComboBox->currentText();

  const std::size_t keySize =
      static_cast<std::size_t>(keySizeString.toLongLong());

  emit updateKeySize(keySize);
}

void SettingsFrame::changeModeOfOperation()
{
  Q_ASSERT(m->modeComboBox);

  emit updateModeOfOperation(m->modeComboBox->currentText());
}

void SettingsFrame::changeCompressionMode()
{
  Q_ASSERT(m->compressionCheckBox);

  emit updateCompressionMode(m->compressionCheckBox->isChecked());
}

SettingsFrame::SettingsFramePrivate::SettingsFramePrivate()
  : cipherComboBox{nullptr}, keySizeComboBox{nullptr}, modeComboBox{nullptr},
    toolTipWidth{250}
{}

QString SettingsFrame::SettingsFramePrivate::splitToolTip(const QString& text,
                                                          const int width) const
{
  QFontMetrics fm{QToolTip::font()};
  QString result;
  auto temp = text;

  auto k = 0;
  while (k < 100000)
  {
    auto i = 0;
    while (i < temp.length())
    {
      i = i + 1;

      if (fm.width(temp.left(i + 1)) > width)
      {
        auto j = temp.lastIndexOf(' ', i);

        if (j > 0)
        {
          i = j;
        }

        result += temp.left(i);
        result += '\n';
        temp = temp.mid(i+1);

        break;
      }
    }

    if (i >= temp.length())
    {
      break;
    }

    ++k;
  }

  return result + temp;
}
