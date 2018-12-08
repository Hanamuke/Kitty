#include "threads.hpp"
#include "misc.hpp"
#include "options.hpp"
#include <iostream>

ThreadPool Threads;

void ThreadPool::init()
{
  if (threads.size() != 0)
    stop_search();
  Option &o = Options["Threads"];
  size_t nbThread = (int)o;
  if (nbThread == threads.size())
    return;
  threads.clear();
  threads.reserve(nbThread);
  for (size_t i = 0; i < nbThread; i++)
    threads.emplace_back(i);
}

void ThreadPool::start_searching()
{
  stop = false;
  threads[0].start_searching();
  return;
}

void Thread::idle()
{
  searching = false;
  cv.notify_one();
  while (!Threads.terminate_thread)
  {
    std::unique_lock<std::mutex> lk(m);
    cv.wait(lk,
            [this]() { return this->searching || Threads.terminate_thread; });
    if (searching)
      search(); // will not search if Threads.stop
    searching = false;
    cv.notify_one();
  }
}

// should not happen, but the compiler doesn't know it
__attribute__((noreturn))
Thread::Thread(Thread &&t __attribute__((unused))) noexcept
{
  exit(-1);
}

void Thread::wait_for_search_finished()
{
  if (searching)
  {
    std::unique_lock<std::mutex> lk(m);
    cv.wait(lk, [this]() { return !this->searching; });
  }
}

void Thread::start_searching()
{
  wait_for_search_finished();
  searching = true;
  cv.notify_one();
}

Thread::Thread(int _idx)
    : pos(), std_thread{&Thread::idle, this}, searching(false), idx(_idx) {}

Thread::~Thread() noexcept
{
  if (std_thread.joinable())
  {
    std_thread.join();
  }
}
void ThreadPool::setPosition(Position &&pos)
{
  assert(threads.size() != 0);
  threads[0].wait_for_search_finished();
  threads[0].setPosition(std::forward<Position>(pos));
}
