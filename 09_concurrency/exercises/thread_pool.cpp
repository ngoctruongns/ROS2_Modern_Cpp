// Example for thread pool implementation

#include <iostream>
#include <thread>
#include <vector>
#include <queue>
#include <functional>
#include <condition_variable>

class ThreadPool {
public:
    ThreadPool(size_t numThreads);
    ~ThreadPool();

    void enqueueTask(std::function<void()> task);
    void waitForCompletion();

private:
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;

    std::mutex queueMutex;
    std::condition_variable condition, doneCondition;
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

void ThreadPool::enqueueTask(std::function<void()> task) {
    {
        std::unique_lock<std::mutex> lock(queueMutex);
        tasks.push(task);
    }
    condition.notify_one();
}

void ThreadPool::waitForCompletion() {
    std::unique_lock<std::mutex> lock(queueMutex);
    doneCondition.wait(lock, [this] { return tasks.empty(); });
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
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            if (tasks.empty()) {
                doneCondition.notify_all();
            }
        }

        // Print result
        std::cout << "Thread [" << id << "] completed a task." << std::endl;
    }
}


// Main function to demonstrate the thread pool usage
int main() {
    ThreadPool pool(4); // Create a thread pool with 4 threads

    // Enqueue some tasks
    for (int i = 0; i < 10; ++i) {
        pool.enqueueTask([i]() {
            // Simulate work
            std::this_thread::sleep_for(std::chrono::milliseconds(100 * i));
            std::cout << "Task " << i << " is done by thread "
                      << std::this_thread::get_id() << std::endl;
        });
    }

    pool.waitForCompletion();
    std::cout << "All tasks completed." << std::endl;

    return 0;
}