// Example for thread pool implementation with return values

#include <iostream>
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <future>
#include <memory>

class ThreadPool {
public:
    ThreadPool(size_t numThreads);
    ~ThreadPool();

    std::future<int> enqueueTask(std::function<int()> task);

private:
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;

    std::mutex queueMutex;
    std::condition_variable condition;
    bool stop;

    void workerThread(size_t id);
};

// Implement methods for ThreadPool class
ThreadPool::ThreadPool(size_t numThreads) : stop(false) {
    // Create worker threads
    for (size_t i = 0; i < numThreads; ++i) {
        workers.emplace_back(&ThreadPool::workerThread, this, i);
    }
}

ThreadPool::~ThreadPool() {
    {
        std::unique_lock<std::mutex> lock(queueMutex);
        stop = true;
    }

    // Notify all worker threads to wake up and check the stop condition
    condition.notify_all();

    // Join all worker threads
    for (std::thread &worker : workers) {
        if (worker.joinable()) {
            worker.join();
        }
    }
}

std::future<int> ThreadPool::enqueueTask(std::function<int()> task) {
    // Create a packaged_task to wrap the task and get a future
    auto packagedTask = std::make_shared<std::packaged_task<int()>>(task);
    std::future<int> future = packagedTask->get_future();

    {
        std::unique_lock<std::mutex> lock(queueMutex);
        // If stop is true, we should not enqueue new tasks
        if (stop) {
            throw std::runtime_error("ThreadPool is stopped. Cannot enqueue new tasks.");
        }

        // Add the task to the queue
        tasks.push([packagedTask]() {
            (*packagedTask)();
        });
    }
    condition.notify_one();

    return future;
}

void ThreadPool::workerThread(size_t id) {
    while (true) {
        std::function<void()> task;
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            // Wait until there is a task to process or the pool is stopped
            condition.wait(lock, [this] { return stop || !tasks.empty(); });

            // If the pool is stopped and there are no tasks left, exit the thread
            if (stop && tasks.empty()) {
                return;
            }

            // Get the next task from the queue
            task = std::move(tasks.front());
            tasks.pop();
        }

        // Execute the task outside of the lock to allow other threads to enqueue tasks
        task(); // Execute the task with time-consuming work

        // Print result
        std::cout << "Thread [" << id << "] completed a task." << std::endl;
    }
}


// Main function to demonstrate the thread pool usage
int main() {
    ThreadPool pool(4); // Create a thread pool with 4 threads

    // future to hold the results of the tasks
    std::vector<std::future<int>> results;

    // Enqueue some tasks
    for (int i = 0; i < 10; ++i) {
        results.push_back(pool.enqueueTask([i]() {
            // Simulate work
            std::this_thread::sleep_for(std::chrono::milliseconds(100 * i));
            return i * i;
        }));
    }

    // Collect the results
    for (auto& result : results) {
        std::cout << "Result: " << result.get() << std::endl;
    }

    return 0;
}