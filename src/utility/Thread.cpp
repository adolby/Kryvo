#include "Thread.hpp"

Kryvos::Thread::Thread(QObject* parent)
  : QThread{parent} {
}

Kryvos::Thread::~Thread() {
  quit();
  requestInterruption();
  wait();
}
