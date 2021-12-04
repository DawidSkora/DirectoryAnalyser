#include <condition_variable>
#include <functional>
#include <filesystem>
#include <iostream>
#include <vector>
#include <thread>
#include <memory>
#include <queue>

class Counter;
extern bool TPdone;

class ThreadPool
{
public:
    ThreadPool();
    ~ThreadPool();

    void feedQueue(std::function<void(std::filesystem::path, ThreadPool&)> _task, std::filesystem::path _path);

private:
    std::queue<std::function<void(std::filesystem::path, ThreadPool&)>> taskQueue;
    std::queue<std::filesystem::path> pathQueue;
    std::vector<std::thread> Threads;
    std::condition_variable CV;
    std::mutex Mutex;

    bool StopThreads{ false };

    void threads_work();
    void start(int const _ThreadsNum);
    void stop();
};