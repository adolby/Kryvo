#include "Application.hpp"
#include "gui/MainWindow.hpp"
#include "gui/DesktopMainWindow.hpp"
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

Kryvo::ApplicationPrivate::ApplicationPrivate() = default;

Kryvo::Application::Application(QObject* parent)
  : QObject(parent), d_ptr(std::make_unique<ApplicationPrivate>()) {
  Q_D(Application);

  qRegisterMetaType<std::size_t>("std::size_t");

  // Connect GUI encrypt/decrypt actions
  connect(&d->gui, &MainWindow::encrypt,
          &d->dispatcher, &Dispatcher::encrypt);

  connect(&d->gui, &MainWindow::decrypt,
          &d->dispatcher, &Dispatcher::decrypt);

  // Connections are direct so the cryptography object can receive communication
  // via flags from a different thread while it is runnning a cipher operation
  connect(&d->gui, &MainWindow::pause,
          &d->dispatcher, &Dispatcher::pause,
          Qt::DirectConnection);

  connect(&d->gui, &MainWindow::abort,
          &d->dispatcher, &Dispatcher::abort,
          Qt::DirectConnection);

  connect(&d->gui, &MainWindow::stopFile,
          &d->dispatcher, &Dispatcher::stop,
          Qt::DirectConnection);

  // Update cipher progress bars
  connect(&d->dispatcher, &Dispatcher::fileProgress,
          &d->gui, &MainWindow::updateFileProgress);

  // Update cipher status message
  connect(&d->dispatcher, &Dispatcher::statusMessage,
          &d->gui, &MainWindow::updateStatusMessage);

  // Update cipher error message
  connect(&d->dispatcher, &Dispatcher::errorMessage,
          &d->gui, &MainWindow::updateError);

  // Update cipher operation in progress status
  connect(&d->dispatcher, &Dispatcher::busyStatus,
          &d->gui, &MainWindow::updateBusyStatus);

  d->dispatcher.moveToThread(&d->dispatcherThread);

  d->dispatcherThread.start();

  d->gui.show();
}

Kryvo::Application::~Application() {
  Q_D(Application);

  // Abort current threaded operation
  d->dispatcher.abort();
}
