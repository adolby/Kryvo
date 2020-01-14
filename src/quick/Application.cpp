#include "Application.hpp"
#include "Ui.hpp"
#include "settings/Settings.hpp"
#include "Scheduler.hpp"
#include "Plugin.hpp"
#include "utility/Thread.hpp"
#include <QFileInfo>
#include <QDir>
#include <QHash>
#include <QString>

#if defined(Q_OS_ANDROID)
#include <QtAndroid>

bool checkPermissions() {
  QtAndroid::PermissionResult result =
    QtAndroid::checkPermission(
      QStringLiteral("android.permission.WRITE_EXTERNAL_STORAGE"));

  if (QtAndroid::PermissionResult::Denied == result) {
    QStringList permissions;

    permissions.append(
      QStringLiteral("android.permission.WRITE_EXTERNAL_STORAGE"));

    QtAndroid::requestPermissionsSync(permissions, 2000);

    result =
      QtAndroid::checkPermission(
        QStringLiteral("android.permission.WRITE_EXTERNAL_STORAGE"));

    if (QtAndroid::PermissionResult::Denied == result) {
      return false;
    }
  }

  return true;
}
#endif

class Kryvo::ApplicationPrivate {
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

Kryvo::ApplicationPrivate::ApplicationPrivate(Application* app)
  : q_ptr(app) {
  qRegisterMetaType<std::size_t>("std::size_t");
  qRegisterMetaType<QFileInfo>("QFileInfo");
  qRegisterMetaType<std::vector<QFileInfo>>("std::vector<QFileInfo>");
  qRegisterMetaType<QDir>("QDir");
  qRegisterMetaType<QHash<QString, Plugin>>("QHash<QString, Plugin>");

  QObject::connect(q_ptr, &Application::back, &gui, &Ui::navigateBack);

  QObject::connect(&gui, &Ui::quitApp, q_ptr, &Application::quit);

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

  QObject::connect(&gui, &Ui::abort,
                   &scheduler, &Scheduler::abort,
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

  scheduler.moveToThread(&schedulerThread);

  schedulerThread.start();

#if defined(Q_OS_ANDROID)
  checkPermissions();
#endif
}

Kryvo::Application::Application(int& argc, char** argv)
  : QGuiApplication(argc, argv),
    d_ptr(std::make_unique<ApplicationPrivate>(this)) {
}

Kryvo::Application::~Application() {
  Q_D(Application);

  // Abort current threaded operations
  d->scheduler.abort();
}

bool Kryvo::Application::notify(QObject* receiver, QEvent* event) {
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
