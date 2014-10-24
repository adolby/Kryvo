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
#include <QtWidgets/QLabel>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QAction>
#include <QtGui/QIcon>
#include <QtCore/QStateMachine>
#include <QtCore/QState>

class SettingsFrame::SettingsFramePrivate {
 public:
  /*!
   * \brief SettingsFramePrivate Constructs the settings frame private
   * implementation.
   */
  explicit SettingsFramePrivate();

  QComboBox* cipherComboBox;
  QComboBox* blockSizeComboBox;
  QComboBox* modeComboBox;

  QStateMachine* stateMachine;
};

SettingsFrame::SettingsFrame(QWidget* parent) :
  QFrame{parent}, pimpl{make_unique<SettingsFramePrivate>()}
{
  //auto slideSwitch = new SlideSwitch{this};
  //slideSwitch->setBackgroundPixmap(":/images/sliderBackground.png");
  //slideSwitch->setKnobPixmaps(":/images/sliderKnobEnable.png",
  //                            ":/images/sliderKnobDisable.png");

  auto headerFrame = new QFrame{this};

  auto headerLabel = new QLabel{tr("Settings"), headerFrame};
  headerLabel->setObjectName("headerText");

  auto headerLayout = new QHBoxLayout{headerFrame};
  headerLayout->addWidget(headerLabel);

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
  auto cipherLayout = new QHBoxLayout{cipherFrame};
  cipherLayout->addWidget(cipherLabel);
  cipherLayout->addWidget(pimpl->cipherComboBox);

  auto blockSizeFrame = new QFrame{contentFrame};
  blockSizeFrame->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
  auto blockSizeLabel = new QLabel{tr("Block size: "), blockSizeFrame};
  blockSizeLabel->setObjectName("text");
  pimpl->blockSizeComboBox = new QComboBox{blockSizeFrame};
  pimpl->blockSizeComboBox->setObjectName("settingsComboBox");
  pimpl->cipherComboBox->addItem("128");
  pimpl->cipherComboBox->addItem("256");
  auto blockSizeLayout = new QHBoxLayout{blockSizeFrame};
  blockSizeLayout->addWidget(blockSizeLabel);
  blockSizeLayout->addWidget(pimpl->blockSizeComboBox);

  auto modeFrame = new QFrame{contentFrame};
  modeFrame->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
  auto modeLabel = new QLabel{tr("Mode of operation: "), modeFrame};
  modeLabel->setObjectName("text");
  pimpl->modeComboBox = new QComboBox{modeFrame};
  pimpl->modeComboBox->setObjectName("settingsComboBox");
  auto modeLayout = new QHBoxLayout{modeFrame};
  modeLayout->addWidget(modeLabel);
  modeLayout->addWidget(pimpl->modeComboBox);

  auto contentLayout = new QHBoxLayout{contentFrame};
  contentLayout->addWidget(cipherFrame);
  contentLayout->addWidget(blockSizeFrame);
  contentLayout->addWidget(modeFrame);

  auto layout = new QVBoxLayout{this};
  layout->addWidget(headerFrame, 0);
  layout->addWidget(contentFrame, 1);

  // Return to previous GUI state
  auto returnAction = new QAction{this};
  returnAction->setShortcut(Qt::Key_Escape);
  connect(returnAction, &QAction::triggered,
          this, &SettingsFrame::switchFrame);
  this->addAction(returnAction);

  // Combo box states
  pimpl->stateMachine = new QStateMachine{this};

  QState* aesState = new QState{};
  //aesState->assignProperty(contentFrame, "visible", true);
  //aesState->assignProperty(settingsFrame, "visible", false);

  QState* serpentState = new QState{};

  /*aesState->addTransition(this,
                          SIGNAL(switchFrame()),
                          serpentState);

  serpentState->addTransition(this,
                              SIGNAL(switchFrame()),
                              aesState);*/

  pimpl->stateMachine->addState(aesState);
  pimpl->stateMachine->addState(serpentState);
  pimpl->stateMachine->setInitialState(aesState);

  pimpl->stateMachine->start();
}

SettingsFrame::~SettingsFrame() {}

SettingsFrame::SettingsFramePrivate::SettingsFramePrivate()
{}
