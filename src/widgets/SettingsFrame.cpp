#include "SettingsFrame.hpp"
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
#include <QStringBuilder>
#include <memory>

namespace Kryvo {

const QString keySizeToolTip =
  QObject::tr("The cipher key size is the number of bits "
              "in the key that is created from your "
              "password via a secure hash function. A "
              "larger key size does not necessarily "
              "yield a more secure encrypted output. Key "
              "sizes of 128, 192, and 256 are all "
              "currently considered to be secure key "
              "sizes.");

QString splitToolTip(const QString& text, const int width) {
  const QFontMetrics fm(QToolTip::font());
  QString result;

  QString temp = text;

  int k = 0;
  while (k < 100000) {
    int i = 0;

    while (i < temp.size()) {
      i = i + 1;

      const QRect textRect = fm.boundingRect(temp.left(i + 1));

      if (textRect.width() > width) {
        int j = temp.lastIndexOf(' ', i);

        if (j > 0) {
          i = j;
        }

        result += temp.left(i);
        result += '\n';
        temp = temp.mid(i + 1);

        break;
      }
    }

    if (i >= temp.size()) {
      break;
    }

    k = k + 1;
  }

  return result + temp;
}

class SettingsFramePrivate {
  Q_DISABLE_COPY(SettingsFramePrivate)

 public:
  /*!
   * \brief SettingsFramePrivate Constructs the settings frame private
   * implementation.
   */
  SettingsFramePrivate();

  // Cryptography settings
  QComboBox* providerComboBox{nullptr};
  QComboBox* keySizeComboBox{nullptr};
  QComboBox* compressionComboBox{nullptr};

  // File settings
  QCheckBox* containerCheckBox{nullptr};
  QCheckBox* removeIntermediateFilesCheckBox{nullptr};

  int toolTipWidth{250};
};

SettingsFramePrivate::SettingsFramePrivate() = default;

SettingsFrame::SettingsFrame(QWidget* parent)
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

  // Return to previous GUI state
  auto returnAction = new QAction(this);
  returnAction->setShortcut(Qt::Key_Escape);
  this->addAction(returnAction);

  connect(returnAction, &QAction::triggered, this, &SettingsFrame::switchFrame);

  // Connect back button to return action
  connect(backButton, &QPushButton::clicked, returnAction, &QAction::trigger);

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

  auto providerFrame = new QFrame(cryptoSettingsFrame);
  providerFrame->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

  auto providerLabel = new QLabel(tr("Provider"), providerFrame);
  providerLabel->setObjectName(QStringLiteral("text"));

  d->providerComboBox = new QComboBox(providerFrame);
  d->providerComboBox->setObjectName(QStringLiteral("settingsComboBox"));
  d->providerComboBox->addItem(QStringLiteral("Botan"));
  d->providerComboBox->addItem(QStringLiteral("OpenSsl"));

  connect(d->providerComboBox, &QComboBox::currentIndexChanged,
          this, &SettingsFrame::changeCryptoProvider);

  auto providerLayout = new QHBoxLayout(providerFrame);
  providerLayout->addWidget(providerLabel);
  providerLayout->addWidget(d->providerComboBox);

  auto keySizeFrame = new QFrame(cryptoSettingsFrame);
  keySizeFrame->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

  const QString keySizeSplitToolTip = splitToolTip(keySizeToolTip,
                                                   d->toolTipWidth);

  auto keySizeLabel = new QLabel(tr("Key size (bits)"), keySizeFrame);
  keySizeLabel->setObjectName(QStringLiteral("text"));
  keySizeLabel->setToolTip(keySizeSplitToolTip);

  d->keySizeComboBox = new QComboBox(keySizeFrame);
  d->keySizeComboBox->setObjectName(QStringLiteral("settingsComboBox"));
  d->keySizeComboBox->addItem(QStringLiteral("128"));
  d->keySizeComboBox->addItem(QStringLiteral("192"));
  d->keySizeComboBox->addItem(QStringLiteral("256"));
  d->keySizeComboBox->setToolTip(keySizeSplitToolTip);

  connect(d->keySizeComboBox, &QComboBox::currentIndexChanged,
          this, &SettingsFrame::changeKeySize);

  auto keySizeLayout = new QHBoxLayout(keySizeFrame);
  keySizeLayout->addWidget(keySizeLabel);
  keySizeLayout->addWidget(d->keySizeComboBox);

  auto modeFrame = new QFrame(cryptoSettingsFrame);
  modeFrame->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

  auto cryptoSettingsLayout = new QVBoxLayout(cryptoSettingsFrame);
  cryptoSettingsLayout->addWidget(cryptoSettingsLabel);
  cryptoSettingsLayout->addWidget(providerFrame);
  cryptoSettingsLayout->addWidget(keySizeFrame);

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

  connect(d->compressionComboBox, &QComboBox::currentIndexChanged,
          this, &SettingsFrame::changeCompressionFormat);

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

  connect(d->removeIntermediateFilesCheckBox, &QCheckBox::stateChanged,
          this, &SettingsFrame::changeRemoveIntermediateFiles);

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

// TODO: Re-add container setting when feature is complete
//  connect(d->containerCheckBox, &QCheckBox::stateChanged,
//          this, &SettingsFrame::changeContainerMode);

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
}

SettingsFrame::~SettingsFrame() = default;

void SettingsFrame::init(const QString& cryptoProvider,
                         const QString& compressionFormat,
                         const std::size_t keySize,
                         const bool removeIntermediateFiles,
                         const bool containerMode) {
  Q_D(SettingsFrame);

  d->keySizeComboBox->setCurrentText(QString::number(keySize));

  const int providerIdx = d->providerComboBox->findText(cryptoProvider);

  if (providerIdx > 0) {
    d->providerComboBox->setCurrentIndex(providerIdx);
  } else {
    d->providerComboBox->setCurrentIndex(0);
  }

  const int compressionIdx =
    d->compressionComboBox->findText(compressionFormat);

  if (compressionIdx > 0) {
    d->compressionComboBox->setCurrentIndex(compressionIdx);
  } else {
    d->compressionComboBox->setCurrentIndex(0);
  }

  d->removeIntermediateFilesCheckBox->setChecked(removeIntermediateFiles);
}

void SettingsFrame::changeCryptoProvider() {
  Q_D(SettingsFrame);
  Q_ASSERT(d->providerComboBox);

  emit updateCryptoProvider(d->providerComboBox->currentText());
}

void SettingsFrame::changeKeySize() {
  Q_D(SettingsFrame);
  Q_ASSERT(d->keySizeComboBox);

  const QString keySizeString = d->keySizeComboBox->currentText();

  const std::size_t keySize = keySizeString.toInt();

  emit updateKeySize(keySize);
}

void SettingsFrame::changeCompressionFormat() {
  Q_D(SettingsFrame);
  Q_ASSERT(d->compressionComboBox);

  emit updateCompressionFormat(d->compressionComboBox->currentText());
}

void SettingsFrame::changeRemoveIntermediateFiles() {
  Q_D(SettingsFrame);
  Q_ASSERT(d->removeIntermediateFilesCheckBox);

  emit updateRemoveIntermediateFiles(
    d->removeIntermediateFilesCheckBox->isChecked());
}

void SettingsFrame::changeContainerMode() {
  Q_D(SettingsFrame);
  Q_ASSERT(d->containerCheckBox);

  emit updateContainerMode(d->containerCheckBox->isChecked());
}

} // namespace Kryvo
