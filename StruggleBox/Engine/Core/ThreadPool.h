#ifndef NGN_THREAD_POOL_H
#define NGN_THREAD_POOL_H
//
//  ThreadPool.h
//  Ingenium
//
//  Created by Ville-Veikko Urrila on 8/6/13.
//  Copyright (c) 2013 The Drudgerist. All rights reserved.
//
//  This class creates a pool of worker threads
//  These threads can then be given jobs to execute
//  Latest modification includes a priority value for each job
//

#include <vector>
#include <map>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <functional>
#include <stdexcept>

class ThreadPool {
private:
    // Vector of worker threads
    std::vector< std::thread > workers;
    // The jobs priority map (int holds task priority)
    std::map<int, std::vector<std::function<void()>> > jobs;
    // Queue synchronization vars
    std::mutex queue_mutex;
    std::condition_variable condition;
    // Shutdown toggle
    bool isRunning;
public:
    // Forward declare constructor and destructor
    ThreadPool(size_t);
    ~ThreadPool();
    // Forward declare function to add jobs to pool
    template<class F, class... Args>
    auto AddJob(int priority, F&& f, Args&&... args)
    -> std::future<typename std::result_of<F(Args...)>::type>;
    // Get current number of jobs
    inline size_t NumJobs() { std::lock_guard<std::mutex> lock(queue_mutex); return jobs.size(); };
    inline size_t NumWorkers() { return workers.size(); };
};

// The constructor just launches some amount of workers
inline ThreadPool::ThreadPool(size_t threads) : isRunning(true) {
    for(size_t i = 0;i<threads;++i) {
        workers.emplace_back( [this] {
            while(true) {
                std::unique_lock<std::mutex> lock(this->queue_mutex);
                while(this->isRunning && this->jobs.empty()) { this->condition.wait(lock); }
                if(!this->isRunning && this->jobs.empty()) { return; }
                
                std::function<void()> job( this->jobs.begin()->second.back() );
                this->jobs.begin()->second.pop_back();
                if ( this->jobs.begin()->second.empty() ) {
                    // Clear this priority and move to next
                    this->jobs.erase(jobs.begin());
                }
                lock.unlock();
                job();
            }
        } );
    }
    printf("[ThreadPool] Started with %lu threads, hardware threads: %i\n", threads, std::thread::hardware_concurrency());
};
// The destructor just notifies all threads and waits for them to join before shutting down
inline ThreadPool::~ThreadPool() {
    // Theoretically the lock below should prevent a race condition...
    // In practice it makes some of the threads miss the notify_all()
    // Thus it is commented out for now, investigate later if/when problems arise
    //    std::unique_lock<std::mutex> lock(queue_mutex);
    
    // Stop thread pool from running and adding new jobs
    isRunning = false;
    // Notify all threads
    condition.notify_all();
    // Join all threads
    for(size_t i = 0;i<workers.size();++i) {
        workers[i].join();
    }
    printf("[ThreadPool] Shut down complete, threads released\n");
};
// Function to add a job to the pool
template<class F, class... Args>
auto ThreadPool::AddJob(int priority, F&& f, Args&&... args)
-> std::future<typename std::result_of<F(Args...)>::type> {
    typedef typename std::result_of<F(Args...)>::type return_type;
    // Don't allow new jobs after stopping the pool
    if(!isRunning) { throw "[ThreadPool] Was stopped, shouldn't be adding new jobs\n"; }
    auto job = std::make_shared< std::packaged_task<return_type()> >( std::bind(std::forward<F>(f), std::forward<Args>(args)...) );
    // Add task to list
    std::future<return_type> res = job->get_future();
    std::unique_lock<std::mutex> lock(queue_mutex);
    jobs[priority].push_back([job](){ (*job)(); });
    // Notify a worker thread if any are idle
    condition.notify_one();
    return res;
};


#endif
