#include "src/cryptography/CryptoState.hpp"

CryptoState::CryptoState()
  : aborted{false}, paused{false}, busyStatus{false} {
  // Reserve a small number of elements to improve dictionary performance
  stopped.reserve(100);
}

void CryptoState::abort(const bool abort) {
  aborted = abort;
}

bool CryptoState::isAborted() const {
  return aborted;
}

void CryptoState::pause(const bool pause) {
  paused = pause;
}

bool CryptoState::isPaused() const {
  return paused;
}

void CryptoState::stop(const QString& filePath, const bool stop) {
  stopped.insert(filePath, stop);
}

bool CryptoState::isStopped(const QString& filePath) const {
  return stopped.value(filePath, false);
}

void CryptoState::reset() {
  aborted = false;
  stopped.clear();
}

void CryptoState::busy(const bool busy) {
  busyStatus = busy;
}

bool CryptoState::isBusy() const {
  return busyStatus;
}
