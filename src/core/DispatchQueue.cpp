#include "DispatchQueue.hpp"

Kryvo::DispatchQueue::DispatchQueue(size_t threadCount)
  : threadPool(threadCount) {
  for (size_t i = 0; i < threadPool.size(); ++i) {
    threadPool[i] = std::thread(&DispatchQueue::acquireWork, this);
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

void Kryvo::DispatchQueue::enqueue(const std::function<void(void)>& func) {
  std::unique_lock<std::mutex> lock(queueMutex);

  queue.push(func);

  lock.unlock();

  queueWaitCondition.notify_all();
}

void Kryvo::DispatchQueue::enqueue(std::function<void(void)>&& func) {
    std::unique_lock<std::mutex> lock(queueMutex);

    queue.push(std::move(func));

    lock.unlock();

    queueWaitCondition.notify_all();
}

void Kryvo::DispatchQueue::acquireWork() {
  std::unique_lock<std::mutex> lock(queueMutex);

  do {
    queueWaitCondition.wait(lock,
                            [this]{
                              return (!queue.empty() || quit);
                            });

    if (!queue.empty() && !quit) {
      auto func = std::move(queue.front());

      queue.pop();

      lock.unlock();

      func();

      lock.lock();
    }
  } while (!quit);
}
