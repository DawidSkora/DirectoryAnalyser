#include "ThreadPool.h"

ThreadPool::ThreadPool()
{
    if (std::thread::hardware_concurrency())
    {
        std::cout << "Starting " << std::thread::hardware_concurrency() << " threads" << std::endl;
        start(std::thread::hardware_concurrency());
    }
    else
    {
        std::cout << "Starting 16 threads" << std::endl;
        start(16);
    }
}

ThreadPool::~ThreadPool()
{
    stop();
}

void ThreadPool::feedQueue(std::function<void(std::filesystem::path, ThreadPool&)> _task, std::filesystem::path _path)
{
    {
        std::unique_lock<std::mutex> lock{ Mutex };
        taskQueue.emplace(_task);
        pathQueue.emplace(_path);
    }
    CV.notify_all();
}

void ThreadPool::threads_work()
{
    while (true)
    {
        std::function<void(std::filesystem::path, ThreadPool&)> _task;
        std::filesystem::path _path("");
        {
            std::unique_lock<std::mutex> lock{ Mutex };

            CV.wait(lock, [=] { return StopThreads || !taskQueue.empty(); });

            if (StopThreads && taskQueue.empty())
                break;

            _task = std::move(taskQueue.front());
            taskQueue.pop();

            _path = std::move(pathQueue.front());
            pathQueue.pop();
        }
        _task(_path, *this);
    }
}

void ThreadPool::start(int const _ThreadsNum)
{
    for (int i = 0; i < _ThreadsNum; ++i)
    {
        Threads.emplace_back(std::thread(&ThreadPool::threads_work, this));
    }
}

void ThreadPool::stop()
{
    {
        std::unique_lock<std::mutex> lock{ Mutex };
        StopThreads = true;
    }

    CV.notify_all();

    for (auto& thread : Threads)
    {
        thread.join();
    }
    TPdone = true;
}
