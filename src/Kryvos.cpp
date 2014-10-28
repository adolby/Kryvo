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

#include "Kryvos.hpp"
#include "gui/MainWindow.hpp"
#if defined(Q_OS_ANDROID)
#include "gui/TouchMainWindow.hpp"
#elif defined(Q_OS_IOS)
#include "gui/TouchMainWindow.hpp"
#else
#include "gui/DesktopMainWindow.hpp"
#endif
#include "cryptography/Crypto.hpp"
#include "settings/Settings.hpp"
#include "utility/make_unique.h"
#include <QtCore/QThread>

/*!
 * \brief KryvosPrivate class
 */
class Kryvos::KryvosPrivate {
 public:
  /*!
   * \brief KryvosPrivate Constructs the Kryvos private implementation which
   * contains the GUI and the cryptography object that interfaces with Botan.
   * Initializes the cryptography work thread.
   */
  explicit KryvosPrivate();

  /*!
   * \brief ~KryvosPrivate Destroys the Kryvos private implementation.
   */
  virtual ~KryvosPrivate();

  std::unique_ptr<Settings> settings;
  std::unique_ptr<Crypto> cryptography;
  std::unique_ptr<QThread> cipherThread;
  MainWindow* gui;
};

Kryvos::Kryvos(QObject* parent) :
  QObject{parent}, pimpl{make_unique<KryvosPrivate>()}
{
  qRegisterMetaType<std::size_t>("std::size_t");

#if defined(Q_OS_ANDROID)
  pimpl->gui = new TouchMainWindow(pimpl->settings.get());
#elif defined(Q_OS_IOS)
  pimpl->gui = new TouchMainWindow(pimpl->settings.get());
#else
  pimpl->gui = new DesktopMainWindow(pimpl->settings.get());
#endif

  // Move cryptography object to another thread to prevent GUI from blocking
  pimpl->cryptography->moveToThread(pimpl->cipherThread.get());

  // Connect GUI to cryptography object
  connect(pimpl->gui, &MainWindow::encrypt,
          pimpl->cryptography.get(), &Crypto::encrypt);

  connect(pimpl->gui, &MainWindow::decrypt,
          pimpl->cryptography.get(), &Crypto::decrypt);

  // Connections are direct so the cryptography object can be paused while
  // it is running a cipher operation on another thread
  connect(pimpl->gui, &MainWindow::pauseCipher,
          pimpl->cryptography.get(), &Crypto::pause, Qt::DirectConnection);

  connect(pimpl->gui, &MainWindow::abortCipher,
          pimpl->cryptography.get(), &Crypto::abort, Qt::DirectConnection);

  connect(pimpl->gui, &MainWindow::stopFile,
          pimpl->cryptography.get(), &Crypto::stop, Qt::DirectConnection);

  // Update progress bars
  connect(pimpl->cryptography.get(), &Crypto::progress,
          pimpl->gui, &MainWindow::updateProgress);

  // Update status message
  connect(pimpl->cryptography.get(), &Crypto::statusMessage,
          pimpl->gui, &MainWindow::updateStatusMessage);

  // Update error message
  connect(pimpl->cryptography.get(), &Crypto::errorMessage,
          pimpl->gui, &MainWindow::updateError);

  // Update cipher operation in progress status
  connect(pimpl->cryptography.get(), &Crypto::busyStatus,
          pimpl->gui, &MainWindow::updateBusyStatus);

  pimpl->cipherThread->start();

  // Show the main window
  pimpl->gui->show();
}

Kryvos::~Kryvos()
{
  // Abort current threaded cipher operation
  pimpl->cryptography->abort();

  // Quit the cipher thread
  pimpl->cipherThread->quit();

  auto timedOut = !pimpl->cipherThread->wait(1000);

  // If the thread couldn't quit in one second, terminate it
  if (timedOut)
  {
    pimpl->cipherThread->terminate();
  }

  delete pimpl->gui;
}

Kryvos::KryvosPrivate::KryvosPrivate() :
  settings{make_unique<Settings>()},
  cryptography{make_unique<Crypto>()},
  cipherThread{make_unique<QThread>()} {}

Kryvos::KryvosPrivate::~KryvosPrivate() {}
