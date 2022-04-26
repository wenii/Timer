#include "Timer.h"
#include <stdio.h>
#include <chrono>
#include <thread>
#include <memory>
#include <atomic>
#include <random>
#include <unistd.h>

std::atomic<int> g_count;

const int MAX_TIMEOUT = 10000;
int main(){
    auto t = std::make_shared<timer::Timer>();
    auto AddTimeTask = [t](){
        for(int i = 0; i < MAX_TIMEOUT; ++i){
            int timeout = random() % MAX_TIMEOUT;
            auto startTime = std::chrono::steady_clock::now(); 
            t->AddTask([startTime, timeout](){
                long long dt = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - startTime).count();
                g_count++;
                printf("hello world dt:%lld, timeout:%d, count:%d\n", dt, timeout, g_count.load());
                }, timeout);
            usleep(10);
        }
    };

    int n = 5;
    while(n--){
        std::thread thread1(AddTimeTask);
        std::thread thread2(AddTimeTask);
        std::thread thread3(AddTimeTask);
        std::thread thread4(AddTimeTask);

        thread1.join();
        thread2.join();
        thread3.join();
        thread4.join();
        sleep(20);
    }
    

    std::this_thread::sleep_for(std::chrono::milliseconds(MAX_TIMEOUT + 100));
    return 0;
}