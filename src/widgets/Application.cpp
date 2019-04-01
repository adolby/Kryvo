#include "Application.hpp"
#include "MainWindow.hpp"
#include "DesktopMainWindow.hpp"
#include "settings/Settings.hpp"
#include "Dispatcher.hpp"
#include "utility/Thread.hpp"
#include <memory>

class Kryvo::ApplicationPrivate {
  Q_DISABLE_COPY(ApplicationPrivate)

 public:
  ApplicationPrivate();

  Dispatcher dispatcher;
  Thread dispatcherThread;
  Settings settings;
  DesktopMainWindow gui{&settings};
};

Kryvo::ApplicationPrivate::ApplicationPrivate() {
  qRegisterMetaType<std::size_t>("std::size_t");

  // Connect GUI encrypt/decrypt actions
  QObject::connect(&gui, &MainWindow::encrypt,
                   &dispatcher, &Dispatcher::encrypt);

  QObject::connect(&gui, &MainWindow::decrypt,
                   &dispatcher, &Dispatcher::decrypt);

  // Connections are direct so the cryptography object can receive communication
  // via flags from a different thread while it is runnning a cipher operation
  QObject::connect(&gui, &MainWindow::pause,
                   &dispatcher, &Dispatcher::pause,
                   Qt::DirectConnection);

  QObject::connect(&gui, &MainWindow::abort,
                   &dispatcher, &Dispatcher::abort,
                   Qt::DirectConnection);

  QObject::connect(&gui, &MainWindow::stopFile,
                   &dispatcher, &Dispatcher::stop,
                   Qt::DirectConnection);

  // Update progress bars
  QObject::connect(&dispatcher, &Dispatcher::fileProgress,
                   &gui, &MainWindow::updateFileProgress);

  // Update status message
  QObject::connect(&dispatcher, &Dispatcher::statusMessage,
                   &gui, &MainWindow::updateStatusMessage);

  // Update error message
  QObject::connect(&dispatcher, &Dispatcher::errorMessage,
                   &gui, &MainWindow::updateError);

  // Update operation in progress status
  QObject::connect(&dispatcher, &Dispatcher::busyStatus,
                   &gui, &MainWindow::updateBusyStatus);

  dispatcher.moveToThread(&dispatcherThread);

  dispatcherThread.start();

  gui.show();
}

Kryvo::Application::Application(QObject* parent)
  : QObject(parent), d_ptr(std::make_unique<ApplicationPrivate>()) {
}

Kryvo::Application::~Application() {
  Q_D(Application);

  // Abort current threaded operation
  d->dispatcher.abort();
}
