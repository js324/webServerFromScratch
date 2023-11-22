#include <vector>
#include <mutex>
#include <queue>
#include <condition_variable>
#include <functional>
#include <thread>
#define NUM_THREADS 4

class ThreadPool {
private:
    bool finished = false;
    std::vector<std::thread> m_threads;
    std::queue<std::function<void()>> m_jobs;
    std::mutex m_mutex;
    std::condition_variable m_cond;
    void threadLoop() {
        while (1) {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_cond.wait(lock, [this] {
                
                return !m_jobs.empty() || finished;
            });
            if (finished) {
                return;
            }
        
            auto job = m_jobs.front();
            m_jobs.pop();
            lock.unlock();
            job();
            
        }
    }
public:
    
    void startUp() {
        for (int i = 0; i < NUM_THREADS; i++) {
            std::thread thr(&ThreadPool::threadLoop, this);
            
            m_threads.push_back(move(thr)); //look into emplace vs push
        }
    }
    void queueJob(std::function<void()> job) { 
        //might need to place in block
        
        std::unique_lock<std::mutex> lock(m_mutex);
        
        m_jobs.push(job);
        
        m_cond.notify_one();
    }
    bool isBusy() { //more like "has jobs in queue", doesn't say if thread is serving connection
        bool isBusy = false;
        std::unique_lock<std::mutex> lock(m_mutex);
        isBusy = !m_jobs.empty();
       
        return isBusy;
        
    }
    void close() { //called from main thread
        std::unique_lock<std::mutex> lock(m_mutex);
        finished = true;
        lock.release();
        m_cond.notify_all();
        for (std::thread& active : m_threads) {
            active.join(); 
        } //all threads joined and stopped executing, allow main thread to continue working
        m_threads.clear();
    }


};
