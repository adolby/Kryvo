#include "Kryvos.hpp"
#include "gui/MainWindow.hpp"
#if defined(Q_OS_ANDROID) || defined(Q_OS_IOS)
#include "gui/TouchMainWindow.hpp"
#else
#include "gui/DesktopMainWindow.hpp"
#endif
#include "cryptography/Crypto.hpp"
#include "settings/Settings.hpp"
#include "utility/pimpl_impl.h"
#include <QThread>

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
  KryvosPrivate();

  /*!
   * \brief ~KryvosPrivate Destroys the Kryvos private implementation.
   */
  virtual ~KryvosPrivate();

  std::unique_ptr<Settings> settings;
  std::unique_ptr<MainWindow> gui;
  std::unique_ptr<Crypto> cryptography;
  std::unique_ptr<QThread> cipherThread;
};

Kryvos::Kryvos(QObject* parent)
  : QObject{parent}
{
  qRegisterMetaType<std::size_t>("std::size_t");

  // Move cryptography object to another thread to prevent GUI from blocking
  m->cryptography->moveToThread(m->cipherThread.get());

  // Connect GUI to cryptography object
  connect(m->gui.get(), &MainWindow::encrypt,
          m->cryptography.get(), &Crypto::encrypt);

  connect(m->gui.get(), &MainWindow::decrypt,
          m->cryptography.get(), &Crypto::decrypt);

  // Connections are direct so the cryptography object can receive communication
  // via flags from a different thread while it is runs a cipher operation
  connect(m->gui.get(), &MainWindow::pauseCipher,
          m->cryptography.get(), &Crypto::pause, Qt::DirectConnection);

  connect(m->gui.get(), &MainWindow::abortCipher,
          m->cryptography.get(), &Crypto::abort, Qt::DirectConnection);

  connect(m->gui.get(), &MainWindow::stopFile,
          m->cryptography.get(), &Crypto::stop, Qt::DirectConnection);

  // Update progress bars
  connect(m->cryptography.get(), &Crypto::progress,
          m->gui.get(), &MainWindow::updateProgress);

  // Update status message
  connect(m->cryptography.get(), &Crypto::statusMessage,
          m->gui.get(), &MainWindow::updateStatusMessage);

  // Update error message
  connect(m->cryptography.get(), &Crypto::errorMessage,
          m->gui.get(), &MainWindow::updateError);

  // Update cipher operation in progress status
  connect(m->cryptography.get(), &Crypto::busyStatus,
          m->gui.get(), &MainWindow::updateBusyStatus);

  m->cipherThread->start();

  m->gui->show();
}

Kryvos::~Kryvos()
{
  // Abort current threaded cipher operation
  m->cryptography->abort();

  // Quit the cipher thread
  m->cipherThread->quit();

  auto timedOut = !m->cipherThread->wait(1000);

  // If the thread couldn't quit within one second, terminate it
  if (timedOut)
  {
    m->cipherThread->terminate();
  }
}

Kryvos::KryvosPrivate::KryvosPrivate()
  : settings{std::make_unique<Settings>()},
    #if defined(Q_OS_ANDROID) || defined(Q_OS_IOS)
    gui{std::make_unique<TouchMainWindow>(settings.get())},
    #else
    gui{std::make_unique<DesktopMainWindow>(settings.get())},
    #endif
    cryptography{std::make_unique<Crypto>()},
    cipherThread{std::make_unique<QThread>()} {}

Kryvos::KryvosPrivate::~KryvosPrivate() {}
