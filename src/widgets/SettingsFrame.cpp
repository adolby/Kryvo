#include "SettingsFrame.hpp"
#include "SlideSwitch.hpp"
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
#include <memory>

QString splitToolTip(const QString& text, const int width) {
  const QFontMetrics fm(QToolTip::font());
  QString result;

  QString temp = text;

  int k = 0;
  while (k < 100000) {
    int i = 0;

    while (i < temp.length()) {
      i = i + 1;

      if (fm.width(temp.left(i + 1)) > width) {
        int j = temp.lastIndexOf(' ', i);

        if (j > 0) {
          i = j;
        }

        result += temp.leftRef(i);
        result += '\n';
        temp = temp.mid(i + 1);

        break;
      }
    }

    if (i >= temp.length()) {
      break;
    }

    k = k + 1;
  }

  return result + temp;
}

class Kryvo::SettingsFramePrivate {
  Q_DISABLE_COPY(SettingsFramePrivate)

 public:
  /*!
   * \brief SettingsFramePrivate Constructs the settings frame private
   * implementation.
   */
  SettingsFramePrivate();

  // Cryptography settings
  QComboBox* cipherComboBox{nullptr};
  QComboBox* keySizeComboBox{nullptr};
  QComboBox* modeComboBox{nullptr};
  QComboBox* compressionComboBox{nullptr};

  // File settings
  QCheckBox* containerCheckBox{nullptr};
  QCheckBox* removeIntermediateFilesCheckBox{nullptr};

  int toolTipWidth{250};
};

Kryvo::SettingsFramePrivate::SettingsFramePrivate() = default;

Kryvo::SettingsFrame::SettingsFrame(const QString& cryptoProvider,
                                    const QString& compressionFormat,
                                    const QString& cipher,
                                    const std::size_t keySize,
                                    const QString& mode,
                                    const bool removeIntermediateFiles,
                                    const bool containerMode,
                                    QWidget* parent)
  : QFrame(parent), d_ptr(std::make_unique<SettingsFramePrivate>()) {
  Q_D(SettingsFrame);

  auto headerFrame = new QFrame(this);
  headerFrame->setObjectName(QStringLiteral("headerFrame"));
  headerFrame->setContentsMargins(0, 0, 0, 0);
  headerFrame->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

  auto settingsIcon = new QLabel(headerFrame);
  settingsIcon->setPixmap(QPixmap(QStringLiteral(":/images/settingsIcon.png")));

  const QIcon backIcon(QStringLiteral(":/images/backIcon.png"));
  auto backButton = new QPushButton{backIcon, tr(" Back"), headerFrame};
  backButton->setObjectName(QStringLiteral("backButton"));
  backButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

  auto headerLayout = new QHBoxLayout(headerFrame);
  headerLayout->addWidget(settingsIcon);
  headerLayout->addStretch();
  headerLayout->addWidget(backButton);
  headerLayout->setContentsMargins(0, 0, 0, 0);
  headerLayout->setSpacing(0);

  auto contentFrame = new QFrame(this);
  contentFrame->setContentsMargins(0, 0, 0, 0);

  // Cryptography settings
  auto cryptoSettingsFrame = new QFrame(contentFrame);
  cryptoSettingsFrame->setObjectName(QStringLiteral("settingsSubFrame"));

  auto cryptoSettingsLabel = new QLabel(tr("Cryptography Settings"),
                                        cryptoSettingsFrame);
  cryptoSettingsLabel->setObjectName(QStringLiteral("text"));

  auto cipherFrame = new QFrame(cryptoSettingsFrame);
  cipherFrame->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

  auto cipherLabel = new QLabel(tr("Cipher"), cipherFrame);
  cipherLabel->setObjectName(QStringLiteral("text"));

  d->cipherComboBox = new QComboBox(cipherFrame);
  d->cipherComboBox->setObjectName(QStringLiteral("settingsComboBox"));
  d->cipherComboBox->addItem(QStringLiteral("AES"));
  d->cipherComboBox->addItem(QStringLiteral("Serpent"));
  d->cipherComboBox->setCurrentText(cipher);

  auto cipherLayout = new QHBoxLayout(cipherFrame);
  cipherLayout->addWidget(cipherLabel);
  cipherLayout->addWidget(d->cipherComboBox);

  auto keySizeFrame = new QFrame(cryptoSettingsFrame);
  keySizeFrame->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

  const QString& keySizeToolTip = tr("The cipher key size is the number of "
                                     "bits in the key that is created from "
                                     "your password via a secure hash "
                                     "function. A larger key size does not "
                                     "necessarily yield a more secure "
                                     "encrypted output. Key sizes of 128, 192, "
                                     "and 256 are all currently considered to "
                                     "be secure key sizes.");
  const QString& keySizeSplitToolTip = splitToolTip(keySizeToolTip,
                                                    d->toolTipWidth);

  auto keySizeLabel = new QLabel(tr("Key size (bits)"), keySizeFrame);
  keySizeLabel->setObjectName(QStringLiteral("text"));
  keySizeLabel->setToolTip(keySizeSplitToolTip);

  d->keySizeComboBox = new QComboBox(keySizeFrame);
  d->keySizeComboBox->setObjectName(QStringLiteral("settingsComboBox"));
  d->keySizeComboBox->addItem(QStringLiteral("128"));
  d->keySizeComboBox->addItem(QStringLiteral("192"));
  d->keySizeComboBox->addItem(QStringLiteral("256"));
  d->keySizeComboBox->setCurrentText(QString::number(keySize));
  d->keySizeComboBox->setToolTip(keySizeSplitToolTip);

  auto keySizeLayout = new QHBoxLayout(keySizeFrame);
  keySizeLayout->addWidget(keySizeLabel);
  keySizeLayout->addWidget(d->keySizeComboBox);

  auto modeFrame = new QFrame(cryptoSettingsFrame);
  modeFrame->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

  const QString& modeToolTip = tr("The mode of operation is the algorithm that "
                                  "repeatedly applies the cipher's "
                                  "single-block operation to securely "
                                  "transform data. GCM and EAX are both "
                                  "currently considered to be secure modes of "
                                  "operation.");
  const QString& modeSplitToolTip = splitToolTip(modeToolTip, d->toolTipWidth);

  auto modeLabel = new QLabel(tr("Mode of operation"), modeFrame);
  modeLabel->setObjectName(QStringLiteral("text"));
  modeLabel->setToolTip(modeSplitToolTip);

  d->modeComboBox = new QComboBox(modeFrame);
  d->modeComboBox->setObjectName(QStringLiteral("settingsComboBox"));
  d->modeComboBox->addItem(QStringLiteral("GCM"));
  d->modeComboBox->addItem(QStringLiteral("EAX"));
  d->modeComboBox->setCurrentText(mode);
  d->modeComboBox->setToolTip(modeSplitToolTip);

  auto modeLayout = new QHBoxLayout(modeFrame);
  modeLayout->addWidget(modeLabel);
  modeLayout->addWidget(d->modeComboBox);

  auto cryptoSettingsLayout = new QVBoxLayout(cryptoSettingsFrame);
  cryptoSettingsLayout->addWidget(cryptoSettingsLabel);
  cryptoSettingsLayout->addWidget(cipherFrame);
  cryptoSettingsLayout->addWidget(keySizeFrame);
  cryptoSettingsLayout->addWidget(modeFrame);

  // File settings
  auto fileSettingsFrame = new QFrame(contentFrame);
  fileSettingsFrame->setObjectName(QStringLiteral("settingsSubFrame"));

  auto fileSettingsLabel = new QLabel(tr("File Settings"), cryptoSettingsFrame);
  fileSettingsLabel->setObjectName(QStringLiteral("text"));

  auto compressionFrame = new QFrame(fileSettingsFrame);
  compressionFrame->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

  QLabel* compressionLabel = new QLabel(compressionFrame);
  compressionLabel->setObjectName(QStringLiteral("text"));
  compressionLabel->setText(QStringLiteral("Compress files before encryption"));

  d->compressionComboBox = new QComboBox(compressionFrame);
  d->compressionComboBox->setObjectName(QStringLiteral("settingsComboBox"));
  d->compressionComboBox->addItem(QStringLiteral("gzip"));
  d->compressionComboBox->addItem(QStringLiteral("None"));
  d->compressionComboBox->setCurrentIndex(0);

  auto compressionLayout = new QHBoxLayout(compressionFrame);
  compressionLayout->addWidget(compressionLabel);
  compressionLayout->addWidget(d->compressionComboBox);

  auto removeIntermediateFilesFrame = new QFrame(fileSettingsFrame);
  removeIntermediateFilesFrame->setSizePolicy(QSizePolicy::Maximum,
                                              QSizePolicy::Maximum);

  d->removeIntermediateFilesCheckBox =
    new QCheckBox(tr("Delete intermediate files"), compressionFrame);
  d->removeIntermediateFilesCheckBox->setObjectName(
    QStringLiteral("settingsCheckBox"));
  d->removeIntermediateFilesCheckBox->setChecked(removeIntermediateFiles);

  auto removeIntermediateFilesLayout =
    new QHBoxLayout(removeIntermediateFilesFrame);
  removeIntermediateFilesLayout->addWidget(d->removeIntermediateFilesCheckBox);

// TODO: Re-add container setting when feature is complete
//  auto containerFrame = new QFrame(fileSettingsFrame);
//  containerFrame->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

//  d->containerCheckBox = new QCheckBox(tr("Archive encrypted files"),
//                                       containerFrame);
//  d->containerCheckBox->setObjectName(QStringLiteral("settingsCheckBox"));
//  d->containerCheckBox->setChecked(containerMode);

//  auto containerLayout = new QHBoxLayout(containerFrame);
//  containerLayout->addWidget(d->containerCheckBox);

  auto fileSettingsLayout = new QVBoxLayout(fileSettingsFrame);
  fileSettingsLayout->addWidget(fileSettingsLabel);
  fileSettingsLayout->addWidget(compressionFrame);
  fileSettingsLayout->addWidget(removeIntermediateFilesFrame);
//  fileSettingsLayout->addWidget(containerFrame);

  auto contentLayout = new QVBoxLayout(contentFrame);
  contentLayout->addWidget(cryptoSettingsFrame);
  contentLayout->addStretch();
  contentLayout->addWidget(fileSettingsFrame);
  contentLayout->addStretch();
  contentLayout->setSpacing(0);

  auto centerFrame = new QFrame(this);
  centerFrame->setContentsMargins(0, 0, 0, 0);

  auto centerLayout = new QHBoxLayout(centerFrame);
  centerLayout->addWidget(contentFrame);
  centerLayout->addStretch();
  centerLayout->setContentsMargins(0, 0, 0, 0);
  centerLayout->setSpacing(0);

  auto settingsLayout = new QVBoxLayout(this);
  settingsLayout->addWidget(headerFrame, 0);
  settingsLayout->addWidget(centerFrame, 1);
  settingsLayout->setSpacing(0);

  // Capture function pointer to specific QComboBox signal overload
  void (QComboBox::*indexChangedSignal)(const QString&) =
    &QComboBox::currentIndexChanged;

  connect(d->cipherComboBox, indexChangedSignal,
          this, &SettingsFrame::changeCipher);

  connect(d->keySizeComboBox, indexChangedSignal,
          this, &SettingsFrame::changeKeySize);

  connect(d->modeComboBox, indexChangedSignal,
          this, &SettingsFrame::changeModeOfOperation);

  connect(d->compressionComboBox, indexChangedSignal,
          this, &SettingsFrame::changeCompressionFormat);

  connect(d->removeIntermediateFilesCheckBox, &QCheckBox::stateChanged,
          this, &SettingsFrame::changeRemoveIntermediateFiles);

// TODO: Re-add container setting when feature is complete
//  connect(d->containerCheckBox, &QCheckBox::stateChanged,
//          this, &SettingsFrame::changeContainerMode);

  // Return to previous GUI state
  auto returnAction = new QAction(this);
  returnAction->setShortcut(Qt::Key_Escape);
  connect(returnAction, &QAction::triggered, this, &SettingsFrame::switchFrame);

  // Connect back button to return action
  connect(backButton, &QPushButton::clicked, returnAction, &QAction::trigger);

  this->addAction(returnAction);
}

Kryvo::SettingsFrame::~SettingsFrame() = default;

void Kryvo::SettingsFrame::changeCipher() {
  Q_D(SettingsFrame);
  Q_ASSERT(d->cipherComboBox);

  emit updateCipher(d->cipherComboBox->currentText());
}

void Kryvo::SettingsFrame::changeKeySize() {
  Q_D(SettingsFrame);
  Q_ASSERT(d->keySizeComboBox);

  const QString& keySizeString = d->keySizeComboBox->currentText();

  const std::size_t keySize = keySizeString.toInt();

  emit updateKeySize(keySize);
}

void Kryvo::SettingsFrame::changeModeOfOperation() {
  Q_D(SettingsFrame);
  Q_ASSERT(d->modeComboBox);

  emit updateModeOfOperation(d->modeComboBox->currentText());
}

void Kryvo::SettingsFrame::changeCompressionFormat() {
  Q_D(SettingsFrame);
  Q_ASSERT(d->compressionComboBox);

  emit updateCompressionFormat(d->compressionComboBox->currentText());
}

void Kryvo::SettingsFrame::changeRemoveIntermediateFiles() {
  Q_D(SettingsFrame);
  Q_ASSERT(d->removeIntermediateFilesCheckBox);

  emit updateRemoveIntermediateFiles(
    d->removeIntermediateFilesCheckBox->isChecked());
}

void Kryvo::SettingsFrame::changeContainerMode() {
  Q_D(SettingsFrame);
  Q_ASSERT(d->containerCheckBox);

  emit updateContainerMode(d->containerCheckBox->isChecked());
}
