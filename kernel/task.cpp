#include "task.h"
#include <iostream>

Task::Task(int id, TaskFunc func, uint32_t priority)
    : id(id), function(func), priority(priority), state(TaskState::CREATED),
      delayUntil(std::chrono::steady_clock::now()), period(0),
      nextWakeup(std::chrono::steady_clock::now())
{
}

bool Task::hasDelayExpired() const
{
    if (state != TaskState::DELAYED)
    {
        return true;
    }

    auto now = std::chrono::steady_clock::now();
    return now >= delayUntil;
}

uint32_t Task::getRemainingDelay() const
{
    if (state != TaskState::DELAYED)
    {
        return 0;
    }

    auto now = std::chrono::steady_clock::now();
    if (now >= delayUntil)
    {
        return 0;
    }

    auto remaining = std::chrono::duration_cast<Duration>(delayUntil - now);
    return remaining.count();
}

void Task::setDelay(uint32_t delayMs)
{
    delayUntil = std::chrono::steady_clock::now() + Duration(delayMs);
    state = TaskState::DELAYED;
}

void Task::updateNextWakeup()
{
    if (isPeriodic())
    {
        nextWakeup = std::chrono::steady_clock::now() + Duration(period);
    }
}

void Task::execute()
{
    if (function)
    {
        state = TaskState::RUNNING;
        function();

        if (isPeriodic())
        {
            updateNextWakeup();
            state = TaskState::READY;
        }
        else
        {
            state = TaskState::READY;
        }
    }
}
