#include "src/Application.hpp"
#include "src/gui/MainWindow.hpp"
#include "src/gui/DesktopMainWindow.hpp"
#include "src/cryptography/Crypto.hpp"
#include "src/settings/Settings.hpp"
#include "src/utility/Thread.hpp"
#include "src/utility/pimpl_impl.h"
#include <memory>

/*!
 * \brief ApplicationPrivate class
 */
class Kryvos::App::Application::ApplicationPrivate {
 public:
  /*!
   * \brief ApplicationPrivate Constructs the app private implementation which
   * contains the GUI and the cryptography object that interfaces with Botan.
   * Initializes the cryptography work thread.
   */
  ApplicationPrivate();

  Settings settings;
  Crypto cryptography;
  Thread cipherThread;
  DesktopMainWindow gui;
};

Kryvos::App::Application::Application(QObject* parent)
  : QObject{parent} {
  qRegisterMetaType<std::size_t>("std::size_t");

  // Move cryptography object to another thread to prevent GUI from blocking
  m->cryptography.moveToThread(&m->cipherThread);

  // Connect GUI to cryptography object
  connect(&m->gui, &MainWindow::encrypt,
          &m->cryptography, &Crypto::encrypt);

  connect(&m->gui, &MainWindow::decrypt,
          &m->cryptography, &Crypto::decrypt);

  // Connections are direct so the cryptography object can receive communication
  // via flags from a different thread while it is runs a cipher operation
  connect(&m->gui, &MainWindow::pauseCipher,
          &m->cryptography, &Crypto::pause,
          Qt::DirectConnection);

  connect(&m->gui, &MainWindow::abortCipher,
          &m->cryptography, &Crypto::abort,
          Qt::DirectConnection);

  connect(&m->gui, &MainWindow::stopFile,
          &m->cryptography, &Crypto::stop,
          Qt::DirectConnection);

  // Update progress bars
  connect(&m->cryptography, &Crypto::fileProgress,
          &m->gui, &MainWindow::updateFileProgress);

  // Update status message
  connect(&m->cryptography, &Crypto::statusMessage,
          &m->gui, &MainWindow::updateStatusMessage);

  // Update error message
  connect(&m->cryptography, &Crypto::errorMessage,
          &m->gui, &MainWindow::updateError);

  // Update cipher operation in progress status
  connect(&m->cryptography, &Crypto::busyStatus,
          &m->gui, &MainWindow::updateBusyStatus);

  m->cipherThread.start();

  m->gui.show();
}

Kryvos::App::Application::~Application() {
  // Abort current threaded cipher operation
  m->cryptography.abort();
}

Kryvos::App::Application::ApplicationPrivate::ApplicationPrivate()
  : gui{&settings} {
}
