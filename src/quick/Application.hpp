#ifndef KRYVO_APPLICATION_HPP_
#define KRYVO_APPLICATION_HPP_

#include "utility/pimpl.h"
#include <QObject>
#include <QMetaType>
#include <QGuiApplication>
#include <memory>

Q_DECLARE_METATYPE(std::size_t);

namespace Kryvo {

class ApplicationPrivate;

/*!
 * \brief The Application class controls the functionality of the
 * application.
 */
class Application : public QGuiApplication {
  Q_OBJECT
  Q_DISABLE_COPY(Application)
  DECLARE_PRIVATE(Application)
  std::unique_ptr<ApplicationPrivate> const d_ptr;

 public:
  /*!
   * \brief Application Constructs the application.
   */
  Application(int& argc, char** argv);

  /*!
   * \brief ~Application Destroys the application.
   */
  ~Application() override;

  bool notify(QObject* receiver, QEvent* event) override;

 signals:
  void back();
};

} // namespace Kryvo

#endif // KRYVO_APPLICATION_HPP_
