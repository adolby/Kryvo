#ifndef KRYVO_DISPATCHERSTATE_HPP_
#define KRYVO_DISPATCHERSTATE_HPP_

#include <QString>
#include <QReadWriteLock>
#include <atomic>
#include <deque>
#include <vector>

namespace Kryvo {

class DispatcherState {
 public:
  DispatcherState();

  /*!
   * \brief init Reset the abort flag and initialize the stop flags
   */
  void init(std::size_t maxId);

  /*!
   * \brief abort Updates the abort status
   * \param abort Boolean representing the abort status
   */
  void abort(bool abort);

  /*!
   * \brief isAborted Returns the current abort status
   * \return Boolean representing the abort status
   */
  bool isAborted() const;

  /*!
   * \brief pause Updates the pause status
   * \param pause Boolean representing the pause status
   */
  void pause(bool pause);

  /*!
   * \brief isPaused Returns the current pause status
   * \return Boolean representing the pause status
   */
  bool isPaused() const;

  /*!
   * \brief stop Updates a stop status in the stop status container. A stop
   * status corresponds with a file path and is used to decide whether to stop
   * encrypting/decrypting a file.
   * \param id ID representing the file to set to stopped
   * \param stop Boolean representing the stop status for the file represented
   * by filePath
   */
  void stop(std::size_t id, bool stop);

  /*!
   * \brief isStopped Returns a stop status in the stop status container. A stop
   * status corresponds with a file path and is used to decide whether to stop
   * encrypting/decrypting a file.
   * \param id ID representing the file to check the stop status
   * for
   * \return Boolean Boolean representing the stop status for the file
   * represented by filePath
   */
  bool isStopped(std::size_t id);

  /*!
   * \brief busy Updates the busy status
   * \param busy Boolean representing the busy status
   */
  void busy(bool busy);

  /*!
   * \brief isBusy Returns the busy status
   * \return Boolean representing the busy status
   */
  bool isBusy() const;

 private:
  // The abort status, when set to true, will stop an executing job
  // operation and prevent new job operations from starting until it is reset
  // to false.
  std::atomic<bool> aborted;

  // The pause status, when set to false, will pause an executing cipher
  // operation. When the pause status is set to true, the job operation
  // that was in progress when the pause status was set will resume execution.
  std::atomic<bool> paused;

  // The container of stopped flags, which are used to stop
  // encrypting/decrypting a file.
  std::deque<bool> stopped;

  QReadWriteLock stoppedLock;

  // The busy status, when set to true, indicates that this class is currently
  // executing a cipher operation.
  std::atomic<bool> busyStatus;
};

} // namespace Kryvo

#endif // KRYVO_DISPATCHERSTATE_HPP_
