#include "SchedulerState.hpp"
#include <QWriteLocker>
#include <QReadLocker>

Kryvo::SchedulerState::SchedulerState()
  : state(ExecutionState::Idle), aborted(false) {
}

void Kryvo::SchedulerState::init(const std::size_t maxPipelineId) {
  aborted = false;

  QWriteLocker locker(&stoppedLock);
  stopped.clear();
  stopped.resize(maxPipelineId);
}

void Kryvo::SchedulerState::abort(const bool abort) {
  aborted = abort;
}

bool Kryvo::SchedulerState::isAborted() const {
  return aborted;
}

void Kryvo::SchedulerState::pause(const bool pause) {
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

bool Kryvo::SchedulerState::isPaused() {
  QReadLocker lock(&stateLock);

  return ExecutionState::Paused == state;
}

void Kryvo::SchedulerState::pauseWait(const std::size_t pipelineId) {
  QReadLocker lock(&stateLock);

  while (isPaused() && !(isAborted() || isStopped(pipelineId))) {
    pauseWaitCondition.wait(&stateLock);
  }
}

void Kryvo::SchedulerState::stop(const std::size_t pipelineId,
                                  const bool stop) {
  QWriteLocker locker(&stoppedLock);

  if (pipelineId < stopped.size()) {
    stopped[pipelineId] = stop;
  }
}

bool Kryvo::SchedulerState::isStopped(const std::size_t pipelineId) {
  QReadLocker locker(&stoppedLock);

  bool hasBeenStopped = false;

  if (pipelineId < stopped.size()) {
    hasBeenStopped = stopped.at(pipelineId);
  }

  return hasBeenStopped;
}

void Kryvo::SchedulerState::running(const bool run) {
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

bool Kryvo::SchedulerState::isRunning() {
  QReadLocker lock(&stateLock);

  return ExecutionState::Running == state;
}
