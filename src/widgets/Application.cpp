#include "Application.hpp"
#include "MainWindow.hpp"
#include "DesktopMainWindow.hpp"
#include "cryptography/EncryptFileConfig.hpp"
#include "cryptography/DecryptFileConfig.hpp"
#include "cryptography/EncryptConfig.hpp"
#include "cryptography/DecryptConfig.hpp"
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
  Q_DECLARE_PUBLIC(Application)

 public:
  ApplicationPrivate(Application* a);

  Application* const q_ptr{nullptr};

  Scheduler scheduler;
  Thread schedulerThread;
  Settings settings;
  DesktopMainWindow gui{&settings};
};

ApplicationPrivate::ApplicationPrivate(Application* a)
  : q_ptr(a) {
  Q_Q(Application);

  qRegisterMetaType<std::size_t>("std::size_t");
  qRegisterMetaType<QFileInfo>("QFileInfo");
  qRegisterMetaType<std::vector<QFileInfo>>("std::vector<QFileInfo>");
  qRegisterMetaType<QDir>("QDir");
  qRegisterMetaType<QHash<QString, Plugin>>("QHash<QString, Plugin>");
  qRegisterMetaType<Kryvo::EncryptConfig>("Kryvo::EncryptConfig");
  qRegisterMetaType<Kryvo::DecryptConfig>("Kryvo::DecryptConfig");
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

  QObject::connect(&gui, &MainWindow::cancel,
                   &scheduler, &Scheduler::cancel,
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

  QObject::connect(&scheduler, &Scheduler::cryptoProvidersLoaded,
                   &settings, &Settings::cryptoProvidersLoaded);

  scheduler.moveToThread(&schedulerThread);

  schedulerThread.start();

  QTimer::singleShot(0, q,
                     [this]() {
                       gui.show();
                     });
}

Application::Application(int& argc, char** argv)
  : QApplication(argc, argv),
    d_ptr(std::make_unique<ApplicationPrivate>(this)) {
}

Application::~Application() {
  Q_D(Application);

  // Cancel current threaded operation
  d->scheduler.cancel();
}

} // namespace Kryvo
