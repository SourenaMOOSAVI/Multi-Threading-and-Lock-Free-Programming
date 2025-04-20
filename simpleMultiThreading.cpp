#include <iostream> 
#include <thread>   // For using std::thread

// Function to be executed by each thread
void printMessage(int id) {
    std::cout << "Thread " << id << " is running\n";
}

int main() {
    // Create two threads, each running the printMessage function with a unique ID
    std::thread t1(printMessage, 1); // Thread t1 with ID 1
    std::thread t2(printMessage, 2); // Thread t2 with ID 2


    t1.join(); // Wait for t1 to finish
    t2.join(); // Wait for t2 to finish

    return 0;
}