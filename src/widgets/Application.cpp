#include "Application.hpp"
#include "MainWindow.hpp"
#include "DesktopMainWindow.hpp"
#include "cryptography/EncryptFileConfig.hpp"
#include "cryptography/DecryptFileConfig.hpp"
#include "settings/Settings.hpp"
#include "Scheduler.hpp"
#include "Plugin.hpp"
#include "utility/Thread.hpp"
#include <QFileInfo>
#include <QDir>
#include <QHash>
#include <QString>

namespace Kryvo {

class ApplicationPrivate {
  Q_DISABLE_COPY(ApplicationPrivate)

 public:
  ApplicationPrivate();

  Scheduler scheduler;
  Thread schedulerThread;
  Settings settings;
  DesktopMainWindow gui{&settings};
};

ApplicationPrivate::ApplicationPrivate() {
  qRegisterMetaType<std::size_t>("std::size_t");
  qRegisterMetaType<QFileInfo>("QFileInfo");
  qRegisterMetaType<std::vector<QFileInfo>>("std::vector<QFileInfo>");
  qRegisterMetaType<QDir>("QDir");
  qRegisterMetaType<QHash<QString, Plugin>>("QHash<QString, Plugin>");
  qRegisterMetaType<Kryvo::EncryptFileConfig>("Kryvo::EncryptFileConfig");
  qRegisterMetaType<Kryvo::DecryptFileConfig>("Kryvo::DecryptFileConfig");

  // Connect GUI encrypt/decrypt actions
  QObject::connect(&gui, &MainWindow::encrypt,
                   &scheduler, &Scheduler::encrypt);

  QObject::connect(&gui, &MainWindow::decrypt,
                   &scheduler, &Scheduler::decrypt);

  // Connections are direct so the cryptography object can receive communication
  // via flags from a different thread while it is runnning a cipher operation
  QObject::connect(&gui, &MainWindow::pause,
                   &scheduler, &Scheduler::pause,
                   Qt::DirectConnection);

  QObject::connect(&gui, &MainWindow::abort,
                   &scheduler, &Scheduler::abort,
                   Qt::DirectConnection);

  QObject::connect(&gui, &MainWindow::stopFile,
                   &scheduler, &Scheduler::stop,
                   Qt::DirectConnection);

  // Update progress bars
  QObject::connect(&scheduler, &Scheduler::fileProgress,
                   &gui, &MainWindow::updateFileProgress);

  // Update status message
  QObject::connect(&scheduler, &Scheduler::statusMessage,
                   &gui, &MainWindow::updateStatusMessage);

  // Update error message
  QObject::connect(&scheduler, &Scheduler::errorMessage,
                   &gui, &MainWindow::updateError);

  scheduler.moveToThread(&schedulerThread);

  schedulerThread.start();

  gui.show();
}

Application::Application(int& argc, char** argv)
  : QApplication(argc, argv), d_ptr(std::make_unique<ApplicationPrivate>()) {
}

Application::~Application() {
  Q_D(Application);

  // Abort current threaded operation
  d->scheduler.abort();
}

} // namespace Kryvo
