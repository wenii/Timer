#ifndef __TIMER_H__
#define __TIMER_H__
#include <condition_variable>
#include <mutex>
#include <thread>
#include <vector>
#include <list>
#include <functional>
#include "ThreadPool.h"

struct TimedTask;
namespace timer{
class Timer
{
public:
    typedef std::function<void()> Task;
    static const int WHELL_LEN = 1024;
    static const int INDEX_MASK = WHELL_LEN - 1;
public:
    Timer();
    ~Timer();
public:
    void AddTask(const Task& task, int msec);

private:
    void ThreadRoutine();
    int GetNextWaitMs();
    void Tick(long long tickCount);
    void AddNewTask(const std::vector<TimedTask*>& taskList);
 
private:
    bool _stop;
    long long _tickCount;
    std::mutex _mutex;
    std::thread _thread;
    std::condition_variable _cond;
    threadpool::ThreadPool _threadPool;
    std::vector<TimedTask*> _inputTaskList;
    std::list<TimedTask*> _timedTask[WHELL_LEN];
    std::chrono::time_point<std::chrono::steady_clock> _startTime;
};

}   // namespace timer

#endif // __TIMER_H__