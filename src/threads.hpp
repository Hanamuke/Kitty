#ifndef THREADS_DEFINED
#define THREADS_DEFINED
#include "position.hpp"
#include <atomic>
#include <condition_variable>
#include <thread>
#include <vector>

class Thread {
public:
  Thread(int idx);
  Thread() = delete;
  Thread(Thread &&) noexcept;
  ~Thread() noexcept;
  void idle();
  void search();
  void start_searching();
  void notify() { cv.notify_one(); }
  void wait_for_search_finished();
  inline void setPosition(Position &&_pos) { pos = std::move(_pos); }

private:
  Position pos;
  std::condition_variable cv;
  std::thread std_thread;
  std::atomic_bool searching;
  std::mutex m;
  int idx; // thread index, 0 if main thread
};

class ThreadPool {
  friend class Thread;

public:
  ThreadPool() = default;
  inline void stop_search() noexcept {
    stop = true;
    threads[0].wait_for_search_finished();
    stop = false;
  }
  inline void terminate() noexcept {
    terminate_thread = true;
    stop = true;
    threads[0].notify();
  }
  void setPosition(Position &&position);
  void start_searching();
  inline void reset() {}
  void init();
  ~ThreadPool() = default;

private:
  std::vector<Thread> threads;
  std::atomic_bool stop, terminate_thread;
};

extern ThreadPool Threads;

#endif
