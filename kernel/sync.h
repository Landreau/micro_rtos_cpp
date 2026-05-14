#ifndef SYNC_H
#define SYNC_H

#include <mutex>
#include <condition_variable>
#include <queue>
#include <memory>

class Mutex
{
public:
    Mutex() = default;
    ~Mutex() = default;

    void lock()
    {
        mtx.lock();
    }

    void unlock()
    {
        mtx.unlock();
    }

    bool tryLock()
    {
        return mtx.try_lock();
    }

private:
    std::mutex mtx;
};

class Semaphore
{
public:
    explicit Semaphore(int initialCount = 1) : count(initialCount) {}
    ~Semaphore() = default;

    // Acquérir le sémaphore
    void wait()
    {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [this]()
                { return count > 0; });
        count--;
    }

    // Vérifier sans bloquer
    bool tryWait()
    {
        std::lock_guard<std::mutex> lock(mtx);
        if (count > 0)
        {
            count--;
            return true;
        }
        return false;
    }

    // Libérer le sémaphore (signal/V)
    void signal()
    {
        std::lock_guard<std::mutex> lock(mtx);
        count++;
        cv.notify_one();
    }

    int getValue() const
    {
        return count;
    }

private:
    int count;
    std::mutex mtx;
    std::condition_variable cv;
};

class LockGuard
{
public:
    explicit LockGuard(Mutex &m) : mutex(m)
    {
        mutex.lock();
    }

    ~LockGuard()
    {
        mutex.unlock();
    }

    LockGuard(const LockGuard &) = delete;
    LockGuard &operator=(const LockGuard &) = delete;

private:
    Mutex &mutex;
};

#endif
