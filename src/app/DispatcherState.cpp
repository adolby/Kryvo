#include "DispatcherState.hpp"

Kryvo::DispatcherState::DispatcherState()
  : aborted(false), paused(false), busyStatus(false) {
}

void Kryvo::DispatcherState::abort(const bool abort) {
  aborted = abort;
}

bool Kryvo::DispatcherState::isAborted() const {
  return aborted;
}

void Kryvo::DispatcherState::pause(const bool pause) {
  paused = pause;
}

bool Kryvo::DispatcherState::isPaused() const {
  return paused;
}

void Kryvo::DispatcherState::stop(const int id, const bool stop) {
  stopped[id] = stop;
}

bool Kryvo::DispatcherState::isStopped(const int id) const {
  bool hasBeenStopped = false;

  if (id < stopped.size()) {
    hasBeenStopped = stopped.at(id);
  }

  return hasBeenStopped;
}

void Kryvo::DispatcherState::reset() {
  aborted = false;
  stopped.clear();
}

void Kryvo::DispatcherState::busy(const bool busy) {
  busyStatus = busy;
}

bool Kryvo::DispatcherState::isBusy() const {
  return busyStatus;
}
