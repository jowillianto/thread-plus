import moderna.thread_plus;
import moderna.lock_free;
import moderna.test_lib;
#include <queue>
#include <mutex>

using namespace moderna;

auto lockfree_queue_tests =
  test_lib::Tester{"LockFree Queue"}
    .add_test(
      "basic_push_pop",
      []() {
        auto queue = moderna::lock_free::Queue<int>{};
        queue.push(2);
        queue.push(3);
        test_lib::assert_equal(queue.size(), 2);
        auto first_pop = queue.pop();
        test_lib::assert_equal(first_pop.has_value(), true);
        test_lib::assert_equal(first_pop.value(), 2);
        test_lib::assert_equal(queue.size(), 1);
        auto second_pop = queue.pop();
        test_lib::assert_equal(second_pop.has_value(), true);
        test_lib::assert_equal(second_pop.value(), 3);
        test_lib::assert_equal(queue.size(), 0);
        queue.push(4);
        queue.push(5);
        test_lib::assert_equal(queue.size(), 2);
        auto third_pop = queue.pop();
        test_lib::assert_equal(third_pop.has_value(), true);
        test_lib::assert_equal(third_pop.value(), 4);
        test_lib::assert_equal(queue.size(), 1);
        auto fourth_pop = queue.pop();
        test_lib::assert_equal(fourth_pop.has_value(), true);
        test_lib::assert_equal(fourth_pop.value(), 5);
        test_lib::assert_equal(queue.size(), 0);
      }
    )
    .add_test(
      "multithreaded_basic_push_pop",
      []() {
        auto queue = moderna::lock_free::Queue<int>{};
        auto pool = moderna::thread_plus::pool::Pool{10};
        auto task_count = moderna::test_lib::random_integer(30, 1000000);
        auto sig = moderna::thread_plus::channel::Channel<void>{};
        auto norm_queue = std::queue<int>{};
        std::mutex norm_lock;
        for (size_t i = 0; i < task_count; i += 1) {
          auto _ = pool.add_task([&sig, &queue, &norm_lock, &norm_queue]() {
            auto _ = sig.recv();
            auto push_value = test_lib::random_integer(1, 100);
            queue.push(push_value);
            std::unique_lock l{norm_lock};
            norm_queue.push(push_value);
          });
        }
        sig.send(task_count);
        for (size_t i = 0; i < task_count; i += 1) {
          auto _ = pool.add_task([&sig, &queue, &norm_lock, &norm_queue]() {
            auto _ = sig.recv();
            std::unique_lock l{norm_lock};
            auto popped_value = norm_queue.front();
            norm_queue.pop();
            l.unlock();
            l.release();
            auto queue_pop_result = queue.pop();
            test_lib::assert_equal(queue_pop_result.has_value(), true);
            test_lib::assert_equal(queue_pop_result.value(), popped_value);
          });
        }
        sig.send(task_count);
        pool.join();
      }
    )
    .add_test(
      "fuzzed_push_pop",
      []() {
        auto queue = moderna::lock_free::Queue<int>{};
        auto normal_queue = std::queue<int>{};
        for (size_t i = 0; i < 1000000; i += 1) {
          auto action = 0;
          if (queue.size() != 0) action = test_lib::random_integer(0, 1);
          if (action == 0) {
            auto random_value = test_lib::random_integer(0, 10);
            queue.push(random_value);
            normal_queue.push(random_value);
            test_lib::assert_equal(queue.size(), normal_queue.size());
          } else if (action == 1) {
            auto value = queue.pop();
            auto other_value = normal_queue.front();
            normal_queue.pop();
            test_lib::assert_equal(value.has_value(), true);
            test_lib::assert_equal(value.value(), other_value);
            test_lib::assert_equal(queue.size(), normal_queue.size());
          }
        }
      }
    )
    .add_test("multithreaded_push_pop", []() {

    });

int main() {

}
