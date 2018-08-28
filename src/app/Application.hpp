#ifndef KRYVO_APPLICATION_HPP_
#define KRYVO_APPLICATION_HPP_

#include "utility/pimpl.h"
#include <QObject>
#include <QMetaType>
#include <memory>

Q_DECLARE_METATYPE(std::size_t);

namespace Kryvo {

class ApplicationPrivate;

/*!
 * \brief The Application class controls the functionality of the
 * application.
 */
class Application : public QObject {
  Q_OBJECT
  DECLARE_PRIVATE(Application)
  std::unique_ptr<ApplicationPrivate> const d_ptr;

 public:
  /*!
   * \brief Application Constructs the application. Connects the GUI to the
   * cryptography object. Moves the cryptography object to a work thread. Starts
   * the thread and shows the GUI main window.
   * \param parent Parent object
   */
  explicit Application(QObject* parent = nullptr);

  /*!
   * \brief ~Application Destroys the application. Aborts the current threaded
   * operation and then quits the thread. If the thread doesn't respond, it will
   * be terminated so the application can exit.
   */
  ~Application() override;
};

} // namespace Kryvo

#endif // KRYVO_APPLICATION_HPP_
