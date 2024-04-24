import test_lib;
import thread_plus;
#include <thread>
#include <memory>
#include <string>
#include <vector>
#include <chrono>
#include <future>

auto channel_tests = test_lib::Tester{ "Basic Generic Channel" }
  .add_test("send-rec", [](){
    auto channel = thread_plus::Channel<int>{};
    int to_send = test_lib::random_integer(0, 10);
    channel.send(std::make_unique<int>(to_send));
    std::thread {
      [to_send, &channel]() mutable {
        auto msg = channel.recv();
        test_lib::assert_equal(*msg.value(), to_send);
      }
    }.join();
  })
  .add_test("send-kill-rec", [](){
    auto channel = thread_plus::Channel<int>{};
    int to_send = test_lib::random_integer(0, 10);
    channel.send(std::make_unique<int>(to_send));
    channel.kill();
    std::thread {
      [&channel]() mutable {
        auto msg = channel.recv();
        test_lib::assert_equal(msg.has_value(), false);
      }
    }.join();
  })
  .add_test("send-rec-send-rec", [](){
    auto channel = thread_plus::Channel<int>{};
    auto signal_run = thread_plus::Channel<void>{};
    int to_send = test_lib::random_integer(0, 10);
    int to_recv = test_lib::random_integer(0, 10);
    channel.send(std::make_unique<int>(to_send));
    std::thread thread {
      [&]() mutable {
        auto msg = channel.recv();
        signal_run.send();
        test_lib::assert_equal(*msg.value(), to_send);
        channel.send(std::make_unique<int>(to_recv));
      }
    };
    // Sleep so that the thread runs first. 
    auto ptr = signal_run.recv();
    auto msg = channel.recv();
    test_lib::assert_equal(*msg.value(), to_recv);
    thread.join();
  })
  .add_test("send-bulk-order", [](){
    auto channel = thread_plus::Channel<std::string>{};
    auto send_count = test_lib::random_integer(5, 10);
    std::vector<std::string> orig_messages;
    std::vector<std::unique_ptr<std::string> > messages;
    messages.reserve(send_count);
    orig_messages.reserve(send_count);
    for (int i = 0; i < send_count; i += 1) {
      std::string message = test_lib::random_string(20);
      messages.push_back(std::make_unique<std::string>( message ));
      orig_messages.push_back(message);
    }
    channel.send(messages);
    // Do not use messages after this. It has became invalid
    for (int i = 0; i < send_count; i += 1) {
      auto message = channel.recv();
      test_lib::assert_equal(message.has_value(), true);
      test_lib::assert_equal((*message.value()), orig_messages[i]);
    }
  })
  .add_test("send-construct", [](){
    auto channel = thread_plus::Channel<std::string>{};
    auto to_send = test_lib::random_string(10);
    channel.send(to_send);
    auto message = channel.recv();
    test_lib::assert_equal(message.has_value(), true);
    test_lib::assert_equal((*message.value()), to_send);
  })
  .add_test("try-recv", [](){
    auto channel = thread_plus::Channel<std::string>{};
    auto to_send = test_lib::random_string(10);
    channel.send(to_send);
    auto message = channel.try_recv();
    test_lib::assert_equal(message.has_value(), true);
    test_lib::assert_equal(message.value().has_value(), true);
    test_lib::assert_equal(*(message.value().value()), to_send);
  });
auto void_channel_tests = test_lib::Tester { "Basic Void Channel" }
  .add_test("sync-Channel<void>", [](){
    auto channel = thread_plus::Channel<void>();
    uint32_t spawn_count = test_lib::random_integer(0, 10);
    channel.send(spawn_count);
    std::vector<std::thread> threads;
    threads.reserve(spawn_count);
    for (uint32_t i = 0; i < spawn_count; i += 1) {
      threads.emplace_back(std::thread{[&](){
        auto _ = channel.recv();
      }});
    }
    for (auto& thread : threads) thread.join();
  })
  .add_test("send-bulk rec-bulk", [](){
    auto channel = thread_plus::Channel<void>{};
    auto send_count = test_lib::random_integer(5, 10);
    channel.send(send_count);
    for (int i = 0; i < send_count; i += 1) {
      test_lib::assert_equal(channel.try_recv().has_value(), true);
    }
    test_lib::assert_equal(channel.try_recv().has_value(), false);
  });
auto pool_tests = test_lib::Tester { "Basic Thread Pool" }
  .add_test("assign one", [](){
    auto pool = thread_plus::Pool { 10 };
    auto ret_num = test_lib::random_integer(0, 10);
    auto fut = pool.add_task([ret_num](){
      // very expensive calculation
      std::this_thread::sleep_for(std::chrono::milliseconds(5));
      return ret_num;
    });
    test_lib::assert_equal(fut.get(), ret_num);
  });
auto pool_fuzz_tests = test_lib::Tester { "Pool Fuzz Tests" }
  .add_test("fuzz int", [](){
    auto thread_count = test_lib::random_integer(5, 20);
    auto task_count = thread_count * test_lib::random_integer(5, 100);
    auto pool = thread_plus::Pool { static_cast<size_t>(thread_count) };
    std::vector<int> _tasks;
    std::vector<std::future<int> > _tasks_fut;
    _tasks.reserve(task_count);
    _tasks_fut.reserve(task_count);
    for (size_t i = 0; i < task_count; i += 1) {
      auto ret_num = test_lib::random_integer(0, 10000);
      _tasks.push_back(ret_num);
      _tasks_fut.push_back(pool.add_task([ret_num](){
        auto rest_time = test_lib::random_integer(100, 200);
        std::this_thread::sleep_for(std::chrono::milliseconds(rest_time));
        return ret_num;
      }));
    }
    for (size_t i = 0; i < task_count; i += 1){
      test_lib::assert_equal(_tasks[i], _tasks_fut[i].get());
    }
  });

int main(){
  channel_tests.print_or_exit();
  void_channel_tests.print_or_exit();
  pool_tests.print_or_exit();
  pool_fuzz_tests.print_or_exit();
}