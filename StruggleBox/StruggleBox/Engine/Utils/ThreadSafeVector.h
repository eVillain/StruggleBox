#pragma ocne

#include <queue>
#include <mutex>
#include <condition_variable>

template <class ContainerType>
class ThreadSafeVector
{
public:
    ThreadSafeVector(void)
    : m_vector()
    , m_mutex()
    , m_condition()
    {}
    
    ~ThreadSafeVector(void)
    {}
    
    void push_back(const ContainerType& t)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_vector.push_back(t);
        m_condition.notify_one();
    }
    
    ContainerType Dequeue(void)
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        while(m_vector.empty())
        {
            m_condition.wait(lock);
        }
        ContainerType val = m_vector.front();
        m_vector.pop();
        return val;
    }
    
    bool Empty(void)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_vector.empty();
    }
    
private:
    std::vector<ContainerType> m_vector;
    mutable std::mutex m_mutex;
    std::condition_variable m_condition;
};
