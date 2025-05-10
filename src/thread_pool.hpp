#ifndef TRD_POOL_H
#define TRD_POOL_H

#include <thread>
#include <mutex>
#include <queue>
#include <functional>
#include <condition_variable>
#include <atomic>

namespace my_server {

class ThreadPool {
public:
  ThreadPool(size_t worker_count);
  ~ThreadPool();
  void enqueue(std::function<void()> task);


private:

  std::vector<std::thread> _workers;
  std::queue<std::function<void()>> _tasks;
  std::mutex _mtx;
  std::condition_variable _cv;
  bool _stop;

  void worker_loop();

};

}


#endif