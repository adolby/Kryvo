#include "SchedulerState.hpp"
#include <QWriteLocker>
#include <QReadLocker>

Kryvo::SchedulerState::SchedulerState()
  : state_(ExecutionState::Idle), aborted_(false) {
}

void Kryvo::SchedulerState::init(const std::size_t maxPipelineId) {
  aborted_ = false;

  QWriteLocker locker(&stoppedLock_);
  stopped_.clear();
  stopped_.resize(maxPipelineId);
}

void Kryvo::SchedulerState::abort(const bool abort) {
  aborted_ = abort;
}

bool Kryvo::SchedulerState::isAborted() const {
  return aborted_;
}

void Kryvo::SchedulerState::pause(const bool pause) {
  QWriteLocker lock(&stateLock_);

  if (pause) {
    if (state_ != ExecutionState::Running) {
      return;
    }

    state_ = ExecutionState::Paused;
  } else {
    if (state_ != ExecutionState::Paused) {
      return;
    }

    state_ = ExecutionState::Running;

    pauseWaitCondition_.wakeAll();
  }
}

bool Kryvo::SchedulerState::isPaused() {
  QReadLocker lock(&stateLock_);

  return ExecutionState::Paused == state_;
}

void Kryvo::SchedulerState::pauseWait(const std::size_t pipelineId) {
  QReadLocker lock(&stateLock_);

  while (isPaused() && !(isAborted() || isStopped(pipelineId))) {
    pauseWaitCondition_.wait(&stateLock_);
  }
}

void Kryvo::SchedulerState::stop(const std::size_t pipelineId,
                                 const bool stop) {
  QWriteLocker locker(&stoppedLock_);

  if (pipelineId < stopped_.size()) {
    stopped_[pipelineId] = stop;
  }
}

bool Kryvo::SchedulerState::isStopped(const std::size_t pipelineId) {
  QReadLocker locker(&stoppedLock_);

  bool hasBeenStopped = false;

  if (pipelineId < stopped_.size()) {
    hasBeenStopped = stopped_.at(pipelineId);
  }

  return hasBeenStopped;
}

void Kryvo::SchedulerState::running(const bool run) {
  QWriteLocker locker(&stateLock_);

  if (run) {
    if (state_ != ExecutionState::Idle) {
      return;
    }

    state_ = ExecutionState::Running;
  } else {
    if (state_ != ExecutionState::Running) {
      return;
    }

    state_ = ExecutionState::Idle;
  }
}

bool Kryvo::SchedulerState::isRunning() {
  QReadLocker lock(&stateLock_);

  return ExecutionState::Running == state_;
}
