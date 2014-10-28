/**
 * Kryvos File Encryptor - Encrypts and decrypts files.
 * Copyright (C) 2014 Andrew Dolby
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Contact : andrewdolby@gmail.com
 */

#include "gui/SettingsFrame.hpp"
#include "gui/SlideSwitch.hpp"
#include "gui/flowlayout.h"
#include "utility/make_unique.h"
#include <QtWidgets/QComboBox>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QLabel>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QAction>
#include <QtWidgets/QToolTip>
#include <QtGui/QIcon>
#include <QtGui/QFontMetrics>
#include <QtCore/QStringRef>
#include <QtCore/QStringBuilder>

class SettingsFrame::SettingsFramePrivate {
 public:
  /*!
   * \brief SettingsFramePrivate Constructs the settings frame private
   * implementation.
   */
  explicit SettingsFramePrivate();

  QString splitToolTip(const QString& text, int width);

  QComboBox* cipherComboBox;
  QComboBox* keySizeComboBox;
  QComboBox* modeComboBox;

  int toolTipWidth;
};

SettingsFrame::SettingsFrame(const QString& cipher,
                             std::size_t keySize,
                             const QString& mode,
                             QWidget* parent) :
  QFrame{parent}, pimpl{make_unique<SettingsFramePrivate>()}
{
  // TODO: Fix this slide switch or remove it.
  //auto slideSwitch = new SlideSwitch{this};
  //slideSwitch->setBackgroundPixmap(":/images/sliderBackground.png");
  //slideSwitch->setKnobPixmaps(":/images/sliderKnobEnable.png",
  //                            ":/images/sliderKnobDisable.png");

  auto headerFrame = new QFrame{this};

  auto headerLabel = new QLabel{tr("Settings"), headerFrame};
  headerLabel->setObjectName("headerText");

  // TODO: Create icon for back button.
  //const auto backIcon = QIcon{":/images/backIcon.png"};
  auto backButton = new QPushButton{tr(" Back"), this};
  backButton->setObjectName("backButton");
  backButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

  auto headerLayout = new QHBoxLayout{headerFrame};
  headerLayout->addWidget(headerLabel);
  headerLayout->addStretch();
  headerLayout->addWidget(backButton);

  auto contentFrame = new QFrame{this};
  contentFrame->setObjectName("settingsSubFrame");

  auto cipherFrame = new QFrame{contentFrame};
  cipherFrame->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

  auto cipherLabel = new QLabel{tr("Cipher: "), cipherFrame};
  cipherLabel->setObjectName("text");

  pimpl->cipherComboBox = new QComboBox{cipherFrame};
  pimpl->cipherComboBox->setObjectName("settingsComboBox");
  pimpl->cipherComboBox->addItem(tr("AES"));
  pimpl->cipherComboBox->addItem(tr("Serpent"));
  pimpl->cipherComboBox->setCurrentText(cipher);

  auto cipherLayout = new QHBoxLayout{cipherFrame};
  cipherLayout->addWidget(cipherLabel);
  cipherLayout->addWidget(pimpl->cipherComboBox);

  auto keySizeFrame = new QFrame{contentFrame};
  keySizeFrame->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

  const auto keySizeToolTip = QString{tr("The cipher key size is the number of "
                                         "bits in the key that is created from "
                                         "your password via a secure hash "
                                         "function. A larger key size does not "
                                         "necessarily yield a more secure "
                                         "encrypted output. Key sizes of 128, "
                                         "192, and 256 are all currently "
                                         "considered to be secure key sizes.")};
  const auto keySizeSplitToolTip = pimpl->splitToolTip(keySizeToolTip,
                                                       pimpl->toolTipWidth);

  auto keySizeLabel = new QLabel{tr("Key size (bits): "), keySizeFrame};
  keySizeLabel->setObjectName("text");
  keySizeLabel->setToolTip(keySizeSplitToolTip);

  pimpl->keySizeComboBox = new QComboBox{keySizeFrame};
  pimpl->keySizeComboBox->setObjectName("settingsComboBox");
  pimpl->keySizeComboBox->addItem(tr("128"));
  pimpl->keySizeComboBox->addItem(tr("192"));
  pimpl->keySizeComboBox->addItem(tr("256"));
  pimpl->keySizeComboBox->setCurrentText(QString::number(keySize));
  pimpl->keySizeComboBox->setToolTip(keySizeSplitToolTip);

  auto keySizeLayout = new QHBoxLayout{keySizeFrame};
  keySizeLayout->addWidget(keySizeLabel);
  keySizeLayout->addWidget(pimpl->keySizeComboBox);

  auto modeFrame = new QFrame{contentFrame};
  modeFrame->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

  const auto modeToolTip = QString{tr("The mode of operation is the algorithm "
                                      "that repeatedly applies the cipher's "
                                      "single-block operation to securely "
                                      "transform data. GCM and EAX are both "
                                      "currently considered to be secure modes "
                                      "of operation.")};
  const auto modeSplitToolTip = pimpl->splitToolTip(modeToolTip,
                                                    pimpl->toolTipWidth);

  auto modeLabel = new QLabel{tr("Mode of operation: "), modeFrame};
  modeLabel->setObjectName("text");
  modeLabel->setToolTip(modeSplitToolTip);

  pimpl->modeComboBox = new QComboBox{modeFrame};
  pimpl->modeComboBox->setObjectName("settingsComboBox");
  pimpl->modeComboBox->addItem(tr("GCM"));
  pimpl->modeComboBox->addItem(tr("EAX"));
  pimpl->modeComboBox->setCurrentText(mode);
  pimpl->modeComboBox->setToolTip(modeSplitToolTip);

  auto modeLayout = new QHBoxLayout{modeFrame};
  modeLayout->addWidget(modeLabel);
  modeLayout->addWidget(pimpl->modeComboBox);

  auto contentLayout = new QHBoxLayout{contentFrame};
  contentLayout->addWidget(cipherFrame);
  contentLayout->addWidget(keySizeFrame);
  contentLayout->addWidget(modeFrame);

  auto layout = new QVBoxLayout{this};
  layout->addWidget(headerFrame, 0);
  layout->addWidget(contentFrame, 1);

  // Capture function pointer to specific QComboBox signal overload
  void (QComboBox::*indexChangedSignal)(const QString&) =
      &QComboBox::currentIndexChanged;

  // Connect cipher combo box change signal to change cipher slot
  connect(pimpl->cipherComboBox, indexChangedSignal,
          this, &SettingsFrame::changeCipher);

  // Connect key size combo box change signal to change key size slot
  connect(pimpl->keySizeComboBox, indexChangedSignal,
          this, &SettingsFrame::changeKeySize);

  // Connect mode combo box change signal to change mode of operation slot
  connect(pimpl->modeComboBox, indexChangedSignal,
          this, &SettingsFrame::changeModeOfOperation);

  // Return to previous GUI state
  auto returnAction = new QAction{this};
  returnAction->setShortcut(Qt::Key_Escape);
  connect(returnAction, &QAction::triggered,
          this, &SettingsFrame::switchFrame);
  this->addAction(returnAction);

  // Connect back button to return action
  connect(backButton, &QPushButton::clicked,
          returnAction, &QAction::trigger);
}

SettingsFrame::~SettingsFrame() {}

void SettingsFrame::changeCipher()
{
  Q_ASSERT(pimpl);
  Q_ASSERT(pimpl->cipherComboBox);

  emit newCipher(pimpl->cipherComboBox->currentText());
}

void SettingsFrame::changeKeySize()
{
  Q_ASSERT(pimpl);
  Q_ASSERT(pimpl->keySizeComboBox);

  const auto keySizeString = pimpl->keySizeComboBox->currentText();

  const std::size_t keySize =
      static_cast<std::size_t>(keySizeString.toLongLong());

  emit newKeySize(keySize);
}

void SettingsFrame::changeModeOfOperation()
{
  Q_ASSERT(pimpl);
  Q_ASSERT(pimpl->modeComboBox);

  emit newModeOfOperation(pimpl->modeComboBox->currentText());
}

SettingsFrame::SettingsFramePrivate::SettingsFramePrivate() :
  cipherComboBox{nullptr}, keySizeComboBox{nullptr}, modeComboBox{nullptr},
  toolTipWidth{250}
{}

QString SettingsFrame::SettingsFramePrivate::splitToolTip(const QString& text,
                                                          int width)
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
      if (fm.width(temp.left(++i + 1)) > width)
      {
        int j = temp.lastIndexOf(' ', i);
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
