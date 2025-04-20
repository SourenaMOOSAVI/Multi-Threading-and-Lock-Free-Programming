#include <iostream>
#include <thread>

int counter = 0; // Shared resource

void incrementCounter() {
    for (int i = 0; i < 1000000; i++) {
        counter++; // Unsafe operation!
    }
}

int main() {
    std::thread t1(incrementCounter);
    std::thread t2(incrementCounter);

    t1.join();
    t2.join();

    std::cout << "Final counter value: " << counter << std::endl;
    return 0;
}