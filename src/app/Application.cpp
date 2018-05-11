#include "Application.hpp"
#include "gui/MainWindow.hpp"
#include "gui/DesktopMainWindow.hpp"
#include "cryptography/Crypto.hpp"
#include "archive/Archiver.hpp"
#include "settings/Settings.hpp"
#include "utility/Thread.hpp"
#include <memory>

#include <QDebug>

/*!
 * \brief ApplicationPrivate class
 */
class Kryvo::ApplicationPrivate {
  Q_DISABLE_COPY(ApplicationPrivate)

 public:
  /*!
   * \brief ApplicationPrivate Constructs the app private implementation which
   * contains the GUI and the cryptography object that interfaces with Botan.
   * Initializes the cryptography work thread.
   */
  ApplicationPrivate();

  Settings settings;
  Crypto cryptography;
  Archiver archiver;
  Thread cryptographyThread;
  Thread archiverThread;
  DesktopMainWindow gui;
};

Kryvo::ApplicationPrivate::ApplicationPrivate()
  : gui{&settings} {
}

Kryvo::Application::Application(QObject* parent)
  : QObject{parent}, d_ptr{std::make_unique<ApplicationPrivate>()} {
  Q_D(Application);

  qRegisterMetaType<std::size_t>("std::size_t");

  // Move cryptography object to another thread to prevent GUI from blocking
  d->cryptography.moveToThread(&d->cryptographyThread);

  // Connect GUI to cryptography object
  connect(&d->gui, &MainWindow::encrypt,
          &d->cryptography, &Crypto::encrypt);

  connect(&d->gui, &MainWindow::decrypt,
          &d->cryptography, &Crypto::decrypt);

  // Connections are direct so the cryptography object can receive communication
  // via flags from a different thread while it is runs a cipher operation
  connect(&d->gui, &MainWindow::pauseCipher,
          &d->cryptography, &Crypto::pause,
          Qt::DirectConnection);

  connect(&d->gui, &MainWindow::abortCipher,
          &d->cryptography, &Crypto::abort,
          Qt::DirectConnection);

  connect(&d->gui, &MainWindow::stopFile,
          &d->cryptography, &Crypto::stop,
          Qt::DirectConnection);

  // Update cipher progress bars
  connect(&d->cryptography, &Crypto::fileProgress,
          &d->gui, &MainWindow::updateFileProgress);

  // Update cipher status message
  connect(&d->cryptography, &Crypto::statusMessage,
          &d->gui, &MainWindow::updateStatusMessage);

  // Update cipher error message
  connect(&d->cryptography, &Crypto::errorMessage,
          &d->gui, &MainWindow::updateError);

  // Update cipher operation in progress status
  connect(&d->cryptography, &Crypto::busyStatus,
          &d->gui, &MainWindow::updateBusyStatus);

  // Update archive operation in progress status
//  connect(&d->archiver, &Archiver::progress,
//          &gui, &MainWindow::archiveFileProgress);

  // Connect archiver's extracted signal to crypto provider's decrypt
//  connect(&d->archiver, &Archiver::extractedFile,
//          this, &Crypto::decrypt);

//  connect(&d->cryptography, &Crypto::extract,
//          &d->archiver, &Archiver::extract);

  d->cryptographyThread.start();
  d->archiverThread.start();

  d->gui.show();
}

Kryvo::Application::~Application() {
  Q_D(Application);

  // Abort current threaded cipher operation
  d->cryptography.abort();
}
