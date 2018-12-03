#include "DispatcherState.hpp"

Kryvo::DispatcherState::DispatcherState()
  : aborted(false), paused(false), busyStatus(false) {
  // Reserve a small number of elements to improve dictionary performance
  stopped.reserve(100);
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

void Kryvo::DispatcherState::stop(const QString& filePath, const bool stop) {
  stopped.insert(filePath, stop);
}

bool Kryvo::DispatcherState::isStopped(const QString& filePath) const {
  return stopped.value(filePath, false);
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
