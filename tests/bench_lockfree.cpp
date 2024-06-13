import moderna.thread_plus;
import moderna.test_lib;
import moderna.lock_free;
#include <chrono>
#include <climits>
#include <iostream>
#include <mutex>
#include <queue>

template <std::invocable<> F, typename time_unit_t = std::chrono::milliseconds>
void bench_function(std::string_view bench_name, F &&func) {
  auto beg = std::chrono::high_resolution_clock::now();
  func();
  auto end = std::chrono::high_resolution_clock::now();
  std::cout << bench_name << " : " << std::chrono::duration_cast<time_unit_t>(end - beg)
            << std::endl;
}

int main() {
  bench_function("normal_queue_sequential", []() {
    auto std_queue = std::queue<int>{};
    std::mutex std_queue_mut;
    for (size_t i = 0; i < 1000000; i += 1) {
      std_queue.push(moderna::test_lib::random_integer(1, INT_MAX));
    }
  });
  bench_function("lockfree_queue_sequential", []() {
    auto pool = moderna::thread_plus::pool::Pool{10};
    auto lock_free_queue = moderna::lock_free::Queue<int>{};
    for (size_t i = 0; i < 1000000; i += 1) {
      lock_free_queue.push(moderna::test_lib::random_integer(1, INT_MAX));
    }
    pool.join();
  });
  bench_function("normal_queue_multithread", []() {
    auto pool = moderna::thread_plus::pool::Pool{10};
    auto std_queue = std::queue<int>{};
    auto sig = moderna::thread_plus::channel::Channel<void>{};
    std::mutex std_queue_mut;
    for (size_t i = 0; i < 1000000; i += 1) {
      auto _ = pool.add_task([&std_queue_mut, &std_queue, &sig]() {
        auto _ = sig.recv();
        auto l = std::unique_lock{std_queue_mut};
        std_queue.push(moderna::test_lib::random_integer(1, INT_MAX));
      });
    }
    sig.send(1000000);
    pool.join();
  });
  bench_function("lockfree_queue_multithread", []() {
    auto pool = moderna::thread_plus::pool::Pool{10};
    auto lock_free_queue = moderna::lock_free::Queue<int>{};
    auto sig = moderna::thread_plus::channel::Channel<void>{};
    for (size_t i = 0; i < 1000000; i += 1) {
      auto _ = pool.add_task([&lock_free_queue, &sig]() {
        auto _ = sig.recv();
        lock_free_queue.push(moderna::test_lib::random_integer(1, INT_MAX));
      });
    }
    sig.send(1000000);
    pool.join();
  });
}