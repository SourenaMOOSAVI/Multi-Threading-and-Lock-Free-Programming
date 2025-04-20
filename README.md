# Multithreading and lock-free programming

This repository contains C++ code examples demonstrating high-performance,
low-latency programming techniques, focusing on **multithreading** and **lock-free** programming.
The goal is to provide a clear and concise introduction to common techniques used in
low-latency and high-performance software development.

## Table of Contents

- Prerequisites
- Compilation Instructions
- Basic Multithreading
- Race Conditions
- Mutexes
- Atomic Operations
- Further Considerations

## Prerequisites

To compile and run the examples, you need:

- A `C++11` (or later) compliant compiler (e.g., `g++`, `clang++`).
- A standard `C++` library with support for `<thread>`, `<mutex>`, and `<atomic>`.
- A system with multithreading support (most modern operating systems).

## Compilation Instructions

To compile any of the example files (e.g., `simpleMultiThreading.cpp`), use a command like:

```bash
clang++ -std=c++11 -pthread simpleMultiThreading.cpp -o simpleMultiThreading
```

Run the compiled program:

```bash
./simpleMultiThreading
```

You can replace `simpleMultiThreading.cpp` with any other example file name to compile and run it.
Make sure to include the `-pthread` flag to enable multithreading support. The `-pthread` flag is required
to link the threading library on some platforms (e.g., Linux).

## Basic Multithreading

**File**: `simpleMultiThreading.cpp`

This example introduces basic multithreading using the C++ Standard Library's `std::thread`.
It demonstrates creating and managing multiple threads to execute functions concurrently.

**Key Concepts**:

1. **Thread Creation**: Two threads (`t1` and `t2`) are created to run the `printMessage` function with unique IDs.
2. **Thread Execution**: Each thread prints a message to the console, running in parallel (subject to the system’s scheduler).
3. **Thread Joining**: The main thread waits for both threads to complete using `join()`.

**Observation**  
Running the program multiple times may produce outputs like:

```bash
Thread 2Thread  is running
1 is running
```

or

```bash
Thread 2 is running
Thread 1 is running
```

This is due to a phenomenon in multithreading known as a **race condition**.
Essentially, both threads (`t1` and `t2`) are writing to the standard output (`std::cout`)
at the same time, and their access to it isn't coordinated. As a result, their messages are
getting interleaved, leading to jumbled output like what shown above.

**Why This Happens**:

`std::cout` is a shared resource between the threads. Since threads run in parallel,
they can simultaneously call `std::cout`, causing pieces of their outputs to overlap.
This behavior is completely dependent on the thread scheduler, threads are scheduled
and executed by the operating system which determines when each thread runs so the output may
vary based on the system's thread scheduling policy and the number of available CPU cores.  
To prevent such interleaving, you can synchronize access to `std::cout`. More on **Race Condition**
in the next example `raceCondition.cpp`.

## Race Condition in Multithreading

**File**: `raceCondition.cpp`  

`raceCondition.cpp` is an excellent example of **Race Condition** in multithreading.
It demonstrates how multiple threads can interfere with each other when accessing shared
resources without proper synchronization. Let us break it down further:

```cpp
int counter = 0;
```

In this case, `counter` is shared between two threads. When both `t1` and `t2` increment `counter`
simultaneously, there is no synchronization, leading to inconsistent or incorrect results.  

**Typical Output**:  
Instead of the expected 2000000 (since each thread increments `counter` a million times),
the output will likely be less than that. The actual result depends on how the threads interleave during execution.

**Why This Happens**:

- The operation `counter++` may seem simple but is actually composed of multiple steps:
  - Load the value of counter into a register.
  - Increment the value in the register.
  - Write the updated value back to counter.
- If two threads perform this sequence at the same time, they might overwrite each other’s
updates, resulting in missed increments.  

**How to Fix It**:  
To fix this issue, you can use synchronization mechanisms like mutexes or atomic operations
to ensure that only one thread can modify `counter` at a time.

### Good Practices

- Use thread-local variables whenever possible. If each thread has its own copy of a variable,
there’s no need for synchronization.
- Clearly document shared resources and their synchronization strategies in the code to avoid surprises later on.
- Use tools like thread analyzers or runtime sanitizers (e.g., ThreadSanitizer) to detect race conditions during testing.

## Mutex in Multithreading

**File**: `mutex.cpp`  

`mutexExample.cpp` demonstrates the use of mutexes to prevent **Race Condition**
in multithreading. It shows how to use `std::mutex` to synchronize access to shared resources
and ensure that only one thread can modify a shared variable at a time.

```cpp
std::mutex mtx;
```

The use of a mutex (**mutual exclusion**) ensures that only one thread can modify `counter` at a time.

```cpp
std::lock_guard<std::mutex> lock(mtx);
```

This creates a *critical section* for incrementing `counter`, making the operation **thread-safe**.
this ensures that the current thread has exclusive access to the shared resource
until it goes out of scope.
When one thread holds the lock, other threads must wait, preventing race condition.
By locking the mutex, you ensure that the three steps of counter++ (load, increment, store)
are performed atomically—no other thread can interfere during this time.
Now The program will reliably print:

```bash
Final counter value:: 2000000
```

### Observation

**Placement of `std::lock_guard<std::mutex>`**

1. Why Place `std::lock_guard` Right Before counter++?
    - `std::lock_guard<std::mutex>` ensures that only one thread can execute the critical section (`counter++`)
at any given time. By placing the lock directly before the increment operation, you minimize the scope
of locking, allowing other threads to access unrelated parts of the code without waiting unnecessarily.
    - This is a good practice because it keeps the critical section as small and efficient as possible,
reducing the risk of bottlenecks.

2. What Happens If You Place It Before the Loop?
    - If you lock the mutex before the loop:

    ```cpp
    std::lock_guard<std::mutex> lock(mtx);
    for (int i = 0; i < 1000000; i++) {
        counter++;
    }
    ```

    The entire loop becomes a critical section, meaning that only one thread can execute the loop at a time.
    This defeats the purpose of multithreading and results in a program that behaves like single-threaded code.
    Threads will be serialized, and performance gains from parallelism will be lost.

For efficient multithreading, the key is balancing safety and performance. Use locks judiciously:

- Protect shared resources from race conditions.
- Avoid unnecessary locking to maximize concurrency.

### Best Practices

- **Minimize Lock Scope**: Place `std::lock_guard` as close as possible to the critical section
(e.g., around `counter++` rather than the entire loop) to maximize concurrency.
- **Avoid Deadlocks**: When using multiple mutexes, lock them in a consistent order or use `std::scoped_lock`
to avoid deadlocks.
- **Consider Alternatives**: For simple operations like incrementing a counter, atomic operations
(next section) may be more efficient.

### Trade-offs

- **Overhead**: Mutexes introduce locking overhead and potential contention, which can impact performance.
- **Complexity**: Improper mutex usage can lead to deadlocks or performance bottlenecks.

We will cover more advanced synchronization techniques in the next examples with atomic operations and lock-free programming.

## Atomic Operations in Multithreading

**File**: `atomic.cpp`  

`atomicExample.cpp` demonstrates the use of atomic operations.

### How It Works

1. Atomic Variable:

    - `std::atomic<int> counter(0);`

        - This declares an atomic integer `counter`. Atomic variables provide thread-safe operations,
        ensuring that no two threads can interfere with each other when accessing or modifying the variable.
        This eliminates race conditions.

2. Atomic Increment:

    - `counter.fetch_add(1, std::memory_order_relaxed);`

        - The `fetch_add` method atomically increments the value of `counter` by `1`.

        - `std::memory_order_relaxed` specifies the memory ordering semantics. In this case,
        it indicates that no synchronization or ordering constraints are applied apart from atomicity.
        It’s efficient but assumes there's no dependency between threads regarding this operation.

3. Thread Creation:

    - Two threads (`t1` and `t2`) are created to run the incrementCounter function.
    Each thread increments the `counter` 1,000,000 times.

4. Result Retrieval:

    - After the threads complete their execution (`join()` ensures this),
    the final value of `counter` is printed using `counter.load()`. The `load()` function
    retrieves the value of the atomic variable in a thread-safe manner.

### Advantages of Atomic Operations

1. Thread Safety:

    - The atomicity of `counter.fetch_add()` ensures that no updates are lost even when multiple
    threads access `counter` simultaneously. This avoids race conditions without needing additional
    synchronization mechanisms like mutexes.

2. Efficiency:

    - Using atomic operations is generally faster than using mutexes since there’s no need to block
    threads or context-switch between them. This makes atomic variables a great choice for performance-critical applications.

3. Simplicity:

The code is cleaner compared to managing mutexes explicitly. You don’t need to worry about
deadlocks or forgetting to release a lock.

### Potential Downsides

1. Memory Ordering:

    - While `std::memory_order_relaxed` is the fastest option, it does not enforce any ordering constraints.
    This can lead to subtle bugs if other parts of the program depend on a specific order of operations.
    For most use cases, `std::memory_order_seq_cst` (the default memory order) is safer, as it provides
    a total sequential consistency across threads.
    To use the default memory order:

    ```cpp
    counter.fetch_add(1); // Default is std::memory_order_seq_cst
    ```

2. Scalability:

    - Atomic operations work well for individual variables like `counter`. However, for more complex
    data structures or logic involving multiple variables, using atomic operations becomes difficult
    and error-prone. In such cases, synchronization primitives like mutexes are more appropriate.

### When to Use Atomics Instead of Mutexes

Atomics are a great choice when:

1. You are working with simple shared data (e.g., counters, flags, or pointers).
2. You need minimal contention between threads.
3. The code must avoid blocking or deadlocks entirely (e.g., real-time systems).

However, **atomics are not a silver bullet**. They don’t completely replace mutexes because:

- They work well only for simple operations on individual variables.
- They can be challenging to use for managing more complex, multi-variable state.
For example, if you’re protecting a large data structure like a `std::map`, a mutex is more practical.

## Further Considerations

- **Advanced Synchronization**: Explore condition variables, semaphores, or reader-writer
locks for more complex threading scenarios.
- **Lock-Free Data Structures**: Investigate lock-free queues or stacks for high-performance,
thread-safe data sharing.
- **Performance Testing**: Use profiling tools (e.g., `Valgrind`, `Perf`) to measure the
impact of synchronization mechanisms.
- **Portability**: Ensure code is tested across platforms, as threading behavior may vary
(e.g., Windows vs. Linux).

**Learning Resources**:  

- C++ Reference: `<thread>`, `<mutex`>, `<atomic>`
- "C++ Concurrency in Action" by Anthony Williams

This repository provides a foundation for understanding multithreading and lock-free programming. Experiment with the examples, modify them, and explore advanced techniques to deepen your knowledge.
Feel free to contribute by adding more examples or improving existing ones. Happy coding!
