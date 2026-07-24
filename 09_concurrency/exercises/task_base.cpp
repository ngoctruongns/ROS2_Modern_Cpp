// Ví dụ sử dụng task base thay cho thread base sử dụng std::future và std::promise

#include <iostream>
#include <future>
using namespace std;

// Function to compute the sum of integers from 1 to n
void calcSumTask(std::promise<int> prm, int n) {
    int sum = 0;
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

    return 0;
}