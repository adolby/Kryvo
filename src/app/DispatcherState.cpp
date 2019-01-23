#include "DispatcherState.hpp"
#include <QMutexLocker>
#include <QDebug>

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

void Kryvo::DispatcherState::stop(std::size_t id, const bool stop) {
  QMutexLocker locker(&stoppedMutex);

  qDebug() << Q_FUNC_INFO << id << stop;

  if (id < stopped.size()) {
    stopped[id] = stop;
  }
}

bool Kryvo::DispatcherState::isStopped(std::size_t id) {
  QMutexLocker locker(&stoppedMutex);

  bool hasBeenStopped = false;

  qDebug() << Q_FUNC_INFO << "Stopped size: " << stopped.size();

  if (id < stopped.size()) {
    hasBeenStopped = stopped.at(id);
  }

  return hasBeenStopped;
}

void Kryvo::DispatcherState::init(std::size_t maxId) {
  QMutexLocker locker(&stoppedMutex);

  qDebug() << Q_FUNC_INFO << "Max ID: " << maxId;

  aborted = false;
  stopped.clear();
  stopped.resize(maxId);
}

void Kryvo::DispatcherState::busy(const bool busy) {
  busyStatus = busy;
}

bool Kryvo::DispatcherState::isBusy() const {
  return busyStatus;
}
