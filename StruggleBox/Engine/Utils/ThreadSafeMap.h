#ifndef THREAD_SAFE_MAP_H
#define THREAD_SAFE_MAP_H

#include <unordered_map>
#include <mutex>
#include <condition_variable>

template <typename KeyType, typename ContainerType>
class ThreadSafeMap
{
public:
    ThreadSafeMap(void)
    : _queue()
    , _mutex()
    , _condition()
    {}
    
    ~ThreadSafeMap(void)
    {}
    
    void enqueue(ContainerType t)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        _map.push(t);
        _condition.notify_one();
    }
    
    ContainerType dequeue(void)
    {
        std::unique_lock<std::mutex> lock(_mutex);
        while(_queue.empty())
        {
            _condition.wait(lock);
        }
        ContainerType val = _queue.front();
        _map.pop();
        return val;
    }
    
private:
    std::unordered_map<ContainerType> _map;
    mutable std::mutex _mutex;
    std::condition_variable _condition;
};

#endif /* THREAD_SAFE_MAP_H */
