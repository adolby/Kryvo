#ifndef KRYVOS_CRYPTOGRAPHY_CRYPTOSTATE_HPP_
#define KRYVOS_CRYPTOGRAPHY_CRYPTOSTATE_HPP_

#include <QHash>
#include <QString>
#include <atomic>

namespace Kryvos {

inline namespace Cryptography {

class State {
 public:
  /*!
   * \brief State Constructs the State class
   */
  explicit State();

  /*!
   * \brief reset Resets the status, except pause and busy, to default values
   */
  void reset();

  /*!
   * \brief abort Updates the abort status
   * \param abort Boolean representing the abort status
   */
  void abort(const bool abort);

  /*!
   * \brief isAborted Returns the current abort status
   * \return Boolean representing the abort status
   */
  bool isAborted() const;

  /*!
   * \brief pause Updates the pause status
   * \param pause Boolean representing the pause status
   */
  void pause(const bool pause);

  /*!
   * \brief isPaused Returns the current pause status
   * \return Boolean representing the pause status
   */
  bool isPaused() const;

  /*!
   * \brief stop Updates a stop status in the stop status container. A stop
   * status corresponds with a file path and is used to decide whether to stop
   * encrypting/decrypting a file.
   * \param filePath String containing the path of the file to set to stopped
   * \param stop Boolean representing the stop status for the file represented
   * by filePath
   */
  void stop(const QString& filePath, const bool stop);

  /*!
   * \brief isStopped Returns a stop status in the stop status container. A stop
   * status corresponds with a file path and is used to decide whether to stop
   * encrypting/decrypting a file.
   * \param filePath String containing the file path to check the stop status
   * for
   * \return Boolean Boolean representing the stop status for the file
   * represented by filePath
   */
  bool isStopped(const QString& filePath) const;

  /*!
   * \brief busy Updates the busy status
   * \param busy Boolean representing the busy status
   */
  void busy(const bool busy);

  /*!
   * \brief isBusy Returns the busy status
   * \return Boolean representing the busy status
   */
  bool isBusy() const;

 private:
  // The abort status, when set to true, will stop an executing cryptopgraphic
  // operation and prevent new cipher operations from starting until it is reset
  // to false.
  std::atomic<bool> aborted;

  // The pause status, when set to false, will pause an executing cipher
  // operation. When the pause status is set to true, the cipher operation
  // that was in progress when the pause status was set will resume execution.
  std::atomic<bool> paused;

  // The container of stopped flags, which are used to stop
  // encrypting/decrypting a file.
  QHash<QString, bool> stopped;

  // The busy status, when set to true, indicates that this class is currently
  // executing a cipher operation.
  std::atomic<bool> busyStatus;
};

}

}

#endif // KRYVOS_CRYPTOGRAPHY_CRYPTOSTATE_HPP_
