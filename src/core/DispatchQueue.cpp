#include "DispatchQueue.hpp"

Kryvo::DispatchQueue::DispatchQueue(size_t threadCount)
  : threadPool_(threadCount) {
  for (size_t i = 0; i < threadPool_.size(); ++i) {
    threadPool_[i] = std::thread(&DispatchQueue::performWork, this);
  }
}

Kryvo::DispatchQueue::~DispatchQueue() {
  std::unique_lock<std::mutex> lock(queueMutex_);
  quit_ = true;
  lock.unlock();

  queueWaitCondition_.notify_all();

  for (size_t i = 0; i < threadPool_.size(); ++i) {
    if (threadPool_[i].joinable()) {
      threadPool_[i].join();
    }
  }
}

void Kryvo::DispatchQueue::enqueue(const Kryvo::DispatchTask& task) {
  std::unique_lock<std::mutex> lock(queueMutex_);

  queue_.push(task);

  lock.unlock();

  queueWaitCondition_.notify_all();
}

void Kryvo::DispatchQueue::performWork() {
  std::unique_lock<std::mutex> lock(queueMutex_);

  do {
    queueWaitCondition_.wait(lock,
                            [this]{
                              return (!queue_.empty() || quit_);
                            });

    if (!queue_.empty() && !quit_) {
      const DispatchTask task = queue_.front();

      queue_.pop();

      lock.unlock();

      task.func();

      lock.lock();
    }
  } while (!quit_);
}
