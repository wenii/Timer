#include <assert.h>
#include "Timer.h"

using namespace timer;

struct TimedTask
{
    TimedTask(const Timer::Task& task, int timeout);
    Timer::Task task;
    int timeout;
    long long deadline;
};

TimedTask::TimedTask(const Timer::Task& task, int timeout)
    : task(task)
    , timeout(timeout)
    , deadline(0)
{

}

Timer::Timer()
    : _stop(false) 
    , _tickCount(0)
{
    _startTime = std::chrono::steady_clock::now();
    _thread = std::thread(&Timer::ThreadRoutine, this);
}

Timer::~Timer()
{
    {
        std::lock_guard<std::mutex> lock(_mutex);
        _stop = true; 
    }
    _cond.notify_one();
    _thread.join();
}

void Timer::AddTask(const Task& task, int msec)
{
    if(msec <= 0){
        _threadPool.AddTask(task);
        return;
    }else{
        std::lock_guard<std::mutex> lock(_mutex);
        _inputTaskList.push_back(new TimedTask(task, msec));
    }
    _cond.notify_one();
}

void Timer::Tick(long long tickCount)
{
    const int index = tickCount & INDEX_MASK;
    auto& taskList = _timedTask[index];
    for(auto itr = taskList.begin(); itr != taskList.end();){
        auto task = *itr;
        if(tickCount >= task->deadline){
            _threadPool.AddTask(task->task);
            itr = taskList.erase(itr);
            delete task;
            continue;
        }
        break;
    }
}

void Timer::ThreadRoutine()
{
    while(!_stop){
        long long lastMillSec = _tickCount;
        int waitMillSec = GetNextWaitMs();
        assert(waitMillSec);
        std::vector<TimedTask*> taskList;
        std::cv_status status = std::cv_status::no_timeout;
        {
            std::unique_lock<std::mutex> lock(_mutex);
            while(_inputTaskList.size() == 0 && status == std::cv_status::no_timeout && !_stop){
                if(waitMillSec > 0){
                    status = _cond.wait_for(lock, std::chrono::milliseconds(waitMillSec));
                }else{
                    _cond.wait(lock);
                }
            }
            if(_stop){
                return;
            }
            if(_inputTaskList.size() > 0){
                taskList.swap(_inputTaskList);
            }
        }
        _tickCount = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - _startTime).count(); 
        long long dt = _tickCount - lastMillSec;
        if(dt >= waitMillSec && waitMillSec != -1){
            for(int i = 0; i <= dt - waitMillSec; ++i){
                Tick(lastMillSec + waitMillSec + i);
            }
        }

        if(taskList.size() > 0){
            AddNewTask(taskList);
        }
    }
}

int Timer::GetNextWaitMs()
{
    int minMs = -1;
    bool findNext = false;
    const int startIndex = _tickCount & INDEX_MASK;
    for(int i = 1; i <= WHELL_LEN; ++i){
        const int index = (startIndex + i) & INDEX_MASK;
        auto& tasks = _timedTask[index];
        if(tasks.size() > 0){
            long long dt = tasks.front()->deadline - _tickCount;
            if(findNext){
                if(dt < minMs){
                    minMs = dt;
                }
            }else{
                assert(dt > 0);
                minMs = dt;
                findNext = true;
            }
            if(minMs < WHELL_LEN){
                break;
            }
        }
    }
    assert(minMs != 0);

    return minMs;
}

void Timer::AddNewTask(const std::vector<TimedTask*>& taskList)
{
    for(auto& task : taskList){
        assert(task->timeout > 0);
        task->deadline = _tickCount + task->timeout;
        const int index = task->deadline & INDEX_MASK;
        auto& bucket = _timedTask[index];
        if(bucket.empty() || task->deadline > bucket.back()->deadline){
            bucket.push_back(task);
        }else{
            for(auto itr = bucket.begin(); itr != bucket.end(); ++itr){
                auto& t = *itr;
                if(task->deadline <= t->deadline){
                    bucket.insert(itr, task);
                    break;
                }
            }
        }
    }
}

