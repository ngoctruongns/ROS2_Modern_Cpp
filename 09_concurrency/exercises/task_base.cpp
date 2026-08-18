// Ví dụ sử dụng task base thay cho thread base sử dụng std::future và std::promise

#include <iostream>
#include <future>
#include <chrono>

using namespace std;

// Function to compute the sum of integers from 1 to n
void calcSumTask(std::promise<int> prm, int n) {
    int sum = 0;

    // Simulate some work with a delay
    // std::this_thread::sleep_for(std::chrono::milliseconds(10 * n));
    for (int i = 1; i <= n; ++i) {
        sum += i;
    }
    prm.set_value(sum); // Set the computed sum in the promise
}

int main() {
    int n = 100; // Calculate the sum from 1 to n
    std::promise<int> prm; // Create a promise to hold the result
    std::future<int> fut = prm.get_future(); // Get the future associated with the promise

    // Launch a thread to compute the sum
    auto fut_async = std::async(std::launch::async, calcSumTask, std::move(prm), n);

    // Wait for the result and get the sum
    int result = fut.get(); // This will block until the result is available
    std::cout << "The sum of integers from 1 to " << n << " is: " << result << std::endl;

    // Lấy từ cùng 1 thread
    std::promise<int> prm2;
    std::future<int> fut2 = prm2.get_future();

    prm2.set_value(42); // Set the value in the promise
    int value = fut2.get(); // Get the value from the future
    std::cout << "The value from the promise is: " << value << std::endl;

    // Share the future between multiple threads
    std::promise<int> sharedPrm;
    std::future<int> fm = sharedPrm.get_future();
    std::shared_future<int> sharedFut = fm.share(); // Convert fm to shared_future -> can't use fm anymore
    // fm.get(); // This will throw an exception because fm is no longer valid

    // Use sharedFut in multiple threads (only get() is allowed)
    std::thread t1([sharedFut]() {
        std::cout << "Thread 1 got value: " << sharedFut.get() << std::endl;
    });
    std::thread t2([sharedFut]() {
        std::cout << "Thread 2 got value: " << sharedFut.get() << std::endl;
    });

    std::thread t3([&sharedPrm]() {
        // std::this_thread::sleep_for(std::chrono::milliseconds(100));
        sharedPrm.set_value(99); // Set the value in the promise
    });

    t1.join();
    t2.join();
    t3.join();

    return 0;
}