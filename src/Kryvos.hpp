#ifndef KRYVOS_KRYVOS_HPP_
#define KRYVOS_KRYVOS_HPP_

#include "src/utility/pimpl.h"
#include <QObject>
#include <QMetaType>

Q_DECLARE_METATYPE(std::size_t);

/*!
 * \brief The Kryvos class controls the functionality of the Kryvos application.
 */
class Kryvos : public QObject {
  Q_OBJECT

 public:
  /*!
   * \brief Kryvos Constructs the Kryvos class. Connects the GUI to the
   * cryptography object. Moves the cryptography object to a work
   * thread. Starts the thread and shows the GUI main window.
   * \param parent Parent object
   */
  explicit Kryvos(QObject* parent = nullptr);

  /*!
   * \brief ~Kryvos Destroys the Kryvos class. Aborts the current threaded
   * operation and then quits the thread. If the thread doesn't respond, it will
   * be terminated so the application can exit.
   */
  virtual ~Kryvos();

 private:
  class KryvosPrivate;
  pimpl<KryvosPrivate> m;
};

#endif // KRYVOS_KRYVOS_HPP_
