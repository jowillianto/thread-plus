import test_lib;
import thread_plus;
#include <chrono>
#include <future>
#include <memory>
#include <string>
#include <thread>
#include <vector>

auto channel_tests =
  test_lib::Tester{"Basic Generic Channel"}
    .add_test(
      "send-rec",
      []() {
        auto channel = thread_plus::channel::Channel<int>{};
        int to_send = test_lib::random_integer(0, 10);
        channel.send(std::make_unique<int>(to_send));
        std::thread{[to_send, &channel]() mutable {
          auto msg = channel.recv();
          test_lib::assert_equal(*msg.value(), to_send);
        }}.join();
      }
    )
    .add_test(
      "send-kill-rec",
      []() {
        auto channel = thread_plus::channel::Channel<int>{};
        int to_send = test_lib::random_integer(0, 10);
        channel.send(std::make_unique<int>(to_send));
        channel.kill();
        std::thread{[&channel]() mutable {
          auto msg = channel.recv();
          test_lib::assert_equal(msg.has_value(), false);
        }}.join();
      }
    )
    .add_test(
      "send-rec-send-rec",
      []() {
        auto channel = thread_plus::channel::Channel<int>{};
        auto signal_run = thread_plus::channel::Channel<void>{};
        int to_send = test_lib::random_integer(0, 10);
        int to_recv = test_lib::random_integer(0, 10);
        channel.send(std::make_unique<int>(to_send));
        std::thread thread{[&]() mutable {
          auto msg = channel.recv();
          signal_run.send();
          test_lib::assert_equal(*msg.value(), to_send);
          channel.send(std::make_unique<int>(to_recv));
        }};
        // Sleep so that the thread runs first.
        auto ptr = signal_run.recv();
        auto msg = channel.recv();
        test_lib::assert_equal(*msg.value(), to_recv);
        thread.join();
      }
    )
    .add_test(
      "send-bulk-order",
      []() {
        auto channel = thread_plus::channel::Channel<std::string>{};
        auto send_count = test_lib::random_integer(5, 10);
        std::vector<std::string> orig_messages;
        std::vector<std::unique_ptr<std::string>> messages;
        messages.reserve(send_count);
        orig_messages.reserve(send_count);
        for (int i = 0; i < send_count; i += 1) {
          std::string message = test_lib::random_string(20);
          messages.push_back(std::make_unique<std::string>(message));
          orig_messages.push_back(message);
        }
        channel.send(messages);
        // Do not use messages after this. It has became invalid
        for (int i = 0; i < send_count; i += 1) {
          auto message = channel.recv();
          test_lib::assert_equal(message.has_value(), true);
          test_lib::assert_equal((*message.value()), orig_messages[i]);
        }
      }
    )
    .add_test(
      "channel-destructor-no-block",
      []() {
        std::future<void> destroy_channel = std::async(std::launch::async, []() {
          auto channel = thread_plus::channel::Channel<int>{};
          channel.send(test_lib::random_integer(10, 200));
          /*
            Channel destruction here, assert that it can be destroyed by timing out the time
          */
        });
        /*
          Assert that waiting for one second implies that the channel can be destroyed.
        */
        test_lib::assert_equal(
          static_cast<int>(destroy_channel.wait_for(std::chrono::seconds{1})),
          static_cast<int>(std::future_status::ready)
        );
      }
    )
    .add_test(
      "send-construct",
      []() {
        auto channel = thread_plus::channel::Channel<std::string>{};
        auto to_send = test_lib::random_string(10);
        channel.send(to_send);
        auto message = channel.recv();
        test_lib::assert_equal(message.has_value(), true);
        test_lib::assert_equal((*message.value()), to_send);
      }
    )
    .add_test(
      "try-recv",
      []() {
        auto channel = thread_plus::channel::Channel<std::string>{};
        auto to_send = test_lib::random_string(10);
        channel.send(to_send);
        auto message = channel.try_recv();
        test_lib::assert_equal(message.has_value(), true);
        test_lib::assert_equal(*message.value(), to_send);
      }
    )
    .add_test("send-bulk-join", []() {
      auto channel = thread_plus::channel::Channel<int>{};
      auto sig = thread_plus::channel::Channel<void>{};
      int n_sends = test_lib::random_integer(0, 10);
      // Send n, recv n
      for (int i = 0; i < n_sends; i += 1) {
        channel.send(test_lib::random_integer(0, 10));
      }
      std::vector<std::thread> receivers;
      receivers.reserve(n_sends + 1);
      receivers.push_back(std::thread{[&channel, &sig, n_sends]() {
        sig.send(n_sends + 1);
        channel.join();
      }});
      for (int i = 0; i < n_sends; i += 1) {
        receivers.emplace_back(std::thread{[&channel, &sig]() {
          // Wait for join signal
          auto _ = sig.recv();
          std::this_thread::sleep_for(std::chrono::microseconds(test_lib::random_integer(0, 100)));
          auto ptr = channel.recv();
        }});
      }
      try {
        // Wait for join signal
        auto _ = sig.recv();
        channel.send(test_lib::random_integer(0, 10));
      } catch (const thread_plus::channel::ChannelError<
               thread_plus::channel::ErrorCode::SEND_ERROR_NOT_LISTENING> &e) {
        // Correct behaviour
        for (auto &receiver : receivers)
          receiver.join();
        return;
      }
      for (auto &receiver : receivers)
        receiver.join();
      test_lib::assert_equal(true, false);
    });
auto void_channel_tests =
  test_lib::Tester{"Basic Void Channel"}
    .add_test(
      "sync-Channel<void>",
      []() {
        auto channel = thread_plus::channel::Channel<void>();
        uint32_t spawn_count = test_lib::random_integer(0, 10);
        channel.send(spawn_count);
        std::vector<std::thread> threads;
        threads.reserve(spawn_count);
        for (uint32_t i = 0; i < spawn_count; i += 1) {
          threads.emplace_back(std::thread{[&]() { auto _ = channel.recv(); }});
        }
        for (auto &thread : threads)
          thread.join();
      }
    )
    .add_test(
      "destroy-noblock",
      []() {
        std::future<void> destroy_channel = std::async(std::launch::async, []() {
          auto channel = thread_plus::channel::Channel<void>{};
          channel.send(test_lib::random_integer(0, 10));
          /*
            Channel destruction here, assert that it can be destroyed by timing out the time
          */
        });
        /*
          Assert that waiting for one second implies that the channel can be destroyed.
        */
        test_lib::assert_equal(
          static_cast<int>(destroy_channel.wait_for(std::chrono::seconds{1})),
          static_cast<int>(std::future_status::ready)
        );
      }
    )
    .add_test("send-bulk rec-bulk", []() {
      auto channel = thread_plus::channel::Channel<void>{};
      auto send_count = test_lib::random_integer(5, 10);
      channel.send(send_count);
      for (int i = 0; i < send_count; i += 1) {
        test_lib::assert_equal(channel.try_recv().has_value(), true);
      }
      test_lib::assert_equal(channel.try_recv().has_value(), false);
    });
auto pool_tests =
  test_lib::Tester{"Basic Thread Pool"}
    .add_test(
      "assign one",
      []() {
        auto pool = thread_plus::pool::Pool{10};
        auto ret_num = test_lib::random_integer(0, 10);
        auto fut = pool.add_task([ret_num]() {
          // very expensive calculation
          std::this_thread::sleep_for(std::chrono::milliseconds(5));
          return ret_num;
        });
        test_lib::assert_equal(fut.value().get(), ret_num);
      }
    )
    .add_test(
      "assign_task_kill",
      []() {
        auto pool = thread_plus::pool::Pool{10};
        auto ret_num = test_lib::random_integer(0, 10);
        auto fut = pool.add_task([ret_num]() {
          std::this_thread::sleep_for(std::chrono::milliseconds(5));
          return ret_num;
        });
        test_lib::assert_equal(fut.has_value(), true);
        pool.kill();
        test_lib::assert_equal(pool.joinable(), false);
        auto shd_fail =
          pool.add_task([]() { std::this_thread::sleep_for(std::chrono::milliseconds(150)); });
        test_lib::assert_equal(shd_fail.has_value(), false);
      }
    )
    .add_test("assign_task_join", []() {
      auto pool = thread_plus::pool::Pool{10};
      auto ret_num = test_lib::random_integer(0, 10);
      auto fut = pool.add_task([ret_num]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
        return ret_num;
      });
      test_lib::assert_equal(fut.has_value(), true);
      pool.join();
      test_lib::assert_equal(pool.joinable(), false);
    });
auto pool_fuzz_tests = test_lib::Tester{"Pool Fuzz Tests"}.add_test("fuzz int", []() {
  auto thread_count = test_lib::random_integer(5, 20);
  auto task_count = thread_count * test_lib::random_integer(5, 100);
  auto pool = thread_plus::pool::Pool{static_cast<size_t>(thread_count)};
  std::vector<int> _tasks;
  std::vector<std::future<int>> _tasks_fut;
  _tasks.reserve(task_count);
  _tasks_fut.reserve(task_count);
  for (size_t i = 0; i < task_count; i += 1) {
    auto ret_num = test_lib::random_integer(0, 10000);
    _tasks.push_back(ret_num);
    _tasks_fut.emplace_back(pool
                              .add_task([ret_num]() {
                                auto rest_time = test_lib::random_integer(50, 150);
                                std::this_thread::sleep_for(std::chrono::milliseconds(rest_time));
                                return ret_num;
                              })
                              .value());
  }
  for (size_t i = 0; i < task_count; i += 1) {
    test_lib::assert_equal(_tasks[i], _tasks_fut[i].get());
  }
  pool.join();
});

int main() {
  channel_tests.print_or_exit();
  void_channel_tests.print_or_exit();
  pool_tests.print_or_exit();
  pool_fuzz_tests.print_or_exit();
}