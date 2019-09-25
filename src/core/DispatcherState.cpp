#include "DispatcherState.hpp"
#include <QWriteLocker>
#include <QReadLocker>
#include <QMutexLocker>

Kryvo::DispatcherState::DispatcherState()
  : aborted(false), paused(false), busyStatus(false) {
}

void Kryvo::DispatcherState::init(const std::size_t maxPipelineId) {
  aborted = false;

  QWriteLocker locker(&stoppedLock);
  stopped.clear();
  stopped.resize(maxPipelineId);
}

void Kryvo::DispatcherState::abort(const bool abort) {
  aborted = abort;
}

bool Kryvo::DispatcherState::isAborted() const {
  return aborted;
}

void Kryvo::DispatcherState::pause(const bool pause) {
  QMutexLocker lock(&pauseMutex);

  paused = pause;

  if (!pause) {
    pauseWaitCondition.wakeAll();
  }
}

bool Kryvo::DispatcherState::isPaused() const {
  return paused;
}

void Kryvo::DispatcherState::pauseWait(const std::size_t pipelineId) {
  QMutexLocker lock(&pauseMutex);

  while (isPaused() && !(isAborted() || isStopped(pipelineId))) {
    pauseWaitCondition.wait(&pauseMutex);
  }
}

void Kryvo::DispatcherState::stop(const std::size_t pipelineId,
                                  const bool stop) {
  QWriteLocker locker(&stoppedLock);

  if (pipelineId < stopped.size()) {
    stopped[pipelineId] = stop;
  }
}

bool Kryvo::DispatcherState::isStopped(const std::size_t pipelineId) {
  QReadLocker locker(&stoppedLock);

  bool hasBeenStopped = false;

  if (pipelineId < stopped.size()) {
    hasBeenStopped = stopped.at(pipelineId);
  }

  return hasBeenStopped;
}

void Kryvo::DispatcherState::busy(const bool busy) {
  busyStatus = busy;
}

bool Kryvo::DispatcherState::isBusy() const {
  return busyStatus;
}
