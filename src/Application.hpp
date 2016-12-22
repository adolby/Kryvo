#ifndef KRYVOS_APPLICATION_HPP_
#define KRYVOS_APPLICATION_HPP_

#include "src/utility/pimpl.h"
#include <QObject>
#include <QMetaType>

Q_DECLARE_METATYPE(std::size_t);

namespace Kryvos {

inline namespace App {

/*!
 * \brief The Application class controls the functionality of the application.
 */
class Application : public QObject {
  Q_OBJECT

 public:
  /*!
   * \brief Application Constructs the application. Connects the GUI to the
   * cryptography object. Moves the cryptography object to a work
   * thread. Starts the thread and shows the GUI main window.
   * \param parent Parent object
   */
  explicit Application(QObject* parent = nullptr);

  /*!
   * \brief ~Application Destroys the application. Aborts the current threaded
   * operation and then quits the thread. If the thread doesn't respond, it will
   * be terminated so the application can exit.
   */
  virtual ~Application();

 private:
  class ApplicationPrivate;
  Kryvos::pimpl<ApplicationPrivate> m;
};

}

}

#endif // KRYVOS_APPLICATION_HPP_
