#ifndef __THREAD_POOL_H__
#define __THREAD_POOL_H__
#include <functional>
#include <queue>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <atomic>
namespace threadpool
{

class ThreadPool
{
    static const int INIT_NUM = 1;
    static const int MAX_NUM = 4;
    static const int MAX_IDLE_TIME_SECOND = 6;

    typedef std::function<void()> Task;
public:
    ThreadPool(int initNum, int maxNum, int idleSec);
    ThreadPool();
    ~ThreadPool();

public:
    void AddTask(const Task& task);

private:
    void Init();
    void ThreadRoutine(int index);
    void PoolGrow();
    
private:
    int _initNum;
    int _maxNum;
    int _idleSec;
    bool _stop;
    std::queue<Task> _taskQueue;
    std::vector<std::thread*> _pool;
    std::mutex _mutex;
    std::condition_variable _cond;
    std::atomic<int> _busyCount;
    std::atomic<int> _threadCount;
};


} // namespace threadpool

#endif // __THREAD_POOL_H__