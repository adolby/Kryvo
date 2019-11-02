// Adapted from Embedded Artistry's dispatch queue implementation
// https://embeddedartistry.com/blog/2017/2/1/c11-implementing-a-dispatch-queue-using-stdfunction?rq=queue

#ifndef DISPATCHQUEUE_HPP
#define DISPATCHQUEUE_HPP

#include <thread>
#include <functional>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>

namespace Kryvo {

struct DispatchTask {
  std::function<void(void)> func;
  int priority = 0;
};

class DispatchQueue {
 public:
  DispatchQueue(size_t threadCount = 1);
  virtual ~DispatchQueue();

  void enqueue(const DispatchTask& func);

  // Deleted operations
  DispatchQueue(const DispatchQueue& rhs) = delete;
  DispatchQueue& operator=(const DispatchQueue& rhs) = delete;
  DispatchQueue(DispatchQueue&& rhs) = delete;
  DispatchQueue& operator=(DispatchQueue&& rhs) = delete;

 private:
  void performWork();

  std::vector<std::thread> threadPool;

  std::mutex queueMutex;
  std::queue<DispatchTask> queue;
  std::condition_variable queueWaitCondition;

  bool quit = false;
};

}

#endif // DISPATCHQUEUE_HPP
