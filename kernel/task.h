#ifndef TASK_H
#define TASK_H

#include <cstdint>
#include <functional>
#include <chrono>

// États possibles d'une tâche
enum class TaskState
{
    CREATED,
    READY,
    RUNNING,
    BLOCKED,
    DELAYED,
    TERMINATED
};

class Task
{
public:
    using TaskFunc = std::function<void()>;
    using TimePoint = std::chrono::steady_clock::time_point;
    using Duration = std::chrono::milliseconds;

    Task(int id, TaskFunc func, uint32_t priority);
    ~Task() = default;

    // Getters
    int getId() const { return id; }
    TaskState getState() const { return state; }
    uint32_t getPriority() const { return priority; }
    TaskFunc getFunction() const { return function; }

    // Tâches périodiques
    uint32_t getPeriod() const { return period; }
    bool isPeriodic() const { return period > 0; }
    TimePoint getNextWakeup() const { return nextWakeup; }

    // Délai
    bool hasDelayExpired() const;
    uint32_t getRemainingDelay() const;

    // Setters
    void setState(TaskState newState) { state = newState; }
    void setPeriod(uint32_t periodMs) { period = periodMs; }
    void setDelay(uint32_t delayMs);
    void updateNextWakeup();

    void execute();

private:
    int id;
    TaskFunc function;
    uint32_t priority;
    TaskState state;

    // Délai et timing
    TimePoint delayUntil; // Moment jusqu'auquel la tâche est bloquée
    uint32_t period = 0;
    TimePoint nextWakeup; // Prochain moment d'exécution pour les tâches périodiques
};

#endif // TASK_H