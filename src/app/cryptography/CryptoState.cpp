#include "cryptography/CryptoState.hpp"

Kryvo::CryptoState::CryptoState()
  : aborted(false), paused(false), busyStatus(false) {
  // Reserve a small number of elements to improve dictionary performance
  stopped.reserve(100);
}

void Kryvo::CryptoState::abort(const bool abort) {
  aborted = abort;
}

bool Kryvo::CryptoState::isAborted() const {
  return aborted;
}

void Kryvo::CryptoState::pause(const bool pause) {
  paused = pause;
}

bool Kryvo::CryptoState::isPaused() const {
  return paused;
}

void Kryvo::CryptoState::stop(const QString& filePath, const bool stop) {
  stopped.insert(filePath, stop);
}

bool Kryvo::CryptoState::isStopped(const QString& filePath) const {
  return stopped.value(filePath, false);
}

void Kryvo::CryptoState::reset() {
  aborted = false;
  stopped.clear();
}

void Kryvo::CryptoState::busy(const bool busy) {
  busyStatus = busy;
}

bool Kryvo::CryptoState::isBusy() const {
  return busyStatus;
}
