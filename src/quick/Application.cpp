#include "Application.hpp"
#include "Ui.hpp"
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
  Ui gui{&settings};
};

Kryvo::ApplicationPrivate::ApplicationPrivate() {
  qRegisterMetaType<std::size_t>("std::size_t");
  qRegisterMetaType<QFileInfo>("QFileInfo");
  qRegisterMetaType<std::vector<QFileInfo>>("std::vector<QFileInfo>");
  qRegisterMetaType<QDir>("QDir");

  // Connect GUI encrypt/decrypt actions
  QObject::connect(&gui, &Ui::encrypt,
                   &dispatcher, &Dispatcher::encrypt);

  QObject::connect(&gui, &Ui::decrypt,
                   &dispatcher, &Dispatcher::decrypt);

  // Connections are direct so the cryptography object can receive communication
  // via flags from a different thread while it is runnning a cipher operation
  QObject::connect(&gui, &Ui::pause,
                   &dispatcher, &Dispatcher::pause,
                   Qt::DirectConnection);

  QObject::connect(&gui, &Ui::abort,
                   &dispatcher, &Dispatcher::abort,
                   Qt::DirectConnection);

  QObject::connect(&gui, &Ui::stopFile,
                   &dispatcher, &Dispatcher::stop,
                   Qt::DirectConnection);

  // Update progress bars
  QObject::connect(&dispatcher, &Dispatcher::fileProgress,
                   &gui, &Ui::updateFileProgress);

  // Update status message
  QObject::connect(&dispatcher, &Dispatcher::statusMessage,
                   &gui, &Ui::appendStatusMessage);

  // Update error message
  QObject::connect(&dispatcher, &Dispatcher::errorMessage,
                   &gui, &Ui::appendErrorMessage);

  // Update operation in progress status
  QObject::connect(&dispatcher, &Dispatcher::busyStatus,
                   &gui, &Ui::updateBusyStatus);

  dispatcher.moveToThread(&dispatcherThread);

  dispatcherThread.start();
}

Kryvo::Application::Application(QObject* parent)
  : QObject(parent), d_ptr(std::make_unique<ApplicationPrivate>()) {
}

Kryvo::Application::~Application() {
  Q_D(Application);

  // Abort current threaded operations
  d->dispatcher.abort();
}
