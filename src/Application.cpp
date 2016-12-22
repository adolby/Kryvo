#include "src/Application.hpp"
#include "src/gui/MainWindow.hpp"
#if defined(Q_OS_ANDROID) || defined(Q_OS_IOS)
#include "src/gui/TouchMainWindow.hpp"
#else
#include "src/gui/DesktopMainWindow.hpp"
#endif
#include "src/cryptography/Manager.hpp"
#include "src/settings/Settings.hpp"
#include "src/utility/pimpl_impl.h"
#include <QThread>
#include <memory>

#if defined(_MSC_VER)
#include "src/utility/make_unique.h"
#endif

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

  std::unique_ptr<Settings> settings;
  std::unique_ptr<MainWindow> gui;
  std::unique_ptr<Manager> cryptography;
  std::unique_ptr<QThread> cipherThread;
};

Kryvos::App::Application::Application(QObject* parent)
  : QObject{parent} {
  qRegisterMetaType<std::size_t>("std::size_t");

  // Move cryptography object to another thread to prevent GUI from blocking
  m->cryptography->moveToThread(m->cipherThread.get());

  // Connect GUI to cryptography object
  connect(m->gui.get(), &MainWindow::encrypt,
          m->cryptography.get(), &Manager::encrypt);

  connect(m->gui.get(), &MainWindow::decrypt,
          m->cryptography.get(), &Manager::decrypt);

  // Connections are direct so the cryptography object can receive communication
  // via flags from a different thread while it is runs a cipher operation
  connect(m->gui.get(), &MainWindow::pauseCipher,
          m->cryptography.get(), &Manager::pause,
          Qt::DirectConnection);

  connect(m->gui.get(), &MainWindow::abortCipher,
          m->cryptography.get(), &Manager::abort,
          Qt::DirectConnection);

  connect(m->gui.get(), &MainWindow::stopFile,
          m->cryptography.get(), &Manager::stop,
          Qt::DirectConnection);

  // Update progress bars
  connect(m->cryptography.get(), &Manager::progress,
          m->gui.get(), &MainWindow::updateProgress);

  // Update status message
  connect(m->cryptography.get(), &Manager::statusMessage,
          m->gui.get(), &MainWindow::updateStatusMessage);

  // Update error message
  connect(m->cryptography.get(), &Manager::errorMessage,
          m->gui.get(), &MainWindow::updateError);

  // Update cipher operation in progress status
  connect(m->cryptography.get(), &Manager::busyStatus,
          m->gui.get(), &MainWindow::updateBusyStatus);

  m->cipherThread->start();

  m->gui->show();
}

Kryvos::App::Application::~Application() {
  // Abort current threaded cipher operation
  m->cryptography->abort();

  // Quit the cipher thread
  m->cipherThread->quit();

  const auto timedOut = !m->cipherThread->wait(1000);

  // If the thread couldn't quit within one second, terminate it
  if (timedOut) {
    m->cipherThread->terminate();
  }
}

Kryvos::App::Application::ApplicationPrivate::ApplicationPrivate()
  : settings{std::make_unique<Settings>()},
    #if defined(Q_OS_ANDROID) || defined(Q_OS_IOS)
    gui{std::make_unique<TouchMainWindow>(settings.get())},
    #else
    gui{std::make_unique<DesktopMainWindow>(settings.get())},
    #endif
    cryptography{std::make_unique<Manager>()},
    cipherThread{std::make_unique<QThread>()} {
}
