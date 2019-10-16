#ifndef DISPATCHQUEUE_HPP
#define DISPATCHQUEUE_HPP

#include <thread>
#include <functional>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>

namespace Kryvo {

class DispatchQueue {
 public:
  DispatchQueue(size_t threadCount = 1);
  virtual ~DispatchQueue();

  void enqueue(const std::function<void(void)>& func);
  void enqueue(std::function<void(void)>&& func);

  // Deleted operations
  DispatchQueue(const DispatchQueue& rhs) = delete;
  DispatchQueue& operator=(const DispatchQueue& rhs) = delete;
  DispatchQueue(DispatchQueue&& rhs) = delete;
  DispatchQueue& operator=(DispatchQueue&& rhs) = delete;

 private:
  void acquireWork();

  std::vector<std::thread> threadPool;

  std::mutex queueMutex;
  std::queue<std::function<void(void)>> queue;
  std::condition_variable queueWaitCondition;

  bool quit = false;
};

}

#endif // DISPATCHQUEUE_HPP
