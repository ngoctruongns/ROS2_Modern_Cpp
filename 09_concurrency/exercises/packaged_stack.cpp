#include <iostream>
#include <future>
#include <thread>

int tinh_tong(int a, int b) {
    return a + b;
}

int main() {
    // 1. Đóng gói hàm tinh_tong
    std::packaged_task<int(int, int)> task(tinh_tong);

    // 2. Lấy future từ task
    std::future<int> ket_qua = task.get_future();

    // 3. Chạy task trên một luồng khác
    std::thread t(std::move(task), 5, 10);

    // 4. Lấy kết quả từ future
    std::cout << "Tong la: " << ket_qua.get() << std::endl;

    t.join();
    return 0;
}
