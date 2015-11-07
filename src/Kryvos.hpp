/**
 * Kryvos File Encryptor - Encrypts and decrypts files.
 * Copyright (C) 2014 Andrew Dolby
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Contact : andrewdolby@gmail.com
 */

#ifndef KRYVOS_KRYVOS_HPP_
#define KRYVOS_KRYVOS_HPP_

#include <QtCore/QObject>
#include <memory>

/*!
 * \brief The Kryvos class
 */
class Kryvos : public QObject {
  Q_OBJECT

 public:
  /*!
   * \brief Kryvos Constructs the Kryvos class. Connects the GUI to the
   * cryptography object. Moves the cryptography object to a work
   * thread. Starts the thread and shows the GUI main window.
   * \param parent
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
  std::unique_ptr<KryvosPrivate> pimpl;
};

#endif // KRYVOS_KRYVOS_HPP_
