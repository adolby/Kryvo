#include "Application.hpp"
#include "Ui.hpp"
#include "cryptography/EncryptFileConfig.hpp"
#include "cryptography/DecryptFileConfig.hpp"
#include "cryptography/EncryptConfig.hpp"
#include "cryptography/DecryptConfig.hpp"
#include "archive/CompressFileConfig.hpp"
#include "archive/DecompressFileConfig.hpp"
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
  explicit ApplicationPrivate(Application* app);

  Application* const q_ptr{nullptr};

  Scheduler scheduler;
  Thread schedulerThread;
  Settings settings;
  Ui gui{&settings};
};

ApplicationPrivate::ApplicationPrivate(Application* app)
  : q_ptr(app) {
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
  qRegisterMetaType<Kryvo::CompressFileConfig>("Kryvo::CompressFileConfig");
  qRegisterMetaType<Kryvo::DecompressFileConfig>("Kryvo::DecompressFileConfig");

  QObject::connect(q, &Application::back, &gui, &Ui::navigateBack);

  QObject::connect(&gui, &Ui::quitApp, q, &Application::quit);

  // Connect GUI encrypt/decrypt actions
  QObject::connect(&gui, &Ui::encrypt,
                   &scheduler, &Scheduler::encrypt);

  QObject::connect(&gui, &Ui::decrypt,
                   &scheduler, &Scheduler::decrypt);

  // Connections are direct so the cryptography object can receive communication
  // via flags from a different thread while it is runnning a cipher operation
  QObject::connect(&gui, &Ui::pause,
                   &scheduler, &Scheduler::pause,
                   Qt::DirectConnection);

  QObject::connect(&gui, &Ui::cancel,
                   &scheduler, &Scheduler::cancel,
                   Qt::DirectConnection);

  QObject::connect(&gui, &Ui::stopFile,
                   &scheduler, &Scheduler::stop,
                   Qt::DirectConnection);

  // Update progress bars
  QObject::connect(&scheduler, &Scheduler::fileProgress,
                   &gui, &Ui::updateFileProgress);

  // Update status message
  QObject::connect(&scheduler, &Scheduler::statusMessage,
                   &gui, &Ui::appendStatusMessage);

  // Update error message
  QObject::connect(&scheduler, &Scheduler::errorMessage,
                   &gui, &Ui::appendErrorMessage);

  QObject::connect(&scheduler, &Scheduler::cryptoProvidersLoaded,
                   &settings, &Settings::cryptoProvidersLoaded);

  scheduler.moveToThread(&schedulerThread);

  schedulerThread.start();
}

Application::Application(int& argc, char** argv)
  : QGuiApplication(argc, argv),
    d_ptr(std::make_unique<ApplicationPrivate>(this)) {
}

Application::~Application() {
  Q_D(Application);

  // Cancel current threaded operations
  d->scheduler.cancel();
}

bool Application::notify(QObject* receiver, QEvent* event) {
#if defined(Q_OS_ANDROID)
  if (QEvent::Close == event->type()) {
    emit back();
    return false;
  } else {
    return QGuiApplication::notify(receiver, event);
  }
#else
  return QGuiApplication::notify(receiver, event);
#endif
}

} // namespace Kryvo
