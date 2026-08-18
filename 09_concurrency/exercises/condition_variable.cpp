// Example to used condition variable

#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>

std::mutex mtx;
std::condition_variable cv;
bool ready = false;

void print_id(int id) {
    {
        std::unique_lock<std::mutex> lck(mtx);
        cv.wait(lck, []{return ready;});
    }
    // after wait, we own the lock.
    // delay to simulate work
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Lock std::cout to avoid interleaving output
    std::lock_guard<std::mutex> lck(mtx);
    std::cout << "thread " << id << '\n';
}

void go() {
    std::unique_lock<std::mutex> lck(mtx);
    ready = true;
    cv.notify_all();
}

int main() {
    std::thread threads[10];
    // spawn 10 threads:
    // for (int i = 1; i <= 10; ++i)
    int i = 1;
    for (auto& th : threads) {
        th = std::thread(print_id, i++);
    }

    std::cout << "10 threads ready to race...\n";
    go();                       // go!

    for (auto &th : threads) {
        if (th.joinable())
            th.join();
    }

    return 0;
}