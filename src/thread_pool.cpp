#include "thread_pool.hpp"

namespace my_server {

ThreadPool::ThreadPool(size_t worker_count)
    : _stop(false)
  {
    for (size_t i = 0; i < worker_count; ++i) {
      _workers.emplace_back([this](){ this->worker_loop(); });
    }
  }


ThreadPool::~ThreadPool() {
    {
        std::unique_lock lk(_mtx);
        _stop = true;
    }
    _cv.notify_all();
    for (auto &w : _workers) 
    {
        if (w.joinable()) w.join();
    }
}

void ThreadPool::enqueue(std::function<void()> task) {
    {
        std::unique_lock lk(_mtx);
        _tasks.push(std::move(task));
    }
    _cv.notify_one();
}

void ThreadPool::worker_loop() {
    while (true) {
        std::function<void()> task;
        {
            std::unique_lock lk(_mtx);
            _cv.wait(lk, [&]{
                 return _stop || !_tasks.empty(); 
            });
            if (_stop && _tasks.empty()) return;
            task = std::move(_tasks.front());
            _tasks.pop();
        }
        task();
    }

}

}