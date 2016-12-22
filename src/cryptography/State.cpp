#include "src/cryptography/State.hpp"

Kryvos::State::State()
  : aborted{false}, paused{false}, busyStatus{false} {
  // Reserve a small number of elements to improve dictionary performance
  stopped.reserve(100);
}

void Kryvos::State::abort(const bool abort) {
  aborted = abort;
}

bool Kryvos::State::isAborted() const {
  return aborted;
}

void Kryvos::State::pause(const bool pause) {
  paused = pause;
}

bool Kryvos::State::isPaused() const {
  return paused;
}

void Kryvos::State::stop(const QString& filePath, const bool stop) {
  stopped.insert(filePath, stop);
}

bool Kryvos::State::isStopped(const QString& filePath) const {
  return stopped.value(filePath, false);
}

void Kryvos::State::reset() {
  aborted = false;
  stopped.clear();
}

void Kryvos::State::busy(const bool busy) {
  busyStatus = busy;
}

bool Kryvos::State::isBusy() const {
  return busyStatus;
}
