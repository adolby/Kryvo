#include "DispatcherState.hpp"
#include <QWriteLocker>
#include <QReadLocker>

Kryvo::DispatcherState::DispatcherState()
  : state(ExecutionState::Idle), aborted(false) {
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
  QWriteLocker lock(&stateLock);

  if (pause) {
    if (state != ExecutionState::Running) {
      return;
    }

    state = ExecutionState::Paused;
  } else {
    if (state != ExecutionState::Paused) {
      return;
    }

    state = ExecutionState::Running;

    pauseWaitCondition.wakeAll();
  }
}

bool Kryvo::DispatcherState::isPaused() {
  QReadLocker lock(&stateLock);

  return ExecutionState::Paused == state;
}

void Kryvo::DispatcherState::pauseWait(const std::size_t pipelineId) {
  QReadLocker lock(&stateLock);

  while (isPaused() && !(isAborted() || isStopped(pipelineId))) {
    pauseWaitCondition.wait(&stateLock);
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

void Kryvo::DispatcherState::running(const bool run) {
  QWriteLocker locker(&stateLock);

  if (run) {
    if (state != ExecutionState::Idle) {
      return;
    }

    state = ExecutionState::Running;
  } else {
    if (state != ExecutionState::Running) {
      return;
    }

    state = ExecutionState::Idle;
  }
}

bool Kryvo::DispatcherState::isRunning() {
  QReadLocker lock(&stateLock);

  return ExecutionState::Running == state;
}
