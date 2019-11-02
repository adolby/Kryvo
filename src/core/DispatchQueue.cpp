#include "DispatchQueue.hpp"

Kryvo::DispatchQueue::DispatchQueue(size_t threadCount)
  : threadPool(threadCount) {
  for (size_t i = 0; i < threadPool.size(); ++i) {
    threadPool[i] = std::thread(&DispatchQueue::performWork, this);
  }
}

Kryvo::DispatchQueue::~DispatchQueue() {
  std::unique_lock<std::mutex> lock(queueMutex);
  quit = true;
  lock.unlock();

  queueWaitCondition.notify_all();

  for (size_t i = 0; i < threadPool.size(); ++i) {
    if (threadPool[i].joinable()) {
      threadPool[i].join();
    }
  }
}

void Kryvo::DispatchQueue::enqueue(const Kryvo::DispatchTask& task) {
  std::unique_lock<std::mutex> lock(queueMutex);

  queue.push(task);

  lock.unlock();

  queueWaitCondition.notify_all();
}

void Kryvo::DispatchQueue::performWork() {
  std::unique_lock<std::mutex> lock(queueMutex);

  do {
    queueWaitCondition.wait(lock,
                            [this]{
                              return (!queue.empty() || quit);
                            });

    if (!queue.empty() && !quit) {
      const DispatchTask& task = queue.front();

      queue.pop();

      lock.unlock();

      task.func();

      lock.lock();
    }
  } while (!quit);
}
