#include "Thread.hpp"

Kryvo::Thread::Thread(QObject* parent)
  : QThread(parent) {
}

Kryvo::Thread::~Thread() {
  quit();
  requestInterruption();
  wait();
}
