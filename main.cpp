#include <atomic>
#include <iostream>
#include <thread>

int main() {
  std::atomic_int num(10);
  std::thread t {
    [&](){
      num.store(5, std::memory_order_relaxed);
    }
  };
  int expected = 5;
  while (num.compare_exchange_weak(expected, 6, std::memory_order_relaxed)) {
    expected = 5;
  }
  t.join();
  std::cout<<"done"<<std::endl;
}