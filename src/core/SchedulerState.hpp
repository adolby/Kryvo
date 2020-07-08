#ifndef KRYVO_SCHEDULERSTATE_HPP_
#define KRYVO_SCHEDULERSTATE_HPP_

#include <QWaitCondition>
#include <QReadWriteLock>
#include <QString>
#include <atomic>
#include <deque>
#include <vector>

namespace Kryvo {

class SchedulerState {
 public:
  SchedulerState();

  /*!
   * \brief init Reset the abort flag and initialize the stop flags
   */
  void init(std::size_t maxPipelineId);

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
  bool isPaused();

  void pauseWait(std::size_t pipelineId);

  /*!
   * \brief stop Updates a stop status in the stop status container. A stop
   * status corresponds with a file path and is used to decide whether to stop
   * encrypting/decrypting a file.
   * \param pipelineId ID representing the file to set to stopped
   * \param stop Boolean representing the stop status for the file represented
   * by filePath
   */
  void stop(std::size_t pipelineId, bool stop);

  /*!
   * \brief isStopped Returns a stop status in the stop status container. A stop
   * status corresponds to a file path and is used to decide whether to stop
   * encrypting/decrypting a file.
   * \param pipelineId ID representing the file's stopped status
   * \return Boolean Boolean representing the stopped status for the file
   * represented by filePath
   */
  bool isStopped(std::size_t pipelineId);

  void running(bool run);

  bool isRunning();

 private:
  enum ExecutionState {
    Idle,
    Running,
    Paused };

  ExecutionState state_;
  QReadWriteLock stateLock_;
  QWaitCondition pauseWaitCondition_;

  // The abort status, when set to true, will stop an executing job
  // operation and prevent new job operations from starting until it is reset
  // to false.
  std::atomic<bool> aborted_;

  // The container of stopped flags, which are used to stop
  // encrypting/decrypting a file.
  std::deque<bool> stopped_;
  QReadWriteLock stoppedLock_;
};

} // namespace Kryvo

#endif // KRYVO_SCHEDULERSTATE_HPP_
