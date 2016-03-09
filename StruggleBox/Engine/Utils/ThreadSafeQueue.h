//
//  ThreadSafeQueue.h
//  DungeonSmith
//
//  Created by The Drudgerist on 11/08/15.
//  Copyright (c) 2015 The Drudgerist. All rights reserved.
//

#ifndef THREAD_SAFE_QUEUE_H
#define THREAD_SAFE_QUEUE_H

#include <queue>
#include <mutex>
#include <condition_variable>

template <class ContainerType>
class ThreadSafeQueue
{
public:
    ThreadSafeQueue(void)
    : _queue()
    , _mutex()
    , _condition()
    {}
    
    ~ThreadSafeQueue(void)
    {}
    
    void Enqueue(ContainerType t)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        _queue.push(t);
        _condition.notify_one();
    }
    
    ContainerType Dequeue(void)
    {
        std::unique_lock<std::mutex> lock(_mutex);
        while(_queue.empty())
        {
            _condition.wait(lock);
        }
        ContainerType val = _queue.front();
        _queue.pop();
        return val;
    }
    
    bool Empty(void)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        return _queue.empty();
    }
    
private:
    std::queue<ContainerType> _queue;
    mutable std::mutex _mutex;
    std::condition_variable _condition;
};

#endif /* THREAD_SAFE_QUEUE_H */
